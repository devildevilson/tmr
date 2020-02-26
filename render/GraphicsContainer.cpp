#include "GraphicsContainer.h"

#include "Containers.h"
#include "VulkanRender.h"
#include "Window.h"

#include "Utility.h"
#include "Globals.h"
#include "settings.h"

static const std::vector<const char*> instanceLayers = {
  "VK_LAYER_LUNARG_standard_validation",
//   "VK_LAYER_LUNARG_object_tracker",
//   "VK_LAYER_GOOGLE_unique_objects",
//   "VK_LAYER_LUNARG_parameter_validation",
//   "VK_LAYER_LUNARG_api_dump",
//   "VK_LAYER_LUNARG_assistant_layer"
};

struct WindowData {
  bool fullscreen;
  uint32_t width;
  uint32_t height;
  float fov;
  GLFWmonitor* monitor;
  GLFWwindow* glfwWindow;
  VkSurfaceKHR surface;
};

void createGLFWwindow(yavf::Instance* inst, WindowData &data) {
//   const bool fullscreen = false;
  //const bool fullscreen = bool(Global::settings()->get<int64_t>("game.graphics.fullscreen"));
  const bool fullscreen = Global::get<devils_engine::utils::settings>()->graphics.fullscreen;
//   uint32_t width = 1280;
//   uint32_t height = 720;
//   uint32_t width = Global::settings()->get<int64_t>("game.graphics.width");
//   uint32_t height = Global::settings()->get<int64_t>("game.graphics.height");
  uint32_t width = Global::get<devils_engine::utils::settings>()->graphics.width;
  uint32_t height = Global::get<devils_engine::utils::settings>()->graphics.height;
//   const float fov = 60.0f;
  //const float fov = Global::settings()->get<float>("game.graphics.fov");
  const float fov = Global::get<devils_engine::utils::settings>()->graphics.fov;
  GLFWmonitor* monitor = nullptr;
  GLFWwindow* glfwWindow = nullptr;
  VkSurfaceKHR surface = VK_NULL_HANDLE;

//   int32_t count;
//   auto monitors = glfwGetMonitors(&count);
//   for (int32_t i = 0; i < count; ++i) {
//     std::cout << "Monitor name: " << glfwGetMonitorName(monitors[i]) << "\n";
//     int32_t x, y;
//     glfwGetMonitorPhysicalSize(monitors[i], &x, &y);
//     std::cout << "Monitor phys size: x " << x << " y " << y << "\n";
//     glfwGetMonitorPos(monitors[i], &x, &y);
//     std::cout << "Monitor pos: x " << x << " y " << y << "\n";
//   }

  if (fullscreen) {
    monitor = glfwGetPrimaryMonitor();

    const auto data = glfwGetVideoMode(monitor);
    width = data->width;
    height = data->height;

    Global::get<devils_engine::utils::settings>()->graphics.width = data->width;
    Global::get<devils_engine::utils::settings>()->graphics.height = data->height;

    // в какой то момент мне может потребоваться даунсэмплить изображение чтоб зернистость появилась
    // возможно это делается другим способом
  }

  glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_FALSE);
  glfwWindowHint(GLFW_REFRESH_RATE, GLFW_DONT_CARE);
  if (fullscreen) {
    glfwWindow = glfwCreateWindow(width, height, APPLICATION_NAME, monitor, nullptr);
  } else {
    glfwWindow = glfwCreateWindow(width, height, APPLICATION_NAME, nullptr, nullptr);
  }

//   glfwMakeContextCurrent(glfwWindow);
//   glfwSwapInterval(0);

  yavf::vkCheckError("glfwCreateWindowSurface", nullptr,
  glfwCreateWindowSurface(inst->handle(), glfwWindow, nullptr, &surface));

  data.fullscreen = fullscreen;
  data.width = width;
  data.height = height;
  data.fov = fov;
  data.monitor = monitor;
  data.glfwWindow = glfwWindow;
  data.surface = surface;
}

