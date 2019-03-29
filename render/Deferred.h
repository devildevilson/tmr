#ifndef DEFERRED_H
#define DEFERRED_H

#include "RenderTarget.h"

// namespace yavf {
//   class Device;
//   class Image_t;
//   typedef Image_t* Image;
// }

#include "yavf.h"

struct Target {
  yavf::Image* color = nullptr;
  yavf::Image* normal = nullptr;
  yavf::Image* depth = nullptr;
  yavf::DescriptorSet* desc = nullptr;
  yavf::Framebuffer buffer = VK_NULL_HANDLE;
  //yavf::Semaphore waiting = VK_NULL_HANDLE;
  //yavf::Semaphore finish = VK_NULL_HANDLE;
};

class Deferred : public yavf::RenderTarget {
public:
  Deferred();
  Deferred(yavf::Device* device, const uint32_t &count, const uint32_t &width, const uint32_t &height);
  virtual ~Deferred();
  
  void create(yavf::Device* device, const uint32_t &count, const uint32_t &width, const uint32_t &height);
  
  std::vector<VkClearValue> clearValues() const override;
//   VkImage image() const override;
  VkRect2D size() const override;
  yavf::RenderPass renderPass() const override;
  VkViewport viewport() const override;
  VkRect2D scissor() const override;

  yavf::Framebuffer framebuffer() const override;
//   VkSemaphore imageAvailable() const override;
//   VkSemaphore finishRendering() const override;
  //yavf::SemaphoreProxy* createSemaphoreProxy(const VkPipelineStageFlags &flag = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT) override;
//   yavf::SemaphoreProxy* getSemaphoreProxy() const override;
//   void addSemaphoreProxy(yavf::SemaphoreProxy* proxy) override;
  
  void nextframe();
  void resize(const uint32_t &width, const uint32_t &height);
  Target get() const;
  VkDescriptorSetLayout getSetLayout() const;
private:
  void createRenderpass();
  
  uint32_t width = 0, height = 0;
  
  uint32_t currentFrame = 0;
  yavf::Device* device = nullptr;
  yavf::RenderPass pass = VK_NULL_HANDLE;
  yavf::DescriptorSetLayout setLayout = VK_NULL_HANDLE;
  std::vector<Target> images;
  //std::vector<yavf::SemaphoreProxy*> proxies;
};

#endif
