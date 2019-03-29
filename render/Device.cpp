#include "Device.h"

Device::Device() {}

Device::~Device() {
  //std::cout << "device destruction" << std::endl;
  clean();
}

void Device::clean() {
  //int index = (commandPoolIndex1 != 666)? commandPoolIndex1 : commandPoolIndex;
  if (device != VK_NULL_HANDLE) {
    //std::cout << "before vkDeviceWaitIdle2" << "\n";
    vkDeviceWaitIdle(device);

    auto loadStart = std::chrono::steady_clock::now();
    auto loadStart1 = std::chrono::steady_clock::now();

    for (size_t i = 0; i < virtualFrames.size(); i++) {
      if (virtualFrames[i].framebuffer != VK_NULL_HANDLE) {
        vkDestroyFramebuffer(device, virtualFrames[i].framebuffer, nullptr);
      }

      if (virtualFrames[i].commandBuffer != VK_NULL_HANDLE) {
        vkFreeCommandBuffers(device, commandPools[commandPoolIndex], 1, &virtualFrames[i].commandBuffer);
      }

      if (virtualFrames[i].imageAvailableSemaphore != VK_NULL_HANDLE) {
        vkDestroySemaphore(device, virtualFrames[i].imageAvailableSemaphore, nullptr);
      }

      if (virtualFrames[i].finishedRenderingSemaphore != VK_NULL_HANDLE) {
        vkDestroySemaphore(device, virtualFrames[i].finishedRenderingSemaphore, nullptr);
      }

      if (virtualFrames[i].fence != VK_NULL_HANDLE) {
        vkDestroyFence(device, virtualFrames[i].fence, nullptr);
      }
    }

    auto elapsed = std::chrono::steady_clock::now() - loadStart;
    uint64_t microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
    //std::cout << "Virtual frames deleted for " << microseconds << std::endl;
    Global::console()->printf("Virtual frames deleted in %d mcs\n", microseconds);

    loadStart = std::chrono::steady_clock::now();

    for (size_t i = 0; i < res.size(); i++) {
      if (res[i].commandBuffer != VK_NULL_HANDLE) {
        vkFreeCommandBuffers(device, commandPools[transferPoolIndex], 1, &res[i].commandBuffer);
      }

      if (res[i].fence != VK_NULL_HANDLE) {
        vkDestroyFence(device, res[i].fence, nullptr);
      }
    }

    elapsed = std::chrono::steady_clock::now() - loadStart;
    microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
    //std::cout << "Additional resources deleted for " << microseconds << std::endl;
    Global::console()->printf("Additional resources deleted in %d mcs\n", microseconds);

    //std::cout << "virtual frames destroyed successful" << std::endl;

    loadStart = std::chrono::steady_clock::now();

    for (size_t i = 0; i < commandPools.size(); i++) {
      if (commandPools[i] != VK_NULL_HANDLE) {
        vkDestroyCommandPool(device, commandPools[i], nullptr);
      }
      commandPools[i] = VK_NULL_HANDLE;
    }

    elapsed = std::chrono::steady_clock::now() - loadStart;
    microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
    //std::cout << "Command pools deleted for " << microseconds << std::endl;
    Global::console()->printf("Command pools deleted in %d mcs\n", microseconds);
    
    loadStart = std::chrono::steady_clock::now();
    if (queryPool != VK_NULL_HANDLE) {
      vkDestroyQueryPool(device, queryPool, nullptr);
      queryPool = VK_NULL_HANDLE;
    }
    elapsed = std::chrono::steady_clock::now() - loadStart;
    microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
    Global::console()->printf("Query pool deleted in %d mcs\n", microseconds);

    // здесь удаление всех используемых ресурсов (картинки, вершинные и индексные буферы и тд)

    loadStart = std::chrono::steady_clock::now();

//     if (deviceLocal != nullptr) {
//       deviceLocal->clearMemory();
//     }
//     if (hostVisible != nullptr) {
//       hostVisible->clearMemory();
//     }
//     for (uint32_t i = 0; i < bufferObjects.size(); i++) {
//       if (bufferObjects[i].parameters.handle != VK_NULL_HANDLE) {
//         vkDestroyBuffer(device, bufferObjects[i].parameters.handle, nullptr);
//       }
//       if (bufferObjects[i].parameters.memory != VK_NULL_HANDLE) {
//         vkFreeMemory(device, bufferObjects[i].parameters.memory, nullptr);
//       }
//     }

//     delete deviceLocal;
//     delete hostVisible;

    deviceMemory.clean();

    elapsed = std::chrono::steady_clock::now() - loadStart;
    microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
    //std::cout << "Buffers deleted for " << microseconds << std::endl;
    Global::console()->printf("Buffers deleted in %d mcs\n", microseconds);

    loadStart = std::chrono::steady_clock::now();

    if (texSampler.handle != VK_NULL_HANDLE) {
      vkDestroySampler(device, texSampler.handle, nullptr);
      texSampler.handle = VK_NULL_HANDLE;
    }

    if (fontSampler.handle != VK_NULL_HANDLE) {
      vkDestroySampler(device, fontSampler.handle, nullptr);
      fontSampler.handle = VK_NULL_HANDLE;
    }

//     for (uint32_t i = 0; i < imageObjects.size(); i++) {
//       if (imageObjects[i].parameters.view != VK_NULL_HANDLE) {
//         vkDestroyImageView(device, imageObjects[i].parameters.view, nullptr);
//       }
//       if (imageObjects[i].parameters.handle != VK_NULL_HANDLE) {
//         vkDestroyImage(device, imageObjects[i].parameters.handle, nullptr);
//       }
//       if (imageObjects[i].parameters.memory != VK_NULL_HANDLE) {
//         vkFreeMemory(device, imageObjects[i].parameters.memory, nullptr);
//       }
//     }

    if (depthImage.view != VK_NULL_HANDLE) {
      vkDestroyImageView(device, depthImage.view, nullptr);
    }
    if (depthImage.handle != VK_NULL_HANDLE) {
      vkDestroyImage(device, depthImage.handle, nullptr);
    }
    if (depthImage.memory != VK_NULL_HANDLE) {
      vkFreeMemory(device, depthImage.memory, nullptr);
    }
    
    if (occlusionImage.view != VK_NULL_HANDLE) {
      vkDestroyImageView(device, occlusionImage.view, nullptr);
    }
    if (occlusionImage.handle != VK_NULL_HANDLE) {
      vkDestroyImage(device, occlusionImage.handle, nullptr);
    }
    if (occlusionImage.memory != VK_NULL_HANDLE) {
      vkFreeMemory(device, occlusionImage.memory, nullptr);
    }

    elapsed = std::chrono::steady_clock::now() - loadStart;
    microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
    //std::cout << "Images deleted for " << microseconds << std::endl;
    Global::console()->printf("Images deleted in %d mcs\n", microseconds);

    loadStart = std::chrono::steady_clock::now();
    
    queueFamilies.clear();
    
//     for(std::vector<QueueFamily*>::iterator it = queueFamilies.begin(); it!=queueFamilies.end(); ++it)
//       delete (*it);

    elapsed = std::chrono::steady_clock::now() - loadStart;
    microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
    //std::cout << "Queue families deleted for " << microseconds << std::endl;
    Global::console()->printf("Queue families deleted for %d mcs\n", microseconds);

    //std::cout << "queue families destroyed successful" << std::endl;

    loadStart = std::chrono::steady_clock::now();

//     std::set<VkPipelineLayout*> toDelete;
    
    for (size_t i = 0; i < layouts.size(); i++) {
      if (layouts[i].handle != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(device, layouts[i].handle, nullptr);
        layouts[i].handle = VK_NULL_HANDLE;
      }
    }

    for (size_t i = 0; i < graphicsPipelines.size(); i++) {
//       if (*graphicsPipelines[i].layout != VK_NULL_HANDLE) {
//         vkDestroyPipelineLayout(device, *graphicsPipelines[i].layout, nullptr);
//         *graphicsPipelines[i].layout = VK_NULL_HANDLE;
//       }
//       toDelete.insert(graphicsPipelines[i].layout);

      if (graphicsPipelines[i].handle != VK_NULL_HANDLE) {
        vkDestroyPipeline(device, graphicsPipelines[i].handle, nullptr);
        graphicsPipelines[i].handle = VK_NULL_HANDLE;
      }
    }

    for (size_t i = 0; i < computePipelines.size(); i++) {
//       if (*computePipelines[i].layout != VK_NULL_HANDLE) {
//         vkDestroyPipelineLayout(device, *computePipelines[i].layout, nullptr);
//         *computePipelines[i].layout = VK_NULL_HANDLE;
//       }
//       toDelete.insert(graphicsPipelines[i].layout);

      if (computePipelines[i].handle != VK_NULL_HANDLE) {
        vkDestroyPipeline(device, computePipelines[i].handle, nullptr);
        computePipelines[i].handle = VK_NULL_HANDLE;
      }
    }

//     for (auto &elem : toDelete) {
//       delete elem;
//     }

    elapsed = std::chrono::steady_clock::now() - loadStart;
    microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
    Global::console()->printf("Pipelines deleted in %d mcs\n", microseconds);

    loadStart = std::chrono::steady_clock::now();
    
    for (size_t i = 0; i < renderPasses.size(); i++) {
      if(renderPasses[i] != VK_NULL_HANDLE) {
        vkDestroyRenderPass(device, renderPasses[i], nullptr);
        renderPasses[i] = VK_NULL_HANDLE;
      }
    }

    elapsed = std::chrono::steady_clock::now() - loadStart;
    microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
    //std::cout << "Render passes deleted for " << microseconds << std::endl;
    Global::console()->printf("Render passes deleted in %d mcs\n", microseconds);

    loadStart = std::chrono::steady_clock::now();

    if (descriptorPool != VK_NULL_HANDLE) {
      vkDestroyDescriptorPool(device, descriptorPool, nullptr);
      descriptorPool = VK_NULL_HANDLE;
    }
    
//     if (storagePool != VK_NULL_HANDLE) {
//       vkDestroyDescriptorPool(device, storagePool, nullptr);
//       storagePool = VK_NULL_HANDLE;
//     }

    if (layoutSampler != VK_NULL_HANDLE) {
      vkDestroyDescriptorSetLayout(device, layoutSampler, nullptr);
    }

    if (layoutSampledImage != VK_NULL_HANDLE) {
      vkDestroyDescriptorSetLayout(device, layoutSampledImage, nullptr);
    }

    if (layoutSampledImage != VK_NULL_HANDLE) {
      vkDestroyDescriptorSetLayout(device, layoutCombinedImageSampler, nullptr);
    }
    
    if (uboSetLayout != VK_NULL_HANDLE) {
      vkDestroyDescriptorSetLayout(device, uboSetLayout, nullptr);
    }
    
    if (storageSetLayout != VK_NULL_HANDLE) {
      vkDestroyDescriptorSetLayout(device, storageSetLayout, nullptr);
    }

    elapsed = std::chrono::steady_clock::now() - loadStart;
    microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
    //std::cout << "Descriptors deleted for " << microseconds << std::endl;
    Global::console()->printf("Descriptors deleted in %d mcs\n", microseconds);

    loadStart = std::chrono::steady_clock::now();
    
    for (size_t s = 0; s < swapchains.size(); s++) {
      for (size_t i = 0; i < swapchains[s].imageViews.size(); i++) {
        if (swapchains[s].imageViews[i] != VK_NULL_HANDLE) {
          vkDestroyImageView(device, swapchains[s].imageViews[i], nullptr);
        }
      }

      if (swapchains[s].handle != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(device, swapchains[s].handle, nullptr);
      }
    }

    elapsed = std::chrono::steady_clock::now() - loadStart;
    microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
    //std::cout << "Swapchain destroyed for " << microseconds << std::endl;
    Global::console()->printf("Swapchain destroyed in %d mcs\n", microseconds);

    //std::cout << "swapchain destroyed successful" << std::endl;
    vkDestroyDevice(device, nullptr);
    device = VK_NULL_HANDLE;

    auto elapsedAll = std::chrono::steady_clock::now() - loadStart1;
    uint64_t microsecondsAll = std::chrono::duration_cast<std::chrono::microseconds>(elapsedAll).count();
    //std::cout << "All resources deleted for " << microsecondsAll << std::endl;
    Global::console()->printf("All resources deleted in %d mcs\n", microsecondsAll);
  }
}

void Device::init(const VkPhysicalDevice &physicalDevice, 
                  const std::vector<const char*> &deviceExtensions, 
                  const bool &enableValidationLayers, 
                  const std::vector<const char*> &validationLayers, 
                  const VkSurfaceKHR &surface) {
  this->physicalDevice = physicalDevice;
  vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);
  std::vector<QueueFamilyInfo> infos;
  findQueueFamilies(surface, 4, infos);

  // указываем, какие очереди нам постребуются от устройства
  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
//   for (const auto &queueFamily : queueFamilies) {
//     VkDeviceQueueCreateInfo queueCreateInfo = {};
//     queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
//     queueCreateInfo.queueFamilyIndex = queueFamily.getQueueFamilyIndex();
//     queueCreateInfo.queueCount = queueFamily.getQueueHandlesSize();
//     // приоритеты выдаются каждой очереди.
//     /// TODO: Нужно подумать куда это запихнуть и как конкретно расставить приоритеты
//     float *priority = new float[queueFamily.getQueueHandlesSize()];
//     for (uint32_t i = 0; i < queueFamily.getQueueHandlesSize(); i++) {
//       priority[i] = 1.0f;
//     }
//     queueCreateInfo.pQueuePriorities = priority;
//     queueCreateInfos.push_back(queueCreateInfo);
//     //delete [] priority;
//   }
  
  for (size_t i = 0; i < infos.size(); i++) {
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.flags = 0;
    queueCreateInfo.pNext = nullptr;
    queueCreateInfo.queueFamilyIndex = i;
    queueCreateInfo.queueCount = infos[i].queueCount;
    // приоритеты выдаются каждой очереди.
    /// TODO: Нужно подумать куда это запихнуть и как конкретно расставить приоритеты
    float *priority = new float[infos[i].queueCount];
    for (uint32_t i = 0; i < infos[i].queueCount; i++) {
      priority[i] = 1.0f;
    }
    queueCreateInfo.pQueuePriorities = priority;
    queueCreateInfos.push_back(queueCreateInfo);
    
    delete [] priority;
  }

  // возможность отрисовки линий для AABB
  // анизотропная фильтрация
  VkPhysicalDeviceFeatures deviceFeatures = {};
  deviceFeatures.fillModeNonSolid = VK_TRUE;
  deviceFeatures.samplerAnisotropy = VK_TRUE;

  // перейдем непосредственно к созданию логического устройства
  VkDeviceCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  createInfo.pQueueCreateInfos = queueCreateInfos.data();
  createInfo.queueCreateInfoCount = (uint32_t) queueCreateInfos.size();
  createInfo.pEnabledFeatures = &deviceFeatures;
  createInfo.enabledExtensionCount = deviceExtensions.size();
  createInfo.ppEnabledExtensionNames = deviceExtensions.data();
  // такие же расширения слоев валидации, как и у instance
  if (enableValidationLayers) {
      createInfo.enabledLayerCount = validationLayers.size();
      createInfo.ppEnabledLayerNames = validationLayers.data();
  } else {
      createInfo.enabledLayerCount = 0;
  }

  if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
    Global::console()->print("failed to create logical device!\n");
    throw std::runtime_error("failed to create logical device!");
  }

  for (uint32_t i = 0; i < infos.size(); i++) {
    queueFamilies.emplace_back();
    queueFamilies.back().init(infos[i].queueCount, infos[i].flags, infos[i].present, i, device);
  }

  createAdditionalResources();

  //vkGetDeviceQueue(device, 0, 0, &testingQueue);

  //std::cout << "device created successful" << std::endl;
}