static bool vulkan_update_display_mode(uint32_t *width, uint32_t *height, const VkDisplayModePropertiesKHR *mode,
                                       uint32_t desired_width, uint32_t desired_height) {
  uint32_t visible_width = mode->parameters.visibleRegion.width;
  uint32_t visible_height = mode->parameters.visibleRegion.height;

  if (!desired_width || !desired_height) {
    /* Strategy here is to pick something which is largest resolution. */
    uint32_t area = visible_width * visible_height;
    if (area > (*width) * (*height)) {
      *width = visible_width;
      *height = visible_height;
      return true;
    } else return false;
    
  } else {
    /* For particular resolutions, find the closest. */
    int delta_x = int(desired_width) - int(visible_width);
    int delta_y = int(desired_height) - int(visible_height);
    int old_delta_x = int(desired_width) - int(*width);
    int old_delta_y = int(desired_height) - int(*height);

    int dist = delta_x * delta_x + delta_y * delta_y;
    int old_dist = old_delta_x * old_delta_x + old_delta_y * old_delta_y;
    
    if (dist < old_dist) {
      *width = visible_width;
      *height = visible_height;
      return true;
    } else return false;
  }
}

void createKHRdisplay(yavf::Instance* inst, WindowData &data) {
  VkSurfaceKHR surface = VK_NULL_HANDLE;
  
  auto gpu = inst->getPhysicalDevices()[0];

  uint32_t display_count;
  vkGetPhysicalDeviceDisplayPropertiesKHR(gpu, &display_count, nullptr);
  std::vector<VkDisplayPropertiesKHR> displays(display_count);
  vkGetPhysicalDeviceDisplayPropertiesKHR(gpu, &display_count, displays.data());

  uint32_t plane_count;
  vkGetPhysicalDeviceDisplayPlanePropertiesKHR(gpu, &plane_count, nullptr);
  std::vector<VkDisplayPlanePropertiesKHR> planes(plane_count);
  vkGetPhysicalDeviceDisplayPlanePropertiesKHR(gpu, &plane_count, planes.data());

#ifdef KHR_DISPLAY_ACQUIRE_XLIB
  VkDisplayKHR best_display = VK_NULL_HANDLE;
#endif
  VkDisplayModeKHR best_mode = VK_NULL_HANDLE;
  uint32_t best_plane = UINT32_MAX;

  const char *desired_display = nullptr;

  uint32_t actual_width = 0;
  uint32_t actual_height = 0;
  VkDisplayPlaneAlphaFlagBitsKHR alpha_mode = VK_DISPLAY_PLANE_ALPHA_OPAQUE_BIT_KHR;

  for (uint32_t dpy = 0; dpy < display_count; dpy++) {
    VkDisplayKHR display = displays[dpy].display;
    best_mode = VK_NULL_HANDLE;
    best_plane = UINT32_MAX;

    if (desired_display && strstr(displays[dpy].displayName, desired_display) != displays[dpy].displayName)
      continue;

    uint32_t mode_count;
    vkGetDisplayModePropertiesKHR(gpu, display, &mode_count, nullptr);
    std::vector<VkDisplayModePropertiesKHR> modes(mode_count);
    vkGetDisplayModePropertiesKHR(gpu, display, &mode_count, modes.data());

    for (uint32_t i = 0; i < mode_count; i++) {
      const VkDisplayModePropertiesKHR &mode = modes[i];
      
      if (vulkan_update_display_mode(&actual_width, &actual_height, &mode, 0, 0)) best_mode = mode.displayMode;
    }

    if (best_mode == VK_NULL_HANDLE) continue;

    for (uint32_t i = 0; i < plane_count; i++) {
      uint32_t supported_count = 0;
      VkDisplayPlaneCapabilitiesKHR plane_caps;
      vkGetDisplayPlaneSupportedDisplaysKHR(gpu, i, &supported_count, nullptr);

      if (!supported_count) continue;

      std::vector<VkDisplayKHR> supported(supported_count);
      vkGetDisplayPlaneSupportedDisplaysKHR(gpu, i, &supported_count, supported.data());

      uint32_t j;
      for (j = 0; j < supported_count; j++) {
        if (supported[j] == display) {
          if (best_plane == UINT32_MAX) best_plane = j;
          
          break;
        }
      }

      if (j == supported_count) continue;

      if (planes[i].currentDisplay == VK_NULL_HANDLE || planes[i].currentDisplay == display) best_plane = j;
      else continue;

      vkGetDisplayPlaneCapabilitiesKHR(gpu, best_mode, i, &plane_caps);

      if (plane_caps.supportedAlpha & VK_DISPLAY_PLANE_ALPHA_OPAQUE_BIT_KHR) {
        best_plane = j;
        alpha_mode = VK_DISPLAY_PLANE_ALPHA_OPAQUE_BIT_KHR;
#ifdef KHR_DISPLAY_ACQUIRE_XLIB
        best_display = display;
#endif
        break;
      }
    }
    
    if (best_plane != UINT32_MAX) break;
  }

  if (best_mode == VK_NULL_HANDLE) throw std::runtime_error("cannot find best screen mode");
  if (best_plane == UINT32_MAX) throw std::runtime_error("cannot find best screen plane");

  const VkDisplaySurfaceCreateInfoKHR create_info{
    VK_STRUCTURE_TYPE_DISPLAY_SURFACE_CREATE_INFO_KHR,
    nullptr,
    0,
    best_mode,
    best_plane,
    planes[best_plane].currentStackIndex,
    VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
    1.0f,
    alpha_mode,
    {actual_width, actual_height}
  };

#ifdef KHR_DISPLAY_ACQUIRE_XLIB
  dpy = XOpenDisplay(nullptr);
  if (dpy)
  {
    if (vkAcquireXlibDisplayEXT(gpu, dpy, best_display) != VK_SUCCESS)
      LOGE("Failed to acquire Xlib display. Surface creation may fail.\n");
  }
#endif

  yavf::vkCheckError("vkCreateDisplayPlaneSurfaceKHR", nullptr, 
  vkCreateDisplayPlaneSurfaceKHR(inst->handle(), &create_info, NULL, &surface)); 
  
  const float fov = Global::get<devils_engine::utils::settings>()->graphics.fov;
  data.fov = fov;
  data.fullscreen = true;
  data.glfwWindow = nullptr;
  data.monitor = nullptr;
  data.surface = surface;
  data.width = actual_width;
  data.height = actual_height;
}

