//
// Datum Platform
//

//
// Copyright (C) 2016 Peter Niekamp
//


#include "platform.h"
#include <leap/pathstring.h>
#include <iostream>
#include <fstream>
#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <QWindow>

#include <QtDebug>

using namespace std;
using namespace lml;
using namespace leap;
using namespace DatumPlatform;

//|---------------------- GameMemory ----------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// GameMemory::initialise ////////////////////////////
void gamememory_initialise(GameMemory &pool, void *data, size_t capacity)
{
  pool.size = 0;
  pool.data = data;
  pool.capacity = capacity;

  std::align(alignof(max_align_t), 0, pool.data, pool.capacity);
}


//|---------------------- FileHandle ----------------------------------------
//|--------------------------------------------------------------------------

class FileHandle
{
  public:
    FileHandle(const char *path);

    void read(uint64_t position, void *buffer, std::size_t n);

  private:

    std::mutex m_lock;

    std::fstream m_fio;
};


///////////////////////// FileHandle::Constructor /////////////////////////
FileHandle::FileHandle(const char *path)
{
  m_fio.open(path, ios::in | ios::binary);

  if (!m_fio)
    throw runtime_error(string("FileHandle Open Error: ") + path);
}


///////////////////////// FileHandle::Read ////////////////////////////////
void FileHandle::read(uint64_t position, void *buffer, size_t n)
{
  lock_guard<mutex> lock(m_lock);

  m_fio.seekg(position);

  m_fio.read((char*)buffer, n);

  if (!m_fio)
    throw runtime_error("FileHandle Read Error");
}



//|---------------------- WorkQueue -----------------------------------------
//|--------------------------------------------------------------------------

class WorkQueue
{
  public:
    WorkQueue(int threads = 4);
    ~WorkQueue();

    template<typename Func>
    void push(Func &&func)
    {
      std::unique_lock<std::mutex> lock(m_mutex);

      m_queue.push_back(std::forward<Func>(func));

      m_signal.notify_one();
    }

  private:

    std::atomic<bool> m_done;

    std::mutex m_mutex;

    std::condition_variable m_signal;

    std::deque<std::function<void()>> m_queue;

    std::vector<std::thread> m_threads;
};


///////////////////////// WorkQueue::Constructor //////////////////////////
WorkQueue::WorkQueue(int threads)
{
  m_done = false;

  for(int i = 0; i < threads; ++i)
  {
    m_threads.push_back(std::thread([=]() {

      while (!m_done)
      {
        std::function<void()> work;

        {
          unique_lock<std::mutex> lock(m_mutex);

          while (m_queue.empty())
          {
            m_signal.wait(lock);
          }

          work = std::move(m_queue.front());

          m_queue.pop_front();
        }

        work();
      }

    }));
  }
}


///////////////////////// WorkQueue::Destructor ///////////////////////////
WorkQueue::~WorkQueue()
{
  for(size_t i = 0; i < m_threads.size(); ++i)
    push([=]() { m_done = true; });

  for(auto &thread : m_threads)
    thread.join();
}


//|---------------------- Platform ------------------------------------------
//|--------------------------------------------------------------------------

class Platform : public PlatformInterface
{
  public:

    RenderDevice render_device() override;

    handle_t open_handle(const char *identifier) override;

    void read_handle(handle_t handle, uint64_t position, void *buffer, size_t bytes) override;

    void close_handle(handle_t handle) override;

    void submit_work(void (*func)(PlatformInterface &, void*, void*), void *ldata, void *rdata) override;

    void terminate() override;

  public:

    VkInstance instance;
    VkPhysicalDevice physicaldevice;
    VkDevice device;

  protected:

    WorkQueue m_workqueue;

} platform;

RenderDevice Platform::render_device()
{
  return { physicaldevice, device };
}

PlatformInterface::handle_t Platform::open_handle(const char *identifier)
{
  return new FileHandle(pathstring(identifier).c_str());
}

void Platform::read_handle(handle_t handle, uint64_t position, void *buffer, size_t bytes)
{
  static_cast<FileHandle*>(handle)->read(position, buffer, bytes);
}

void Platform::close_handle(PlatformInterface::handle_t handle)
{
  delete static_cast<FileHandle*>(handle);
}

void Platform::submit_work(void (*func)(PlatformInterface &, void*, void*), void *ldata, void *rdata)
{
  m_workqueue.push([=]() { func(*this, ldata, rdata); });
}

void Platform::terminate()
{
}

#ifndef NDEBUG
#define VALIDATION 0
#endif

