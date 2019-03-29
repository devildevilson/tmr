#ifndef GPU_NARROWPHASE_H
#define GPU_NARROWPHASE_H

#include "NarrowphaseInterface.h"
#include "GPUArray.h"

#include "yavf.h"

struct InternalGPUNarrowphaseData {
  uint32_t islandIterationCount;
  uint32_t workGroupSizeIslandCalc;
  uint32_t workGroupSizeBatching;
  uint32_t workGroupSizeSamePairsChecking;
  uint32_t workGroupSizeSorting;
  uint32_t workGroupSizeCalcIslandData;
};

class GPUNarrowphase : public Narrowphase {
public:
  GPUNarrowphase(yavf::Device* device, yavf::ComputeTask* task, const InternalGPUNarrowphaseData* data = nullptr);
  virtual ~GPUNarrowphase();

//   void setPairBuffer(ArrayInterface<BroadphasePair>* buffer, void* indirectPairCount = nullptr) override;
  void setInputBuffers(const InputBuffers &inputs, void* indirectPairCount = nullptr) override;
  void setOutputBuffers(const OutputBuffers &outputs, void* indirectIslandCount = nullptr) override;
  // для вычисления констреинтов еще потребуются буферы с данными объекта и прочие

  void updateBuffers(const uint32_t &lastPairCount, const uint32_t &lastStaticPairCount) override;

  void calculateIslands() override; // либо острова либо батчи
  void calculateBatches() override;
  void checkIdenticalPairs() override;
  //void sortPairs() override;
  void postCalculation() override;

  // здесь возвращать будем буфер констраинтов

//   ArrayInterface<IslandData>* getIslandDataBuffer() override;
//   const ArrayInterface<IslandData>* getIslandDataBuffer() const override;

  void* getIndirectIslandCount() override;

  void printStats() override;
protected:
  yavf::Device* device = nullptr;
  yavf::ComputeTask* task = nullptr;

  yavf::DescriptorSet* pairsDescriptor;
  yavf::DescriptorSet* staticPairsDescriptor;
  //ArrayInterface<BroadphasePair>* pairArray = nullptr;
  yavf::Buffer* indirectPairCount = nullptr;
  yavf::Buffer* indirectIslandCount = nullptr;

  yavf::Pipeline islandCalc;
  yavf::Pipeline batchCalc;
  yavf::Pipeline sameCalc;
  yavf::Pipeline sortingPairs;
  yavf::Pipeline calcIslandData;

  GPUArray<IslandData> islands;

  void construct(yavf::Device* device, yavf::ComputeTask* task, const InternalGPUNarrowphaseData* data = nullptr);
};

class GPUNarrowphaseDirectPipeline : public GPUNarrowphase {
public:
  GPUNarrowphaseDirectPipeline(yavf::Device* device, yavf::ComputeTask* task, const InternalGPUNarrowphaseData* data = nullptr);
  virtual ~GPUNarrowphaseDirectPipeline();

  void calculateIslands() override; // либо острова либо батчи
  void calculateBatches() override;
  void checkIdenticalPairs() override;
  //void sortPairs() override;
  void postCalculation() override;
};

#endif // !GPU_NARROWPHASE_H