void Device::createSwapChain(const SwapChainSupportDetails &swapChainSupport, 
                             const VkSurfaceFormatKHR &surfaceFormat, 
                             const VkPresentModeKHR &presentMode, 
                             const VkExtent2D &extent,
                             const VkSurfaceKHR &surface) {
  uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
  if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
    imageCount = swapChainSupport.capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = surface;
  createInfo.minImageCount = imageCount;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

  // мы должны указать, как изображения будут обрабатываться разными семействами очередей
  // у меня два семейства: 16 очередей и 1. Последнее может только перемещать данные
  // необходимо продумать случай, когда устройств больше чем одно
  // Вроде бы продумал, создав отдельный класс для каждого устройства
  std::vector<uint32_t> indices;
  for (uint32_t i = 0; i < queueFamilies.size(); i++) {
    if (queueFamilies[i].isGraphicsSupport())
      indices.push_back(queueFamilies[i].getQueueFamilyIndex());
  }

  if (indices.size() == 1) {
    // изображение обрабатывается только в одном семействе очередей. Требуется явная передача другому семейству
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0; // Optional
    createInfo.pQueueFamilyIndices = nullptr; // Optional
  } else {
    // изображения могут быть использованы в разных очередях без явной передачи между очередями
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = indices.size();
    createInfo.pQueueFamilyIndices = indices.data();
  }

  createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.presentMode = presentMode;
  createInfo.clipped = VK_TRUE;

  VkSwapchainKHR oldSwapChain = swapchains[0].handle;
  createInfo.oldSwapchain = oldSwapChain;

  if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchains[0].handle) != VK_SUCCESS) {
    Global::console()->print("failed to create swap chain!\n");
    throw std::runtime_error("failed to create swap chain!");
  }

  if(oldSwapChain != VK_NULL_HANDLE) {
    vkDestroySwapchainKHR(device, oldSwapChain, nullptr);
  }

  vkGetSwapchainImagesKHR(device, swapchains[0].handle, &imageCount, nullptr);
  swapchains[0].images.resize(imageCount);
  vkGetSwapchainImagesKHR(device, swapchains[0].handle, &imageCount, swapchains[0].images.data());

  swapchains[0].imageFormat = surfaceFormat.format;
  swapchains[0].extent = extent;
  //std::cout << "swap chain created successful" << std::endl;
}

void Device::createImageViews() {
  // прежде всего мы должны удалить старые image view
  clearImageViews(0);
  swapchains[0].imageViews.resize(swapchains[0].images.size(), VK_NULL_HANDLE);

  for (uint32_t i = 0; i < swapchains[0].images.size(); i++) {
    createImageView(swapchains[0].images[i], swapchains[0].imageFormat, VK_IMAGE_ASPECT_COLOR_BIT, swapchains[0].imageViews[i]);
    //std::cout << "Image View: " << swapChainImageViews[i] << std::endl;
  }
}

void Device::createRenderPass() {
  // я хочу сделать 2 саб пасса, один для изображения игры,
  // второй для менюшки или консоли, которая накладывается на изображение игры
  // Как мне организовать саб пассы? Зависимости между ними?
  // вроде бы я теперь понял: аттачменты ко всему рендер пассу и к зависимостям они не относятся
  // Возможно мне это не нужно
  VkAttachmentDescription colorAttachment1 = {};
  colorAttachment1.format = swapchains[0].imageFormat;
  colorAttachment1.samples = VK_SAMPLE_COUNT_1_BIT;
  colorAttachment1.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment1.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment1.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment1.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment1.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment1.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference colorAttachmentRef1 = {};
  colorAttachmentRef1.attachment = 0;
  colorAttachmentRef1.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentDescription depthAttachment1 = {};
  depthAttachment1.format = findDepthFormat();
  depthAttachment1.samples = VK_SAMPLE_COUNT_1_BIT;
  depthAttachment1.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depthAttachment1.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment1.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  depthAttachment1.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment1.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  depthAttachment1.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentReference depthAttachmentRef1 = {};
  depthAttachmentRef1.attachment = 1;
  depthAttachmentRef1.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subPass1 = {};
  subPass1.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subPass1.colorAttachmentCount = 1;
  subPass1.pColorAttachments = &colorAttachmentRef1;
  subPass1.pDepthStencilAttachment = &depthAttachmentRef1;

//   VkSubpassDescription subPass2 = {};
//   subPass2.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
//   subPass2.colorAttachmentCount = 1;
//   subPass2.pColorAttachments = &colorAttachmentRef1;
//   subPass2.pDepthStencilAttachment = &depthAttachmentRef1;

  std::vector<VkSubpassDependency> dependencies = {
	  {
	    VK_SUBPASS_EXTERNAL,                            // uint32_t                       srcSubpass
	    0,                                              // uint32_t                       dstSubpass
	    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,           // VkPipelineStageFlags           srcStageMask
	    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,  // VkPipelineStageFlags           dstStageMask
	    VK_ACCESS_MEMORY_READ_BIT,                      // VkAccessFlags                  srcAccessMask
	    VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
	    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,           // VkAccessFlags                  dstAccessMask
	    VK_DEPENDENCY_BY_REGION_BIT                     // VkDependencyFlags              dependencyFlags
	  },
      {
	    0,                                              // uint32_t                       srcSubpass
	    VK_SUBPASS_EXTERNAL,                            // uint32_t                       dstSubpass
	    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,  // VkPipelineStageFlags           srcStageMask
	    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,              // VkPipelineStageFlags           dstStageMask
	    VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
	    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,// VkAccessFlags                  srcAccessMask
	    VK_ACCESS_MEMORY_READ_BIT,           // VkAccessFlags                  dstAccessMask
	    VK_DEPENDENCY_BY_REGION_BIT                     // VkDependencyFlags              dependencyFlags
	  },
// 	  {
// 	    1,                                              // uint32_t                       srcSubpass
// 	    VK_SUBPASS_EXTERNAL,                            // uint32_t                       dstSubpass
// 	    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,  // VkPipelineStageFlags           srcStageMask
// 	    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,           // VkPipelineStageFlags           dstStageMask
// 	    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,           // VkAccessFlags                  srcAccessMask
// 	    VK_ACCESS_MEMORY_READ_BIT,                      // VkAccessFlags                  dstAccessMask
// 	    VK_DEPENDENCY_BY_REGION_BIT                     // VkDependencyFlags              dependencyFlags
// 	  }
	};

//   VkSubpassDependency dependency = {};
//   dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
//   dependency.dstSubpass = 0;
//   dependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
//   dependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
//   dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//   dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  std::array<VkAttachmentDescription, 2> attachments = {colorAttachment1, depthAttachment1};
  std::array<VkSubpassDescription, 1> subpasses = {subPass1};
  //std::array<VkAttachmentDescription, 1> attachments = {colorAttachment};
  VkRenderPassCreateInfo renderPassInfo = {};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = attachments.size();
  renderPassInfo.pAttachments = attachments.data();
  renderPassInfo.subpassCount = subpasses.size();
  renderPassInfo.pSubpasses = subpasses.data();
  renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
  renderPassInfo.pDependencies = dependencies.data();

  if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPasses[0]) != VK_SUCCESS) {
    Global::console()->print("failed to create render pass!");
    throw std::runtime_error("failed to create render pass!");
  }
}

// void Device::createDepthSwapChain(const SwapChainSupportDetails &swapChainSupport, 
//                                   const VkSurfaceFormatKHR &surfaceFormat, 
//                                   const VkPresentModeKHR &presentMode, 
//                                   const VkExtent2D &extent,
//                                   const VkSurfaceKHR &surface) {
//   uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
//   if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
//     imageCount = swapChainSupport.capabilities.maxImageCount;
//   }
// 
//   VkSwapchainCreateInfoKHR createInfo = {};
//   createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
//   createInfo.surface = surface;
//   createInfo.minImageCount = imageCount;
//   createInfo.imageFormat = surfaceFormat.format; // VK_FORMAT_D32_SFLOAT
//   createInfo.imageColorSpace = surfaceFormat.colorSpace;
//   createInfo.imageExtent = extent; // здесь должно быть разрешение ниже чем у основного
//   createInfo.imageArrayLayers = 1;
//   createInfo.imageUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
// 
//   // мы должны указать, как изображения будут обрабатываться разными семействами очередей
//   // у меня два семейства: 16 очередей и 1 (на AMD не так! Но при отрисовки у меня есть барьер между семействами).
//   // Последнее может только перемещать данные
//   // необходимо продумать случай, когда устройств больше чем одно
//   // Вроде бы продумал, создав отдельный класс для каждого устройства
//   std::vector<uint32_t> indices;
//   for (uint32_t i = 0; i < deviceQueues.size(); i++) {
//     if (deviceQueues[i]->isGraphicsSupport())
//       indices.push_back(deviceQueues[i]->getQueueFamilyIndex());
//   }
// 
//   if (indices.size() == 1) {
//     // изображение обрабатывается только в одном семействе очередей. Требуется явная передача другому семейству
//     createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
//     createInfo.queueFamilyIndexCount = 0; // Optional
//     createInfo.pQueueFamilyIndices = nullptr; // Optional
//   } else {
//     // изображения могут быть использованы в разных очередях без явной передачи между очередями
//     createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
//     createInfo.queueFamilyIndexCount = indices.size();
//     createInfo.pQueueFamilyIndices = indices.data();
//   }
// 
//   createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
//   createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
//   createInfo.presentMode = presentMode;
//   createInfo.clipped = VK_TRUE;
// 
//   VkSwapchainKHR oldSwapChain = swapchains[1].handle;
//   createInfo.oldSwapchain = oldSwapChain;
// 
//   if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchains[1].handle) != VK_SUCCESS) {
//     Global::console()->print("failed to create swap chain!\n");
//     throw std::runtime_error("failed to create swap chain!");
//   }
// 
//   if(oldSwapChain != VK_NULL_HANDLE) {
//     vkDestroySwapchainKHR(device, oldSwapChain, nullptr);
//   }
// 
//   vkGetSwapchainImagesKHR(device, swapchains[1].handle, &imageCount, nullptr);
//   swapchains[1].images.resize(imageCount);
//   vkGetSwapchainImagesKHR(device, swapchains[1].handle, &imageCount, swapchains[1].images.data());
// 
//   swapchains[1].imageFormat = surfaceFormat.format;
//   swapchains[1].extent = extent;
// }
// 
// void Device::createDepthImageViews() {
//   // прежде всего мы должны удалить старые image view
//   clearImageViews(1);
//   swapchains[1].imageViews.resize(swapchains[1].imageViews.size(), VK_NULL_HANDLE);
// 
//   for (uint32_t i = 0; i < swapchains[1].images.size(); i++) {
//     createImageView(swapchains[1].images[i], swapchains[1].imageFormat, VK_IMAGE_ASPECT_COLOR_BIT, swapchains[1].imageViews[i]);
//   }
// }

void Device::createOcclusionDepthImage() {
  if (occlusionImage.view != VK_NULL_HANDLE) {
    vkDestroyImageView(device, occlusionImage.view, nullptr);
    occlusionImage.view = VK_NULL_HANDLE;
  }
  if (occlusionImage.handle != VK_NULL_HANDLE) {
    vkDestroyImage(device, occlusionImage.handle, nullptr);
    occlusionImage.handle = VK_NULL_HANDLE;
  }
  if (occlusionImage.memory != VK_NULL_HANDLE) {
    vkFreeMemory(device, occlusionImage.memory, nullptr);
    occlusionImage.memory = VK_NULL_HANDLE;
  }

  VkFormat depthFormat = findDepthFormat();

  //std::cout << "Depth resources creation" << std::endl;
  createImage(swapchains[0].extent.width/2, swapchains[0].extent.height/2,
              depthFormat,
              VK_IMAGE_TILING_OPTIMAL,
              VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
              occlusionImage.handle,
              occlusionImage.memory);
  createImageView(occlusionImage.handle, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, occlusionImage.view);

  uint16_t index = getResIndex();
  blockResource(index);
  transitionImageLayout(occlusionImage.handle, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, index);
  unblockResource(index);
  waitResource(index, 1000000000);
}

void Device::createDepthRenderPass() {
  VkAttachmentDescription depthAttachment1 = {};
  depthAttachment1.format = findDepthFormat();
  depthAttachment1.samples = VK_SAMPLE_COUNT_1_BIT;
  depthAttachment1.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depthAttachment1.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment1.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  depthAttachment1.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment1.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  depthAttachment1.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentReference depthAttachmentRef1 = {};
  depthAttachmentRef1.attachment = 0;
  depthAttachmentRef1.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subPass1 = {};
  subPass1.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subPass1.colorAttachmentCount = 0;
  subPass1.pColorAttachments = nullptr;
  subPass1.pDepthStencilAttachment = &depthAttachmentRef1;

  std::vector<VkSubpassDependency> dependencies = {
	  {
	    VK_SUBPASS_EXTERNAL,                            // uint32_t                       srcSubpass
	    0,                                              // uint32_t                       dstSubpass
	    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,           // VkPipelineStageFlags           srcStageMask
	    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,  // VkPipelineStageFlags           dstStageMask
	    VK_ACCESS_MEMORY_READ_BIT,                      // VkAccessFlags                  srcAccessMask
	    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,           // VkAccessFlags                  dstAccessMask
	    VK_DEPENDENCY_BY_REGION_BIT                     // VkDependencyFlags              dependencyFlags
	  },
    {
	    0,                                              // uint32_t                       srcSubpass
	    VK_SUBPASS_EXTERNAL,                            // uint32_t                       dstSubpass
	    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,  // VkPipelineStageFlags           srcStageMask
	    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,  // VkPipelineStageFlags           dstStageMask
	    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,           // VkAccessFlags                  srcAccessMask
	    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,           // VkAccessFlags                  dstAccessMask
	    VK_DEPENDENCY_BY_REGION_BIT                     // VkDependencyFlags              dependencyFlags
	  }
	};

  std::array<VkAttachmentDescription, 1> attachments = {depthAttachment1};
  std::array<VkSubpassDescription, 1> subpasses = {subPass1};
  //std::array<VkAttachmentDescription, 1> attachments = {colorAttachment};
  VkRenderPassCreateInfo renderPassInfo = {};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = attachments.size();
  renderPassInfo.pAttachments = attachments.data();
  renderPassInfo.subpassCount = subpasses.size();
  renderPassInfo.pSubpasses = subpasses.data();
  renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
  renderPassInfo.pDependencies = dependencies.data();

  if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPasses[1]) != VK_SUCCESS) {
    Global::console()->print("failed to create render pass!");
    throw std::runtime_error("failed to create render pass!");
  }
}

void Device::createPipelineLayout(const std::vector<VkPushConstantRange> &ranges, VkPipelineLayout* layout) {
  // возможно все пуш константы нужно отнести к шейдерам
  //VkDescriptorSetLayout setLayouts[] = {descriptorSetLayout};
  //std::cout << "before pipeline layout creation" << std::endl;
  VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  VkDescriptorSetLayout setLayouts[] = {layoutSampler, layoutSampledImage};
  pipelineLayoutInfo.setLayoutCount = 2; // Optional
  pipelineLayoutInfo.pSetLayouts = setLayouts; // Optional
  if (ranges.empty()) {
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional
  } else {
    pipelineLayoutInfo.pushConstantRangeCount = ranges.size(); // Optional
    pipelineLayoutInfo.pPushConstantRanges = ranges.data(); // Optional
  }

  //pipelineLayoutInfo.setLayoutCount = 1;
  //pipelineLayoutInfo.pSetLayouts = setLayouts;

  if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, layout) != VK_SUCCESS) {
    Global::console()->print("failed to create pipeline layout!\n");
    throw std::runtime_error("failed to create pipeline layout!");
  }
}

uint32_t Device::createPipelineLayout(const std::vector<VkPushConstantRange> &ranges, const std::vector<VkDescriptorSetLayout> &descLayouts) {
  VkPipelineLayoutCreateInfo pipelineLayoutInfo;
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.pNext = nullptr;
  pipelineLayoutInfo.flags = 0;
  pipelineLayoutInfo.pushConstantRangeCount = ranges.size();
  pipelineLayoutInfo.pPushConstantRanges = ranges.data();
  pipelineLayoutInfo.setLayoutCount = descLayouts.size();
  pipelineLayoutInfo.pSetLayouts = descLayouts.data();
  
  PipelineLayout l;
  if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &l.handle) != VK_SUCCESS) {
    Global::console()->print("failed to create pipeline layout!\n");
    throw std::runtime_error("failed to create pipeline layout!");
  }
  
  
  layouts.push_back(l);
  return layouts.size()-1;
}