void createDevice(yavf::Instance* inst, const WindowData &data, yavf::Device** device) {
  const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
  };

  VkSurfaceKHR s = data.surface;

  auto physDevices = inst->getPhysicalDevices();

  // как выбирать устройство?
  size_t maxMem = 0;

  yavf::PhysicalDevice choosen = VK_NULL_HANDLE;
  for (size_t i = 0; i < physDevices.size(); ++i) {
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    VkPhysicalDeviceMemoryProperties memProp;
    physDevices[i].getProperties(&deviceProperties);
    physDevices[i].getFeatures(&deviceFeatures);
    physDevices[i].getMemoryProperties(&memProp);

    size_t a = 0;
    for (uint32_t j = 0; j < memProp.memoryHeapCount; ++j) {
      if ((memProp.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) == VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
        a = std::max(memProp.memoryHeaps[i].size, a);
      }
    }

//     std::cout << "Device name: " << deviceProperties.deviceName << "\n";

    bool extSupp = yavf::checkDeviceExtensions(physDevices[i], instanceLayers, deviceExtensions);

    uint32_t count = 0;
    physDevices[i].getQueueFamilyProperties(&count, nullptr);

    bool presentOk = false;
    for (uint32_t i = 0; i < count; ++i) {
      VkBool32 present;
      vkGetPhysicalDeviceSurfaceSupportKHR(physDevices[i], i, s, &present);

      if (present) {
        presentOk = true;
        break;
      }
    }

    if (extSupp && presentOk && maxMem < a) {
      maxMem = a;
      choosen = physDevices[i];
      //break;
    }
  }
  
  VkPhysicalDeviceProperties deviceProperties;
  choosen.getProperties(&deviceProperties);
  Global::console()->printf("Using device: %s", deviceProperties.deviceName);

  yavf::DeviceMaker dm(inst);
  VkPhysicalDeviceFeatures f = {};
  f.samplerAnisotropy = VK_TRUE;
