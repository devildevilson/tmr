#ifndef GPU_PHYSICS_SORTER_H
#define GPU_PHYSICS_SORTER_H

#include "PhysicsSorter.h"
#include "yavf.h"
#include <vector>
#include <string>

struct GPUPhysicsSorterCreateInfo {
  std::vector<std::string> pairAlgo;
  std::vector<std::string> overlappingAlgo;
};

class GPUPhysicsSorter : public PhysicsSorter {
public:
  GPUPhysicsSorter(yavf::Device* device, yavf::ComputeTask* task, const GPUPhysicsSorterCreateInfo &info);
  virtual ~GPUPhysicsSorter();

  void sort(ArrayInterface<BroadphasePair>* pairs, const uint32_t &algorithmIndex = 0) override;
  void sort(ArrayInterface<OverlappingData>* overlappingData, ArrayInterface<DataIndices>* dataIndixes, const uint32_t &algorithmIndex = 0) override;
  void barrier() override;

  void printStats() override;
protected:
  yavf::Device* device = nullptr;
  yavf::ComputeTask* task = nullptr;

  std::vector<yavf::Pipeline> pairAlgos;
  std::vector<yavf::Pipeline> overlappingAlgos;

  void construct(yavf::Device* device, yavf::ComputeTask* task, const GPUPhysicsSorterCreateInfo &info);
};

class GPUPhysicsSorterDirectPipeline : public GPUPhysicsSorter {
public:
  GPUPhysicsSorterDirectPipeline(yavf::Device* device, yavf::ComputeTask* task, const GPUPhysicsSorterCreateInfo &info);
  virtual ~GPUPhysicsSorterDirectPipeline();

  void sort(ArrayInterface<BroadphasePair>* pairs, const uint32_t &algorithmIndex = 0) override;
  void sort(ArrayInterface<OverlappingData>* overlappingData, ArrayInterface<DataIndices>* dataIndixes, const uint32_t &algorithmIndex = 0) override;
  void barrier() override;
};

#endif // !GPU_PHYSIC_SORTER_H