void Device::createGraphicPipelines(const std::vector<PipelineCreationInfo> &pipelineCrationInfos) {
  if (pipelineCrationInfos.empty()) return;

  //std::cout << "entering pipeline creation" << std::endl;
  //VkPipeline tmpPipeline;
  //graphicsPipelines.push_back(tmpPipeline);

  // здесь описываются данные, которые мы хотим передать в шейдеры
  VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
  vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputInfo.vertexAttributeDescriptionCount = pipelineCrationInfos[0].attributeDescriptions.size();
  vertexInputInfo.pVertexAttributeDescriptions    = pipelineCrationInfos[0].attributeDescriptions.data();
  vertexInputInfo.vertexBindingDescriptionCount   = pipelineCrationInfos[0].bindingDescriptions.size();
  vertexInputInfo.pVertexBindingDescriptions      = pipelineCrationInfos[0].bindingDescriptions.data();

  // объясняет как будет строиться геометрия
//   VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
//   inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
//   inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
//   // оптимизация переиспользования вершин (triangle strip, triangle fan)
//   inputAssembly.primitiveRestartEnable = VK_FALSE;

  // вьпорт, обычно обозначает место куда будет рендерится картинка
//   VkViewport viewport = {};
//   viewport.x = 0.0f;
//   viewport.y = 0.0f;
//   // высота и ширина зачастую зависят от ширины и высоты свопчейна
//   viewport.width = (float) swapChainExtent.width;
//   viewport.height = (float) swapChainExtent.height;
//   viewport.minDepth = 0.0f;
//   viewport.maxDepth = 1.0f;
//
//   VkRect2D scissor = {};
//   scissor.offset = {0, 0};
//   scissor.extent = swapChainExtent;

  // возможно создать несколько вью портов, надо глядеть создание логического устройства
  VkPipelineViewportStateCreateInfo viewportState = {};
  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.viewportCount = 1;
  //viewportState.pViewports = &viewport;
  viewportState.scissorCount = 1;
  //viewportState.pScissors = &scissor;

  // разстеризатор берет геометрию вершин и превращает ее в фрагменты для фрагментного шейдера
  VkPipelineRasterizationStateCreateInfo rasterizer = {};
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.depthClampEnable = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
  // толщина линий с точки зрения фрагмента
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = VK_CULL_MODE_NONE;
  rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rasterizer.depthBiasEnable = VK_FALSE;
  rasterizer.depthBiasConstantFactor = 0.0f; // Optional
  rasterizer.depthBiasClamp = 0.0f; // Optional
  rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

  // мультисамплинг
  VkPipelineMultisampleStateCreateInfo multisampling = {};
  multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisampling.minSampleShading = 1.0f; // Optional
  multisampling.pSampleMask = nullptr; // Optional
  multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
  multisampling.alphaToOneEnable = VK_FALSE; // Optional

  // проверяем глубину объектов
  VkPipelineDepthStencilStateCreateInfo depthStencil = {};
  depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depthStencil.depthTestEnable = VK_TRUE;
  depthStencil.depthWriteEnable = VK_TRUE;
  // придерживаемся того, что глубина_близкого_объекта < глубина_далекого_объекта, соотвественно у близких глубина меньше
  depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
  depthStencil.depthBoundsTestEnable = VK_FALSE;
  depthStencil.minDepthBounds = 0.0f; // Optional
  depthStencil.maxDepthBounds = 1.0f; // Optional
  depthStencil.stencilTestEnable = VK_FALSE;
  depthStencil.front = {}; // Optional
  depthStencil.back = {}; // Optional

//   VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
//   colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
//   colorBlendAttachment.blendEnable = VK_TRUE;
//   colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
//   colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
//   colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
//   colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA; // Optional
//   colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
//   colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

  // лучше всего блендинг показывает следующий псевдокод:
  /* *************************************************************************************************************** */
  //  if (blendEnable) {
  //    finalColor.rgb = (srcColorBlendFactor * newColor.rgb) <colorBlendOp> (dstColorBlendFactor * oldColor.rgb);
  //    finalColor.a = (srcAlphaBlendFactor * newColor.a) <alphaBlendOp> (dstAlphaBlendFactor * oldColor.a);
  //  } else {
  //    finalColor = newColor;
  //  }
  //  finalColor = finalColor & colorWriteMask;

  // для полной картины курить спецификацию по VkBlendFactor и VkBlendOp
  /* *************************************************************************************************************** */

  VkPipelineColorBlendStateCreateInfo colorBlending = {};
  colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlending.attachmentCount = pipelineCrationInfos[0].colorBlendings.size();
  colorBlending.pAttachments = pipelineCrationInfos[0].colorBlendings.data();
  colorBlending.logicOpEnable = VK_FALSE;
  colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
  colorBlending.blendConstants[0] = 0.0f; // Optional
  colorBlending.blendConstants[1] = 0.0f; // Optional
  colorBlending.blendConstants[2] = 0.0f; // Optional
  colorBlending.blendConstants[3] = 0.0f; // Optional

  // динамические части в пайплайне (их нужно создавать непосредственно перед отрисовкой)
  // их там на самом деле дохрена (неплохо было бы узнать что это за штуки)
  std::vector<VkDynamicState> dynamicStates = {
	  VK_DYNAMIC_STATE_VIEWPORT,
	  VK_DYNAMIC_STATE_SCISSOR,
	};

  VkPipelineDynamicStateCreateInfo dynamicState = {};
  dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicState.dynamicStateCount = dynamicStates.size();
  dynamicState.pDynamicStates = dynamicStates.data();

  //std::cout << "Error before pipeline creation" << std::endl;
  // переходим непосредственно к пайплайну
  VkGraphicsPipelineCreateInfo pipelineInfo = {};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = pipelineCrationInfos[0].shaders.size();
  pipelineInfo.pStages = pipelineCrationInfos[0].shaders.data();
  pipelineInfo.pVertexInputState = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &pipelineCrationInfos[0].inputAssembly;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pDepthStencilState = &depthStencil;
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.pDynamicState = &dynamicState; // Optional
  pipelineInfo.layout = layouts[pipelineCrationInfos[0].layoutIndex].handle;
  pipelineInfo.renderPass = renderPasses[0];
  pipelineInfo.subpass = 0;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
  pipelineInfo.basePipelineIndex = -1; // Optional

  for (uint32_t i = 0; i < pipelineCrationInfos.size(); i++) {
    //std::cout << "pipeline index: " << i << std::endl;
    Pipeline tmpPipeline = {};
    //VkPipeline tmpPipeline = VK_NULL_HANDLE;
    graphicsPipelines.push_back(tmpPipeline);

    vertexInputInfo.vertexAttributeDescriptionCount = pipelineCrationInfos[i].attributeDescriptions.size();
    vertexInputInfo.pVertexAttributeDescriptions    = pipelineCrationInfos[i].attributeDescriptions.data();
    vertexInputInfo.vertexBindingDescriptionCount   = pipelineCrationInfos[i].bindingDescriptions.size();
    vertexInputInfo.pVertexBindingDescriptions      = pipelineCrationInfos[i].bindingDescriptions.data();

    colorBlending.attachmentCount = pipelineCrationInfos[i].colorBlendings.size();
    colorBlending.pAttachments = pipelineCrationInfos[i].colorBlendings.data();
    rasterizer.polygonMode = pipelineCrationInfos[i].mode;

    depthStencil.depthTestEnable = pipelineCrationInfos[i].depthTest;
    depthStencil.depthWriteEnable = pipelineCrationInfos[i].depthWrite;

    //pipelineInfo.pDepthStencilState  = &depthStencil;
    pipelineInfo.stageCount          =  pipelineCrationInfos[i].shaders.size();
    pipelineInfo.pStages             =  pipelineCrationInfos[i].shaders.data();
    pipelineInfo.pInputAssemblyState = &pipelineCrationInfos[i].inputAssembly;
    pipelineInfo.layout              =  layouts[pipelineCrationInfos[i].layoutIndex].handle;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.renderPass = renderPasses[pipelineCrationInfos[i].renderPassIndex];

    //std::cout << "Pipeline layout " << i << ": " << *pipelineCrationInfos[i].layout << std::endl;

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipelines.back().handle) != VK_SUCCESS) {
      Global::console()->print("failed to create graphics pipeline!\n");
      throw std::runtime_error("failed to create graphics pipeline!");
    }

    graphicsPipelines.back().layoutIndex = pipelineCrationInfos[i].layoutIndex;

    for (uint32_t j = 0; j < pipelineCrationInfos[i].shaders.size(); j++) {
      vkDestroyShaderModule(device, pipelineCrationInfos[i].shaders[j].module, nullptr);
    }
  }
}

int Device::createCommandPool(const VkQueueFlags &flags, const bool &present, const VkCommandPoolCreateFlags &commandPoolFlags) {
  //VDeleter<VkCommandPool> commandPool{device, vkDestroyCommandPool};
  VkCommandPool commandPool;
  commandPools.push_back(commandPool);
  // последний параметр строгая проверка того, может ли семейство очередей выводить на экран
  int index = pickQueueFamilies(flags, present);
  // устройства могу поддерживать далеко не весь спектр очередей
  if (index < 0) {
    //std::cout << "Все очень плохо!" << std::endl;
    Global::console()->print("Все очень плохо!\n");
    return index;
  }

  VkCommandPoolCreateInfo commandPoolInfo = {};
  commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  commandPoolInfo.queueFamilyIndex = index;
  commandPoolInfo.flags = commandPoolFlags; // Optional

  if (vkCreateCommandPool(device, &commandPoolInfo, nullptr, &commandPools.back()) != VK_SUCCESS) {
    Global::console()->print("failed to create command pool!\n");
    throw std::runtime_error("failed to create command pool!");
  }

  //std::cout << "command pool creating successful" << std::endl;
  return commandPools.size()-1;
}

void Device::createVirtualFrames(int graphicsCommandPoolIndex) {
  virtualFrames.resize(framesCount);
  commandPoolIndex = graphicsCommandPoolIndex;
  for (size_t i = 0; i < virtualFrames.size(); i++) {
    // фреймбуферы будут создаваться позже
    // каким то образом здесь нужно принимать графический командный пул
    // хотя графическим его делает только то, какие буферы из него будут получаться
    // поэтому наверное передать нужный через параметр более менее нормально (?)
    allocateCommandBuffers(commandPools[graphicsCommandPoolIndex], VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1, &virtualFrames[i].commandBuffer);
    createSemaphores(virtualFrames[i].imageAvailableSemaphore);
    createSemaphores(virtualFrames[i].finishedRenderingSemaphore);
    createFence(virtualFrames[i].fence);
  }
}

void Device::createDesriptorSet() {
  // юниформ байндинг для МВП матриц
//   VkDescriptorSetLayoutBinding uboLayoutBinding = {};
//   uboLayoutBinding.binding = 0;
//   uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//   uboLayoutBinding.descriptorCount = 1;
//   uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
//   // используется для сэмплинга картинок
//   uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

  // возможно доступ к МВП матрицам через дескриптор куда быстрее
  // нужно проверить

  // сэмплер
  VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
  samplerLayoutBinding.binding = 0;
  samplerLayoutBinding.descriptorCount = 1;
  samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
  samplerLayoutBinding.pImmutableSamplers = nullptr;
  samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

//   std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};
  std::array<VkDescriptorSetLayoutBinding, 1> bindings = {samplerLayoutBinding};
  VkDescriptorSetLayoutCreateInfo layoutInfo = {};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = bindings.size();
  layoutInfo.pBindings = bindings.data();

  if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &layoutSampler) != VK_SUCCESS) {
    Global::console()->print("failed to create descriptor set layout!\n");
    throw std::runtime_error("failed to create descriptor set layout!");
  }

  // текстурка
  VkDescriptorSetLayoutBinding sampledImageLayoutBinding = {};
  sampledImageLayoutBinding.binding = 0;
  sampledImageLayoutBinding.descriptorCount = 1;
  sampledImageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
  sampledImageLayoutBinding.pImmutableSamplers = nullptr;
  sampledImageLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  std::array<VkDescriptorSetLayoutBinding, 1> bindings2 = {sampledImageLayoutBinding};
  VkDescriptorSetLayoutCreateInfo layoutInfo2 = {};
  layoutInfo2.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo2.bindingCount = bindings2.size();
  layoutInfo2.pBindings = bindings2.data();

  if (vkCreateDescriptorSetLayout(device, &layoutInfo2, nullptr, &layoutSampledImage) != VK_SUCCESS) {
    Global::console()->print("failed to create descriptor set layout!");
    throw std::runtime_error("failed to create descriptor set layout!");
  }

  // текстурка и сэмплер
  VkDescriptorSetLayoutBinding combinedImageSamplerLayoutBinding = {};
  combinedImageSamplerLayoutBinding.binding = 0;
  combinedImageSamplerLayoutBinding.descriptorCount = 1;
  combinedImageSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  combinedImageSamplerLayoutBinding.pImmutableSamplers = nullptr;
  combinedImageSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  std::array<VkDescriptorSetLayoutBinding, 1> bindings3 = {combinedImageSamplerLayoutBinding};
  VkDescriptorSetLayoutCreateInfo layoutInfo3 = {};
  layoutInfo3.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo3.bindingCount = bindings3.size();
  layoutInfo3.pBindings = bindings3.data();

  if (vkCreateDescriptorSetLayout(device, &layoutInfo3, nullptr, &layoutCombinedImageSampler) != VK_SUCCESS) {
    Global::console()->print("failed to create descriptor set layout!");
    throw std::runtime_error("failed to create descriptor set layout!");
  }
  
  VkDescriptorSetLayoutBinding uboLayoutBinding;
  uboLayoutBinding.binding = 0;
  uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  uboLayoutBinding.descriptorCount = 1;
  uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  uboLayoutBinding.pImmutableSamplers = nullptr;
  
  //VkDescriptorSetLayoutCreateInfo layoutInfo;
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.pNext = nullptr;
  layoutInfo.flags = 0;
  layoutInfo.bindingCount = 1;
  layoutInfo.pBindings = &uboLayoutBinding;
  if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &uboSetLayout) != VK_SUCCESS) {
    Global::console()->print("failed to create descriptor set layout!");
    throw std::runtime_error("failed to create descriptor set layout!");
  }
  
  VkDescriptorSetLayoutBinding storageLayoutBinding;
  storageLayoutBinding.binding = 0;
  storageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  storageLayoutBinding.descriptorCount = 1;
  storageLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  storageLayoutBinding.pImmutableSamplers = nullptr;
  
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.pNext = nullptr;
  layoutInfo.flags = 0;
  layoutInfo.bindingCount = 1;
  layoutInfo.pBindings = &storageLayoutBinding;
  if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &storageSetLayout) != VK_SUCCESS) {
    Global::console()->print("failed to create descriptor set layout!");
    throw std::runtime_error("failed to create descriptor set layout!");
  }
  
//   VkDescriptorSetLayoutBinding bindingsOccl[] = {uboLayoutBinding, storageLayoutBinding};
//   
//   layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
//   layoutInfo.pNext = nullptr;
//   layoutInfo.flags = 0;
//   layoutInfo.bindingCount = 2;
//   layoutInfo.pBindings = bindingsOccl;
//   if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &storageSetLayout) != VK_SUCCESS) {
//     Global::console()->print("failed to create descriptor set layout!");
//     throw std::runtime_error("failed to create descriptor set layout!");
//   }

  std::array<VkDescriptorPoolSize, 5> poolSizes = {};
  poolSizes[0].type = VK_DESCRIPTOR_TYPE_SAMPLER;
  poolSizes[0].descriptorCount = 2;
  poolSizes[1].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
  poolSizes[1].descriptorCount = MAX_TEXTURES;
  poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  poolSizes[2].descriptorCount = 3;
  poolSizes[3].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  poolSizes[3].descriptorCount = 3;
  poolSizes[4].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  poolSizes[4].descriptorCount = 1;

  VkDescriptorPoolCreateInfo poolInfo = {};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.pNext = nullptr;
  poolInfo.flags = 0;
  poolInfo.maxSets = MAX_TEXTURES + 2 + 3 + 3;
  poolInfo.poolSizeCount = poolSizes.size();
  poolInfo.pPoolSizes = poolSizes.data();

  if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
    Global::console()->print("failed to create descriptor pool!\n");
    throw std::runtime_error("failed to create descriptor pool!");
  }
  
