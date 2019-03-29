#ifndef CPU_PARALLEL_NARROWPHASE_H
#define CPU_PARALLEL_NARROWPHASE_H

#include "NarrowphaseInterface.h"
#include "CPUArray.h"

class CPUNarrowphase : public Narrowphase {
public:
  CPUNarrowphase(const uint32_t &octreeDepth);
  virtual ~CPUNarrowphase();

  //void setPairBuffer(ArrayInterface<BroadphasePair>* buffer, void* indirectPairCount = nullptr) override;
  void setInputBuffers(const InputBuffers &inputs, void* indirectPairCount = nullptr) override;
  void setOutputBuffers(const OutputBuffers &outputs, void* indirectIslandCount = nullptr) override;

  void updateBuffers(const uint32_t &lastPairCount, const uint32_t &lastStaticPairCount) override;

  void calculateIslands() override; // либо острова либо батчи
  void calculateBatches() override;
  void checkIdenticalPairs() override;
  void postCalculation() override;

  // здесь возвращать будем буфер констраинтов (пока нет)
//   ArrayInterface<IslandData>* getIslandDataBuffer() override;
//   const ArrayInterface<IslandData>* getIslandDataBuffer() const override;

  void printStats() override;
protected:
  uint32_t iterationCount = 50;
  
  uint32_t octreeDepth = 0;

  ArrayInterface<BroadphasePair>* pairs = nullptr;
  ArrayInterface<BroadphasePair>* staticPairs = nullptr;
//   CPUArray<IslandData> islands;
//   CPUArray<IslandData> staticIslands;
  ArrayInterface<IslandData>* islands = nullptr;
  ArrayInterface<IslandData>* staticIslands = nullptr;
  CPUArray<glm::uvec4> octreeLevels;
};

#endif // !CPU_NARROWPHASE_H
