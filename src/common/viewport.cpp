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
        platform->resources()->request(*platform->instance(), *asset);
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

      if (auto lump = resources.acquire_lump(mesh->vertexbuffer.size))
      {
        read_asset_payload(fin, mhdr.dataoffset, lump->memory(mesh->vertexbuffer.verticesoffset), mesh->vertexbuffer.vertexcount*mesh->vertexbuffer.vertexsize);
        read_asset_payload(fin, mhdr.dataoffset + mesh->vertexbuffer.vertexcount*mesh->vertexbuffer.vertexsize, lump->memory(mesh->vertexbuffer.indicesoffset), mesh->vertexbuffer.indexcount*mesh->vertexbuffer.indexsize);

        resources.update(mesh, lump);

        resources.release_lump(lump);
      }

      resources.update(mesh, Bound3(Vec3(mhdr.mincorner[0], mhdr.mincorner[1], mhdr.mincorner[2]), Vec3(mhdr.maxcorner[0], mhdr.maxcorner[1], mhdr.maxcorner[2])));
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

  surface = platform->create_surface(winId());

  acquirecomplete = create_semaphore(m_rendercontext.vulkan);
  rendercomplete = create_semaphore(m_rendercontext.vulkan);

  camera.set_projection(60.0f*pi<float>()/180.0f, 1920.0f/1080.0f);
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
  if (m_rendercontext.ready)
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

  while (!prepare_render_context(*platform->instance(), m_rendercontext, *platform->assets()))
    ;

  camera.set_projection(60.0f*pi<float>()/180.0f, (float)width() / (float)height());

  if (m_rendercontext.width != width() || m_rendercontext.height != height())
  {
    VkSurfaceCapabilitiesKHR surfacecapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_rendercontext.vulkan.physicaldevice, surface, &surfacecapabilities);

    VkSwapchainCreateInfoKHR swapchaininfo = {};
    swapchaininfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchaininfo.surface = surface;
    swapchaininfo.minImageCount = surfacecapabilities.minImageCount;
    swapchaininfo.imageFormat = VK_FORMAT_B8G8R8A8_SRGB;
    swapchaininfo.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
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

    for(size_t i = 0; i < imagescount; ++i)
    {
      setimagelayout(m_rendercontext.vulkan, presentimages[i], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });
    }

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
      entry->forwardcommands = forwardlist.forwardcommands;
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