//   std::array<VkDescriptorPoolSize, 1> storageSizes = {};
//   poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
//   poolSizes[0].descriptorCount = 1;
// 
//   poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
//   poolInfo.pNext = nullptr;
//   poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
//   poolInfo.maxSets = 1;
//   poolInfo.poolSizeCount = storageSizes.size();
//   poolInfo.pPoolSizes = storageSizes.data();
// 
//   if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &storagePool) != VK_SUCCESS) {
//     Global::console()->print("failed to create descriptor pool!\n");
//     throw std::runtime_error("failed to create descriptor pool!");
//   }
}

void Device::createMemory(const VkDeviceSize &size) {
  uint32_t memoryTypeBits = showBufferMemoryTypes();
  uint32_t imageMemoryTypeBits = showImageMemoryTypes();
  //exit(-1);

  Global::console()->printf("Using buffer memory type %d\n", memoryTypeBits);
  Global::console()->printf("Using image  memory type %d\n", imageMemoryTypeBits);

//   hostVisible = new Memory(this);
//   hostVisible->allocateMemory(MIN_MEMORY_SIZE, memoryType, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT);
//   deviceLocal = new Memory(this);
//   deviceLocal->allocateMemory(size, memoryType, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  int32_t memoryType = findMemoryType(memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                      VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                                                      VK_MEMORY_PROPERTY_HOST_CACHED_BIT);
  if (memoryType == -1) {
    memoryType = findMemoryType(memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  }

  if (memoryType == -1) {
      std::runtime_error("Suitable memory type didnt found!");
  }

  deviceMemory.init(this);
  deviceMemory.allocateChunk(MIN_MEMORY_SIZE, memoryType, CHUNK_TYPE_BUFFER);

  memoryType = findMemoryType(memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  if (memoryType == -1) {
      std::runtime_error("Suitable memory type didnt found!");
  }

  deviceMemory.allocateChunk(size, memoryType, CHUNK_TYPE_BUFFER);

  int32_t imageMemoryType = findMemoryType(imageMemoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  if (imageMemoryType == -1) {
      std::runtime_error("Suitable memory type didnt found!");
  }

  deviceMemory.allocateChunk(MIN_IMAGE_MEMORY_SIZE, imageMemoryType, CHUNK_TYPE_IMAGE);

  std::cout << "Memory created" << std::endl;
  //deviceLocal->setHostVisibleMemoryPtr(hostVisible);
}

uint16_t Device::getResIndex() {
  size_t index = 0;
  while ((vkWaitForFences(device, 1, &res[index].fence, VK_FALSE, 10000) != VK_SUCCESS) &&
         (res[index].free)) {
    index = (index + 1) % resCount;
    //std::cout << "Index fence: " << index << std::endl;
  }

  clearResource(index);

  return index;
}

uint16_t Device::getResIndex(ImageParameters param) {
  size_t index = 0;
  while ((vkWaitForFences(device, 1, &res[index].fence, VK_FALSE, 10000) != VK_SUCCESS) &&
         (res[index].free)) {
    index = (index + 1) % resCount;
    //std::cout << "Index fence: " << index << std::endl;
  }

  clearResource(index);
  res[index].temporaryImageObject = param;

  return index;
}

uint16_t Device::getResIndex(BufferParameters param) {
  size_t index = 0;
  //std::cout << "res count: " << res.size() << std::endl;
  while ((vkWaitForFences(device, 1, &res[index].fence, VK_FALSE, 10000) != VK_SUCCESS) &&
         (res[index].free)) {
    index = (index + 1) % resCount;
    //std::cout << "Index fence: " << index << std::endl;
  }

  clearResource(index);
  res[index].temporaryBufferObject = param;

  return index;
}

VkResult Device::waitForRes(uint16_t index) {
  return vkWaitForFences(device, 1, &res[index].fence, VK_FALSE, 1000000000);
}

void Device::blockResource(uint16_t index) {
  res[index].free = false;
  beginSingleTimeCommands(index);
}

void Device::unblockResource(uint16_t index) {
  endSingleTimeCommands(index);
  res[index].free = true;
}

void Device::waitResource(uint16_t index, uint64_t nanoSec) {
  vkWaitForFences(device, 1, &res[index].fence, VK_FALSE, nanoSec);
}

// операции с буферами
// uint32_t Device::createBuffer(VkDeviceSize size,
//                               VkBufferUsageFlags usage,
//                               VkMemoryPropertyFlags properties,
//                               std::vector<VkDeviceSize> offsets,
//                               uint32_t indicesSize) {
//   uint32_t memoryIndex;
//   bool newObjectNeeded = true;
//   // если у нас имеется не размещенный в памяти объект memoryObject, то берем его
//
//   for (uint32_t i = 0; i < bufferObjects.size(); i++) {
//     if (!bufferObjects[i].allocated) {
//       if (deviceLocal->compareProperties(properties)) {
//         bufferObjects[i].memPtr = deviceLocal;
//         //std::cout << "Using device local" << std::endl;
//       } else {
//         bufferObjects[i].memPtr = hostVisible;
//         //std::cout << "Using host visible" << std::endl;
//       }
//
//       bufferObjects[i].parameters = bufferObjects[i].memPtr->createBuffer(size, usage);
//       bufferObjects[i].indicesSize = indicesSize;
//       bufferObjects[i].offsets.swap(offsets);
//       memoryIndex = i;
//       newObjectNeeded = false;
//     }
//   }
//
//   if (newObjectNeeded) {
//     BufferObject tmpObject;
//     tmpObject.allocated = true;
//     tmpObject.indicesSize = indicesSize;
//     bufferObjects.push_back(tmpObject);
//     memoryIndex = bufferObjects.size()-1;
//
//     if (deviceLocal->compareProperties(properties)) {
//       bufferObjects[memoryIndex].memPtr = deviceLocal;
//       //std::cout << "Using device local" << std::endl;
//     } else {
//       bufferObjects[memoryIndex].memPtr = hostVisible;
//       //std::cout << "Using host visible" << std::endl;
//     }
//
//     bufferObjects[memoryIndex].parameters = bufferObjects[memoryIndex].memPtr->createBuffer(size, usage);
//     bufferObjects[memoryIndex].offsets.swap(offsets);
//   }
//
//   // возвращаем индекс буфера, в который только что записали данные
//   return memoryIndex;
// }

/* -------------------------------------------------------------------------------------- */
// for (uint32_t i = 0; i < bufferObjects.size(); i++) {
//     if (!bufferObjects[i].allocated) {
//       Memory* memory;
//       for (size_t j = 0; j < deviceMemory.size(); j++) {
//         if (deviceMemory[j].compareProperties(properties)) {
//           memory = &deviceMemory[j];
//           break;
//         }
//       }
//       uint32_t buffer = memory->createBuffer(size, usage);
//       bufferObjects[i].parameters = memory->getBuffer(buffer);
//       bufferObjects[i].bufferIndex = buffer;
//       memoryIndex = i;
//       newObjectNeeded = false;
//     }
//   }
//
//   if (newObjectNeeded) {
//     Memory* memory;
//     for (size_t j = 0; j < deviceMemory.size(); j++) {
//       if (deviceMemory[j].compareProperties(properties)) {
//         memory = &deviceMemory[j];
//         break;
//       }
//     }
//
//     BufferObject tmpObject;
//     tmpObject.allocated = true;
//     tmpObject.indicesSize = indicesSize;
//     bufferObjects.push_back(tmpObject);
//     memoryIndex = bufferObjects.size()-1;
//
//     uint32_t buffer = memory->createBuffer(size, usage);
//     bufferObjects[memoryIndex].parameters = memory->getBuffer(buffer);
//     bufferObjects[memoryIndex].bufferIndex = buffer;
//     bufferObjects[memoryIndex].offsets.swap(offsets);
//   }
/* -------------------------------------------------------------------------------------- */

// uint32_t Device::createBuffer(VkDeviceSize size,
//                               VkBufferUsageFlags usage,
//                               VkMemoryPropertyFlags properties,
//                               std::vector<VkDeviceSize> offsets) {
//   return createBuffer(size, usage, properties, offsets, 0);
// }

Buffer Device::createBuffer(const VkDeviceSize &size,
                            const VkBufferUsageFlags &usage,
                            const VkMemoryPropertyFlags &properties,
                            const uint32_t &dataCount) {
  VkBuffer tmp;
  VkBufferCreateInfo buffInfo = {};
  buffInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buffInfo.pNext = nullptr;
  buffInfo.flags = 0;
  buffInfo.size = size;
  buffInfo.usage = usage;
  buffInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  buffInfo.queueFamilyIndexCount = 0;
  buffInfo.pQueueFamilyIndices = nullptr;
  if (vkCreateBuffer(device, &buffInfo, nullptr, &tmp) != VK_SUCCESS) {
    Global::console()->print("failed to create temporary buffer!\n");
    throw std::runtime_error("failed to create temporary buffer!");
  }

  VkMemoryRequirements req;
  vkGetBufferMemoryRequirements(device, tmp, &req);

  vkDestroyBuffer(device, tmp, nullptr);

  int32_t memoryType = findMemoryType(req.memoryTypeBits, properties);
  if (memoryType == -1) {
    Global::console()->print("Cannot find memory type for this properties!");
    throw std::runtime_error("Cannot find memory type for this properties!");
  }

  BufferAllocateInfo info = {};
  info.dataCount = dataCount;
  info.memoryType = memoryType;
  info.size = (req.size > size)?req.size:size;

  Buffer b;
  if (!deviceMemory.allocateBuffer(info, b)) {
    Global::console()->print("Allocation failed!");
    throw std::runtime_error("Allocation failed!");
  }

  return b;
}

void Device::createStagingBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, uint16_t resIndex) {
  createStagingBuffer(size, usage, properties, res[resIndex].temporaryBufferObject.handle, res[resIndex].temporaryBufferObject.memory);
}

void Device::createStagingBuffer(VkDeviceSize size,
                                 VkBufferUsageFlags usage,
                                 VkMemoryPropertyFlags properties,
                                 VkBuffer& buffer,
                                 VkDeviceMemory& bufferMemory) {
  VkBufferCreateInfo bufferInfo = {};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = size;
  bufferInfo.usage = usage;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
    Global::console()->print("failed to create buffer!\n");
    throw std::runtime_error("failed to create buffer!");
  }

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

  VkMemoryAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

  if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
    Global::console()->print("failed to allocate buffer memory!\n");
    throw std::runtime_error("failed to allocate buffer memory!");
  }

  vkBindBufferMemory(device, buffer, bufferMemory, 0);
}


void Device::copyBuffer(const VkBuffer &srcBuffer, const VkBuffer &dstBuffer, const VkDeviceSize &size, const uint16_t &resIndex) {
    //beginSingleTimeCommands(resIndex);

    // этап копирования. Необходимо точно указать размеры копируемой области (нельзя указать VK_WHOLE_SIZE)
    VkBufferCopy copyRegion = {};
    copyRegion.size = size;
    vkCmdCopyBuffer(res[resIndex].commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    //endSingleTimeCommands(resIndex);
}

// void Device::copyBuffer(const uint16_t &resIndex, const uint32_t &dstBuffer, const VkDeviceSize &size) {
//     //beginSingleTimeCommands(resIndex);
//
//     // этап копирования. Необходимо точно указать размеры копируемой области (нельзя указать VK_WHOLE_SIZE)
//     VkBufferCopy copyRegion = {};
//     copyRegion.size = size;
//     //Object* obj = bufferObjects[dstBuffer].memPtr->getObject(bufferObjects[dstBuffer].bufferIndex);
//     vkCmdCopyBuffer(res[resIndex].commandBuffer,
//                     res[resIndex].temporaryBufferObject.handle,
//                     bufferObjects[dstBuffer].parameters->buffer,
//                     //bufferObjects[dstBuffer].parameters->buffer,
//                     1,
//                     &copyRegion);
//
//     //endSingleTimeCommands(resIndex);
// }

void Device::copyBuffer(const uint16_t &resIndex, const Buffer &dstBuffer, const VkDeviceSize &size) {
  VkBufferCopy copyRegion = {};
  copyRegion.srcOffset = 0;
  copyRegion.dstOffset = dstBuffer->offset;
  copyRegion.size = size;

  vkCmdCopyBuffer(res[resIndex].commandBuffer,
                  res[resIndex].temporaryBufferObject.handle,
                  dstBuffer->param.handle,
                  1,
                  &copyRegion);
}

void Device::copyQueryPoolResults(const uint16_t &resIndex,
                                  const uint32_t &firstQuery, 
                                  const uint32_t &queryCount, 
                                  const Buffer &dstBuffer, 
                                  const VkDeviceSize &dstOffset, 
                                  const VkDeviceSize &stride, 
                                  const VkQueryResultFlags &flags) {
  vkCmdCopyQueryPoolResults(res[resIndex].commandBuffer, queryPool, firstQuery, queryCount, dstBuffer->param.handle, dstOffset, stride, flags);
}

void Device::createImage(uint32_t width, uint32_t height,
                         VkFormat format,
                         VkImageTiling tiling,
                         VkImageUsageFlags usage,
                         VkMemoryPropertyFlags properties,
                         uint16_t resIndex) {
  createImage(width, height, format, tiling, usage, properties, res[resIndex].temporaryImageObject.handle, res[resIndex].temporaryImageObject.memory);
}

// операции с картинками
void Device::createImage(uint32_t width, uint32_t height,
                         VkFormat format,
                         VkImageTiling tiling,
                         VkImageUsageFlags usage,
                         VkMemoryPropertyFlags properties,
                         VkImage& stagingImage,
                         VkDeviceMemory& stagingImageMemory) {
  VkExtent3D extent = {};
  extent.width = width;
  extent.height = height;
  extent.depth = 1;

  VkImageCreateInfo imageInfo = {};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.extent = extent;
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = 1;
  // распространенный формат, так что оставим как есть
  imageInfo.format = format;
  imageInfo.tiling = tiling;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
  imageInfo.usage = usage; //VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.flags = 0; // Optional

  if (vkCreateImage(device, &imageInfo, nullptr, &stagingImage) != VK_SUCCESS) {
    Global::console()->print("failed to create image!\n");
    throw std::runtime_error("failed to create image!");
  }

  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(device, stagingImage, &memRequirements);

  VkMemoryAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

  if (vkAllocateMemory(device, &allocInfo, nullptr, &stagingImageMemory) != VK_SUCCESS) {
    Global::console()->print("failed to allocate image memory!\n");
    throw std::runtime_error("failed to allocate image memory!");
  }

  if (vkBindImageMemory(device, stagingImage, stagingImageMemory, 0) != VK_SUCCESS) throw std::runtime_error("Bind image mem error");
}


// uint32_t Device::createImage(uint32_t width,
//                              uint32_t height,
//                              VkFormat format,
//                              VkImageTiling tiling,
//                              VkImageUsageFlags usage,
//                              VkMemoryPropertyFlags properties,
//                              VkDeviceSize imageSize) {
//   uint32_t imageIndex;
//   bool newObjectNeeded = true;
//   // если у нас имеется не размещенный в памяти объект memoryObject, то берем его
//   for (uint32_t i = 0; i < imageObjects.size(); i++) {
//     if (!imageObjects[i].allocated) {
//       imageIndex = i;
//       newObjectNeeded = false;
//     }
//   }
//
//   if (newObjectNeeded) {
//     ImageObject tmpObject;
//     tmpObject.allocated = false;
//     tmpObject.parameters.handle  = VK_NULL_HANDLE;
//     tmpObject.parameters.memory  = VK_NULL_HANDLE;
//     tmpObject.parameters.view    = VK_NULL_HANDLE;
//     imageObjects.push_back(tmpObject);
//     imageIndex = imageObjects.size()-1;
//   }
//
//   VkImageCreateInfo imageInfo = {};
//   imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
//   imageInfo.imageType = VK_IMAGE_TYPE_2D;
//   imageInfo.extent.width = width;
//   imageInfo.extent.height = height;
//   imageInfo.extent.depth = 1;
//   imageInfo.mipLevels = 1;
//   imageInfo.arrayLayers = 1;
//   // распространенный формат, так что оставим как есть
//   imageInfo.format = format;
//   imageInfo.tiling = tiling;
//   imageInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
//   imageInfo.usage = usage; //VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
//   imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
//   imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
//   imageInfo.flags = 0; // Optional
//
//   if (vkCreateImage(device, &imageInfo, nullptr, &imageObjects[imageIndex].parameters.handle) != VK_SUCCESS) {
//     throw std::runtime_error("failed to create image!");
//   }
//
//   VkMemoryRequirements memRequirements;
//   vkGetImageMemoryRequirements(device, imageObjects[imageIndex].parameters.handle, &memRequirements);
//
//   //std::cout << "Image size     : " << memRequirements.size << std::endl;
//   //std::cout << "Image flag bits: " << memRequirements.memoryTypeBits << std::endl;
//   Global::console()->printf("Image size     : %d\n", memRequirements.size);
//   Global::console()->printf("Image flag bits: %d\n", memRequirements.memoryTypeBits);
//
//   VkMemoryAllocateInfo allocInfo = {};
//   allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
//   allocInfo.allocationSize = memRequirements.size;
//   allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);
//
//   if (vkAllocateMemory(device, &allocInfo, nullptr, &imageObjects[imageIndex].parameters.memory) != VK_SUCCESS) {
//     Global::console()->printString("failed to allocate image memory!");
//     throw std::runtime_error("failed to allocate image memory!");
//   }
//
//   vkBindImageMemory(device, imageObjects[imageIndex].parameters.handle, imageObjects[imageIndex].parameters.memory, 0);
//   imageObjects[imageIndex].size = imageSize;
//   imageObjects[imageIndex].sizeWHD = imageInfo.extent;
//   imageObjects[imageIndex].allocated = true;
//
//   return imageIndex;
// }

Image Device::createImage(const uint32_t &width,
                          const uint32_t &height,
                          const VkFormat &format,
                          const VkImageTiling &tiling,
                          const VkImageUsageFlags &usage,
                          const VkMemoryPropertyFlags &properties,
                          const VkDeviceSize &imageSize) {
  VkImage tmp;
  VkImageCreateInfo imageInfo = {};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width = width;
  imageInfo.extent.height = height;
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = 1;
  // распространенный формат, так что оставим как есть
  imageInfo.format = format;
  imageInfo.tiling = tiling;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
  imageInfo.usage = usage; //VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.flags = 0; // Optional

  if (vkCreateImage(device, &imageInfo, nullptr, &tmp) != VK_SUCCESS) {
    Global::console()->print("failed to create image!");
    throw std::runtime_error("failed to create image!");
  }

  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(device, tmp, &memRequirements);

//   std::cout << "Image size     : " << memRequirements.size << std::endl;
//   std::cout << "Image flag bits: " << memRequirements.memoryTypeBits << std::endl;

  vkDestroyImage(device, tmp, nullptr);

  int32_t memoryType = findMemoryType(memRequirements.memoryTypeBits, properties);
  if (memoryType == -1) {
    Global::console()->print("Cannot find memory type for this properties!");
    throw std::runtime_error("Cannot find memory type for this properties!");
  }

  ImageAllocateInfo info = {};
  info.size = (memRequirements.size > imageSize)? memRequirements.size : imageSize;
  info.width = width;
  info.height = height;
  info.depth = 1;
  info.format = format;
  info.tiling = tiling;
  info.usage = usage;
  info.memoryType = memoryType;

  Image b;
  if (!deviceMemory.allocateImage(info, b)) {
    Global::console()->print("Allocation failed!");
    throw std::runtime_error("Allocation failed!");
  }

  return b;
}

void Device::transitionImageLayout(const VkImage &image, const VkImageLayout &oldLayout, const VkImageLayout &newLayout, const uint16_t &resIndex) {
  //beginSingleTimeCommands(resIndex);
  
  // теперь (25.10.2017) походу необходимо указывать точно Stage Flags 
  VkPipelineStageFlags srcStageMask;
  VkPipelineStageFlags dstStageMask;
   
  // устанавливаем барьер для изменения лайаута
  VkImageMemoryBarrier barrier = {};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = oldLayout;
  barrier.newLayout = newLayout;

  // если мы хотим передать права другой очереди, то здесь должны быть индексы
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

  barrier.image = image;
  if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
  } else {
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  }
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;

  // указываем необходимые операции до и после барьера
  if (oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
    // если лайаут меняется из ДЕФОЛТНОГО в ИСТОЧНИК ДЛЯ ПЕРЕМЕЩЕНИЯ, то нужно подождать пока хост закончит запись
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    
    srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
  } else if (oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    // если ДЕФОЛТ -> ПРИЕМНИК ПЕРЕМЕЩЕНИЯ, тоже что и предыдущее
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    
    srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
  } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    // если ПРИЕМНИК -> ТОЛЬКО ДЛЯ ЧТЕНИЯ, то необходимо подождать пока перемещение совершится
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    
    srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
    dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; // тут поаккуратнее
  } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    
    srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT; // возможно
    dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  } else if (oldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    
    srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT; // возможно
    dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
  } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    
    srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT; // возможно
    dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    
    srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
    dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
  } else if (oldLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    
    srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
  } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    
    srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
    dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_GENERAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
    
    srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
    dstStageMask = VK_PIPELINE_STAGE_HOST_BIT;
  } else {
    Global::console()->print("unsupported layout transition!\n");
    throw std::invalid_argument("unsupported layout transition!");
  }
  
