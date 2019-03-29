#ifndef CPU_ARRAY_H
#define CPU_ARRAY_H

#include "ArrayInterface.h"
#include <vector>

// всем этим вещам возможно неплохо было бы дать инструменты для синхронизации
// и затем сделать параллельное добавление объектов в разные места

template <typename T>
class CPUArray : public ArrayInterface<T> {
public:
  CPUArray() {
    update();
  }
  CPUArray(const uint32_t &size) {
    array.resize(size);
    update();
  }

  ~CPUArray() {}
  
  void resize(const uint32_t &size) override {
    array.resize(size);
    update();
  }

  void descriptorPtr(void* ptr) const override { (void)ptr; }
  void push_back(const T &value) override { 
    array.push_back(value);
    update();
  }

  std::vector<T> & vector() {
    return array;
  }
  
  const std::vector<T> & vector() const {
    return array;
  }
  
  void update() {
    ArrayInterface<T>::sizeVar = array.size();
    ArrayInterface<T>::ptr = array.data();
  }
protected:
  std::vector<T> array;
};

template <typename T>
class CPUContainer : public Container<T> {
public:
  CPUContainer() {}
  
  CPUContainer(const uint32_t &size) {
    array.resize(size);
  }
  
  ~CPUContainer() {}
  
  void resize(const uint32_t &size) override {
    array.resize(size);
    
    update();
  }
  
  // тут надо бы вернуть буфер вместе с дескриптором
  void descriptorPtr(void* ptr) const override {
    (void)ptr;
  }

  void push_back(const T &value) override {
    array.push_back(value);
    update();
  }

  uint32_t insert(const T &value) override {
    if (Container<T>::freeIndex != UINT32_MAX) {
      const uint32_t index = Container<T>::freeIndex;
      Container<T>::freeIndex = reinterpret_cast<Slot<T>*>(&array[Container<T>::freeIndex])->nextIndex;
      reinterpret_cast<Slot<T>*>(&array[Container<T>::freeIndex])->value = value;
      
//       Container<T>::freeIndex = reinterpret_cast<Slot<T>&>(array[Container<T>::freeIndex]).nextIndex;
//       reinterpret_cast<Slot<T>&>(array[Container<T>::freeIndex]).value = value;
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
    
//     reinterpret_cast<Slot<T>&>(array[index]).value.~T();
//     reinterpret_cast<Slot<T>&>(array[index]).nextIndex = Container<T>::freeIndex;
    Container<T>::freeIndex = index;
  }
  
  std::vector<T> & vector() {
    return array;
  }
  
  const std::vector<T> & vector() const {
    return array;
  }
  
  void update() {
    ArrayInterface<T>::sizeVar = array.size();
    ArrayInterface<T>::ptr = reinterpret_cast<T*>(array.data());
  }
protected:
  std::vector<T> array;
};

#endif
