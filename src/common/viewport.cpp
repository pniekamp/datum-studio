//
// Datum Viewport
//

//
// Copyright (C) 2016 Peter Niekamp
//


#include "viewport.h"
#include "assetfile.h"

#include <QtDebug>

using namespace std;
using namespace lml;

///////////////////////// Resource::load_mesh ///////////////////////////////
template<>
unique_resource<Mesh> Viewport::ResourceProxy::load<Mesh>(size_t asset)
{
  auto platform = Studio::Core::instance()->find_object<Studio::Platform>();

  return create<Mesh>(platform->assets()->find(asset));
}


///////////////////////// Resource::load_mesh ///////////////////////////////
template<>
unique_resource<Mesh> Viewport::ResourceProxy::load<Mesh>(Studio::Document *document, size_t index)
{
  unique_resource<Mesh> mesh;

  document->lock();

  PackMeshHeader mhdr;

  if (read_asset_header(document, index, &mhdr))
  {
    mesh = create<Mesh>(mhdr.vertexcount, mhdr.indexcount);

    if (auto lump = acquire_lump(mesh->vertexbuffer.size))
    {
      uint64_t position = mhdr.dataoffset + sizeof(PackChunk);

      position += document->read(position, (uint8_t*)lump->transfermemory + mesh->vertexbuffer.verticiesoffset, mesh->vertexbuffer.vertexcount*mesh->vertexbuffer.vertexsize);
      position += document->read(position, (uint8_t*)lump->transfermemory + mesh->vertexbuffer.indicesoffset, mesh->vertexbuffer.indexcount*mesh->vertexbuffer.indexsize);

      update<Mesh>(mesh, lump);

      release_lump(lump);
    }

    update<Mesh>(mesh, Bound3(Vec3(mhdr.mincorner[0], mhdr.mincorner[1], mhdr.mincorner[2]), Vec3(mhdr.maxcorner[0], mhdr.maxcorner[1], mhdr.maxcorner[2])));
  }

  document->unlock();

  return mesh;
}


///////////////////////// Resource::load_texture ////////////////////////////
template<>
unique_resource<Texture> Viewport::ResourceProxy::load<Texture>(istream &fin, size_t index, Texture::Format format)
{
  unique_resource<Texture> texture;

  PackImageHeader imag;

  if (read_asset_header(fin, index, &imag))
  {
    texture = create<Texture>(imag.width, imag.height, imag.layers, imag.levels, format);

    if (auto lump = acquire_lump(imag.datasize))
    {
      read_asset_payload(fin, imag.dataoffset, lump->transfermemory, imag.datasize);

      update<Texture>(texture, lump);

      release_lump(lump);
    }
  }

  return texture;
}


///////////////////////// Resource::load_skybox /////////////////////////////
template<>
unique_resource<SkyBox> Viewport::ResourceProxy::load<SkyBox>(istream &fin, size_t index)
{
  unique_resource<SkyBox> skybox;

  PackImageHeader imag;

  if (read_asset_header(fin, index, &imag))
  {
    skybox = create<SkyBox>(imag.width, imag.height, EnvMap::Format::RGBE);

    if (auto lump = acquire_lump(imag.datasize))
    {
      read_asset_payload(fin, imag.dataoffset, lump->transfermemory, imag.datasize);

      update<SkyBox>(skybox, lump);

      release_lump(lump);
    }
  }

  return skybox;
}


//|---------------------- Viewport ------------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// Viewport::Constructor /////////////////////////////
Viewport::Viewport(size_t slabsize, size_t storagesize, QWidget *parent)
  : QWidget(parent),
    resources(Studio::Core::instance()->find_object<Studio::Platform>()->resources()),
    scratchmemory{0, slabsize, new char[slabsize]},
    m_pushbuffer(scratchmemory, slabsize)
{ 
  setAttribute(Qt::WA_NativeWindow);
  setAttribute(Qt::WA_DontCreateNativeAncestors);
  setAttribute(Qt::WA_PaintOnScreen);

  setFocusPolicy(Qt::ClickFocus);

  auto platform = Studio::Core::instance()->find_object<Studio::Platform>();

  auto renderdevice = platform->instance()->render_device();

  initialise_vulkan_device(&vulkan, renderdevice.physicaldevice, renderdevice.device, 0);

  initialise_resource_pool(*platform->instance(), m_rendercontext.resourcepool, storagesize);

  surface = platform->create_surface(winId());

  acquirecomplete = create_semaphore(vulkan);
  rendercomplete = create_semaphore(vulkan);

  camera.set_projection(60.0f*pi<float>()/180.0f, 1920.0f/1080.0f);
}