///////////////////////// initialise_platform ///////////////////////////////
void initialise_platform(Platform &platform, size_t gamememorysize)
{
  gamememory_initialise(platform.gamememory, new char[gamememorysize], gamememorysize);

  //
  // Vulkan
  //

  VkApplicationInfo appinfo = {};
  appinfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appinfo.pApplicationName = "Datum Test";
  appinfo.pEngineName = "Datum";
  appinfo.apiVersion = VK_MAKE_VERSION(1, 0, 8);

#if VALIDATION
  const char *validationlayers[] = { "VK_LAYER_LUNARG_standard_validation" };
  const char *instanceextensions[] = { VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME, VK_EXT_DEBUG_REPORT_EXTENSION_NAME };
#else
  const char *validationlayers[] = { };
  const char *instanceextensions[] = { VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME };
#endif

  VkInstanceCreateInfo instanceinfo = {};
  instanceinfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instanceinfo.pApplicationInfo = &appinfo;
  instanceinfo.enabledExtensionCount = std::extent<decltype(instanceextensions)>::value;
  instanceinfo.ppEnabledExtensionNames = instanceextensions;
  instanceinfo.enabledLayerCount = std::extent<decltype(validationlayers)>::value;
  instanceinfo.ppEnabledLayerNames = validationlayers;

  VkInstance instance;
  if (vkCreateInstance(&instanceinfo, nullptr, &instance) != VK_SUCCESS)
    throw runtime_error("Vulkan CreateInstance failed");

  uint32_t physicaldevicecount = 0;
  vkEnumeratePhysicalDevices(instance, &physicaldevicecount, nullptr);

  if (physicaldevicecount == 0)
    throw runtime_error("Vulkan EnumeratePhysicalDevices failed");

  vector<VkPhysicalDevice> physicaldevices(physicaldevicecount);
  vkEnumeratePhysicalDevices(instance, &physicaldevicecount, physicaldevices.data());

  for(uint32_t i = 0; i < physicaldevicecount; ++i)
  {
    VkPhysicalDeviceProperties physicaldevicesproperties;
    vkGetPhysicalDeviceProperties(physicaldevices[i], &physicaldevicesproperties);

    cout << "Vulkan Physical Device " << i << ": " << physicaldevicesproperties.deviceName << endl;
  }

  VkPhysicalDevice physicaldevice;
  physicaldevice = physicaldevices[0];

  uint32_t queuecount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(physicaldevice, &queuecount, nullptr);

  if (queuecount == 0)
    throw runtime_error("Vulkan vkGetPhysicalDeviceQueueFamilyProperties failed");

  vector<VkQueueFamilyProperties> queueproperties(queuecount);
  vkGetPhysicalDeviceQueueFamilyProperties(physicaldevice, &queuecount, queueproperties.data());

  uint32_t queueindex = 0;
  while (queueindex < queuecount && !(queueproperties[queueindex].queueFlags & VK_QUEUE_GRAPHICS_BIT))
    ++queueindex;

  array<float, 1> queuepriorities = { 0.0f };

  VkDeviceQueueCreateInfo queueinfo = {};
  queueinfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queueinfo.queueFamilyIndex = queueindex;
  queueinfo.queueCount = 3;
  queueinfo.pQueuePriorities = queuepriorities.data();

  const char* deviceextensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

  VkPhysicalDeviceFeatures devicefeatures = {};
  devicefeatures.shaderClipDistance = true;
  devicefeatures.shaderCullDistance = true;
  devicefeatures.geometryShader = true;
  devicefeatures.shaderTessellationAndGeometryPointSize = true;
  devicefeatures.shaderStorageImageWriteWithoutFormat = true;

  VkDeviceCreateInfo deviceinfo = {};
  deviceinfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  deviceinfo.queueCreateInfoCount = 1;
  deviceinfo.pQueueCreateInfos = &queueinfo;
  deviceinfo.pEnabledFeatures = &devicefeatures;
  deviceinfo.enabledExtensionCount = std::extent<decltype(deviceextensions)>::value;
  deviceinfo.ppEnabledExtensionNames = deviceextensions;
  deviceinfo.enabledLayerCount = std::extent<decltype(validationlayers)>::value;
  deviceinfo.ppEnabledLayerNames = validationlayers;

  VkDevice device;
  if (vkCreateDevice(physicaldevice, &deviceinfo, nullptr, &device) != VK_SUCCESS)
    throw runtime_error("Vulkan vkCreateDevice failed");

#if VALIDATION

  //
  // Debug
  //

  static auto debugmessagecallback = [](VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objtype, uint64_t srcobject, size_t location, int32_t msgcode, const char *layerprefix, const char *msg, void *userdata) -> VkBool32 {
    cout << msg << endl;
    return false;
  };

  auto VkCreateDebugReportCallback = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");

  VkDebugReportCallbackCreateInfoEXT debugreportinfo = {};
  debugreportinfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
  debugreportinfo.pfnCallback = (PFN_vkDebugReportCallbackEXT)debugmessagecallback;
  debugreportinfo.pUserData = nullptr;
  debugreportinfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;// | VK_DEBUG_REPORT_INFORMATION_BIT_EXT;

  VkDebugReportCallbackEXT debugreportcallback;
  VkCreateDebugReportCallback(instance, &debugreportinfo, nullptr, &debugreportcallback);

#endif

  platform.instance = instance;
  platform.physicaldevice = physicaldevice;
  platform.device = device;
}


