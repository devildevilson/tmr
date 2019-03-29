#ifndef CPU_PHYSICS_SORTER_H
#define CPU_PHYSICS_SORTER_H

#include "PhysicsSorter.h"
#include <vector>
#include <string>

// алгоритм?

class CPUPhysicsSorter : public PhysicsSorter {
public:
  CPUPhysicsSorter();
  ~CPUPhysicsSorter();

  void sort(ArrayInterface<BroadphasePair>* pairs, const uint32_t &algorithmIndex = 0) override;
  void sort(ArrayInterface<OverlappingData>* overlappingData, ArrayInterface<DataIndices>* dataIndixes, const uint32_t &algorithmIndex = 0) override;
  void barrier() override;

  void printStats() override;
private:
  // тут что-нибудь вроде 
  //std::vector<std::function<void()>> sortAlgos;
};

#endif