///////////////////////// Viewport::Destructor //////////////////////////////
Viewport::~Viewport()
{
  auto platform = Studio::Core::instance()->find_object<Studio::Platform>();

  platform->resources()->release(platform->resources()->token());

  delete static_cast<char*>(scratchmemory.data);

  vkDeviceWaitIdle(vulkan.device);
}


///////////////////////// Viewport::hideEvent ///////////////////////////////
void Viewport::hideEvent(QHideEvent *event)
{
  if (m_rendercontext.initialised)
  {
    swapchain = {};

    release_render_pipeline(m_rendercontext);
  }

  QWidget::hideEvent(event);
}


///////////////////////// Viewport::paintEngine /////////////////////////////
QPaintEngine *Viewport::paintEngine() const
{
  return nullptr;
}


///////////////////////// Viewport::paintEvent //////////////////////////////
void Viewport::paintEvent(QPaintEvent *event)
{
  prepare();
  render();
}


///////////////////////// Viewport::prepare /////////////////////////////////
bool Viewport::prepare()
{
  auto platform = Studio::Core::instance()->find_object<Studio::Platform>();

  while (!prepare_render_context(*platform->instance(), m_rendercontext, platform->assets()))
    ;

  camera.set_projection(60.0f*pi<float>()/180.0f, (float)width() / (float)height());

  if (m_rendercontext.width != width() || m_rendercontext.height != height())
  {
    VkSurfaceCapabilitiesKHR surfacecapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vulkan.physicaldevice, surface, &surfacecapabilities);

    VkSwapchainCreateInfoKHR swapchaininfo = {};
    swapchaininfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchaininfo.surface = surface;
    swapchaininfo.minImageCount = surfacecapabilities.minImageCount;
    swapchaininfo.imageFormat = VK_FORMAT_B8G8R8A8_SRGB;
    swapchaininfo.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
    swapchaininfo.imageExtent = surfacecapabilities.currentExtent;
    swapchaininfo.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    swapchaininfo.preTransform = surfacecapabilities.currentTransform;
    swapchaininfo.imageArrayLayers = 1;
    swapchaininfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchaininfo.queueFamilyIndexCount = 0;
    swapchaininfo.pQueueFamilyIndices = nullptr;
    swapchaininfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    swapchaininfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchaininfo.oldSwapchain = swapchain;
    swapchaininfo.clipped = true;

    VkSwapchainKHR newswapchain;
    if (vkCreateSwapchainKHR(vulkan.device, &swapchaininfo, nullptr, &newswapchain) != VK_SUCCESS)
      throw runtime_error("Vulkan vkCreateSwapchainKHR failed");

    swapchain = Vulkan::Swapchain(newswapchain, { vulkan.device });

    uint32_t imagescount = 0;
    vkGetSwapchainImagesKHR(vulkan.device, swapchain, &imagescount, nullptr);

    if (extent<decltype(presentimages)>::value < imagescount)
      throw runtime_error("Vulkan vkGetSwapchainImagesKHR failed");

    vkGetSwapchainImagesKHR(vulkan.device, swapchain, &imagescount, presentimages);

    for(size_t i = 0; i < imagescount; ++i)
    {
      setimagelayout(vulkan, presentimages[i], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });
    }

    renderparams.width = width();
    renderparams.height = height();
    renderparams.aspect = camera.aspect();
  }

  prepare_render_pipeline(m_rendercontext, renderparams);

  m_pushbuffer.reset();

  while (renderparams.skybox && !renderparams.skybox->ready())
    platform->resources()->request(*platform->instance(), renderparams.skybox);

  m_resourcetoken = platform->resources()->token();

  return true;
}


