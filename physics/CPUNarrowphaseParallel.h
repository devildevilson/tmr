#ifndef CPU_NARROWPHASE_PARALLEL_H
#define CPU_NARROWPHASE_PARALLEL_H

#include "NarrowphaseInterface.h"
#include "CPUArray.h"
#include "ThreadPool.h"

class CPUNarrowphaseParallel : public Narrowphase {
public:
  CPUNarrowphaseParallel(dt::thread_pool* pool, const uint32_t &octreeDepth);
  virtual ~CPUNarrowphaseParallel();

  void setInputBuffers(const InputBuffers &inputs, void* indirectPairCount = nullptr) override;
  void setOutputBuffers(const OutputBuffers &outputs, void* indirectIslandCount = nullptr) override;

  void updateBuffers(const uint32_t &lastPairCount, const uint32_t &lastStaticPairCount) override;

  void calculateIslands() override; // сейчас у мня не используются острова и батчи вообще
  void calculateBatches() override;
  void checkIdenticalPairs() override; // используется только это
  void postCalculation() override;

  void printStats() override;
protected:
  dt::thread_pool* pool = nullptr;
  
  uint32_t iterationCount = 50;
  
  uint32_t octreeDepth = 0;

  ArrayInterface<BroadphasePair>* pairs = nullptr;
  ArrayInterface<BroadphasePair>* staticPairs = nullptr;

  ArrayInterface<IslandData>* islands = nullptr;
  ArrayInterface<IslandData>* staticIslands = nullptr;
  
  CPUArray<glm::uvec4> octreeLevels;
};

#endif // !CPU_NARROWPHASE_H
