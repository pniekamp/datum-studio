//
// Datum Viewport
//

//
// Copyright (C) 2016 Peter Niekamp
//


#include "viewport.h"
#include "assetfile.h"
#include <QTimer>

#include <QtDebug>

using namespace std;
using namespace lml;

namespace
{
  void lock(istream &)
  {
  }

  void unlock(istream &)
  {
  }

  void lock(Studio::Document *document)
  {
    document->lock();
  }

  void unlock(Studio::Document *document)
  {
    document->unlock();
  }

  ///////////////////////// load_asset //////////////////////////////////////
  template<typename Resource, typename ...Args>
  unique_resource<Resource> load_asset(Viewport::ResourceProxy &resources, size_t id, Args... args)
  {
    auto platform = Studio::Core::instance()->find_object<Studio::Platform>();

    auto asset = resources.create<Resource>(platform->assets()->find(id), args...);

    if (asset)
    {
      asset_guard lock(platform->assets());

      while (!asset->ready())
      {
        platform->resources()->request(*platform->instance(), asset);
      }
    }

    return asset;
  }

  ///////////////////////// load_mesh ///////////////////////////////////////
  template<typename File>
  unique_resource<Mesh> load_mesh(Viewport::ResourceProxy &resources, File &fin, size_t index)
  {
    unique_resource<Mesh> mesh;

    lock(fin);

    PackMeshHeader mhdr;

    if (read_asset_header(fin, index, &mhdr))
    {
      mesh = resources.create<Mesh>(mhdr.vertexcount, mhdr.indexcount);

      if (auto lump = resources.acquire_lump(mesh->size()))
      {
        int verticessize = mesh->vertexbuffer.vertexcount * mesh->vertexbuffer.vertexsize;
        int indicessize = mesh->vertexbuffer.indexcount * mesh->vertexbuffer.indexsize;

        read_asset_payload(fin, mhdr.dataoffset, lump->memory(mesh->verticesoffset()), verticessize);
        read_asset_payload(fin, mhdr.dataoffset + verticessize, lump->memory(mesh->indicesoffset()), indicessize);

        resources.update(mesh, lump, Bound3(Vec3(mhdr.mincorner[0], mhdr.mincorner[1], mhdr.mincorner[2]), Vec3(mhdr.maxcorner[0], mhdr.maxcorner[1], mhdr.maxcorner[2])));

        resources.release_lump(lump);
      }
    }

    unlock(fin);

    return mesh;
  }

  ///////////////////////// load_texture ////////////////////////////////////
  template<typename File>
  unique_resource<Texture> load_texture(Viewport::ResourceProxy &resources, File &fin, size_t index, Texture::Format format)
  {
    unique_resource<Texture> texture;

    lock(fin);

    PackImageHeader imag;

    if (read_asset_header(fin, index, &imag))
    {
      texture = resources.create<Texture>(imag.width, imag.height, imag.layers, imag.levels, format);

      if (auto lump = resources.acquire_lump(imag.datasize))
      {
        read_asset_payload(fin, imag.dataoffset, lump->memory(), imag.datasize);

        resources.update(texture, lump);

        resources.release_lump(lump);
      }
    }

    unlock(fin);

    return texture;
  }

  ///////////////////////// load_skybox /////////////////////////////////////
  template<typename File>
  unique_resource<SkyBox> load_skybox(Viewport::ResourceProxy &resources, File &fin, size_t index)
  {
    unique_resource<SkyBox> skybox;

    lock(fin);

    PackImageHeader imag;

    if (read_asset_header(fin, index, &imag))
    {
      skybox = resources.create<SkyBox>(imag.width, imag.height, EnvMap::Format::RGBE);

      if (auto lump = resources.acquire_lump(imag.datasize))
      {
        read_asset_payload(fin, imag.dataoffset, lump->memory(), imag.datasize);

        resources.update(skybox, lump);

        resources.release_lump(lump);
      }
    }

    unlock(fin);

    return skybox;
  }
}


///////////////////////// Resource::load_mesh ///////////////////////////////
template<>
unique_resource<Mesh> Viewport::ResourceProxy::load<Mesh>(size_t asset)
{
  return load_asset<Mesh>(*this, asset);
}

template<>
unique_resource<Mesh> Viewport::ResourceProxy::load<Mesh>(istream &fin, size_t index)
{
  return load_mesh(*this, fin, index);
}

template<>
unique_resource<Mesh> Viewport::ResourceProxy::load<Mesh>(Studio::Document *document, size_t index)
{
  return load_mesh(*this, document, index);

}


///////////////////////// Resource::load_texture ////////////////////////////
template<>
unique_resource<Texture> Viewport::ResourceProxy::load<Texture>(size_t asset, Texture::Format format)
{
  return load_asset<Texture>(*this, asset, format);
}

template<>
unique_resource<Texture> Viewport::ResourceProxy::load<Texture>(istream &fin, size_t index, Texture::Format format)
{
  return load_texture(*this, fin, index, format);
}

