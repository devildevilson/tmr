#ifndef INTERPOLATE_DATA_H
#define INTERPOLATE_DATA_H

#include "PhysicsTemporary.h"
#include "ArrayInterface.h"
#include "Utility.h"

#include "ThreadPool.h"

#include <cstring>

class IntepolateData {
public:
  virtual ~IntepolateData() {}
  
  virtual void updateLastData() = 0;
  virtual void interpolate(dt::thread_pool* pool, const float &alpha) = 0;
};

template <typename T>
class ContainerInterpolation : public IntepolateData {
public:
  struct CreateInfo {
    Container<T>* prevData;
    Container<T>* newData;
    Container<T>* interpolateData;
  };
  
  ContainerInterpolation(const CreateInfo &info) : prevData(info.prevData), newData(info.newData), interpolateData(info.interpolateData) {}
  ~ContainerInterpolation() {}
  
  void updateLastData() override {
    memcpy(prevData->data(), newData->data(), newData->size()*sizeof(T));
  }
  
  void interpolate(dt::thread_pool* pool, const float &alpha) override {
    static const auto interpolateFunc = [&] (const size_t &start, const size_t &count, const float &alpha) {
      for (size_t index = start; index < start+count; ++index) {
        if (physicsDatas[index].objectIndex == UINT32_MAX) return;

  //       const simd::vec4 vel = globalVel[index];

  //       const size_t sixteeFrames = 16667;
        const uint32_t &transIndex = physicsDatas[index].transformIndex;
        //transforms->at(transIndex).pos = transforms->at(transIndex).pos + vel*alpha*MCS_TO_SEC(sixteeFrames);
        transforms->at(transIndex).pos = simd::mix(prevState[transIndex].pos, currState[transIndex].pos, alpha);
        // потом добавится кватернион который мы будем slerp'ать
      }
    };
    
    const size_t count = std::ceil(float(physicsDatas.size()) / float(pool->size()+1));
    size_t start = 0;
    for (uint32_t i = 0; i < pool->size()+1; ++i) {
      const size_t jobCount = std::min(count, physicsDatas.size()-start);
      if (jobCount == 0) break;

      pool->submitnr(interpolateFunc, start, jobCount, alpha);

      start += jobCount;
    }

  //   for (uint32_t i = 0; i < physicsDatas.size(); ++i) {
  //     pool->submitnr(interpolateFunc, i, alpha);
  //   }

    pool->compute();
    pool->wait();
  }
  
  uint32_t insert(const T &value) {
    const uint32_t index1 = prevData->insert(value);
    const uint32_t index2 = newData->insert(value);
    const uint32_t index3 = interpolateData->insert(value);
    
    ASSERT(index1 == index2 == index3);
    
    return index1;
  }
  
  void erase(const uint32_t &index) {
    prevData->erase(index);
    newData->erase(index);
    interpolateData->erase(index);
  }
private:
  // в контейнерах не известно что определено а что нет
  
  Container<T>* prevData;
  Container<T>* newData;
  Container<T>* interpolateData;
};

#endif
