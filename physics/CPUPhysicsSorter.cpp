#include "CPUPhysicsSorter.h"

#include <chrono>
#include <iostream>
#include <stdexcept>
#include <algorithm>

CPUPhysicsSorter::CPUPhysicsSorter() {}
CPUPhysicsSorter::~CPUPhysicsSorter() {}

struct PairCompare1 {
  bool operator() (const BroadphasePair &first, const BroadphasePair &second) const {
    if (first.islandIndex < second.islandIndex) return true;
    else if (first.islandIndex == second.islandIndex) {
      if (first.dist < second.dist) return true;
    }

    return false;
  }
};

struct PairCompare2 {
  bool operator() (const BroadphasePair &first, const BroadphasePair &second) const {
    if (first.firstIndex < second.firstIndex) return true;
    else if (first.firstIndex == second.firstIndex) {
      if (first.secondIndex < second.secondIndex) return true;
    }

    return false;
  }
};

struct OverlappingDataCompare1 {
  bool operator() (const OverlappingData &first, const OverlappingData &second) const {
    if (first.firstIndex < second.firstIndex) return true;
    else if (first.firstIndex == second.firstIndex) {
      if (first.secondIndex < second.secondIndex) return true;
    }

    return false;
  }
};

struct OverlappingDataCompare2 {
  bool operator() (const OverlappingData &first, const OverlappingData &second) const {
    if (first.firstIndex < second.firstIndex) return true;
    else if (first.firstIndex == second.firstIndex) {
      if (first.dist < second.dist) return true;
    }

    return false;
  }
};

void CPUPhysicsSorter::sort(ArrayInterface<BroadphasePair>* pairs, const uint32_t &algorithmIndex) {
  //std::cout << "pairs count " << pairs->at(0).firstIndex << "\n";
  
  if (pairs->at(0).firstIndex < 2) return;
  
  //throw std::runtime_error("CPU sort");
  
//   RegionLog rl("Sort pairs");

  if (algorithmIndex > 1) throw std::runtime_error("CPU sort not fully implemented");
  
  if (algorithmIndex == 0) {
    const glm::uvec4* pairsCount = pairs->structure_from_begin<glm::uvec4>();
    BroadphasePair* pairsPtr = pairs->data_from<glm::uvec4>();
    
    //std::sort(pairs->data()+1, pairs->data()+pairs->at(0).islandIndex+1, PairCompare1());
    std::sort(pairsPtr, pairsPtr+pairsCount->x, PairCompare1());
  } else {
    const glm::uvec4* pairsCount = pairs->structure_from_begin<glm::uvec4>();
    BroadphasePair* pairsPtr = pairs->data_from<glm::uvec4>();
    
    std::sort(pairsPtr, pairsPtr+pairsCount->x, PairCompare2());
  }
}

void CPUPhysicsSorter::sort(ArrayInterface<OverlappingData>* overlappingData, ArrayInterface<DataIndices>* dataIndixes, const uint32_t &algorithmIndex) {
//   RegionLog rl("Sort overlapping data");
  
  if (dataIndixes->data()->count < 2) return;

  if (algorithmIndex == 0) {
    std::sort(overlappingData->data(), overlappingData->data()+dataIndixes->data()->count, OverlappingDataCompare1());
  } else if (algorithmIndex == 1) {
    std::sort(overlappingData->data(), overlappingData->data()+dataIndixes->data()->count, OverlappingDataCompare2());
  } else {
    throw std::runtime_error("CPU sort not fully implemented");
  }
}

void CPUPhysicsSorter::barrier() {}

void CPUPhysicsSorter::printStats() {
  std::cout << "CPU sorter" << '\n';
}
