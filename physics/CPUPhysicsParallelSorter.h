#ifndef CPU_PHYSICS_PARALLEL_SORTER_H
#define CPU_PHYSICS_PARALLEL_SORTER_H

#include "PhysicsSorter.h"
#include "ThreadPool.h"

class CPUPhysicsParallelSorter : public PhysicsSorter {
public:
  CPUPhysicsParallelSorter(dt::thread_pool* pool);
  ~CPUPhysicsParallelSorter();

  void sort(ArrayInterface<BroadphasePair>* pairs, const uint32_t &algorithmIndex = 0) override;
  void sort(ArrayInterface<OverlappingData>* overlappingData, ArrayInterface<DataIndices>* dataIndixes, const uint32_t &algorithmIndex = 0) override;
  void barrier() override;

  void printStats() override;
private:
  dt::thread_pool* pool = nullptr;
  
  // тут что-нибудь вроде 
  //std::vector<std::function<void()>> sortAlgos;
};

#endif
