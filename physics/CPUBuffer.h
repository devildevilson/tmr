#ifndef CPU_BUFFER_H
#define CPU_BUFFER_H

#include "ArrayInterface.h"

template<typename T>
class CPUBuffer : public ArrayInterface<T> {
public:
  CPUBuffer() {
    dataBuf = new T();
    
    ArrayInterface<T>::sizeVar = 1;
    ArrayInterface<T>::ptr = dataBuf;
  }
  ~CPUBuffer() {
    delete dataBuf;
  }

  void resize(const uint32_t &size) override { (void)size; }
  void* gpu_buffer() const override { return nullptr; }
  void push_back(const T &value) override { (void)value; }
private:
  T* dataBuf;
};

#endif // !CPU_BUFFER_H
