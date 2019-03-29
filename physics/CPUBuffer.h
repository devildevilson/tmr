#ifndef CPU_BUFFER_H
#define CPU_BUFFER_H

#include "ArrayInterface.h"

template<typename T>
class CPUBuffer : public ArrayInterface<T> {
public:
  CPUBuffer() {
    ArrayInterface<T>::sizeVar = 1;
    ArrayInterface<T>::ptr = &dataBuf;
  }
  virtual ~CPUBuffer() {}

  void resize(const uint32_t &size) override { (void)size; }
  void descriptorPtr(void* ptr) const override { (void)ptr; }
  void push_back(const T &value) override { (void)value; }
private:
  T dataBuf;
};

#endif // !CPU_BUFFER_H