template<>
unique_resource<Texture> Viewport::ResourceProxy::load<Texture>(Studio::Document *document, size_t index, Texture::Format format)
{
  return load_texture(*this, document, index, format);
}


///////////////////////// Resource::load_skybox /////////////////////////////
template<>
unique_resource<SkyBox> Viewport::ResourceProxy::load<SkyBox>(size_t asset)
{
  return load_asset<SkyBox>(*this, asset);
}

template<>
unique_resource<SkyBox> Viewport::ResourceProxy::load<SkyBox>(istream &fin, size_t index)
{
  return load_skybox(*this, fin, index);
}

template<>
unique_resource<SkyBox> Viewport::ResourceProxy::load<SkyBox>(Studio::Document *document, size_t index)
{
  return load_skybox(*this, document, index);
}


///////////////////////// Resource::make_plane //////////////////////////////
unique_resource<Mesh> Viewport::ResourceProxy::make_plane(int sizex, int sizey, float scale, float tilex, float tiley)
{
  return { m_manager, ::make_plane(*m_manager, sizex, sizey, scale, tilex, tiley) };
}


//|---------------------- Viewport ------------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// Viewport::Constructor /////////////////////////////
Viewport::Viewport(size_t slabsize, size_t storagesize, QWidget *parent)
  : QWidget(parent),
    resources(Studio::Core::instance()->find_object<Studio::Platform>()->resources()),
    pushbuffermemory{0, slabsize, new char[slabsize]},
    m_pushbuffer(pushbuffermemory, slabsize)
{ 
  setAttribute(Qt::WA_NativeWindow);
  setAttribute(Qt::WA_DontCreateNativeAncestors);
  setAttribute(Qt::WA_PaintOnScreen);

  setFocusPolicy(Qt::ClickFocus);

  auto platform = Studio::Core::instance()->find_object<Studio::Platform>();

  initialise_render_context(*platform->instance(), m_rendercontext, storagesize, Studio::Platform::RenderQueue);

  renderparams.ssaoscale = 0;
  renderparams.ssrstrength = 0;
  renderparams.fogdensity = 0;
  renderparams.bloomstrength = 0;

  surface = platform->create_surface(winId());

  acquirecomplete = create_semaphore(m_rendercontext.vulkan);
  rendercomplete = create_semaphore(m_rendercontext.vulkan);

  camera.set_projection(60.0f*pi<float>()/180.0f, 1920.0f/1080.0f);

  m_resizetimer = new QTimer(this);
  m_resizetimer->setSingleShot(true);

  connect(m_resizetimer, &QTimer::timeout, this, &Viewport::resizedEvent);
}


///////////////////////// Viewport::Destructor //////////////////////////////
Viewport::~Viewport()
{
  auto platform = Studio::Core::instance()->find_object<Studio::Platform>();

  platform->resources()->release(platform->resources()->token());

  delete static_cast<char*>(pushbuffermemory.data);

  vkDeviceWaitIdle(m_rendercontext.vulkan);
}


///////////////////////// Viewport::hideEvent ///////////////////////////////
void Viewport::hideEvent(QHideEvent *event)
{
  swapchain = {};

  release_render_pipeline(m_rendercontext);

  QWidget::hideEvent(event);
}


///////////////////////// Viewport::resizeEvent /////////////////////////////
void Viewport::resizeEvent(QResizeEvent *event)
{
  setUpdatesEnabled(false);

  release_render_pipeline(m_rendercontext);

  m_resizetimer->start(250);

  QWidget::resizeEvent(event);
}


///////////////////////// Viewport::resizedEvent ////////////////////////////
void Viewport::resizedEvent()
{
  setUpdatesEnabled(true);
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

  while (!prepare_render_context(*platform->instance(), m_rendercontext, *platform->assets()))
    ;

  camera.set_projection(60.0f*pi<float>()/180.0f, (float)width() / (float)height());

  if (m_rendercontext.width != width() || m_rendercontext.height != height())
  {
    auto setuppool = create_commandpool(m_rendercontext.vulkan, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
    auto setupbuffer = allocate_commandbuffer(m_rendercontext.vulkan, setuppool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    VkSurfaceCapabilitiesKHR surfacecapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_rendercontext.vulkan.physicaldevice, surface, &surfacecapabilities);

    VkSwapchainCreateInfoKHR swapchaininfo = {};
    swapchaininfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchaininfo.surface = surface;
    swapchaininfo.minImageCount = surfacecapabilities.minImageCount;
    swapchaininfo.imageFormat = VK_FORMAT_B8G8R8A8_SRGB;
    swapchaininfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    swapchaininfo.imageExtent = surfacecapabilities.currentExtent;
    swapchaininfo.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
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
    if (vkCreateSwapchainKHR(m_rendercontext.vulkan, &swapchaininfo, nullptr, &newswapchain) != VK_SUCCESS)
      throw runtime_error("Vulkan vkCreateSwapchainKHR failed");

    swapchain = Vulkan::Swapchain(newswapchain, { m_rendercontext.vulkan.device });

    uint32_t imagescount = 0;
    vkGetSwapchainImagesKHR(m_rendercontext.vulkan.device, swapchain, &imagescount, nullptr);

    if (extent<decltype(presentimages)>::value < imagescount)
      throw runtime_error("Vulkan vkGetSwapchainImagesKHR failed");

    vkGetSwapchainImagesKHR(m_rendercontext.vulkan.device, swapchain, &imagescount, presentimages);

    VkCommandBufferBeginInfo begininfo = {};
    begininfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(setupbuffer, &begininfo) != VK_SUCCESS)
      throw runtime_error("Vulkan vkBeginCommandBuffer failed");

    for (size_t i = 0; i < imagescount; ++i)
    {
      VkImageMemoryBarrier memorybarrier = {};
      memorybarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
      memorybarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
      memorybarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      memorybarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      memorybarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
      memorybarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
      memorybarrier.image = presentimages[i];
      memorybarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

      vkCmdPipelineBarrier(setupbuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &memorybarrier);
    }

    vkEndCommandBuffer(setupbuffer);

    VkSubmitInfo submitinfo = {};
    submitinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitinfo.commandBufferCount = 1;
    submitinfo.pCommandBuffers = setupbuffer.data();

    vkQueueSubmit(m_rendercontext.vulkan.queue, 1, &submitinfo, VK_NULL_HANDLE);

    vkQueueWaitIdle(m_rendercontext.vulkan.queue);

    renderparams.width = width();
    renderparams.height = height();
    renderparams.aspect = camera.aspect();
  }

  prepare_render_pipeline(m_rendercontext, renderparams);

  m_pushbuffer.reset();

  m_resourcetoken = platform->resources()->token();

  return true;
}


