#ifndef IMAGE_RESOURCE_CONTAINER_H
#define IMAGE_RESOURCE_CONTAINER_H

#include "vulkan_lite.h"

VK_DEFINE_HANDLE(VkDescriptorSetLayout)

namespace yavf {
  class DescriptorSet;
  typedef VkDescriptorSetLayout DescriptorSetLayout;
}

class ImageResourceContainer {
public:
  virtual ~ImageResourceContainer() = default;
  
  virtual uint32_t imagesCount() const = 0;
  virtual uint32_t samplersCount() const = 0;

  virtual yavf::DescriptorSet* resourceDescriptor() const = 0;
  virtual yavf::DescriptorSetLayout resourceLayout() const = 0;
};

#endif
