#ifndef PHYSIC_SORTER_H
#define PHYSIC_SORTER_H

#include "ArrayInterface.h"
#include "PhysicsUtils.h"

class PhysicsSorter {
public:
  virtual ~PhysicsSorter() {}

  virtual void sort(ArrayInterface<BroadphasePair>* pairs, const uint32_t &algorithmIndex = 0) = 0;
  virtual void sort(ArrayInterface<OverlappingData>* overlappingData, ArrayInterface<DataIndices>* dataIndixes, const uint32_t &algorithmIndex = 0) = 0;
  virtual void barrier() = 0;

  virtual void printStats() = 0;
};

#endif // !PHYSIC_SORTER_H