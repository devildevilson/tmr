#include "GraphicsContainer.h"

#include "Containers.h"
#include "VulkanRender.h"
#include "Window.h"

#include "Utility.h"
#include "Globals.h"
#include "Settings.h"

static const std::vector<const char*> instanceLayers = {
  "VK_LAYER_LUNARG_standard_validation",
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
  const bool fullscreen = Global::settings()->get<int64_t>("game.graphics.fullscreen");
//   uint32_t width = 1280;
//   uint32_t height = 720;
  uint32_t width = Global::settings()->get<int64_t>("game.graphics.width");
  uint32_t height = Global::settings()->get<int64_t>("game.graphics.height");
//   const float fov = 60.0f;
  const float fov = Global::settings()->get<float>("game.graphics.fov");
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

    Global::settings()->get<int64_t>("game.graphics.width") = data->width;
    Global::settings()->get<int64_t>("game.graphics.height") = data->height;

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

    std::cout << "Device name: " << deviceProperties.deviceName << "\n";


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

//   auto devices = inst->getDevices([s, deviceExtensions] (VkPhysicalDevice physDevice) -> bool {
//     VkPhysicalDeviceProperties deviceProperties;
//     VkPhysicalDeviceFeatures deviceFeatures;
//     vkGetPhysicalDeviceProperties(physDevice, &deviceProperties);
//     vkGetPhysicalDeviceFeatures(physDevice, &deviceFeatures);
//
//     std::cout << "Device name: " << deviceProperties.deviceName << "\n";
//
//     bool extSupp = yavf::checkDeviceExtensions(physDevice, yavf::Instance::getLayers(), deviceExtensions);
//
//     uint32_t count = 0;
//     vkGetPhysicalDeviceQueueFamilyProperties(physDevice, &count, nullptr);
//
//     bool presentOk = false;
//     for (uint32_t i = 0; i < count; ++i) {
//       VkBool32 present;
//       vkGetPhysicalDeviceSurfaceSupportKHR(physDevice, i, s, &present);
//
//       if (present) {
//         presentOk = true;
//         break;
//       }
//     }
//
//     return //deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
//            extSupp &&
//            presentOk;
//   });

  yavf::DeviceMaker dm(inst);
//   yavf::DeviceMaker::setExtensions(deviceExtensions);
  VkPhysicalDeviceFeatures f = {};
  f.samplerAnisotropy = VK_TRUE;
//   f.multiDrawIndirect = VK_TRUE;
//   f.drawIndirectFirstInstance = VK_TRUE;
//   f.fragmentStoresAndAtomics = VK_TRUE;
  //*device = dm.beginDevice(devices[0]).createQueues().features(f).create("Graphic device");
  *device = dm.beginDevice(choosen).setExtensions(deviceExtensions).createQueues().features(f).create(instanceLayers, "Graphics device");
}

void createWindow(yavf::Instance* inst, yavf::Device* device, const WindowData &data, Window** window) {
  const Window::CreateInfo info{
    inst,
    device,
    data.monitor,
    data.glfwWindow,
    data.surface,
    data.fov,
    data.fullscreen
  };
  *window = new Window(info);

  //window.create(info);
}

void createRender(yavf::Instance* inst,
                  yavf::Device* device,
                  const uint32_t &frameCount,
                  const size_t &stageContainerSize,
                  GameSystemContainer* container,
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
    task, //reinterpret_cast<yavf::TaskInterface**>(task),// &i,
    stageContainerSize
  };

  *render = container->addSystem<VulkanRender>(info);

//   yavf::TaskInterface** tasks = reinterpret_cast<yavf::TaskInterface**>(task);
//   for (uint32_t i = 0; i < frameCount; ++i) {
//     std::cout << "task " << i << " family " << task[i]->getFamily() << "\n";
//   }
}

GraphicsContainer::GraphicsContainer() : dev(nullptr), task(nullptr), task1(nullptr), task2(nullptr), task3(nullptr), windows(nullptr), render(nullptr) {
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

//   yavf::Instance::setExtensions(extensions);
//   yavf::Instance::setLayers({
//     "VK_LAYER_LUNARG_standard_validation",
//     "VK_LAYER_LUNARG_api_dump",
//     "VK_LAYER_LUNARG_assistant_layer"
//   });

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

  inst.construct(info);
}

GraphicsContainer::~GraphicsContainer() {
  if (task != nullptr) {
    for (uint32_t i = 0; i < windows->getFrameCount(); ++i) {
      dev->deallocate(task[i]);
    }
  }

  if (task != nullptr) delete [] task;
  if (task1 != nullptr) delete [] task1;
  if (task2 != nullptr) delete [] task2;
  if (task3 != nullptr) delete [] task3;
  if (windows != nullptr) delete windows;
}

void GraphicsContainer::construct(CreateInfo &info) {
  WindowData data;
  createGLFWwindow(instance(), data);

  createDevice(instance(), data, &dev);

  Window* window;
  createWindow(instance(), dev, data, &window);

  windows = window;

  const uint32_t count = window->getFrameCount();
  task  = new yavf::CombinedTask*[count];
  task1 = new yavf::ComputeTask*[count];
  task2 = new yavf::GraphicTask*[count];
  task3 = new yavf::TaskInterface*[count];
  createRender(instance(), dev, count, info.containerSize, info.systemContainer, &render, task);

  for (uint32_t i = 0; i < count; ++i) {
//     std::cout << "command buffer " << task[i]->getCommandBuffer() << "\n";
    task1[i] = task[i];
    task2[i] = task[i];
    task3[i] = task[i];
  }

//   throw std::runtime_error("no more");

//   for (uint32_t i = 0; i < 3; ++i) {
//     std::cout << "task pointer " << i << " " << task[i] << "\n";
//   }
//   std::cout << "pointer to pointer " << task << "\n";

  for (uint32_t i = 0; i < count; ++i) {
    task[i]->pushWaitSemaphore(window->at(i).imageAvailableSemaphore, window->at(i).flags);
    task[i]->pushSignalSemaphore(window->at(i).finishedRenderingSemaphore);
  }

  window->setRender(render);

  //Global::renderPtr = render;
  Global g;
  g.setRender(render);
  g.setWindow(window);
}

void GraphicsContainer::update(const uint64_t &time) {
  {
    // RegionLog rl("window->nextFrame()");

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
//     RegionLog rl("render->update()");

    uint32_t index = windows->currentFrame();
    render->setContextIndex(index);
    render->update(time);
  }

  {
    // RegionLog rl("render->start()");

    render->start();
  }

  {
    // RegionLog rl("windows->present()");

    windows->present(); // по идее нет никакой разницы где это стоит
  }
}

yavf::Instance* GraphicsContainer::instance() {
  return &inst;
}

yavf::Device* GraphicsContainer::device() const {
  return dev;
}

yavf::CombinedTask** GraphicsContainer::tasks() const {
  return task;
}

yavf::ComputeTask** GraphicsContainer::tasks1() const {
  return task1;
}

yavf::GraphicTask** GraphicsContainer::tasks2() const {
  return task2;
}

yavf::TaskInterface** GraphicsContainer::tasks3() const {
  return task3;
}
