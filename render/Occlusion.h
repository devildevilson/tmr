#ifndef OCCLUSION_H
#define OCCLUSION_H

#include "RenderTarget.h"

class VulkanRender;

namespace yavf {
  class Device;
  class Image;
};

class Occlusion : public yavf::RenderTarget {
public:
  Occlusion();
  virtual ~Occlusion();
  
  void create(yavf::Device* device, const uint32_t &width, const uint32_t &height);
  
  std::vector<VkClearValue> clearValues() const override;
//   VkImage image() const override;
  VkRect2D size() const override;
  VkRenderPass renderPass() const override;
  VkViewport viewport() const override;
  VkRect2D scissor() const override;

  VkFramebuffer framebuffer() const override;
//   VkSemaphore imageAvailable() const override;
//   VkSemaphore finishRendering() const override;
  
  yavf::SemaphoreProxy* getSemaphoreProxy() const override;
  void addSemaphoreProxy(yavf::SemaphoreProxy* proxy) override;
private:
  void createRenderpass();
  
  VkRect2D rect;
  VkSemaphore finish = VK_NULL_HANDLE;
  VkRenderPass pass = VK_NULL_HANDLE;
  VkFramebuffer frame = VK_NULL_HANDLE;
  yavf::Device* device = nullptr;
  yavf::Image* depth = nullptr;
  
  //yavf::SemaphoreOwner* owner = nullptr;
};

#endif