//   f.multiDrawIndirect = VK_TRUE;
//   f.drawIndirectFirstInstance = VK_TRUE;
//   f.fragmentStoresAndAtomics = VK_TRUE;
  *device = dm.beginDevice(choosen).setExtensions(deviceExtensions).createQueues().features(f).create(instanceLayers, "Graphics device");
}

void createWindow(yavf::Instance* inst, yavf::Device* device, const WindowData &data, char* memory, Window** window) {
  const Window::CreateInfo info{
    inst,
    device,
    data.monitor,
    data.glfwWindow,
    data.surface,
    data.fov,
    data.fullscreen
  };
  *window = new (memory) Window(info);

  //window.create(info);
}

void createRender(yavf::Instance* inst,
                  yavf::Device* device,
                  const uint32_t &frameCount,
                  const size_t &stageContainerSize,
                  //GameSystemContainer* container,
                  char* memory,
                  VulkanRender** render,
                  yavf::CombinedTask** task) {
  for (uint32_t i = 0; i < frameCount; ++i) {
    task[i] = device->allocateCombinedTask(frameCount);

//     std::cout << "task " << i << " family " << task[i]->getFamily() << "\n";
  }

  //const size_t stageContainerSize = sizeof(BeginTaskStage) + sizeof(EndTaskStage) + sizeof(GBufferStage) + sizeof(DefferedLightStage);
//   yavf::TaskInterface* i = task[0];
  const VulkanRender::CreateInfo info{
    inst,
    device,
//     task, //reinterpret_cast<yavf::TaskInterface**>(task),// &i,
    stageContainerSize
  };

  //*render = container->addSystem<VulkanRender>(info);
  *render = new (memory) VulkanRender(info);

//   yavf::TaskInterface** tasks = reinterpret_cast<yavf::TaskInterface**>(task);
//   for (uint32_t i = 0; i < frameCount; ++i) {
//     std::cout << "task " << i << " family " << task[i]->getFamily() << "\n";
//   }
}

GraphicsContainer::GraphicsContainer() : dev(nullptr), task(nullptr), windows(nullptr), render(nullptr) {
  uint32_t count;
  const char** ext = glfwGetRequiredInstanceExtensions(&count);
  if (count == 0) {
    Global::console()->print("Found no extensions\n");
    throw std::runtime_error("Extensions not founded!");
  }

  std::vector<const char*> extensions;
  for (uint32_t i = 0; i < count; ++i) {
    extensions.push_back(ext[i]);
  }
  
//   extensions.push_back("VK_KHR_display");
//   extensions.push_back("VK_EXT_direct_mode_display");

  const yavf::Instance::ApplicationInfo appInfo{
    APPLICATION_NAME,
    APP_VERSION,
    ENGINE_NAME,
    EGINE_VERSION,
    VK_API_VERSION_1_0
  };

  const yavf::Instance::CreateInfo info{
    nullptr,
    &appInfo,
#ifdef _DEBUG
    instanceLayers,
#else
    {},
#endif
    extensions,
#ifdef _DEBUG
    true,
#else
    false,
#endif
    false,
    false
  };
  
//   const yavf::Instance::CreateInfo info{
//     nullptr,
//     &appInfo,
//     {},
//     extensions,
//     false,
//     false,
//     false
//   };

  inst.construct(info);
}

GraphicsContainer::~GraphicsContainer() {
  if (task != nullptr) {
    for (uint32_t i = 0; i < windows->getFrameCount(); ++i) {
      dev->deallocate(task[i]);
    }
  }

  //if (task != nullptr) delete [] task;
  //if (windows != nullptr) delete windows;
  
  if (windows != nullptr) {
    windows->~Window();
    windows = nullptr;
  }
  
  if (render != nullptr) {
    render->~VulkanRender();
    render = nullptr;
  }
}