//   if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
//     barrier.srcAccessMask = 0;
//     barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
// 
//     srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
//     dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
//   } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
//     barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
//     barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
// 
//     srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
//     dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
//   } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
//     barrier.srcAccessMask = 0;
//     barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
// 
//     srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
//     dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
//   } else {
//     Global::console()->print("unsupported layout transition!\n");
//     throw std::invalid_argument("unsupported layout transition!");
//   }

  vkCmdPipelineBarrier(
    res[resIndex].commandBuffer,
    //VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
    srcStageMask, dstStageMask,
    0,
    0, nullptr,
    0, nullptr,
    1, &barrier
  );

  //endSingleTimeCommands(resIndex);
}

void Device::transitionImageLayout(const uint16_t &resIndex, const VkImageLayout &oldLayout, const VkImageLayout &newLayout) {
  transitionImageLayout(res[resIndex].temporaryImageObject.handle, oldLayout, newLayout, resIndex);
}

// void Device::transitionImageLayout(uint32_t imageIndex, VkImageLayout oldLayout, VkImageLayout newLayout, uint16_t resIndex) {
//   transitionImageLayout(imageObjects[imageIndex].parameters.handle, oldLayout, newLayout, resIndex);
// }

void Device::transitionImageLayout(const Image &image, const VkImageLayout &oldLayout, const VkImageLayout &newLayout, const uint16_t &resIndex) {
  transitionImageLayout(image->param.handle, oldLayout, newLayout, resIndex);
}

void Device::copyImage(const VkImage &srcImage, const VkImage &dstImage, const uint32_t &width, const uint32_t &height, const uint16_t &resIndex) {
  //beginSingleTimeCommands(resIndex);

  VkImageSubresourceLayers subResource = {};
  subResource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  subResource.baseArrayLayer = 0;
  subResource.mipLevel = 0;
  subResource.layerCount = 1;

  VkImageCopy region = {};
  region.srcSubresource = subResource;
  region.dstSubresource = subResource;
  region.srcOffset = {0, 0, 0};
  region.dstOffset = {0, 0, 0};
  region.extent.width = width;
  region.extent.height = height;
  region.extent.depth = 1;

  vkCmdCopyImage(
    res[resIndex].commandBuffer,
    srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
    dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    1, &region
  );

  //endSingleTimeCommands(resIndex);
}

void Device::copyImage(const uint16_t &resIndex, const Image &dstImage, const uint32_t &width, const uint32_t &height) {
  //beginSingleTimeCommands(resIndex);

  VkImageSubresourceLayers subResource = {};
  subResource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  subResource.baseArrayLayer = 0;
  subResource.mipLevel = 0;
  subResource.layerCount = 1;

  VkImageCopy region = {};
  region.srcSubresource = subResource;
  region.dstSubresource = subResource;
  region.srcOffset = {0, 0, 0};
  region.dstOffset = {0, 0, 0};
  region.extent.width = width;
  region.extent.height = height;
  region.extent.depth = 1;

  vkCmdCopyImage(
    res[resIndex].commandBuffer,
    res[resIndex].temporaryImageObject.handle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
    dstImage->param.handle,                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    1, &region
  );

  //endSingleTimeCommands(resIndex);
}

void Device::copyImageToBuffer(const VkImage &srcImage, const VkBuffer &dstBuffer, const uint16_t &resIndex) {
  //beginSingleTimeCommands(resIndex);

  VkImageSubresourceLayers subResource = {};
  subResource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  subResource.baseArrayLayer = 0;
  subResource.mipLevel = 0;
  subResource.layerCount = 1;

  VkBufferImageCopy region = {};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;
  region.imageSubresource = subResource;
  region.imageOffset = {0, 0, 0};
  region.imageExtent.width = swapchains[0].extent.width;
  region.imageExtent.height = swapchains[0].extent.height;
  region.imageExtent.depth = 1;

  vkCmdCopyImageToBuffer(res[resIndex].commandBuffer, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstBuffer, 1, &region);

  //endSingleTimeCommands(resIndex);
}

void Device::blitImage(const VkImage &srcImage,
                       const VkImageLayout &srcImageLayout,
                       const VkImage &dstImage,
                       const VkImageLayout &dstImageLayout,
                       const uint32_t &width,
                       const uint32_t &height,
                       const VkFilter& filter,
                       const uint16_t &resIndex) {
  VkOffset3D blitSize;
  blitSize.x = width;
  blitSize.y = height;
  blitSize.z = 1;
  VkImageBlit imageBlitRegion{};
  imageBlitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  imageBlitRegion.srcSubresource.layerCount = 1;
  imageBlitRegion.srcOffsets[1] = blitSize;
  imageBlitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  imageBlitRegion.dstSubresource.layerCount = 1;
  imageBlitRegion.dstOffsets[1] = blitSize;

  vkCmdBlitImage(res[resIndex].commandBuffer,
                 srcImage, srcImageLayout,
                 dstImage, dstImageLayout,
                 1,
                 &imageBlitRegion,
                 filter);
}

void Device::createImageView(const Image &image, const VkFormat &format, const VkImageAspectFlags &aspectFlags) {
  createImageView(image->param.handle, format, aspectFlags, image->param.view);
}

void Device::createTextureSampler(const VkFilter &magFilter, const VkFilter &minFilter) {
  VkSamplerCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  // если забуду, вкуривать https://vulkan-tutorial.com/Texture_mapping/Image_view_and_sampler
  createInfo.magFilter = magFilter;
  createInfo.minFilter = minFilter;
  createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT; // VK_SAMPLER_ADDRESS_MODE_REPEAT
  createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  createInfo.anisotropyEnable = VK_TRUE;
  createInfo.maxAnisotropy = 16.0f; // странно!!!!!!!!!!
  createInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
  createInfo.unnormalizedCoordinates = VK_FALSE;
  createInfo.compareEnable = VK_FALSE;
  createInfo.compareOp = VK_COMPARE_OP_ALWAYS;
  createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  createInfo.mipLodBias = 0.0f;
  createInfo.minLod = 0.0f;
  createInfo.maxLod = 1.0f;

  if (vkCreateSampler(device, &createInfo, nullptr, &texSampler.handle) != VK_SUCCESS) {
    Global::console()->print("failed to create texture sampler!");
    throw std::runtime_error("failed to create texture sampler!");
  }

  VkDescriptorSetLayout layouts[] = {layoutSampler};
  VkDescriptorSetAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = descriptorPool;
  allocInfo.descriptorSetCount = 1;
  allocInfo.pSetLayouts = layouts;

  if (vkAllocateDescriptorSets(device, &allocInfo, &texSampler.descriptor) != VK_SUCCESS) {
    Global::console()->print("failed to allocate descriptor set!\n");
    throw std::runtime_error("failed to allocate descriptor set!");
  }

  VkDescriptorImageInfo samplerInfo = {};
  samplerInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  samplerInfo.sampler = texSampler.handle;

  std::array<VkWriteDescriptorSet, 1> descriptorWrites = {};
  descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrites[0].dstSet = texSampler.descriptor;
  // биндинг сэмплера
  descriptorWrites[0].dstBinding = 0;
  descriptorWrites[0].dstArrayElement = 0;
  descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
  descriptorWrites[0].descriptorCount = 1;
  descriptorWrites[0].pImageInfo = &samplerInfo;

  vkUpdateDescriptorSets(device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);

  VkSamplerCreateInfo info = {};
  info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  info.magFilter = VK_FILTER_LINEAR;
  info.minFilter = VK_FILTER_LINEAR;
  info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  info.minLod = -1000;
  info.maxLod = 1000;
  info.maxAnisotropy = 1.0f;
  if (vkCreateSampler(device, &info, nullptr, &fontSampler.handle) != VK_SUCCESS) {
    Global::console()->print("failed to create font sampler!");
    throw std::runtime_error("failed to create font sampler!");
  }
}

int Device::addJob(const JobInfo &info) {
  int index = pickQueueFamilies(info.queueFlags, info.present);

//   if (vkQueueSubmit(testingQueue, info.submitCount, info.submitInfo, *info.fence) != VK_SUCCESS) {
//     throw std::runtime_error("Cannot submit commands to a queue");
//   }

  //std::cout << "Submit commads" << std::endl;
  //int result = 
  //std::cout << "Submit end" << std::endl;

  return queueFamilies[index].submitCommands(info.submitCount, info.submitInfo, info.fence);
}

VkResult Device::presentImage(const VkPresentInfoKHR &presentInfo, const int &queueIndex) {
  int index = pickQueueFamilies(VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_SPARSE_BINDING_BIT, true);
  //return vkQueuePresentKHR(testingQueue, &presentInfo);
  return vkQueuePresentKHR(queueFamilies[index].getQueue(queueIndex), &presentInfo);
}

void Device::beginCommandBuffer(const VkCommandBufferUsageFlags &flags) {
  vkResetCommandBuffer(virtualFrames[currentVirtualFrameIndex].commandBuffer, 0);

  VkCommandBufferBeginInfo commandBufferBeginInfo = {};
  commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  commandBufferBeginInfo.flags = flags;//VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT
  commandBufferBeginInfo.pInheritanceInfo = nullptr;//&inheritanceInfo;

  if (vkBeginCommandBuffer(virtualFrames[currentVirtualFrameIndex].commandBuffer, &commandBufferBeginInfo) != VK_SUCCESS) {
    Global::console()->print("Cannot begin record command buffer");
    throw std::runtime_error("Cannot begin record command buffer");
  }
}

