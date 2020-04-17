#ifndef GPU_ARRAY_H
#define GPU_ARRAY_H

#include "ArrayInterface.h"
#include "yavf.h"

//#include "Utility.h"
//
//#include <execinfo.h>
//#include <csignal>
//#include <unistd.h>

template <typename T>
class GPUArray : public ArrayInterface<T> {
public:
  GPUArray() {}
  
  GPUArray(yavf::Device* device) : array(device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) {
    update();
  }
  
  GPUArray(yavf::Device* device, const VkBufferUsageFlags &usage) : array(device, usage) {
    update();
  }
  
  GPUArray(yavf::Device* device, const VkBufferUsageFlags &usage, const uint32_t &size) : array(device, usage, size) {
    //throw std::runtime_error("FIX THIS");
    update();
  }
  
  ~GPUArray() {}
  
  void construct(yavf::Device* device) {
    array.construct(device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    update();
  }
  
  void construct(yavf::Device* device, const uint32_t &size) {
    throw std::runtime_error("FIX THIS");
    array.construct(device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, size);
    update();
  }
  
  void construct(yavf::Device* device, const uint32_t &usage, const uint32_t &size) {
    array.construct(device, usage, size);
    update();
  }
  
  void resize(const uint32_t &size) override {
//    std::cout << "\n";

//    PRINT_VAR("array buffer", array.handle()->handle())
//    PRINT_VAR("array resize", size)

    array.resize(size);
    update();
  }
  
  // тут надо бы вернуть буфер вместе с дескриптором
  void* gpu_buffer() const override {
//     yavf::Descriptor* d = (yavf::Descriptor*)ptr;
//     yavf::Buffer** buffer = reinterpret_cast<yavf::Buffer**>(ptr);
//     *buffer = array.handle();
    return array.handle();
  }

  void push_back(const T &value) override {
    array.push_back(value);
    update();
  }
  
  yavf::vector<T> & vector() {
    return array;
  }
  
  const yavf::vector<T> & vector() const {
    return array;
  }
  
  void update() {
    ArrayInterface<T>::sizeVar = array.size();
    ArrayInterface<T>::ptr = array.data();
  }
protected:
  yavf::vector<T> array;
};

template <typename T>
class GPUContainer : public Container<T> {
public:
  GPUContainer() {}
  
  GPUContainer(yavf::Device* device) : array(device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) {
    update();
  }
  
  GPUContainer(yavf::Device* device, const uint32_t &size) : array(device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, size) {
    update();
  }
  
  ~GPUContainer() {}
  
  void construct(yavf::Device* device) {
    array.construct(device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    update();
  }
  
  void construct(yavf::Device* device, const uint32_t &size) {
    array.construct(device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, size);
    update();
  }
  
  void resize(const uint32_t &size) override {
    array.resize(size);
    update();
  }
  
  // тут надо бы вернуть буфер вместе с дескриптором
  void* gpu_buffer() const override {
//     yavf::Descriptor* d = (yavf::Descriptor*)ptr;
//     *d = array.descriptor();
//     yavf::Buffer** buffer = reinterpret_cast<yavf::Buffer**>(ptr);
//     *buffer = array.handle();
    return array.handle();
  }

  void push_back(const T &value) override {
    array.push_back(value);
    update();
  }

  uint32_t insert(const T &value) override {
    if (Container<T>::freeIndex != UINT32_MAX) {
      const uint32_t index = Container<T>::freeIndex;
      Container<T>::freeIndex = reinterpret_cast<Slot<T>&>(array[Container<T>::freeIndex]).nextIndex;
      reinterpret_cast<Slot<T>*>(&array[index])->value = value;
      return index;
    }

    const uint32_t index = array.size();
    array.push_back(value);
    update();
    return index;
  }

  void erase(const uint32_t &index) override {
    reinterpret_cast<Slot<T>*>(&array[index])->value.~T();
    reinterpret_cast<Slot<T>*>(&array[index])->nextIndex = Container<T>::freeIndex;
    Container<T>::freeIndex = index;
  }
  
  yavf::vector<T> & vector() {
    return array;
  }
  
  const yavf::vector<T> & vector() const {
    return array;
  }
  
  void update() {
    ArrayInterface<T>::sizeVar = array.size();
    ArrayInterface<T>::ptr = array.data();
  }
protected:
  yavf::vector<T> array;
};

#endif
