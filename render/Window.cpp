#include "Window.h"

#include <iostream>
#include "yavf.h"

#include "Utility.h"

#include "Globals.h"
#include "VulkanRender.h"

#define REFRESH_RATE_TO_MCS(rate) (1000000.0f / float(rate))

VkResult createSwapChain(const SurfaceData &surfaceData,
                         yavf::Device* device,
                         Swapchain &swapchainData) {
  uint32_t imageCount = surfaceData.capabilities.minImageCount + 1;
  if (surfaceData.capabilities.maxImageCount > 0 && imageCount > surfaceData.capabilities.maxImageCount) {
    imageCount = surfaceData.capabilities.maxImageCount;
  }

  yavf::Swapchain old = swapchainData.swapchain;

  const VkSwapchainCreateInfoKHR info{
    VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    nullptr,
    0,
    surfaceData.handle,
    imageCount,
    surfaceData.format.format,
    surfaceData.format.colorSpace,
    surfaceData.extent,
    1,
    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
    VK_SHARING_MODE_EXCLUSIVE,
    0,
    nullptr,
    surfaceData.capabilities.currentTransform,
    VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    surfaceData.presentMode,
    VK_TRUE,
    old
  };

  swapchainData.swapchain = device->recreate(info, "window_swapchain");

  swapchainData.images = device->getSwapchainImages(swapchainData.swapchain);

  return VK_SUCCESS;
}

WindowInterface::WindowInterface() : iconified(false), focused(true), refreshTimeVar(SIZE_MAX) {}

size_t WindowInterface::refreshTime() {
  return refreshTimeVar;
}

void WindowInterface::setIconify(const int &i) {
  iconified = bool(i);
}

void WindowInterface::setFocus(const int &f) {
  focused = bool(f);
}

bool WindowInterface::isIconified() const {
  return iconified;
}

bool WindowInterface::isFocused() const {
  return focused;
}

WindowRenderTarget::WindowRenderTarget() : renderPassHandle(VK_NULL_HANDLE), buffer(VK_NULL_HANDLE), clearValuePtr(nullptr) {}
WindowRenderTarget::~WindowRenderTarget() {}

std::vector<VkClearValue> WindowRenderTarget::clearValues() const {
  return *clearValuePtr;
}

VkRect2D WindowRenderTarget::size() const {
  return windowSize;
}

yavf::RenderPass WindowRenderTarget::renderPass() const {
  return renderPassHandle;
}

VkViewport WindowRenderTarget::viewport() const {
  return {
    0.0f, 0.0f,
    static_cast<float>(windowSize.extent.width),
    static_cast<float>(windowSize.extent.height),
    0.0f, 1.0f
  };
}

VkRect2D WindowRenderTarget::scissor() const {
  return windowSize;
}

yavf::Framebuffer WindowRenderTarget::framebuffer() const {
  return buffer;
}

void WindowRenderTarget::set(const VkRect2D &windowSize, const yavf::RenderPass &renderPassHandle, const yavf::Framebuffer &buffer, std::vector<VkClearValue>* clearValuePtr) {
  this->windowSize = windowSize;
  this->renderPassHandle = renderPassHandle;
  this->buffer = buffer;
  this->clearValuePtr = clearValuePtr;
}

Window::Window() :
  vsync(true),
  fullscreenMode(false),
  windowedNoFrame(false),
  iconified(false),
  focused(false),
  fov(0.0f),
  imageIndex(0),
  currentVirtualFrame(0),
  instance(nullptr),
  device(nullptr),
  monitor(nullptr),
  window(nullptr),
  renderPassHandle(VK_NULL_HANDLE) {}

Window::Window(const CreateInfo &info)
  : vsync(true),
    fullscreenMode(info.fullscreen),
    windowedNoFrame(false),
    iconified(false),
    focused(false),
    fov(info.fov),
    imageIndex(0),
    currentVirtualFrame(0),
    instance(info.inst),
    device(info.device),
    monitor(info.monitor),
    window(info.window),
    renderPassHandle(VK_NULL_HANDLE) {
  create(info);
}