void GraphicsContainer::construct(CreateInfo &info) {
  TimeLogDestructor graphicsSystemLog("Render system initialization");
  
  ASSERT(WINDOW_CLASS_SIZE == sizeof(Window));
  ASSERT(VULKAN_RENDER_CLASS_SIZE == sizeof(VulkanRender));
  
  WindowData data;
  createGLFWwindow(instance(), data);
//   createKHRdisplay(instance(), data);

  createDevice(instance(), data, &dev);
  
//   throw std::runtime_error("end");
  
  char* windowMemory = memory+0;
  Window* window;
  createWindow(instance(), dev, data, windowMemory, &window);

  windows = window;

  const uint32_t count = window->getFrameCount();
  if (count > MAX_TASK_SIZE) throw std::runtime_error("window->getFrameCount() > MAX_TASK_SIZE");
  
  char* renderMemory = windowMemory+sizeof(Window);
  char* tasksMemory = renderMemory+sizeof(VulkanRender);
  //task = new yavf::CombinedTask*[count];
  task = reinterpret_cast<yavf::CombinedTask**>(tasksMemory);
  //createRender(instance(), dev, count, info.containerSize, info.systemContainer, &render, task);
  createRender(instance(), dev, count, info.containerSize, renderMemory, &render, task);
  
  render->setContext(this);

  for (uint32_t i = 0; i < count; ++i) {
    task[i]->pushWaitSemaphore(window->at(i).imageAvailableSemaphore, window->at(i).flags);
    task[i]->pushSignalSemaphore(window->at(i).finishedRenderingSemaphore);
  }

  window->setRender(render);
  
  Global g;
  g.setRender(render);
  g.setWindow(window);
}

void GraphicsContainer::update(const size_t &time) {
  {
    windows->nextFrame();
  }
//   Global::render()->update(time);

  // без семафор и соответственно без презента работает нормально
  // а с ними начинается какой то пиздец
  // я выключил абсолютно все другие вычисления
  // + ко всему у меня рандомно вылетало в физике и в других местах
  // я подумал сначало что я опять где заговнил указатель
  // но нет, я вроде все проверил из того что переделывал
  // меня не покидает чувство что я просто где изменил что то по недогляду
  // но вернуть я ничего не могу потому что kdevelop вылетел =(
  // санитизеры молчат, правда иногда все же что-то говорят
  // но не могут сказать точно где у меня проблема
  // что мне делать?

  // опять вечер и опять теже симптомы =(
  // хотя соседний скомпилированный файл работает нормально
  // я так и знал что проблема где то в другом месте
  // я более чем уверен что проблема где то с указателями
  // то есть я что то изменяю на соседнем участке из-за чего его начинает ломать

  // после перезагрузки все встало на свои места
  // я переписал raii, так как он был очень не безопасным
  // на момент 30 марта этих дурацких лагов нет

  {
    render->update(time);
  }

  {
    render->start();
  }

  {
    windows->present(); // по идее нет никакой разницы где это стоит
  }
}

yavf::Instance* GraphicsContainer::instance() {
  return &inst;
}

yavf::Device* GraphicsContainer::device() const {
  return dev;
}

yavf::TaskInterface* GraphicsContainer::interface() const {
  const uint32_t index = windows->currentFrame();
  return task[index];
}

yavf::CombinedTask* GraphicsContainer::combined() const {
  const uint32_t index = windows->currentFrame();
  return task[index];
}

yavf::ComputeTask* GraphicsContainer::compute() const {
  const uint32_t index = windows->currentFrame();
  return task[index];
}

yavf::GraphicTask* GraphicsContainer::graphics() const {
  const uint32_t index = windows->currentFrame();
  return task[index];
}

yavf::TransferTask* GraphicsContainer::transfer() const {
//   const uint32_t index = windows->currentFrame();
//   return task[index];
  return nullptr;
}
