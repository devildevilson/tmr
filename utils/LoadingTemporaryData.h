#ifndef LOADING_TEMPORARY_DATA_H
#define LOADING_TEMPORARY_DATA_H

#include <vector>
#include "MemoryPool.h"

template <typename T, size_t N>
struct LoadingTemporaryData {
  ~LoadingTemporaryData() {
    for (auto ptr : dataPtr) {
      dataPool.deleteElement(ptr);
    }
  }
  
  template <typename ...Args>
  T* create(Args&&... args) {
    T* ptr = dataPool.newElement(std::forward<Args>(args)...);
    dataPtr.push_back(ptr);
    return ptr;
  }
  
  MemoryPool<T, sizeof(T)*N> dataPool;
  std::vector<T*> dataPtr;
  std::vector<T*> dataToLoad;
};

#endif