// Window::Window(const float &fov) {
//   this->fov = fov;
// }

Window::~Window() {
  for (size_t i = 0; i < frames.size(); ++i) {
    device->destroy(frames[i].imageAvailableSemaphore);
    device->destroy(frames[i].finishedRenderingSemaphore);

    device->destroy(depthImages[i]);

    device->destroy(targets[i].framebuffer());
  }

  device->destroy(renderPassHandle);

  device->destroy(swapchain.swapchain);

  vkDestroySurfaceKHR(instance->handle(), surface.handle, nullptr);
}

void Window::addMonitor(MonitorInfo info) {
  if (surface.handle != VK_NULL_HANDLE) return;
  this->surface.handle = info.surface;
  this->monitor = info.handle;
  this->window = info.window;
}

void Window::create(const CreateInfo &info) {
  // if (surface.handle != VK_NULL_HANDLE) return;

  fullscreenMode = info.fullscreen;
  windowedNoFrame = false;
  iconified = false;
  focused = false;
  fov = info.fov;
  imageIndex = 0;
  currentVirtualFrame = 0;
  instance = info.inst;
  device = info.device;
  monitor = info.monitor;
  window = info.window;

  {
    auto m = glfwGetWindowMonitor(window);
    const auto data = glfwGetVideoMode(m == nullptr ? glfwGetPrimaryMonitor() : m);
//     int count = 0;
//     const auto data1 = glfwGetVideoModes(m == nullptr ? glfwGetPrimaryMonitor() : m, &count);
    //for (size_t i = 0; i < )
//     PRINT_VAR("data->refreshRate",data->refreshRate)
    refreshTimeVar = std::min(size_t(REFRESH_RATE_TO_MCS(data->refreshRate)), refreshTimeVar);
  }

  this->surface.handle = info.surface;

  glfwGetWindowPos(window, &windowSize.offset.x, &windowSize.offset.y);
  int w, h;
  glfwGetWindowSize(window, &w, &h);
//   windowSize.extent.width = w;
//   windowSize.extent.height = h;

  for (size_t i = 0; i < device->getFamiliesCount(); ++i) {
    VkBool32 present;
    vkGetPhysicalDeviceSurfaceSupportKHR(device->physicalHandle(), i, surface.handle, &present);

    if (present == VK_TRUE) {
      family = i;
      break;
    }
  }

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device->physicalHandle(), surface.handle, &surface.capabilities);

  // ???????????????? ?????????? ?????????????????? ???? ?????????????? hdr ????????????
  uint32_t count = 0;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device->physicalHandle(), surface.handle, &count, nullptr);
  std::vector<VkSurfaceFormatKHR> formats(count);
  vkGetPhysicalDeviceSurfaceFormatsKHR(device->physicalHandle(), surface.handle, &count, formats.data());

//   std::cout << "Window formats count " << formats.size() << "\n";
//   for (uint32_t i = 0; i < formats.size(); ++i) {
//     std::cout << "Window format " << i << " : " << (formats[i].format) << "\n";
//   }

  // ?? ?????????????? ?????????????????????? ???????????????????????? ?????? ???????????? ?????????????? ????????
  vkGetPhysicalDeviceSurfacePresentModesKHR(device->physicalHandle(), surface.handle, &count, nullptr);
  std::vector<VkPresentModeKHR> presents(count);
  vkGetPhysicalDeviceSurfacePresentModesKHR(device->physicalHandle(), surface.handle, &count, presents.data());

  surface.format = yavf::chooseSwapSurfaceFormat(formats);
  surface.presentMode = yavf::chooseSwapPresentMode(presents);
  surface.extent = yavf::chooseSwapchainExtent(windowSize.extent.width, windowSize.extent.height, surface.capabilities);
  windowSize.extent.width = surface.extent.width;
  windowSize.extent.height = surface.extent.height;

  immediatePresentMode = yavf::checkSwapchainPresentMode(presents, VK_PRESENT_MODE_IMMEDIATE_KHR);

