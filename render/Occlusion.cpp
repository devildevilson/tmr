#include "Occlusion.h"

#include "VulkanRender2.h"
#include "yavf.h"

#include <array>

Occlusion::Occlusion() {}

Occlusion::~Occlusion() {
//   device->destroySemaphoreProxy(owner);
  device->destroy(finish);
//   vkDestroySemaphore(device->handle(), finish, nullptr);
  vkDestroyFramebuffer(device->handle(), frame, nullptr);
}

void Occlusion::create(yavf::Device* device, const uint32_t &width, const uint32_t &height) {
  this->device = device;
  
  VkFormat depthType = yavf::findSupportedFormat(device->physicalHandle(),
                                            {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                                            VK_IMAGE_TILING_OPTIMAL,
                                            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
  
  rect.offset.x = 0;
  rect.offset.y = 0;
  rect.extent.width = width;
  rect.extent.height = height;
  
  yavf::ImageCreateInfo info{
    0,
    VK_IMAGE_TYPE_2D,
    depthType,
    {width, height, 1},
    1, 1,
    VK_SAMPLE_COUNT_1_BIT,
    VK_IMAGE_TILING_OPTIMAL,
    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
    VK_IMAGE_ASPECT_DEPTH_BIT,
    VMA_MEMORY_USAGE_GPU_ONLY
  };
  
  depth = device->createImage(info);
  
  yavf::TransferTask* task = device->allocateTransferTask();

  task->begin();
  task->setBarrier(depth, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
  task->end();

  task->start();
  task->wait();

  device->deallocate(task);
  
  depth->createView(VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_DEPTH_BIT);
  
  createRenderpass();
  
//   VkSemaphoreCreateInfo semaforeInfo{
//     VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
//     nullptr,
//     0
//   };
//   
//   yavf::vkCheckError("vkCreateSemaphore", nullptr, 
//   vkCreateSemaphore(device->handle(), &semaforeInfo, nullptr, &finish));
  
  finish = device->createSemaphore();
  
  std::array<VkImageView, 1> attachments = {
    depth->view().handle
  };
  
  VkFramebufferCreateInfo framebufferCreateInfo{
    VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
    nullptr,
    0,
    pass,
    attachments.size(),
    attachments.data(),
    rect.extent.width,
    rect.extent.height,
    1
  };
  
  yavf::vkCheckError("vkCreateFramebuffer", nullptr, 
  vkCreateFramebuffer(device->handle(), &framebufferCreateInfo, nullptr, &frame));
  
  //owner = device->createSemaphoreProxy();
}

std::vector<VkClearValue> Occlusion::clearValues() const {
  return {{1.0f, 0}};
}

// VkImage Occlusion::image() const {
//   return VK_NULL_HANDLE;
// }

VkRect2D Occlusion::size() const {
  return rect;
}

VkRenderPass Occlusion::renderPass() const {
  return pass;
}

VkViewport Occlusion::viewport() const {
  return {
    0, 0,
    static_cast<float>(rect.extent.width), static_cast<float>(rect.extent.height),
    0.0, 1.0
  };
}

VkRect2D Occlusion::scissor() const {
  return rect;
}

VkFramebuffer Occlusion::framebuffer() const {
  return frame;
}

// VkSemaphore Occlusion::imageAvailable() const {
//   return VK_NULL_HANDLE;
// }
// 
// VkSemaphore Occlusion::finishRendering() const {
//   return finish;
// }

yavf::SemaphoreProxy* Occlusion::getSemaphoreProxy() const {
  return nullptr;
}

void Occlusion::addSemaphoreProxy(yavf::SemaphoreProxy* proxy) {
  (void)proxy;
}

void Occlusion::createRenderpass() {
  yavf::RenderPassMaker rpm(device);
  
  pass = rpm.attachmentBegin(depth->param().format)
              .attachmentLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR)
              .attachmentStoreOp(VK_ATTACHMENT_STORE_OP_STORE)
              .attachmentInitialLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
              .attachmentFinalLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
            .subpassBegin(VK_PIPELINE_BIND_POINT_GRAPHICS)
              .subpassDepthStencilAttachment(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 0)
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
            .create("occlusion_render_pass");
}