void Device::setViewport() {
  VkViewport viewport = {};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = static_cast<float>(swapchains[0].extent.width);
  viewport.height = static_cast<float>(swapchains[0].extent.height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  vkCmdSetViewport(virtualFrames[currentVirtualFrameIndex].commandBuffer, 0, 1, &viewport);
}

void Device::setScissor() {
  VkRect2D scissor = {};
  scissor.offset.x = 0;
  scissor.offset.y = 0;
  scissor.extent.width = swapchains[0].extent.width;
  scissor.extent.height = swapchains[0].extent.height;

  vkCmdSetScissor(virtualFrames[currentVirtualFrameIndex].commandBuffer, 0, 1, &scissor);
}

void Device::resetQueryPool(const uint32_t &queries) {
  //std::cout << "query pool resseting" << "\n";
  vkCmdResetQueryPool(virtualFrames[currentVirtualFrameIndex].commandBuffer, queryPool, 0, queries);
}

void Device::updateDescriptorSets(const Image &image) {
  VkDescriptorSetLayout layouts[] = {layoutSampledImage};
  VkDescriptorSetAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = descriptorPool;
  allocInfo.descriptorSetCount = 1;
  allocInfo.pSetLayouts = layouts;

  if (vkAllocateDescriptorSets(device, &allocInfo, &image->param.descriptor) != VK_SUCCESS) {
    Global::console()->print("failed to allocate descriptor set!");
    throw std::runtime_error("failed to allocate descriptor set!");
  }

  //   VkDescriptorBufferInfo bufferInfo = {};
//   bufferInfo.buffer = uniformBuffer;
//   bufferInfo.offset = 0;
//   bufferInfo.range = sizeof(UniformBufferObject);

  VkDescriptorImageInfo imageInfo = {};
  imageInfo.sampler = VK_NULL_HANDLE;
  imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  imageInfo.imageView = image->param.view;

  std::array<VkWriteDescriptorSet, 1> descriptorWrites = {};

//   descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//   descriptorWrites[0].dstSet = descriptorSet;
//   // биндинг юниформ буфера (смотреть в шейдере)
//   descriptorWrites[0].dstBinding = 0;
//   descriptorWrites[0].dstArrayElement = 0;
//   descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//   descriptorWrites[0].descriptorCount = 1;
//   descriptorWrites[0].pBufferInfo = &bufferInfo;

  descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrites[0].dstSet = image->param.descriptor;
  // биндинг сэмплера
  descriptorWrites[0].dstBinding = 0;
  descriptorWrites[0].dstArrayElement = 0;
  descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
  descriptorWrites[0].descriptorCount = 1;
  descriptorWrites[0].pImageInfo = &imageInfo;

  vkUpdateDescriptorSets(device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
}

void Device::updateDescriptorSets1(const Image &image) {
  VkDescriptorSetLayout layouts[] = {layoutCombinedImageSampler};
  VkDescriptorSetAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = descriptorPool;
  allocInfo.descriptorSetCount = 1;
  allocInfo.pSetLayouts = layouts;

  if (vkAllocateDescriptorSets(device, &allocInfo, &image->param.descriptor) != VK_SUCCESS) {
    Global::console()->print("failed to allocate descriptor set!");
    throw std::runtime_error("failed to allocate descriptor set!");
  }

  VkDescriptorImageInfo imageInfo = {};
  imageInfo.sampler = fontSampler.handle;
  imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  imageInfo.imageView = image->param.view;

  std::array<VkWriteDescriptorSet, 1> descriptorWrites = {};
  descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrites[0].dstSet = image->param.descriptor;
  // биндинг сэмплера
  descriptorWrites[0].dstBinding = 0;
  descriptorWrites[0].dstArrayElement = 0;
  descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  descriptorWrites[0].descriptorCount = 1;
  descriptorWrites[0].pImageInfo = &imageInfo;

  vkUpdateDescriptorSets(device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
}

void Device::bindPipeline(const bool &drawing, const uint32_t &pipelineIndex) {
  if (pipelineIndex == UINT32_MAX) {
    Global::console()->print("Wrong pipeline number!");
    throw std::runtime_error("Wrong pipeline number!");
  }
  
  if (pipelineIndex != currentPipeline) {
    currentPipeline = pipelineIndex;
    if (drawing) {
      vkCmdBindPipeline(virtualFrames[currentVirtualFrameIndex].commandBuffer,
                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                        graphicsPipelines[currentPipeline].handle);
    } else {
      vkCmdBindPipeline(virtualFrames[currentVirtualFrameIndex].commandBuffer,
                        VK_PIPELINE_BIND_POINT_COMPUTE,
                        computePipelines[currentPipeline].handle);
    }
  }
}

void Device::bindGraphicPipeline(const uint32_t &pipelineIndex) {
  if (pipelineIndex == UINT32_MAX) {
    Global::console()->print("Wrong pipeline number!");
    throw std::runtime_error("Wrong pipeline number!");
  }
  
  if (pipelineIndex != currentPipeline) {
    currentPipeline = pipelineIndex;
    vkCmdBindPipeline(virtualFrames[currentVirtualFrameIndex].commandBuffer,
                      VK_PIPELINE_BIND_POINT_GRAPHICS,
                      graphicsPipelines[currentPipeline].handle);
  }
}

void Device::bindComputePipeline(const uint32_t &pipelineIndex) {
  if (pipelineIndex == UINT32_MAX) {
    Global::console()->print("Wrong pipeline number!");
    throw std::runtime_error("Wrong pipeline number!");
  }
  
  if (pipelineIndex != currentPipeline) {
    currentPipeline = pipelineIndex;
    vkCmdBindPipeline(virtualFrames[currentVirtualFrameIndex].commandBuffer,
                      VK_PIPELINE_BIND_POINT_COMPUTE,
                      computePipelines[currentPipeline].handle);
  }
}

//void pushConst();

// void Device::bindVertexBuffer(uint32_t bufferIndex) {
//   if (bufferIndex == UINT32_MAX) return;
//   //Object* obj = bufferObjects[bufferIndex].memPtr->getObject(bufferObjects[bufferIndex].bufferIndex);
//   vkCmdBindVertexBuffers(virtualFrames[currentVirtualFrameIndex].commandBuffer,
//                          0,
//                          1,
//                          &bufferObjects[bufferIndex].parameters->buffer,
//                          &bufferObjects[bufferIndex].offsets[0]);
// }
//
// void Device::bindVertexBuffer(uint32_t bufferIndex, VkDeviceSize offset) {
//   if (bufferIndex == UINT32_MAX) return;
//   //Object* obj = bufferObjects[bufferIndex].memPtr->getObject(bufferObjects[bufferIndex].bufferIndex);
//   vkCmdBindVertexBuffers(virtualFrames[currentVirtualFrameIndex].commandBuffer,
//                          0,
//                          1,
//                          &bufferObjects[bufferIndex].parameters->buffer,
//                          &offset);
// }
//
// void Device::bindVertexBuffer(uint32_t firstBinding, uint32_t bufferIndex, VkDeviceSize offset) {
//   if (bufferIndex == UINT32_MAX) return;
//   //Object* obj = bufferObjects[bufferIndex].memPtr->getObject(bufferObjects[bufferIndex].bufferIndex);
//   vkCmdBindVertexBuffers(virtualFrames[currentVirtualFrameIndex].commandBuffer,
//                          firstBinding,
//                          1,
//                          &bufferObjects[bufferIndex].parameters->buffer,
//                          &offset);
// }
//
// void Device::bindVertexBuffer(uint32_t firstBinding, uint32_t bindingCount, uint32_t *bufferIndex, VkDeviceSize* offsets) {
//   VkBuffer* buffers = new VkBuffer[bindingCount];
//   for (size_t i = 0; i < bindingCount; i++) {
//     if (bufferIndex[i] == UINT32_MAX) return;
//     //Object* obj = bufferObjects[bufferIndex[i]].memPtr->getObject(bufferObjects[bufferIndex[i]].bufferIndex);
//     buffers[i] = bufferObjects[bufferIndex[i]].parameters->buffer;
//   }
//
//   vkCmdBindVertexBuffers(virtualFrames[currentVirtualFrameIndex].commandBuffer,
//                          firstBinding,
//                          bindingCount,
//                          buffers,
//                          offsets);
//   delete [] buffers;
// }
//
// void Device::bindIndexBuffer(uint32_t bufferIndex) {
//   if (bufferIndex == UINT32_MAX) return;
//   //Object* obj = bufferObjects[bufferIndex].memPtr->getObject(bufferObjects[bufferIndex].bufferIndex);
//   vkCmdBindIndexBuffer(virtualFrames[currentVirtualFrameIndex].commandBuffer,
//                        bufferObjects[bufferIndex].parameters->buffer,
//                        bufferObjects[bufferIndex].offsets[1],
//                        VK_INDEX_TYPE_UINT32);
// }

void Device::bindVertexBuffer(const Buffer &buffer) {
  if (!buffer.isExist()) return;
  if (buffer->memory == VK_NULL_HANDLE) return;
  //Object* obj = bufferObjects[bufferIndex].memPtr->getObject(bufferObjects[bufferIndex].bufferIndex);
  vkCmdBindVertexBuffers(virtualFrames[currentVirtualFrameIndex].commandBuffer,
                         0,
                         1,
                         &buffer->param.handle,
                         &buffer->offset);
}

void Device::bindVertexBuffer(const Buffer &buffer, const VkDeviceSize &offset) {
  if (offset == UINT64_MAX) return;
  if (buffer->memory == VK_NULL_HANDLE) return;
  //Object* obj = bufferObjects[bufferIndex].memPtr->getObject(bufferObjects[bufferIndex].bufferIndex);
  VkDeviceSize finalOffset = buffer->offset + offset;
  vkCmdBindVertexBuffers(virtualFrames[currentVirtualFrameIndex].commandBuffer,
                         0,
                         1,
                         &buffer->param.handle,
                         &finalOffset);
}

void Device::bindVertexBuffer(const uint32_t &firstBinding, const Buffer &buffer, const VkDeviceSize &offset) {
  if (offset == UINT64_MAX) return;
  if (buffer->memory == VK_NULL_HANDLE) return;
  //Object* obj = bufferObjects[bufferIndex].memPtr->getObject(bufferObjects[bufferIndex].bufferIndex);
  VkDeviceSize finalOffset = buffer->offset + offset;
  vkCmdBindVertexBuffers(virtualFrames[currentVirtualFrameIndex].commandBuffer,
                         firstBinding,
                         1,
                         &buffer->param.handle,
                         &finalOffset);
}

void Device::bindVertexBuffer(const uint32_t &firstBinding, const uint32_t &bindingCount, const Buffer *buffer, const VkDeviceSize* offsets) {
  VkBuffer buffers[bindingCount];
  VkDeviceSize finalOffsets[bindingCount];
  for (size_t i = 0; i < bindingCount; i++) {
    if (offsets[i] == UINT64_MAX) return;
    if (buffer[i]->memory == VK_NULL_HANDLE) return;
    //Object* obj = bufferObjects[bufferIndex[i]].memPtr->getObject(bufferObjects[bufferIndex[i]].bufferIndex);
    buffers[i] = buffer[i]->param.handle;
    finalOffsets[i] = buffer[i]->offset + offsets[i];
  }

  vkCmdBindVertexBuffers(virtualFrames[currentVirtualFrameIndex].commandBuffer,
                         firstBinding,
                         bindingCount,
                         buffers,
                         finalOffsets);
}

void Device::bindIndex32bBuffer(const Buffer &buffer) {
  if (buffer->memory == VK_NULL_HANDLE) return;
  //Object* obj = bufferObjects[bufferIndex].memPtr->getObject(bufferObjects[bufferIndex].bufferIndex);
  vkCmdBindIndexBuffer(virtualFrames[currentVirtualFrameIndex].commandBuffer,
                       buffer->param.handle,
                       buffer->offset,
                       VK_INDEX_TYPE_UINT32);
}

void Device::bindIndex16bBuffer(const Buffer &buffer) {
  if (buffer->memory == VK_NULL_HANDLE) return;
  //Object* obj = bufferObjects[bufferIndex].memPtr->getObject(bufferObjects[bufferIndex].bufferIndex);
  vkCmdBindIndexBuffer(virtualFrames[currentVirtualFrameIndex].commandBuffer,
                       buffer->param.handle,
                       buffer->offset,
                       VK_INDEX_TYPE_UINT16);
}

void Device::pushConstants(const uint32_t &pipelineIndex, const VkShaderStageFlags &stage, const VkDeviceSize &offset, const VkDeviceSize &size, void* data) {
  vkCmdPushConstants(virtualFrames[currentVirtualFrameIndex].commandBuffer,
                     layouts[graphicsPipelines[pipelineIndex].layoutIndex].handle,
                     stage,
                     offset,
                     size,
                     data);
}

void Device::bindDescriptorSets(const Texture &texture) {
  if (texture.image->memory == VK_NULL_HANDLE) throw std::runtime_error("Texture index invalid");
  if (texture.image->param.descriptor == VK_NULL_HANDLE) {
    std::cout << "Texture index: " << texture.image->param.handle << "\n";
    throw std::runtime_error("descriptor = 0");
  }
  VkDescriptorSet sets[] = {texSampler.descriptor, texture.image->param.descriptor};
  vkCmdBindDescriptorSets(virtualFrames[currentVirtualFrameIndex].commandBuffer,
                          VK_PIPELINE_BIND_POINT_GRAPHICS,
                          layouts[graphicsPipelines[currentPipeline].layoutIndex].handle,
                          0, 2, sets,
                          0, nullptr);
}

void Device::bindDescriptorSets1(const Texture &texture) {
  if (texture.image->memory == VK_NULL_HANDLE) throw std::runtime_error("Texture index invalid");
  if (texture.image->param.descriptor == VK_NULL_HANDLE) {
    std::cout << "Texture index: " << texture.image->param.handle << "\n";
    throw std::runtime_error("descriptor = 0");
  }
  VkDescriptorSet sets[] = {texture.image->param.descriptor};
  vkCmdBindDescriptorSets(virtualFrames[currentVirtualFrameIndex].commandBuffer,
                          VK_PIPELINE_BIND_POINT_GRAPHICS,
                          layouts[graphicsPipelines[currentPipeline].layoutIndex].handle,
                          0, 1, sets,
                          0, nullptr);
}

void Device::bindDescriptorSets(const uint32_t &firstSet, const Buffer &b) {
  //if (!b.isExist()) throw std::runtime_error("Texture index invalid");
  VkDescriptorSet sets[] = {b->param.descriptor};
  vkCmdBindDescriptorSets(virtualFrames[currentVirtualFrameIndex].commandBuffer,
                          VK_PIPELINE_BIND_POINT_GRAPHICS,
                          layouts[graphicsPipelines[currentPipeline].layoutIndex].handle,
                          firstSet, 1, sets,
                          0, nullptr);
}

// void Device::drawIndexed(uint32_t bufferIndex) {
//   vkCmdDrawIndexed(virtualFrames[currentVirtualFrameIndex].commandBuffer, bufferObjects[bufferIndex].indicesSize, 1, 0, 0, 0);
// }

void Device::drawIndexed(const Buffer &buffer) {
  vkCmdDrawIndexed(virtualFrames[currentVirtualFrameIndex].commandBuffer, buffer->param.dataCount, 1, 0, 0, 0);
}

void Device::drawIndexed(const uint32_t &elemCount,
                         const uint32_t &instanceCount,
                         const uint32_t &firstIndex,
                         const int32_t &vertexOffset,
                         const uint32_t &firstInstance) {
  vkCmdDrawIndexed(virtualFrames[currentVirtualFrameIndex].commandBuffer, elemCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void Device::draw(const uint32_t &vertexCount, const uint32_t &instanceCount, const uint32_t &firstVertex, const uint32_t &firstInstance) {
  vkCmdDraw(virtualFrames[currentVirtualFrameIndex].commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

void Device::beginQuery(const uint32_t &query, const uint32_t &flags) {
  vkCmdBeginQuery(virtualFrames[currentVirtualFrameIndex].commandBuffer, queryPool, query, flags);
}

void Device::endQuery(const uint32_t &query) {
  vkCmdEndQuery(virtualFrames[currentVirtualFrameIndex].commandBuffer, queryPool, query);
}

void Device::nextSubpass(const VkSubpassContents &contents) {
  vkCmdNextSubpass(virtualFrames[currentVirtualFrameIndex].commandBuffer, contents);
}

void Device::endCommandBuffer() {
  if (vkEndCommandBuffer(virtualFrames[currentVirtualFrameIndex].commandBuffer) != VK_SUCCESS) {
    Global::console()->print("Cannot end the command buffer recording\n");
    throw std::runtime_error("Cannot end the command buffer recording");
  }

  currentPipeline = -1;
}

// void Device::executeSecondaryCommandBuffers() {
//   vkCmdExecuteCommands(virtualFrames[currentVirtualFrameIndex].commandBuffer, drawCommandBuffers.size(), drawCommandBuffers.data());
// }
//
// void Device::clearCommandBuffers() {
//   drawCommandBuffers.clear();
// }

bool Device::allocateCommandBuffers(const uint32_t &poolIndex, const VkCommandBufferLevel &level, const uint32_t &count, VkCommandBuffer *command_buffers) {
  return allocateCommandBuffers(commandPools[poolIndex], level, count, command_buffers);
}

void Device::createQueryPool(const uint32_t &queryCount) {
  if (queryPool != VK_NULL_HANDLE) {
    vkDestroyQueryPool(device, queryPool, nullptr);
    queryPool = VK_NULL_HANDLE;
  }
  
  VkQueryPoolCreateInfo info;
  info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
  info.pNext = nullptr;
  info.flags = 0;
  info.queryType = VK_QUERY_TYPE_OCCLUSION;
  info.queryCount = queryCount;
  info.pipelineStatistics = 0;
  
  VK_CHECK_RESULT(vkCreateQueryPool(device, &info, nullptr, &queryPool), "cant create query pool!\n")
}

void Device::getQueryPoolResults(const uint32_t &firstQuery, const uint32_t &queryCount, const size_t &dataSize, void* data, const VkDeviceSize &stride, const VkQueryResultFlags &flags) {
  VK_CHECK_RESULT(vkGetQueryPoolResults(device, queryPool, firstQuery, queryCount, dataSize, data, stride, flags), "cant get query results! must not be blocking!")
}

void Device::updateDescriptorSetUniform(const Buffer &uniform) {
  //VkDescriptorSet sets[] = {b1->param.descriptor, b2->param.descriptor};
  VkDescriptorSetLayout layouts[] = {uboSetLayout};//{uboSetLayout};
  VkDescriptorSetAllocateInfo allocInfo;
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.pNext = nullptr;
  allocInfo.descriptorPool = descriptorPool;
  allocInfo.descriptorSetCount = 1;
  allocInfo.pSetLayouts = layouts;
  
  //VkDescriptorSet &set = this->layouts[graphicsPipelines[occludeePipelineIndex].layoutIndex].set;

  if (vkAllocateDescriptorSets(device, &allocInfo, &uniform->param.descriptor) != VK_SUCCESS) {
    Global::console()->print("failed to allocate descriptor set!");
    throw std::runtime_error("failed to allocate descriptor set!");
  }

  VkDescriptorBufferInfo bufferInfo1;
  bufferInfo1.buffer = uniform->param.handle;
  bufferInfo1.offset = uniform->offset;
  bufferInfo1.range = uniform->size;

  std::array<VkWriteDescriptorSet, 1> descriptorWrites;

  descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrites[0].dstSet = uniform->param.descriptor;
  descriptorWrites[0].dstBinding = 0;
  descriptorWrites[0].dstArrayElement = 0;
  //PRINT_VAR("uniform buffer type", VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
  descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  descriptorWrites[0].descriptorCount = 1;
  descriptorWrites[0].pBufferInfo = &bufferInfo1;
  descriptorWrites[0].pImageInfo = nullptr;
  descriptorWrites[0].pTexelBufferView = nullptr;

  vkUpdateDescriptorSets(device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
}

// void Device::updateDescriptorSetStorage(const Buffer &storage) {
// //   VkDescriptorSetLayout layouts[] = {storageSetLayout};
// //   VkDescriptorSetAllocateInfo allocInfo;
// //   allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
// //   allocInfo.pNext = nullptr;
// //   allocInfo.descriptorPool = storagePool;
// //   allocInfo.descriptorSetCount = 1;
// //   allocInfo.pSetLayouts = layouts;
// // 
// //   if (vkAllocateDescriptorSets(device, &allocInfo, &storage->param.descriptor) != VK_SUCCESS) {
// //     Global::console()->print("failed to allocate descriptor set!");
// //     throw std::runtime_error("failed to allocate descriptor set!");
// //   }
// //   
// //   VkDescriptorBufferInfo bufferInfo;
// //   bufferInfo.buffer = storage->param.handle;
// //   bufferInfo.offset = storage->offset;
// //   bufferInfo.range =  storage->size;
// //   
// //   std::array<VkWriteDescriptorSet, 1> descriptorWrites;
// //   
// //   descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
// //   descriptorWrites[0].dstSet = storage->param.descriptor;
// //   descriptorWrites[0].dstBinding = 0;
// //   descriptorWrites[0].dstArrayElement = 0;
// //   descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
// //   descriptorWrites[0].descriptorCount = 1;
// //   descriptorWrites[0].pBufferInfo = &bufferInfo;
// //   descriptorWrites[0].pImageInfo = nullptr;
// //   descriptorWrites[0].pTexelBufferView = nullptr;
// //   
// //   vkUpdateDescriptorSets(device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
// }
// 
// void Device::freeDescriptorSet(const Buffer &buffer) {
// //   vkFreeDescriptorSets(device, storagePool, 1, &buffer->param.descriptor);
// }
// 
// void Device::resetStoragePool() {
// //   vkResetDescriptorPool(device, storagePool, 0);
// }

void Device::updateDescriptorSets1(const Buffer &uniform) {
  VkDescriptorSetLayout l[] = {uboSetLayout};
  VkDescriptorSetAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.pNext = nullptr;
  allocInfo.descriptorPool = descriptorPool;
  allocInfo.descriptorSetCount = 1;
  allocInfo.pSetLayouts = l;

  if (vkAllocateDescriptorSets(device, &allocInfo, &layouts[graphicsPipelines[basicPipelineIndex].layoutIndex].set) != VK_SUCCESS) {
    Global::console()->print("failed to allocate descriptor set!");
    throw std::runtime_error("failed to allocate descriptor set!");
  }

  VkDescriptorBufferInfo bufferInfo = {};
  bufferInfo.buffer = uniform->param.handle;
  bufferInfo.offset = uniform->offset;
  bufferInfo.range = uniform->size;

  std::array<VkWriteDescriptorSet, 1> descriptorWrites = {};

  descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrites[0].dstSet = layouts[graphicsPipelines[basicPipelineIndex].layoutIndex].set;
  descriptorWrites[0].dstBinding = 1; 
  descriptorWrites[0].dstArrayElement = 0;
  //PRINT_VAR("storage buffer type", VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
  descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  descriptorWrites[0].descriptorCount = 1;
  descriptorWrites[0].pBufferInfo = &bufferInfo;
  descriptorWrites[0].pImageInfo = nullptr;
  descriptorWrites[0].pTexelBufferView = nullptr;

  vkUpdateDescriptorSets(device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
}

// get'еры и set'еры
VkPhysicalDevice Device::getPhysicalDevice() {
  return physicalDevice;
}

VkDevice Device::getDevice() {
  return device;
}

VkSwapchainKHR Device::getSwapChain() {
  return swapchains[0].handle;
}

VkRenderPass Device::getRenderPass(const uint32_t &renderPassIndex) {
  return renderPasses[renderPassIndex];
}

Memory* Device::getMemory() {
  return &deviceMemory;
}

VkExtent2D Device::getSwapchainExtent() {
  return swapchains[0].extent;
}

void Device::setSwapChainExtend(VkExtent2D extend) {
  swapchains[0].extent = extend;
}

//VDeleter<VkCommandPool> Device::getCommandPool(int i) {
//  return commandPools[i];
//}

VkCommandPool Device::getCommandPool(int i) {
  return commandPools[i];
}

VirtualFrame* Device::getNextFrame() {
  currentVirtualFrameIndex = (currentVirtualFrameIndex+1) % framesCount;
  //std::cout << "Using CB number: " << currentVirtualFrameIndex << std::endl;
  return &virtualFrames[currentVirtualFrameIndex];
}

VkImageView Device::getImageView(uint32_t i) {
  return swapchains[0].imageViews[i];
}

VkImageView Device::getDepthImageView() {
  return depthImage.view;
}

VkImageView Device::getOcclusionImageView() {
  return occlusionImage.view;
}

VkImage Device::getImage(uint32_t i) {
  return swapchains[0].images[i];
}

VkImage Device::getOcclusionImage() {
  return occlusionImage.handle;
}

// VkExtent3D Device::getImageWHD(uint32_t i) {
//   return imageObjects[i].sizeWHD;
// }

void Device::setTransferPoolIndex(int i) {
  transferPoolIndex = i;
}

VkBool32 Device::isPresentSupport() {
  return presentSupport;
}

VkPipeline Device::getGraphicPipeline(const uint32_t &pipelineIndex) {
  return graphicsPipelines[pipelineIndex].handle;
}

VkPipelineLayout Device::getPipelineLayout(const uint32_t &pipelineIndex) {
  return layouts[graphicsPipelines[pipelineIndex].layoutIndex].handle;
}

VkDescriptorSet Device::getPipelineLayoutSet(const uint32_t &pipelineIndex) {
  return layouts[graphicsPipelines[pipelineIndex].layoutIndex].set;
}

VkDescriptorSetLayout Device::getLayoutSampler() {
  return layoutSampler;
}

VkDescriptorSetLayout Device::getLayoutSampledImage() {
  return layoutSampledImage;
}

VkDescriptorSetLayout Device::getLayoutCombineImageSampler() {
  return layoutCombinedImageSampler;
}

VkDescriptorSetLayout Device::getUboSetLayout() {
  return uboSetLayout;
}

VkDescriptorSetLayout Device::getStorageSetLayout() {
  return storageSetLayout;
}

uint32_t Device::getBasicPIndex() {
  return basicPipelineIndex;
}

uint32_t Device::getBasicUVPIndex() {
  return basicUVPipelineIndex;
}

uint32_t Device::getBasicTextPIndex() {
  return basicTextPipelineIndex;
}

uint32_t Device::getBasicAABBPIndex() {
  return basicAABBPipelineIndex;
}

uint32_t Device::getBasicImagePIndex() {
  return basicImagePipelineIndex;
}

uint32_t Device::getBasicSpherePIndex() {
  return basicSpherePipelineIndex;
}

uint32_t Device::getBasicGuiPIndex() {
  return basicGuiPipelineIndex;
}

uint32_t Device::getOccluderPIndex() {
  return occluderPipelineIndex;
}

uint32_t Device::getOccludeePIndex() {
  return occludeePipelineIndex;
}

void Device::setBasicPIndex(uint32_t index) {
  basicPipelineIndex = index;
}

void Device::setBasicUVPIndex(uint32_t index) {
  basicUVPipelineIndex = index;
}

void Device::setBasicTextPIndex(uint32_t index) {
  basicTextPipelineIndex = index;
}

void Device::setBasicAABBPIndex(uint32_t index) {
  basicAABBPipelineIndex = index;
}

void Device::setBasicImagePIndex(uint32_t index) {
  basicImagePipelineIndex = index;
}

void Device::setBasicSpherePIndex(uint32_t index) {
  basicSpherePipelineIndex = index;
}

void Device::setBasicGuiPIndex(uint32_t index) {
  basicGuiPipelineIndex = index;
}

void Device::setOccluderPIndex(uint32_t index) {
  occluderPipelineIndex = index;
}

void Device::setOccludeePIndex(uint32_t index) {
  occludeePipelineIndex = index;
}

size_t Device::getGraphicPipelineSize() {
  return graphicsPipelines.size();
}

// BufferObject* Device::getBufferObject(uint32_t bufferIndex) {
//   return &bufferObjects[bufferIndex];
// }
//
// void Device::setMapMemoryPtr(uint32_t bufferIndex, void* ptr) {
//   bufferObjects[bufferIndex].mapMemoryPtr = ptr;
// }
//
// void* Device::getMapMemoryPtr(uint32_t bufferIndex) {
//   return bufferObjects[bufferIndex].mapMemoryPtr;
// }
//
// VkDeviceMemory Device::getDeviceMemory(VkMemoryPropertyFlags properties) {
//   if (deviceLocal->compareProperties(properties)) return deviceLocal->getMemory();
//   return hostVisible->getMemory();
// }

VkPhysicalDeviceMemoryProperties Device::getDeviceMemoryProperties() {
  return memoryProperties;
}

int32_t Device::f(const uint32_t &typefilter, const uint32_t &propeties) {
  return findMemoryType(typefilter, propeties);
}

void Device::deviceWaitIdle() {
  //std::cout << "before vkDeviceWaitIdle3" << "\n";
  
  vkDeviceWaitIdle(device);

  for (size_t i = 0; i < res.size(); i++) {
    clearResource(i);
  }
}

uint32_t Device::showBufferMemoryTypes() {
  VkBuffer buffer1;
  VkBufferCreateInfo bufferInfo = {};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = 65535;
  bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer1) != VK_SUCCESS) {
    Global::console()->print("failed to create buffer!\n");
    throw std::runtime_error("failed to get memory type");
  }

  VkMemoryRequirements memRequirements1;
  vkGetBufferMemoryRequirements(device, buffer1, &memRequirements1);

  VkBuffer buffer2;
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = 65535*2;
  bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer2) != VK_SUCCESS) {
    Global::console()->print("failed to create buffer!\n");
    throw std::runtime_error("failed to get memory type");
  }

  VkMemoryRequirements memRequirements2;
  vkGetBufferMemoryRequirements(device, buffer2, &memRequirements2);

  VkBuffer buffer3;
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = 65535*4;
  bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer3) != VK_SUCCESS) {
    Global::console()->print("failed to create buffer!\n");
    throw std::runtime_error("failed to get memory type");
  }

  VkMemoryRequirements memRequirements3;
  vkGetBufferMemoryRequirements(device, buffer3, &memRequirements3);

  VkBuffer buffer4;
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = 31;
  bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer4) != VK_SUCCESS) {
    Global::console()->print("failed to create buffer!\n");
    throw std::runtime_error("failed to get memory type");
  }

  VkMemoryRequirements memRequirements4;
  vkGetBufferMemoryRequirements(device, buffer4, &memRequirements4);

  VkBuffer buffer5;
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = 124718775;
  bufferInfo.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer5) != VK_SUCCESS) {
    Global::console()->print("failed to create buffer!\n");
    throw std::runtime_error("failed to get memory type");
  }

  VkMemoryRequirements memRequirements5;
  vkGetBufferMemoryRequirements(device, buffer5, &memRequirements5);

  VkBuffer buffer6;
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = 2235;
  bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer6) != VK_SUCCESS) {
    Global::console()->print("failed to create buffer!\n");
    throw std::runtime_error("failed to get memory type");
  }

  VkMemoryRequirements memRequirements6;
  vkGetBufferMemoryRequirements(device, buffer6, &memRequirements6);

  VkBuffer buffer7;
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = 2235;
  bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT |VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer7) != VK_SUCCESS) {
    Global::console()->print("failed to create buffer!\n");
    throw std::runtime_error("failed to get memory type");
  }

  VkMemoryRequirements memRequirements7;
  vkGetBufferMemoryRequirements(device, buffer7, &memRequirements7);

