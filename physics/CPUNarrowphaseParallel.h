#ifndef CPU_NARROWPHASE_PARALLEL_H
#define CPU_NARROWPHASE_PARALLEL_H

#include "NarrowphaseInterface.h"
#include "CPUArray.h"
#include "ThreadPool.h"
#include <pthread.h>

struct UniquePairContainer {
//   uint32_t* container;
  std::vector<uint32_t> container;
  
//   std::mutex mutex;
//   std::condition_variable writeVar;
//   std::condition_variable readVar;
//   std::atomic<uint32_t> atomicMutex;
  
  pthread_rwlock_t rw_lock;
  
  UniquePairContainer(); //const size_t &objCount
  ~UniquePairContainer();
  
  bool write(const uint32_t &first, const uint32_t &second);
  bool read(const uint32_t &first, const uint32_t &second);
  void resize(const size_t &objCount);
};

class CPUNarrowphaseParallel : public Narrowphase {
public:
  CPUNarrowphaseParallel(dt::thread_pool* pool, const uint32_t &octreeDepth);
  ~CPUNarrowphaseParallel();

  void setInputBuffers(const InputBuffers &inputs, void* indirectPairCount = nullptr) override;
  void setOutputBuffers(const OutputBuffers &outputs, void* indirectIslandCount = nullptr) override;

  void updateBuffers(const uint32_t &lastPairCount, const uint32_t &lastStaticPairCount) override;

  void calculateIslands() override; // сейчас у мня не используются острова и батчи вообще
  void calculateBatches() override;
  void checkIdenticalPairs() override; // используется только это
  void postCalculation() override;

  void printStats() override;
protected:
  dt::thread_pool* pool;
  
  uint32_t iterationCount;
  
  uint32_t octreeDepth;
  
  UniquePairContainer pairCont;

  ArrayInterface<BroadphasePair>* pairs = nullptr;
  ArrayInterface<BroadphasePair>* staticPairs = nullptr;

  ArrayInterface<IslandData>* islands = nullptr;
  ArrayInterface<IslandData>* staticIslands = nullptr;
  
  CPUArray<glm::uvec4> octreeLevels;
};

#endif // !CPU_NARROWPHASE_H
