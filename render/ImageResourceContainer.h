#ifndef IMAGE_RESOURCE_CONTAINER_H
#define IMAGE_RESOURCE_CONTAINER_H

#include "yavf.h"

class ImageResourceContainer {
public:
  virtual ~ImageResourceContainer() {}
  
  // мне где то нужно почекать сколько у меня всего памяти и сколько текстурки или объекты занимают места
  // здесь или не здесь?
  // вообще этот интерфейс нужен для рендера, которым все равно на размеры
  // то есть наверное не здесь
  //virtual VkDeviceSize deviceSize() = 0;
  
  virtual uint32_t imageCount() const = 0;
  virtual yavf::DescriptorSet* imageDescriptor() const = 0;
  virtual yavf::DescriptorSetLayout imageSetLayout() const = 0;
  
  virtual uint32_t samplerCount() const = 0;
  virtual yavf::DescriptorSet* samplerDescriptor() const = 0;
  virtual yavf::DescriptorSetLayout samplerSetLayout() const = 0;
};

#endif