//   Global::console()->printf("Vertex Index  buffer type bits: %d\n", memRequirements1.memoryTypeBits);
//   Global::console()->printf("Vertex        buffer type bits: %d\n", memRequirements2.memoryTypeBits);
//   Global::console()->printf("Uniform       buffer type bits: %d\n", memRequirements3.memoryTypeBits);
//   Global::console()->printf("Storage       buffer type bits: %d\n", memRequirements4.memoryTypeBits);
//   Global::console()->printf("Indirect      buffer type bits: %d\n", memRequirements5.memoryTypeBits);
//   Global::console()->printf("Uniform texel buffer type bits: %d\n", memRequirements6.memoryTypeBits);
//   Global::console()->printf("All type      buffer      bits: %d\n", memRequirements7.memoryTypeBits);

//   std::cout << "Vertex Index  buffer mem aligment: " << memRequirements1.alignment << "\n";
//   std::cout << "Vertex        buffer mem aligment: " << memRequirements2.alignment << "\n";
//   std::cout << "Uniform       buffer mem aligment: " << memRequirements3.alignment << "\n";
//   std::cout << "Storage       buffer mem aligment: " << memRequirements4.alignment << "\n";
//   std::cout << "Indirect      buffer mem aligment: " << memRequirements5.alignment << "\n";
//   std::cout << "Uniform texel buffer mem aligment: " << memRequirements6.alignment << "\n";
//   std::cout << "All type      buffer mem aligment: " << memRequirements7.alignment << "\n";

  if (!((memRequirements1.memoryTypeBits == memRequirements2.memoryTypeBits) &&
        (memRequirements1.memoryTypeBits == memRequirements3.memoryTypeBits) &&
        (memRequirements1.memoryTypeBits == memRequirements4.memoryTypeBits) &&
        (memRequirements1.memoryTypeBits == memRequirements5.memoryTypeBits) &&
        (memRequirements1.memoryTypeBits == memRequirements6.memoryTypeBits) &&
        (memRequirements1.memoryTypeBits == memRequirements7.memoryTypeBits))) {
    Global::console()->print("One of the buffers has different memory type!");
    throw std::runtime_error("Failed to get memory type");
  }

  uint32_t memoryType = memRequirements1.memoryTypeBits;

  //std::cout << "Types shown" <<  std::endl;

  vkDestroyBuffer(device, buffer1, nullptr);
  vkDestroyBuffer(device, buffer2, nullptr);
  vkDestroyBuffer(device, buffer3, nullptr);
  vkDestroyBuffer(device, buffer4, nullptr);
  vkDestroyBuffer(device, buffer5, nullptr);
  vkDestroyBuffer(device, buffer6, nullptr);
  vkDestroyBuffer(device, buffer7, nullptr);

  return memoryType;

  // память требует на вход тип, который получается из типа буферов и размещения памяти
  // В этой функции проверил какие вообще есть типы и везде показывает число 1665
  // на ноуте, где стоит AMD HD 8600M, показывает число 15
  // На момент 12 октября 2016 года все что я хочу использовать это Вершинный/Индексный, Вершинный, Юниформ буферы
  // Скорее всего 1665 это единственный возможный memoryTypeBits
  // Может быть в следующих версиях Vulkan добавят еще

  // 19 апреля 2017 года: наконец нашел информацию по типам памяти
  // действительно существует обычно всего несколько типов памяти:
  // у нвидии: обычно всего 2 типа: на хосте (медленная) и на устройстве (быстрая)
  // у АМД: 3 типа: на хосте (медленная), переферийная (маленькая, но на хосте и быстрая) и на устройстве (быстрая)
  // понятное дело что у разных вендоров показывает разные числа
  // в этих функциях мы и находим эти числа
}