///////////////////////// Viewport::begin ///////////////////////////////////
bool Viewport::begin(SpriteList &sprites, SpriteList::BuildState &buildstate)
{
  auto platform = Studio::Core::instance()->find_object<Studio::Platform>();

  return sprites.begin(buildstate, *platform->instance(), m_rendercontext, platform->resources());
}


///////////////////////// Viewport::push_sprites ////////////////////////////
void Viewport::push_sprites(SpriteList const &sprites)
{
  if (auto entry = m_pushbuffer.push<Renderable::Sprites>())
  {
    entry->viewport = Rect2(Vec2(0, 0), Vec2(width(), height()));
    entry->commandlist = sprites.commandlist();
  }
}


///////////////////////// Viewport::begin ///////////////////////////////////
bool Viewport::begin(MeshList &meshes, MeshList::BuildState &buildstate)
{
  auto platform = Studio::Core::instance()->find_object<Studio::Platform>();

  return meshes.begin(buildstate, *platform->instance(), m_rendercontext, platform->resources());
}


///////////////////////// Viewport::push_meshes /////////////////////////////
void Viewport::push_meshes(MeshList const &meshes)
{
  if (auto entry = m_pushbuffer.push<Renderable::Meshes>())
  {
    entry->commandlist = meshes.commandlist();
  }
}


///////////////////////// Viewport::begin ///////////////////////////////////
bool Viewport::begin(ForwardList &objects, ForwardList::BuildState &buildstate)
{
  auto platform = Studio::Core::instance()->find_object<Studio::Platform>();

  return objects.begin(buildstate, *platform->instance(), m_rendercontext, platform->resources());
}


///////////////////////// Viewport::push_objects ////////////////////////////
void Viewport::push_objects(ForwardList const &objects)
{
  if (auto entry = m_pushbuffer.push<Renderable::Objects>())
  {
    entry->commandlist = objects.commandlist();
  }
}


///////////////////////// Viewport::begin ///////////////////////////////////
bool Viewport::begin(OverlayList &overlays, OverlayList::BuildState &buildstate)
{
  auto platform = Studio::Core::instance()->find_object<Studio::Platform>();

  return overlays.begin(buildstate, *platform->instance(), m_rendercontext, platform->resources());
}


///////////////////////// Viewport::push_overlays ///////////////////////////
void Viewport::push_overlays(OverlayList const &overlays)
{
  if (auto entry = m_pushbuffer.push<Renderable::Overlays>())
  {
    entry->commandlist = overlays.commandlist();
  }
}


///////////////////////// Viewport::render //////////////////////////////////
void Viewport::render()
{
  using ::render;

  assert(m_rendercontext.framebuffer);

  auto platform = Studio::Core::instance()->find_object<Studio::Platform>();

  uint32_t imageindex;
  vkAcquireNextImageKHR(vulkan.device, swapchain, UINT64_MAX, acquirecomplete, VK_NULL_HANDLE, &imageindex);

  DatumPlatform::Viewport viewport;
  viewport.x = 0;
  viewport.y = 0;
  viewport.width = width();
  viewport.height = height();
  viewport.image = presentimages[imageindex];
  viewport.acquirecomplete = acquirecomplete;
  viewport.rendercomplete = rendercomplete;

  render(m_rendercontext, viewport, camera, m_pushbuffer, renderparams);

  VkPresentInfoKHR presentinfo = {};
  presentinfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentinfo.swapchainCount = 1;
  presentinfo.pSwapchains = swapchain.data();
  presentinfo.pImageIndices = &imageindex;
  presentinfo.waitSemaphoreCount = 1;
  presentinfo.pWaitSemaphores = rendercomplete.data();

  vkQueuePresentKHR(vulkan.queue, &presentinfo);

  vkQueueWaitIdle(vulkan.queue);

  platform->resources()->release(m_resourcetoken);
}