//|---------------------- Game ----------------------------------------------
//|--------------------------------------------------------------------------

class Game : public Studio::Platform
{
  public:

    Game(StackAllocator<> const &allocator);

    AssetManager *assets() { return &m_assets; }
    ResourceManager *resources() { return &m_resources; }

    AssetManager m_assets;
    ResourceManager m_resources;

    Vulkan::Surface create_surface(WId wid) override;
};


///////////////////////// Game::Contructor //////////////////////////////////
Game::Game(StackAllocator<> const &allocator)
  : m_assets(allocator),
    m_resources(&m_assets, allocator)
{
}


///////////////////////// initialise_platform ///////////////////////////////
void initialise_platform(QWindow *window)
{
  initialise_platform(platform, 16*1024*1024);

  auto game = new Game(platform.gamememory);

  initialise_asset_system(platform, game->m_assets, 1*1024, 2*1024*1024);

  initialise_resource_system(platform, game->m_resources, 2*1024*1024, 8*1024*1024, 64*1024*1024);

  game->m_assets.load(platform, "core.pack");

  Studio::Core::instance()->add_object(game);
}


///////////////////////// instance //////////////////////////////////////////
DatumPlatform::PlatformInterface *Studio::Platform::instance()
{
  return &platform;
}


///////////////////////// surface ///////////////////////////////////////////
Vulkan::Surface Game::create_surface(WId wid)
{
  VkWin32SurfaceCreateInfoKHR surfaceinfo = {};
  surfaceinfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  surfaceinfo.hinstance = GetModuleHandle(NULL);
  surfaceinfo.hwnd = (HWND)wid;

  VkSurfaceKHR surface;
  if (vkCreateWin32SurfaceKHR(platform.instance, &surfaceinfo, nullptr, &surface) != VK_SUCCESS)
    throw runtime_error("Vulkan vkCreateWin32SurfaceKHR failed");

  uint32_t queuecount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(platform.physicaldevice, &queuecount, nullptr);

  if (queuecount == 0)
    throw runtime_error("Vulkan vkGetPhysicalDeviceQueueFamilyProperties failed");

  vector<VkQueueFamilyProperties> queueproperties(queuecount);
  vkGetPhysicalDeviceQueueFamilyProperties(platform.physicaldevice, &queuecount, queueproperties.data());

  uint32_t queueindex = 0;
  while (queueindex < queuecount && !(queueproperties[queueindex].queueFlags & VK_QUEUE_GRAPHICS_BIT))
    ++queueindex;

  VkBool32 surfacesupport = VK_FALSE;
  vkGetPhysicalDeviceSurfaceSupportKHR(platform.physicaldevice, queueindex, surface, &surfacesupport);

  if (surfacesupport != VK_TRUE)
    throw runtime_error("Vulkan vkGetPhysicalDeviceSurfaceSupportKHR error");

  uint32_t formatscount = 0;
  vkGetPhysicalDeviceSurfaceFormatsKHR(platform.physicaldevice, surface, &formatscount, nullptr);

  vector<VkSurfaceFormatKHR> formats(formatscount);
  vkGetPhysicalDeviceSurfaceFormatsKHR(platform.physicaldevice, surface, &formatscount, formats.data());

  if (!any_of(formats.begin(), formats.end(), [](VkSurfaceFormatKHR surface) { return (surface.format == VK_FORMAT_B8G8R8A8_SRGB); }))
    throw runtime_error("Vulkan vkGetPhysicalDeviceSurfaceFormatsKHR error");

  return { surface, { platform.instance } };
}