uint32_t Device::showImageMemoryTypes() {
  VkImage image1;
  VkImageCreateInfo imageInfo1 = {};
  imageInfo1.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo1.imageType = VK_IMAGE_TYPE_2D;
  imageInfo1.extent = {512, 512, 1};
  imageInfo1.mipLevels = 1;
  imageInfo1.arrayLayers = 1;
  // распространенный формат, так что оставим как есть
  imageInfo1.format = VK_FORMAT_R8G8B8A8_UNORM;
  imageInfo1.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageInfo1.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
  imageInfo1.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT; //VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  imageInfo1.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  imageInfo1.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo1.flags = 0; // Optional

  if (vkCreateImage(device, &imageInfo1, nullptr, &image1) != VK_SUCCESS) {
    Global::console()->print("Failed to create image!\n");
    throw std::runtime_error("Failed to get memory type");
  }

  VkMemoryRequirements memRequirements1;
  vkGetImageMemoryRequirements(device, image1, &memRequirements1);

  VkImage image2;
  VkImageCreateInfo imageInfo2 = {};
  imageInfo2.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo2.imageType = VK_IMAGE_TYPE_2D;
  imageInfo2.extent = {512, 512, 1};
  imageInfo2.mipLevels = 1;
  imageInfo2.arrayLayers = 1;
  // распространенный формат, так что оставим как есть
  imageInfo2.format = VK_FORMAT_R8G8B8A8_UNORM;
  imageInfo2.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageInfo2.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
  imageInfo2.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
  imageInfo2.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  imageInfo2.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo2.flags = 0; // Optional

  if (vkCreateImage(device, &imageInfo2, nullptr, &image2) != VK_SUCCESS) {
    Global::console()->print("Failed to create image!\n");
    throw std::runtime_error("Failed to get memory type");
  }

  VkMemoryRequirements memRequirements2;
  vkGetImageMemoryRequirements(device, image2, &memRequirements2);

  VkImage image3;
  VkImageCreateInfo imageInfo3 = {};
  imageInfo3.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo3.imageType = VK_IMAGE_TYPE_2D;
  imageInfo3.extent = {512, 512, 1};
  imageInfo3.mipLevels = 1;
  imageInfo3.arrayLayers = 1;
  // распространенный формат, так что оставим как есть
  imageInfo3.format = VK_FORMAT_R8G8B8A8_UNORM;
  imageInfo3.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageInfo3.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
  imageInfo3.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
  imageInfo3.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  imageInfo3.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo3.flags = 0; // Optional

  if (vkCreateImage(device, &imageInfo3, nullptr, &image3) != VK_SUCCESS) {
    Global::console()->print("Failed to create image!\n");
    throw std::runtime_error("Failed to get memory type");
  }

  VkMemoryRequirements memRequirements3;
  vkGetImageMemoryRequirements(device, image3, &memRequirements3);

  VkImage image4;
  VkImageCreateInfo imageInfo4 = {};
  imageInfo4.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo4.imageType = VK_IMAGE_TYPE_2D;
  imageInfo4.extent = {512, 512, 1};
  imageInfo4.mipLevels = 1;
  imageInfo4.arrayLayers = 1;
  // распространенный формат, так что оставим как есть
  imageInfo4.format = VK_FORMAT_R8G8B8A8_UNORM;
  imageInfo4.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageInfo4.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
  imageInfo4.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                     VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                     VK_IMAGE_USAGE_SAMPLED_BIT |
                     VK_IMAGE_USAGE_STORAGE_BIT;
  imageInfo4.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  imageInfo4.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo4.flags = 0; // Optional

  if (vkCreateImage(device, &imageInfo4, nullptr, &image4) != VK_SUCCESS) {
    Global::console()->print("Failed to create image!\n");
    throw std::runtime_error("Failed to get memory type");
  }

  VkMemoryRequirements memRequirements4;
  vkGetImageMemoryRequirements(device, image4, &memRequirements4);

//   Global::console()->printf("Sampled image             type bits: %d\n", memRequirements1.memoryTypeBits);
//   Global::console()->printf("Sampled storage image     type bits: %d\n", memRequirements2.memoryTypeBits);
//   Global::console()->printf("Only src image            type bits: %d\n", memRequirements3.memoryTypeBits);
//   Global::console()->printf("Sampled storage dst image type bits: %d\n", memRequirements4.memoryTypeBits);

//   std::cout << "Sampled             image mem aligment: " << memRequirements1.alignment << "\n";
//   std::cout << "Sampled storage     image mem aligment: " << memRequirements2.alignment << "\n";
//   std::cout << "Only src            image mem aligment: " << memRequirements3.alignment << "\n";
//   std::cout << "Sampled storage dst image mem aligment: " << memRequirements4.alignment << "\n";

  vkDestroyImage(device, image1, nullptr);
  vkDestroyImage(device, image2, nullptr);
  vkDestroyImage(device, image3, nullptr);
  vkDestroyImage(device, image4, nullptr);

  if (!(memRequirements1.memoryTypeBits == memRequirements2.memoryTypeBits &&
        memRequirements1.memoryTypeBits == memRequirements3.memoryTypeBits &&
        memRequirements1.memoryTypeBits == memRequirements4.memoryTypeBits)) {
    Global::console()->print("One of the images has different memory type!");
    throw std::runtime_error("Failed to get memory type");
  }

  return memRequirements1.memoryTypeBits;

  // картинки в vulkan представлены по всей видимости двумя видами:
  // собственно картинкой и аттачментом
  // картику можно пихать в шейдер, заполнять всяким добром и тд
  // с аттачментом такое не прокатит
  // аттачмент используется совместно с фреймбуфером и я пока что только примерно понимаю
  // что конкретно представляет из себя аттачмент
}

// вспомагательные функции
void Device::findQueueFamilies(const VkSurfaceKHR &surface, const uint32_t &numberOfQueues, std::vector<QueueFamilyInfo> &infos) {
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
  if (queueFamilyCount == 0) {
    Global::console()->print("Could not check queue family properties!\n");
    throw std::runtime_error("Could not check queue family properties!");
  }

  std::vector<VkQueueFamilyProperties> deviceQueueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, deviceQueueFamilies.data());

  //std::vector<QueueFamily *> queueFamiliesStruct;
  for (uint32_t i = 0; i < deviceQueueFamilies.size(); i++) {
    VkBool32 present = VK_FALSE;
    vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &present);
    if (present == VK_TRUE) presentSupport = VK_TRUE;
    uint32_t count = (deviceQueueFamilies[i].queueCount > numberOfQueues)?numberOfQueues : deviceQueueFamilies[i].queueCount;
    //QueueFamily *tmp = new QueueFamily();
//     QueueFamily tmp;
//     queueFamilies.emplace_back();
//     queueFamilies.back().init(count, deviceQueueFamilies[i].queueFlags, present, i, device);
    
    QueueFamilyInfo info;
    info.queueCount = count;
    info.flags = deviceQueueFamilies[i].queueFlags;
    info.present = present;
    infos.push_back(info);
  }
}

void Device::createAdditionalResources() {
  res.resize(resCount);

  for (size_t i = 0; i < res.size(); i++) {
//     VkFenceCreateInfo createInfo = {};
//     createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
//     createInfo.pNext = nullptr;
//     createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
//
//     vkCreateFence(device, &createInfo, nullptr, &res[i].fence);
    createFence(res[i].fence);

    // командный буфер будет создаваться на месте
  }
}

VkFormat Device::findDepthFormat() {
  return findSupportedFormat(
    {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
    VK_IMAGE_TILING_OPTIMAL,
    VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

VkFormat Device::findSupportedFormat(const std::vector<VkFormat>& candidates, const VkImageTiling &tiling, const VkFormatFeatureFlags &features) {
  for (VkFormat format : candidates) {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

    if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
      return format;
    } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
      return format;
    }
  }

  Global::console()->print("failed to find supported format!\n");
  throw std::runtime_error("failed to find supported format!");
}

void Device::clearImageViews(const uint32_t &swapChainIndex) {
  for (size_t i = 0; i < swapchains[swapChainIndex].imageViews.size(); i++) {
    if (swapchains[swapChainIndex].imageViews[i] != VK_NULL_HANDLE) {
      vkDestroyImageView(device, swapchains[swapChainIndex].imageViews[i], nullptr);
    }
  }
}

void Device::createImageView(const VkImage &image, const VkFormat &format, const VkImageAspectFlags &aspectFlags, VkImageView &imageView) {
  VkImageViewCreateInfo viewInfo = {};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.pNext = nullptr;
  viewInfo.image = image;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = format;
  viewInfo.subresourceRange.aspectMask = aspectFlags;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.levelCount = 1;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = 1;
  
  if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
    Global::console()->print("failed to create texture image view!\n");
    throw std::runtime_error("failed to create texture image view!");
  }
}

void Device::createDepthResources() {
  if (depthImage.view != VK_NULL_HANDLE) {
    vkDestroyImageView(device, depthImage.view, nullptr);
  }
  if (depthImage.handle != VK_NULL_HANDLE) {
    vkDestroyImage(device, depthImage.handle, nullptr);
  }
  if (depthImage.memory != VK_NULL_HANDLE) {
    vkFreeMemory(device, depthImage.memory, nullptr);
  }

  VkFormat depthFormat = findDepthFormat();

  //std::cout << "Depth resources creation" << std::endl;
  createImage(swapchains[0].extent.width, swapchains[0].extent.height,
              depthFormat,
              VK_IMAGE_TILING_OPTIMAL,
              VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
              depthImage.handle,
              depthImage.memory);
  
  createImageView(depthImage.handle, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, depthImage.view);

  uint16_t index = getResIndex();
  blockResource(index);
  transitionImageLayout(depthImage.handle, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, index);
  unblockResource(index);
  waitResource(index, 1000000000);
}

int Device::pickQueueFamilies(const VkQueueFlags &flags, const bool &presentSupport) {
  for (uint16_t i = 0; i < queueFamilies.size(); i++) {
    if (presentSupport) {
      if ((queueFamilies[i].getQueueFlags() & flags) && (queueFamilies[i].isPresentSupport() == VK_TRUE)) return i;
    } else {
      if (queueFamilies[i].getQueueFlags() & flags) return i;
    }
  }

  return -1;
}

bool Device::allocateCommandBuffers(const VkCommandPool &pool, const VkCommandBufferLevel &level, const uint32_t &count, VkCommandBuffer *command_buffers) {
  VkCommandBufferAllocateInfo allocateInfo = {
    VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,   // VkStructureType                sType
    nullptr,                                          // const void                    *pNext
    pool,                                             // VkCommandPool                  commandPool
    level,                                            // VkCommandBufferLevel           level
    count                                             // uint32_t                       bufferCount
  };

  if( vkAllocateCommandBuffers(device, &allocateInfo, command_buffers) != VK_SUCCESS ) {
    //std::cout << "failed to allocate command buffers!" << std::endl;
    Global::console()->print("failed to allocate command buffers!\n");
    return false;
  }
  //std::cout << "Allocated " << command_buffers[0] << " buffer." << std::endl;

  return true;
}

void Device::createSemaphores(VkSemaphore &semaphore) {
  // на данном этапе это все что нужно
  VkSemaphoreCreateInfo semaphoreInfo = {};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  semaphoreInfo.pNext = nullptr;
  semaphoreInfo.flags = 0;

  if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &semaphore) != VK_SUCCESS) {
    Global::console()->print("failed to create semaphore!\n");
    throw std::runtime_error("failed to create semaphore!");
  }
}

void Device::createFence(VkFence &fence) {
  VkFenceCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  createInfo.pNext = nullptr;
  createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  if (vkCreateFence(device, &createInfo, nullptr, &fence) != VK_SUCCESS) {
    Global::console()->print("failed to create fence!\n");
    throw std::runtime_error("failed to create fence!");
  }
}

void Device::beginSingleTimeCommands(const uint16_t &resourceIndex) {
  if (vkWaitForFences(device, 1, &res[resourceIndex].fence, VK_FALSE, 1000000000) != VK_SUCCESS) {
    Global::console()->print("Waiting for additional fence too long\n");
    throw std::runtime_error("Waiting for additional fence too long");
  }
  vkResetFences(device, 1, &res[resourceIndex].fence);

  if (res[resourceIndex].commandBuffer != VK_NULL_HANDLE) {
    vkFreeCommandBuffers(device, commandPools[transferPoolIndex], 1, &res[resourceIndex].commandBuffer);
  }
  allocateCommandBuffers(commandPools[transferPoolIndex], VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1, &res[resourceIndex].commandBuffer);

  // указываем, что этот командный буфер будет использоваться всего лишь раз
  VkCommandBufferBeginInfo beginInfo = {};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(res[resourceIndex].commandBuffer, &beginInfo);
}

void Device::endSingleTimeCommands(const uint16_t &resIndex) {
  vkEndCommandBuffer(res[resIndex].commandBuffer);

  // отправляем эти команды в очередь
  VkSubmitInfo submitInfo = {};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &res[resIndex].commandBuffer;

  JobInfo jobInfo = {};
  jobInfo.queueFlags = VK_QUEUE_GRAPHICS_BIT;
  jobInfo.present = true;
  jobInfo.submitCount = 1;
  jobInfo.submitInfo = &submitInfo;
  jobInfo.fence = &res[resIndex].fence;

  //std::cout << "Job start" << std::endl;
  addJob(jobInfo);
  // мы также можем проверить готовность очереди с помощью забора (fence) (предпочтительнее)
  // vkQueueWaitIdle(queueFamilyIndex.transferQueueHandle);
  //std::cout << "Error here" << std::endl;
  //vkWaitForFences(device, 1, &res[resIndex].fence, VK_FALSE, 1000000000);
  //std::cout << "Job end" << std::endl;
}

int32_t Device::findMemoryType(const uint32_t &typeFilter, const VkMemoryPropertyFlags &properties) {
  //VkPhysicalDeviceMemoryProperties memProperties;
  //vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

  for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
    //if ((typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
    if ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties && typeFilter & (1<<i)) {
      return i;
    }
  }

  return -1;
}

void Device::clearResource(const uint16_t &resourceIndex) {
  if (res[resourceIndex].temporaryBufferObject.handle != VK_NULL_HANDLE) {
    vkDestroyBuffer(device, res[resourceIndex].temporaryBufferObject.handle, nullptr);
  }
  res[resourceIndex].temporaryBufferObject.handle = VK_NULL_HANDLE;
  if (res[resourceIndex].temporaryBufferObject.memory != VK_NULL_HANDLE) {
    vkFreeMemory(device, res[resourceIndex].temporaryBufferObject.memory, nullptr);
  }
  res[resourceIndex].temporaryBufferObject.memory = VK_NULL_HANDLE;

  if (res[resourceIndex].temporaryImageObject.view != VK_NULL_HANDLE) {
    vkDestroyImageView(device, res[resourceIndex].temporaryImageObject.view, nullptr);
  }
  res[resourceIndex].temporaryImageObject.view = VK_NULL_HANDLE;
  if (res[resourceIndex].temporaryImageObject.handle != VK_NULL_HANDLE) {
    vkDestroyImage(device, res[resourceIndex].temporaryImageObject.handle, nullptr);
  }
  res[resourceIndex].temporaryImageObject.handle = VK_NULL_HANDLE;
  if (res[resourceIndex].temporaryImageObject.memory != VK_NULL_HANDLE) {
    vkFreeMemory(device, res[resourceIndex].temporaryImageObject.memory, nullptr);
  }
  res[resourceIndex].temporaryImageObject.memory = VK_NULL_HANDLE;
}