///////////////////////// Viewport::begin ///////////////////////////////////
bool Viewport::begin(SpriteList &apritelist, SpriteList::BuildState &buildstate)
{
  auto platform = Studio::Core::instance()->find_object<Studio::Platform>();

  return apritelist.begin(buildstate, m_rendercontext, *platform->resources());
}


///////////////////////// Viewport::push_sprites ////////////////////////////
void Viewport::push_sprites(SpriteList const &apritelist)
{
  if (apritelist)
  {
    if (auto entry = m_pushbuffer.push<Renderable::Sprites>())
    {
      entry->spritecommands = apritelist.spritecommands;
    }
  }
}


///////////////////////// Viewport::begin ///////////////////////////////////
bool Viewport::begin(GeometryList &geometrylist, GeometryList::BuildState &buildstate)
{
  auto platform = Studio::Core::instance()->find_object<Studio::Platform>();

  return geometrylist.begin(buildstate, m_rendercontext, *platform->resources());
}


///////////////////////// Viewport::push_geometry ///////////////////////////
void Viewport::push_geometry(GeometryList const &geometrylist)
{
  if (geometrylist)
  {
    if (auto entry = m_pushbuffer.push<Renderable::Geometry>())
    {
      entry->prepasscommands = geometrylist.prepasscommands;
      entry->geometrycommands = geometrylist.geometrycommands;
    }
  }
}


///////////////////////// Viewport::begin ///////////////////////////////////
bool Viewport::begin(ForwardList &forwardlist, ForwardList::BuildState &buildstate)
{
  auto platform = Studio::Core::instance()->find_object<Studio::Platform>();

  return forwardlist.begin(buildstate, m_rendercontext, *platform->resources());
}


///////////////////////// Viewport::push_forward ////////////////////////////
void Viewport::push_forward(ForwardList const &forwardlist)
{
  if (forwardlist)
  {
    if (auto entry = m_pushbuffer.push<Renderable::Forward>())
    {
      entry->solidcommands = forwardlist.solidcommands;
      entry->blendcommands = forwardlist.blendcommands;
      entry->colorcommands = forwardlist.colorcommands;
    }
  }
}


///////////////////////// Viewport::begin ///////////////////////////////////
bool Viewport::begin(OverlayList &overlaylist, OverlayList::BuildState &buildstate)
{
  auto platform = Studio::Core::instance()->find_object<Studio::Platform>();

  return overlaylist.begin(buildstate, m_rendercontext, *platform->resources());
}


///////////////////////// Viewport::push_overlays ///////////////////////////
void Viewport::push_overlays(OverlayList const &overlaylist)
{
  if (overlaylist)
  {
    if (auto entry = m_pushbuffer.push<Renderable::Overlays>())
    {
      entry->overlaycommands = overlaylist.overlaycommands;
    }
  }
}


///////////////////////// Viewport::render //////////////////////////////////
void Viewport::render()
{
  using ::render;

  assert(m_rendercontext.ready);

  auto platform = Studio::Core::instance()->find_object<Studio::Platform>();

  uint32_t imageindex;
  vkAcquireNextImageKHR(m_rendercontext.vulkan.device, swapchain, UINT64_MAX, acquirecomplete, VK_NULL_HANDLE, &imageindex);

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

  vkQueuePresentKHR(m_rendercontext.vulkan.queue, &presentinfo);

  vkQueueWaitIdle(m_rendercontext.vulkan.queue);

  platform->resources()->release(m_resourcetoken);
}