//   std::cout << "Width: " << width << " height: " << height << "\n";
//   std::cout << "Width: " << surface.extent.width << " height: " << surface.extent.height << "\n";

//   yavf::vkCheckError("createSwapChain", nullptr,
  createSwapChain(surface, device, swapchain);

  values = {
    {0.0f, 0.0f, 0.0f, 1.0f},
    {1.0f, 0}
  };

  const VkFormat depth = yavf::findSupportedFormat(device->physicalHandle(),
                                                   {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                                                   VK_IMAGE_TILING_OPTIMAL,
                                                   VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

  depthImages.resize(swapchain.images.size(), nullptr);
  frames.resize(swapchain.images.size());
  targets.resize(swapchain.images.size());
  for (uint32_t i = 0; i < swapchain.images.size(); ++i) {
    swapchain.images[i]->createView(VK_IMAGE_VIEW_TYPE_2D, {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}, surface.format.format);

    depthImages[i] = device->create(yavf::ImageCreateInfo::texture2D(surface.extent,
                                                                     VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                                                     depth),
                                    VMA_MEMORY_USAGE_GPU_ONLY);
    depthImages[i]->createView(VK_IMAGE_VIEW_TYPE_2D, {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1});

    frames[i].imageAvailableSemaphore = device->createSemaphore();
    frames[i].flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    frames[i].finishedRenderingSemaphore = device->createSemaphore();
  }

  createRenderPass();

  for (uint32_t i = 0; i < swapchain.images.size(); ++i) {
    const std::vector<VkImageView> views = {
      swapchain.images[i]->view()->handle(),
      depthImages[i]->view()->handle()
    };
    yavf::Framebuffer fb = device->create(yavf::FramebufferCreateInfo::framebuffer(renderPassHandle,
                                                                                   views.size(),
                                                                                   views.data(),
                                                                                   surface.extent.width,
                                                                                   surface.extent.height),
                                          "window_framebuffer_"+std::to_string(i));

    targets[i].set({{0, 0}, surface.extent}, renderPassHandle, fb, &values);
  }

  yavf::TransferTask* task = device->allocateTransferTask();

  task->begin();
  for (uint32_t i = 0; i < depthImages.size(); ++i) {
    task->setBarrier(depthImages[i], VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);
  }
  task->end();

  task->start();
  task->wait();

  device->deallocate(task);
}

VirtualFrame & Window::at(const size_t &index) {
  return frames[index];
}

const VirtualFrame & Window::at(const size_t &index) const {
  return frames[index];
}

uint32_t Window::framesCount() const {
  return frames.size();
}

uint32_t Window::currentFrame() const {
  return currentVirtualFrame;
}

uint32_t Window::currentImage() const {
  return imageIndex;
}

yavf::RenderTarget* Window::currentRenderTarget() {
  return &targets[imageIndex];
}

// std::vector<VkClearValue> Window::clearValues() const  {
//   return {{0.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0}};
// }

// VkImage Window::image() const  {
//   return swapchain.images[imageIndex];
// }

// VkRect2D Window::size() const  {
//   VkRect2D rect{
//     {0, 0},
//     surface.extent
//   };
//
//   return rect;
// }
//
// VkRenderPass Window::renderPass() const  {
//   return renderPassHandle;
// }
//
// VkViewport Window::viewport() const  {
//   return view;
// }
//
// VkRect2D Window::scissor() const  {
//   return scis;
// }
//
// yavf::Framebuffer Window::framebuffer() const  {
//   return frames[currentFrame].framebuffer;
// }
//
// yavf::SemaphoreProxy* Window::getSemaphoreProxy() const {
//   return owner;
// }
//
// void Window::addSemaphoreProxy(yavf::SemaphoreProxy* proxy) {
//   proxies.push_back(proxy);
// }

// VkSemaphore Window::imageAvailable() const  {
//   return frames[currentFrame].imageAvailableSemaphore;
// }
//
// VkSemaphore Window::finishRendering() const  {
//   return frames[currentFrame].finishedRenderingSemaphore;
// }

void Window::nextFrame() {
  currentVirtualFrame = (currentVirtualFrame + 1) % frames.size();

  VkResult res = swapchain.swapchain.acquireNextImage(UINT64_MAX, frames[currentVirtualFrame].imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

  switch(res) {
    case VK_SUCCESS:
    case VK_SUBOPTIMAL_KHR:
      break;
    case VK_ERROR_OUT_OF_DATE_KHR:
      resize();
      break;
    default:
      Global::console()->print("Problem occurred during swap chain image acquisition!");
      throw std::runtime_error("Problem occurred during swap chain image acquisition!");
  }

//   if (frames[currentFrame].framebuffer != VK_NULL_HANDLE) {
//     vkDestroyFramebuffer(device->handle(), frames[currentFrame].framebuffer, nullptr);
//   }
//
//   const std::array<VkImageView, 2> attachments = {
//     swapchain.imageViews[imageIndex],
//     depthImages[imageIndex]->view().handle
//   };
//
//   const VkFramebufferCreateInfo framebufferCreateInfo{
//     VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
//     nullptr,
//     0,
//     renderPassHandle,
//     attachments.size(),
//     attachments.data(),
//     surface.extent.width,
//     surface.extent.height,
//     1
//   };
//
// //   std::cout << "Window next frame" << "\n";
//
//   yavf::vkCheckError("vkCreateFramebuffer", nullptr,
//   vkCreateFramebuffer(device->handle(), &framebufferCreateInfo, nullptr, &frames[currentFrame].framebuffer));
}

void Window::present() {
//   if (proxies.empty()) throw std::runtime_error("Proxies == 0 in window");

//   VkSemaphore s[proxies.size()];
//   for (uint32_t i = 0; i < proxies.size(); ++i) {
//     s[i] = proxies[i]->get();
// //     std::cout << "Wait " << s[i] << " semaphore" << "\n";
//   }

//   VkPresentInfoKHR info{
//     VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
//     nullptr,
//     1,
//     &frames[currentFrame].finishedRenderingSemaphore,
//     1,
//     &swapchain.handle,
//     &imageIndex,
//     nullptr
//   };

  VkResult res;
  {
    // RegionLog rl("vkQueuePresent");

    const VkSwapchainKHR s = swapchain.swapchain;

    const VkPresentInfoKHR info{
      VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      nullptr,
      1,
      &frames[currentVirtualFrame].finishedRenderingSemaphore,
      1,
      &s,
      &imageIndex,
      nullptr
    };

    auto queue = device->getQueue(family);

    res = vkQueuePresentKHR(queue.handle, &info);
  }

  // RegionLog rl("switch(res)");

  switch(res) {
    case VK_SUCCESS:
      break;
    case VK_ERROR_OUT_OF_DATE_KHR:
    case VK_SUBOPTIMAL_KHR:
      // int width, height;
      resize();
      break;
    default:
      Global::console()->print("Problem occurred during image presentation!\n");
      throw std::runtime_error("Problem occurred during image presentation!");
  }

//   vkQueueWaitIdle(queue.handle);
}

void Window::resize() {
  device->wait();

  int width, height;
  glfwGetWindowSize(window, &width, &height);
  std::cout << "Window::resize width " << width << " height " << height << '\n';
  
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device->physicalHandle(), surface.handle, &surface.capabilities);
  surface.extent = yavf::chooseSwapchainExtent(width, height, surface.capabilities);
  windowSize.extent.width = surface.extent.width;
  windowSize.extent.height = surface.extent.height;
  std::cout << "surface.extent width " << surface.extent.width << " height " << surface.extent.height << '\n';
  
//   std::cout << "sadadsadd" << "\n";
  
  if (!fullscreenMode) {
    glfwGetWindowPos(window, &windowSize.offset.x, &windowSize.offset.y);
//     windowSize.extent.width = width;
//     windowSize.extent.height = height;
//     windowSize.extent.width = surface.extent.width;
//     windowSize.extent.height = surface.extent.height;
  }

//   yavf::vkCheckError("createSwapChain", nullptr,
//   createSwapChain(device->handle(), surface, swapchain));

  createSwapChain(surface, device, swapchain);

  const VkFormat depth = yavf::findSupportedFormat(device->physicalHandle(),
                                                   {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                                                   VK_IMAGE_TILING_OPTIMAL,
                                                   VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

  for (uint32_t i = 0; i < swapchain.images.size(); ++i) {
    device->destroy(depthImages[i]);
    device->destroy(targets[i].framebuffer());

    swapchain.images[i]->createView(VK_IMAGE_VIEW_TYPE_2D, {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}, surface.format.format);

    depthImages[i] = device->create(yavf::ImageCreateInfo::texture2D(surface.extent,
                                                                     VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                                                     depth),
                                    VMA_MEMORY_USAGE_GPU_ONLY);
    depthImages[i]->createView(VK_IMAGE_VIEW_TYPE_2D, {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1});

    const std::vector<VkImageView> views = {
      swapchain.images[i]->view()->handle(),
      depthImages[i]->view()->handle()
    };
    yavf::Framebuffer fb = device->create(yavf::FramebufferCreateInfo::framebuffer(renderPassHandle,
                                                                                   views.size(),
                                                                                   views.data(),
                                                                                   surface.extent.width,
                                                                                   surface.extent.height),
                                          "window_framebuffer_"+std::to_string(i));

    targets[i].set({{0, 0}, surface.extent}, renderPassHandle, fb, &values);
  }

  yavf::TransferTask* task = device->allocateTransferTask();

  task->begin();
  for (uint32_t i = 0; i < depthImages.size(); ++i) {
    task->setBarrier(depthImages[i], VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);
  }
  task->end();

  task->start();
  task->wait();

  device->deallocate(task);

  if (render == nullptr) return;

  const simd::mat4 &perspective = simd::perspective(glm::radians(fov), float(surface.extent.width) / float(surface.extent.height), 0.1f, FAR_CLIPPING);
  const simd::mat4 &ortho = simd::ortho(0.0f, float(surface.extent.width) / float(surface.extent.height), 0.0f, 1.0f, 0.1f, FAR_CLIPPING);

  render->setPersp(perspective);
  render->setOrtho(ortho);

  render->setCameraDim(surface.extent.width, surface.extent.height);

  render->updateCamera();
}

void Window::fullscreen(const size_t &video_mode) {
  // ???????????? ???????? VK_KHR_display (???? ??????????????????)

  fullscreenMode = !fullscreenMode;

  if (fullscreenMode) {
    if (monitor == nullptr) monitor = glfwGetPrimaryMonitor();
    
    const GLFWvidmode* mode = nullptr;
    if (video_mode == SIZE_MAX) {
      mode = glfwGetVideoMode(monitor);
    } else {
      int count;
      auto modes = glfwGetVideoModes(monitor, &count);
      ASSERT(video_mode < uint32_t(count));
      mode = &modes[video_mode];
    }
    
    glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
    windowedNoFrame = false;
  } else {
    glfwSetWindowMonitor(window, nullptr, windowSize.offset.x, windowSize.offset.y, windowSize.extent.width, windowSize.extent.height, 0);
  }

  // resize();
}

void Window::noFrame() {
  windowedNoFrame = !windowedNoFrame;

  if (windowedNoFrame) {
    if (monitor == nullptr) monitor = glfwGetPrimaryMonitor();

    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    glfwSetWindowMonitor(window, monitor, windowSize.offset.x, windowSize.offset.y, windowSize.extent.width, windowSize.extent.height, mode->refreshRate);
    fullscreenMode = false;
  } else {
    glfwSetWindowMonitor(window, nullptr, windowSize.offset.x, windowSize.offset.y, windowSize.extent.width, windowSize.extent.height, 0);
  }

  // resize();
}

// void Window::iconify() {
//   iconified = !iconified;
// }

// bool Window::isIconified() const {
//   return iconified;
// }

GLFWwindow* Window::handle() const {
  return window;
}

bool Window::isFullscreen() const {
  return fullscreenMode;
}

bool Window::isNoFrame() const {
  return windowedNoFrame;
}

bool Window::shouldClose() const {
  return glfwWindowShouldClose(window);
}

void Window::toggleVsync() {
  if (!immediatePresentMode) return;

  vsync = !vsync;

  if (vsync) {
    surface.presentMode = lastMode;
  } else {
    lastMode = surface.presentMode;
    surface.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
  }

  resize();
}

bool Window::isVsync() const {
  return vsync;
}

bool Window::hasImmediatePresentMode() const {
  return immediatePresentMode;
}

void Window::hide() {
  glfwHideWindow(window);
}

void Window::show() {
  glfwShowWindow(window);
}

void Window::setFov(const float &fov) {
  if (glm::abs(this->fov-fov) > EPSILON) {
    const simd::mat4 &perspective = simd::perspective(glm::radians(fov), float(surface.extent.width) / float(surface.extent.height), 0.1f, FAR_CLIPPING);
    const simd::mat4 &ortho = simd::ortho(0.0f, float(surface.extent.width) / float(surface.extent.height), 0.0f, 1.0f, 0.1f, FAR_CLIPPING);
    render->setPersp(perspective);
    render->setOrtho(ortho);
    render->setCameraDim(surface.extent.width, surface.extent.height);
    render->updateCamera();
  }
  
  this->fov = fov;
}

float Window::getFov() const {
  return fov;
}

//#define PRINT_VEC(name, vec) std::cout << name << " (" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ")" << "\n";

void Window::setRender(VulkanRender* render) {
  this->render = render;

  const simd::mat4 &perspective = simd::perspective(glm::radians(fov), float(surface.extent.width) / float(surface.extent.height), 0.1f, FAR_CLIPPING);
  const simd::mat4 &ortho = simd::ortho(0.0f, float(surface.extent.width) / float(surface.extent.height), 0.0f, 1.0f, 0.1f, FAR_CLIPPING);

//   PRINT_VEC("view ", perspective[0])
//   PRINT_VEC("     ", perspective[1])
//   PRINT_VEC("     ", perspective[2])
//   PRINT_VEC("     ", perspective[3])

  //throw std::runtime_error("no more");

  render->setPersp(perspective);
  render->setOrtho(ortho);

  render->setCameraDim(surface.extent.width, surface.extent.height);

  render->updateCamera();
}

uint32_t Window::getFamily() const {
  return family;
}

uint32_t Window::getFrameCount() const {
  return swapchain.images.size();
}

// VkDescriptorSet Window::getCurrentSet() {
//   return swapchain.sets[imageIndex];
// }

yavf::Image* Window::getImage() const {
  return swapchain.images[imageIndex];
}

yavf::Image* Window::getDepth() const {
  return depthImages[imageIndex];
}

VkRect2D Window::size() const {
  return windowSize;
}

yavf::RenderPass Window::pass() const {
  return renderPassHandle;
}

// void Window::createDescriptors(VkDescriptorSetLayout layout, VkDescriptorPool pool) {
//   yavf::DescriptorMaker dm(device);
//
//   swapchain.sets.resize(swapchain.images.size(), VK_NULL_HANDLE);
//   for (uint32_t i = 0; i < swapchain.images.size(); ++i) {
//     auto descs = dm.layout(layout).create(pool);
//     swapchain.sets[i] = descs[0].handle;
//   }
// }

void Window::createRenderPass() {
  yavf::RenderPassMaker rpm(device);

  renderPassHandle = rpm.attachmentBegin(surface.format.format)
                          .attachmentLoadOp(VK_ATTACHMENT_LOAD_OP_DONT_CARE)
                          .attachmentStoreOp(VK_ATTACHMENT_STORE_OP_STORE)
                          .attachmentFinalLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
                        .attachmentBegin(depthImages[0]->info().format)
                          .attachmentLoadOp(VK_ATTACHMENT_LOAD_OP_LOAD)
                          .attachmentStoreOp(VK_ATTACHMENT_STORE_OP_DONT_CARE)
                          .attachmentInitialLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
                          .attachmentFinalLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
                        .subpassBegin(VK_PIPELINE_BIND_POINT_GRAPHICS)
                          .subpassColorAttachment(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 0)
                          .subpassDepthStencilAttachment(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1)
                        .dependencyBegin(VK_SUBPASS_EXTERNAL, 0)
                          .dependencySrcStageMask(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT)
                          .dependencyDstStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)
                          .dependencySrcAccessMask(VK_ACCESS_MEMORY_READ_BIT)
                          .dependencyDstAccessMask(VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
                        .dependencyBegin(0, VK_SUBPASS_EXTERNAL)
                          .dependencySrcStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)
                          .dependencyDstStageMask(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT)
                          .dependencySrcAccessMask(VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
                          .dependencyDstAccessMask(VK_ACCESS_MEMORY_READ_BIT)
                        .create("default render pass");
}

/**
 * VkDisplayModeKHR mode = VK_NULL_HANDLE;
    VkDisplayKHR display = VK_NULL_HANDLE;
    uint32_t bestPlane = UINT32_MAX;
    VkDisplayPlaneAlphaFlagBitsKHR alpha;

    // ???????????????? ?????? ??????????????
    uint32_t count = 0;
    vkGetPhysicalDeviceDisplayPropertiesKHR(device->physicalHandle(), &count, nullptr);
    std::vector<VkDisplayPropertiesKHR> props(count);
    vkGetPhysicalDeviceDisplayPropertiesKHR(device->physicalHandle(), &count, props.data());

    // ???????????????? ?????? ?????????????????? ???????????????? (?) (?????? ?????? ?????????????????? ?????????????????? ?????? ?? ?????)
    vkGetPhysicalDeviceDisplayPlanePropertiesKHR(device->physicalHandle(), &count, nullptr);
    std::vector<VkDisplayPlanePropertiesKHR> planeProps(count);
    vkGetPhysicalDeviceDisplayPlanePropertiesKHR(device->physicalHandle(), &count, planeProps.data());

    for (uint32_t i = 0; i < props.size(); ++i) {
      std::cout << "Display name: " << props[i].displayName << "\n";

      // ???????????????? ???????????? ?????????? (?????????? ???????????????????????????) ??????????????
      vkGetDisplayModePropertiesKHR(device->physicalHandle(), props[i].display, &count, nullptr);
      std::vector<VkDisplayModePropertiesKHR> displayModes(count);
      vkGetDisplayModePropertiesKHR(device->physicalHandle(), props[i].display, &count, displayModes.data());

      for (uint32_t j = 0; j < displayModes.size(); ++j) {
        // ???????????????????? ?????????????? ???????????????????? ?????? ???????????? ?????????????????????????? ?????????????? (?)

        std::cout << "Mode " << j << " refresh rate: " << displayModes[j].parameters.refreshRate / 1000 << "\n";
        std::cout << "Mode " << j << " visible region: (" << displayModes[j].parameters.visibleRegion.width << ", " << displayModes[j].parameters.visibleRegion.height << ")" << "\n";
      }

      // ?????????? ???????? ?????? ?????????? ????????????
      mode = displayModes[0].displayMode;
      // ???????? ?????? ???? ???????????????? ???????? ????, ???? ???? ?????????? ???????????????????? ???????????? ?????????????????? ??????????????
      if (mode == VK_NULL_HANDLE) continue;

      display = props[i].display;

      // ???????????? ?????????????????? ???? ??????????????????:
      // - ???????????????? ?????????????? + ????????
      // - ???? ???????????????????????? ?? ?????????????? ??????????????
      // - ???????????????????????? ???????????????????????? ?????????? ??????????
      for (uint32_t j = 0; j < planeProps.size(); ++j) {
        std::cout << "Plane " << j << " stack index: " << planeProps[j].currentStackIndex << "\n";

        // ???????????? ?????????????????????????? ?? ?????????????? ????????????????
        vkGetDisplayPlaneSupportedDisplaysKHR(device->physicalHandle(), j, &count, nullptr);

        // ?????????????????? ???? ???????????????????????? ??????????????. ?????????? ?????????? ?????????????????????? ???? ??????????????????,
        // ???? ?????????????? ???????? ?????????????????? ?????????????????????????? ???????????? ????????????????????, ?? ??????????
        // ???? ???????? ?????????????? ???? ???????????????????????? ?? ?????????? ?????????? ???????? ?????????????????? count ?????????? ?????????????????? ????????
        if (count == 0) continue;

        std::vector<VkDisplayKHR> supportedDisplay(count);
        vkGetDisplayPlaneSupportedDisplaysKHR(device->physicalHandle(), j, &count, supportedDisplay.data());

        uint32_t k = 0;
        for (; k < supportedDisplay.size(); ++k) {
          if (supportedDisplay[k] == props[i].display) {
            // ???????? ???? ???? ?????????? ???????????????????? ?????????????????? ???? ?????? ??????,
            // ???? ????????????????????
            if (bestPlane == UINT32_MAX) bestPlane = j;
            break;
          }
        }

        // ???????? ?????????????????? ???? ?????????? ???????? ???????????????????????? ?? ?????????????????? ????????????????, ???????????????????? ????????????
        // ???????????? ?????????????? ???????????? ?????????? ???????? ???? ???????? ???????????????????? ??????????????????
        if (k == supportedDisplay.size()) continue;

        // ???????? ?????????????????? ???????????? ?????? ?????????? ???????????? ?? ???????????????????????? ?????? ?? ?????????????????????????? ?????? ??????????????,
        // ?????? ???? ???????????????????????? ???? ?? ????????, ???? ?????? ???????? ?????? ???????????? ??????????????????
        if ((planeProps[j].currentDisplay == VK_NULL_HANDLE) ||
            (planeProps[j].currentDisplay == props[i].display)) {
          bestPlane = j;
        } else continue;

        VkDisplayPlaneCapabilitiesKHR planeCaps;
        vkGetDisplayPlaneCapabilitiesKHR(device->physicalHandle(), mode, j ,&planeCaps);

        // ???????????????? ???? ?????????????????? ?????????????????????????? ?????????? ????????????
        if (planeCaps.supportedAlpha & VK_DISPLAY_PLANE_ALPHA_PER_PIXEL_BIT_KHR) {
          // ?????????????????? ?????????????????? ???? ?????????? ??????????????????
          bestPlane = j;
          alpha = VK_DISPLAY_PLANE_ALPHA_PER_PIXEL_BIT_KHR;
          break;
        }
      }

      // ???????????? ???? ?? ?????????? ???????????
    }

    if (mode == VK_NULL_HANDLE) {
      // ???? ???????????? ???? ??????????!!!
      // ???????????? ???? ?? ?????????????? ?????????? ???????
    } else {
      VkDisplaySurfaceCreateInfoKHR createInfo{
        VK_STRUCTURE_TYPE_DISPLAY_SURFACE_CREATE_INFO_KHR,
        nullptr,
        0,
        mode,
        bestPlane,
        planeProps[bestPlane].currentStackIndex,
        VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        1.0f,
        alpha,
        {200, 200} // ?????????? ?? ???????????? ???????????? ???????????????? ????????????????, ?????? ???? ????????????? ???????????? ???? ?????????? ???? VkDisplayPlaneCapabilitiesKHR
      };

      vkCreateDisplayPlaneSurfaceKHR(instance->handle(), &createInfo, nullptr, &surface.handle);
    }
    */
