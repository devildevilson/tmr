#include "Deferred.h"

#include <array>

Deferred::Deferred() {}

Deferred::Deferred(yavf::Device* device, const uint32_t &count, const uint32_t &width, const uint32_t &height) {
  create(device, count, width, height);
}

Deferred::~Deferred() {
  device->destroySampler("deferred_sampler_name");

  for (uint32_t i = 0; i < images.size(); ++i) {
//     if (images[currentFrame].buffer.handle() != VK_NULL_HANDLE) {
//       vkDestroyFramebuffer(device->handle(), images[currentFrame].buffer, nullptr);
//       images[currentFrame].buffer = VK_NULL_HANDLE;
//     }
    
    device->destroy(images[currentFrame].buffer);
    
    device->destroy(images[i].color);
    device->destroy(images[i].normal);
    device->destroy(images[i].depth);
    
//     if (images[currentFrame].finish != VK_NULL_HANDLE) {
//       vkDestroySemaphore(device->handle(), images[currentFrame].finish, nullptr);
//       images[currentFrame].finish = VK_NULL_HANDLE;
//     }
  }
}

void Deferred::create(yavf::Device* device, const uint32_t &count, const uint32_t &width, const uint32_t &height) {
  this->width = width;
  this->height = height;
  this->device = device;
  
  VkFormat depth = yavf::findSupportedFormat(device->physicalHandle(),
                                            {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                                            VK_IMAGE_TILING_OPTIMAL,
                                            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
  
  {
    yavf::DescriptorLayoutMaker dlm(device);
    
    setLayout = dlm.binding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT)
                   .binding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT)
                   .binding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT).create("compute_set_layout");
  }
  
  yavf::DescriptorMaker dm(device);
  VkDescriptorPool pool = device->descriptorPool("default_descriptor_pool");
//   yavf::DescriptorUpdater du(device);
  
  yavf::SamplerMaker sm(device);
  yavf::Sampler s = sm.filter(VK_FILTER_NEAREST, VK_FILTER_NEAREST)
                      .addressMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
                      .anisotropy(VK_FALSE)
                      .borderColor(VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE)
                      .create("deferred_sampler_name");
  
//   std::cout << "Width: " << width << " height: " << height << "\n";
//   std::cout << "Deffered creating" << "\n";
  
  images.resize(count);
  for (uint32_t i = 0; i < images.size(); ++i) {
    images[i].desc = dm.layout(setLayout).create(pool)[0];
    
    {
      images[currentFrame].color = device->create(yavf::ImageCreateInfo::texture2D({width, height}, 
                                                                                   VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT), 
                                                  VMA_MEMORY_USAGE_GPU_ONLY);
      yavf::ImageView* view = images[currentFrame].color->createView(VK_IMAGE_VIEW_TYPE_2D, {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});
      images[i].desc->add({s, view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER});
    }
    
    {
      images[currentFrame].normal = device->create(yavf::ImageCreateInfo::texture2D({width, height}, 
                                                                                   VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                                                                   VK_FORMAT_R16G16_UNORM), 
                                                  VMA_MEMORY_USAGE_GPU_ONLY);
      yavf::ImageView* view = images[currentFrame].normal->createView(VK_IMAGE_VIEW_TYPE_2D, {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});
      images[i].desc->add({s, view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER});
    }
    
    {
      images[currentFrame].depth = device->create(yavf::ImageCreateInfo::texture2D({width, height}, 
                                                                                   VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                                                                   depth), 
                                                  VMA_MEMORY_USAGE_GPU_ONLY);
      yavf::ImageView* view = images[currentFrame].depth->createView(VK_IMAGE_VIEW_TYPE_2D, {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1});
      images[i].desc->add({s, view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER});
    }

    images[i].desc->update();
  }
  
  createRenderpass();
  
  for (uint32_t i = 0; i < images.size(); ++i) {
    const std::vector<VkImageView> views = {
      images[i].color->view()->handle(),
      images[i].normal->view()->handle(),
      images[i].depth->view()->handle(),
    };
    
    images[i].buffer = device->create(yavf::FramebufferCreateInfo::framebuffer(pass, views.size(), views.data(), width, height), "deferred_framebuffer_"+std::to_string(i));
  }
  
//   std::cout << "Deffered creating end" << "\n";
}

std::vector<VkClearValue> Deferred::clearValues() const {
  return {{0.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0}};
}

// VkImage Deferred::image() const {
//   return VK_NULL_HANDLE;
// }

VkRect2D Deferred::size() const {
  VkRect2D r{
    {0, 0},
    {width, height}
  };
  
  return r;
}

VkRenderPass Deferred::renderPass() const {
  return pass;
}

VkViewport Deferred::viewport() const {
  VkViewport port{
    0.0f, 0.0f, 
    static_cast<float>(width),
    static_cast<float>(height),
    0.0f, 1.0f
  };
  
  return port;
}

