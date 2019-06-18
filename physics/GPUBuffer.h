#ifndef GPU_BUFFER_H
#define GPU_BUFFER_H

#include "ArrayInterface.h"
#include "yavf.h"

template<typename T>
class GPUBuffer : public ArrayInterface<T> {
public:
  GPUBuffer() {}
  
  GPUBuffer(yavf::Device* device, const VkBufferUsageFlags &flags) {
    construct(device, flags);
  }
  
  GPUBuffer(yavf::Device* device, const VkBufferUsageFlags &flags, const uint32_t &size) {
    construct(device, flags, size);
  }

  ~GPUBuffer() {
    device->destroy(bufferPtr);
  }

  void construct(yavf::Device* device, const VkBufferUsageFlags &flags) {
    this->device = device;
    bufferPtr = device->create(yavf::BufferCreateInfo::buffer(1 * sizeof(T), flags), VMA_MEMORY_USAGE_CPU_ONLY);
    update();
  }
  
  void construct(yavf::Device* device, const VkBufferUsageFlags &flags, const uint32_t &size) {
    this->device = device;
    bufferPtr = device->create(yavf::BufferCreateInfo::buffer(size * sizeof(T), flags), VMA_MEMORY_USAGE_CPU_ONLY);
    
    update();
  }

  void resize(const uint32_t &size) override {
    bufferPtr->recreate(size * sizeof(T));
    
    update();
  }
  
  // тут надо бы вернуть буфер вместе с дескриптором
  void* gpu_buffer() const override {
//     yavf::Descriptor* d = (yavf::Descriptor*)ptr;
//     *d = bufferPtr->descriptor();
//     yavf::Buffer** buffer = reinterpret_cast<yavf::Buffer**>(ptr);
//     *buffer = bufferPtr;
    return bufferPtr;
  }

  void push_back(const T &value) override { (void)value; }

  yavf::Buffer* buffer() {
    return bufferPtr;
  }

  const yavf::Buffer* buffer() const {
    return bufferPtr;
  }

  void update() {
    ArrayInterface<T>::sizeVar = bufferPtr->info().size / sizeof(T);
    ArrayInterface<T>::ptr = (T*)bufferPtr->ptr();
  }
protected:
  yavf::Device* device = nullptr;
  yavf::Buffer* bufferPtr = nullptr;
};

#endif // !GPU_BUFFER_H