VkRect2D Deferred::scissor() const {
  VkRect2D r{
    {0, 0},
    {width, height}
  };
  
  return r;
}

yavf::Framebuffer Deferred::framebuffer() const {
  return images[currentFrame].buffer;
}

// VkSemaphore Deferred::imageAvailable() const {
// //   return images[currentFrame].waiting;
//   return VK_NULL_HANDLE;
// }
// 
// VkSemaphore Deferred::finishRendering() const {
//   return images[currentFrame].finish;
// }

// yavf::SemaphoreProxy* Deferred::getSemaphoreProxy() const {
//   return nullptr;
// }
// 
// void Deferred::addSemaphoreProxy(yavf::SemaphoreProxy* proxy) {
//   (void)proxy;
// }

void Deferred::nextframe() {
  currentFrame = (currentFrame + 1) % images.size();
  
//   if (images[currentFrame].buffer != VK_NULL_HANDLE) {
//     vkDestroyFramebuffer(device->handle(), images[currentFrame].buffer, nullptr);
//     images[currentFrame].buffer = VK_NULL_HANDLE;
//   }
//   
//   const std::array<VkImageView, 3> attachments = {images[currentFrame].color->view().handle, images[currentFrame].normal->view().handle, images[currentFrame].depth->view().handle};
//   
//   const VkFramebufferCreateInfo info{
//     VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
//     nullptr,
//     0,
//     pass,
//     attachments.size(),
//     attachments.data(),
//     width,
//     height,
//     1
//   };
//   
// //   std::cout << "Deferred next frame" << "\n";
//   
//   yavf::vkCheckError("vkCreateFramebuffer", nullptr, 
//   vkCreateFramebuffer(device->handle(), &info, nullptr, &images[currentFrame].buffer));
}

void Deferred::resize(const uint32_t &width, const uint32_t &height) {
  if (this->width == width && this->height == height) return;
  this->width = width;
  this->height = height;
  
  for (uint32_t i = 0; i < images.size(); ++i) {
    images[i].color->recreate({width, height, 1});
    images[i].normal->recreate({width, height, 1});
    images[i].depth->recreate({width, height, 1});
    
    images[i].desc->update();
    
    device->destroyFramebuffer("deferred_framebuffer_"+std::to_string(i));
    
    const std::vector<VkImageView> views = {
      images[i].color->view()->handle(),
      images[i].normal->view()->handle(),
      images[i].depth->view()->handle()
    };
    images[i].buffer = device->create(yavf::FramebufferCreateInfo::framebuffer(pass, views.size(), views.data(), width, height), "deferred_framebuffer_"+std::to_string(i));
  }
}

Target Deferred::get() const {
  return images[currentFrame];
}

VkDescriptorSetLayout Deferred::getSetLayout() const {
  return setLayout;
}

void Deferred::createRenderpass() {
  yavf::RenderPassMaker rpm(device);
  
  pass = rpm.attachmentBegin(images[0].color->info().format)
              .attachmentLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR)
              .attachmentStoreOp(VK_ATTACHMENT_STORE_OP_STORE)
              .attachmentInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
              .attachmentFinalLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
              //.attachmentSamples(VK_SAMPLE_COUNT_8_BIT)
            .attachmentBegin(images[0].normal->info().format)
              .attachmentLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR)
              .attachmentStoreOp(VK_ATTACHMENT_STORE_OP_STORE)
              .attachmentInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
              .attachmentFinalLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
              //.attachmentSamples(VK_SAMPLE_COUNT_1_BIT)
            .attachmentBegin(images[0].depth->info().format)
              .attachmentLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR)
              .attachmentStoreOp(VK_ATTACHMENT_STORE_OP_STORE)
              .attachmentInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
              .attachmentFinalLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
              //.attachmentSamples(VK_SAMPLE_COUNT_1_BIT)
            .subpassBegin(VK_PIPELINE_BIND_POINT_GRAPHICS)
              .subpassColorAttachment(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 0)
              .subpassColorAttachment(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1)
              .subpassDepthStencilAttachment(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 2)
            .dependencyBegin(VK_SUBPASS_EXTERNAL, 0)
              .dependencySrcStageMask(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT)
              .dependencyDstStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)
              .dependencySrcAccessMask(VK_ACCESS_MEMORY_READ_BIT)
              .dependencyDstAccessMask(VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
            .dependencyBegin(0, VK_SUBPASS_EXTERNAL)
              .dependencySrcStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)
              .dependencyDstStageMask(VK_PIPELINE_BIND_POINT_COMPUTE)
              .dependencySrcAccessMask(VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
              .dependencyDstAccessMask(VK_ACCESS_MEMORY_READ_BIT)
            .create("deferred_render_pass");
}
