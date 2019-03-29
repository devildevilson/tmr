#ifndef GPU_SOLVER_H
#define GPU_SOLVER_H

#include "Solver.h"
#include "yavf.h"
#include "GPUArray.h"
#include "GPUBuffer.h"

struct InternalGPUSolverData {
  uint32_t workGroupSizeSolverData;
};

// солвер нужно будет переделать, как?
// разделить вычисление мтв-дист и углов? (тип перевычислить положение, вычислить мтв-дист, вычислить новую скорость-положение-и-прочее)
// но я уже один раз что-то подобное пытался сделать, но безуспешно
class GPUSolver : public Solver {
public:
  struct OverlappingDataForSolver {
    glm::uvec4 pairData; // w == mtvAngle
    glm::vec4 mtvDist;

    glm::vec4 normals[2];
    glm::vec4 satAngleStair;
    glm::uvec4 stairsMoves;
  };

  GPUSolver(yavf::Device* device, yavf::ComputeTask* task, const InternalGPUSolverData* data = nullptr);
  virtual ~GPUSolver();

  //void setBuffers(const SolverBuffers &buffers, void* indirectIslandCount = nullptr, void* indirectPairCount = nullptr) override;
  void setInputBuffers(const InputBuffers &buffers, void* indirectIslandCount = nullptr, void* indirectPairCount = nullptr) override;
  void setOutputBuffers(const OutputBuffers &buffers) override;

  void updateBuffers(const uint32_t &pairsCount, const uint32_t &rayPairs) override;

  void calculateData() override;
  void calculateRayData() override;
  void solve() override;

//   ArrayInterface<OverlappingData>* getOverlappingData() override;
//   const ArrayInterface<OverlappingData>* getOverlappingData() const override;
// 
//   ArrayInterface<DataIndices>* getDataIndices() override;
//   const ArrayInterface<DataIndices>* getDataIndices() const override;
// 
//   ArrayInterface<OverlappingData>* getRayIntersectData() override;
//   const ArrayInterface<OverlappingData>* getRayIntersectData() const override;
// 
//   ArrayInterface<DataIndices>* getRayIndices() override;
//   const ArrayInterface<DataIndices>* getRayIndices() const override;

  void printStats() override;
protected:
  yavf::Device* device = nullptr;
  yavf::ComputeTask* task = nullptr;

  yavf::Pipeline posRecalc;
  yavf::Pipeline overlapForSolver;
  yavf::Pipeline newSolver;

  yavf::Pipeline solvePhysics;
  yavf::Pipeline calcRayData;
  yavf::Pipeline searchPairs;
  yavf::Pipeline calcOverlappingData;

  yavf::DescriptorSet* objData;
  //yavf::Descriptor physDatas = VK_NULL_HANDLE;
  yavf::DescriptorSet* matrices;
  yavf::DescriptorSet* transforms;
  //yavf::Descriptor staticPhysicDatas = VK_NULL_HANDLE;
  yavf::DescriptorSet* rotationDatas;
  yavf::DescriptorSet* pairs;
  yavf::DescriptorSet* islands;
  yavf::DescriptorSet* indicies;
  //yavf::Descriptor velocities = VK_NULL_HANDLE;
  yavf::DescriptorSet* gravity;

  yavf::DescriptorSet* rays;
  yavf::DescriptorSet* rayPairs;

  yavf::Buffer* overlapTmpBuffer = nullptr;

  yavf::Buffer* indirectIslandCount = nullptr;
  yavf::Buffer* indirectPairsCount = nullptr;
  
  yavf::DescriptorSet* overlappingDataDesc;
  yavf::DescriptorSet* dataIndicesDesc;
  yavf::DescriptorSet* raysDataDesc;
  yavf::DescriptorSet* raysIndicesDesc;
  yavf::DescriptorSet* triggerIndicesDesc;
  
  ArrayInterface<OverlappingData>* overlappingData = nullptr;
  ArrayInterface<DataIndices>* dataIndices = nullptr;

  ArrayInterface<OverlappingData>* raysData = nullptr;
  ArrayInterface<DataIndices>* raysIndices = nullptr;

  ArrayInterface<uint32_t>* triggerIndices = nullptr;

  //yavf::Buffer* indirectOverlappingCalc = nullptr;
  GPUBuffer<DataIndices> indirectOverlappingCalc;
  GPUBuffer<DataIndices> rayIndices;

//   GPUArray<OverlappingData> overlappingData;
//   GPUArray<OverlappingData> rayData;

  void construct(yavf::Device* device, yavf::ComputeTask* task, const InternalGPUSolverData* data = nullptr);
};

class GPUSolverDirectPipeline : public GPUSolver {
public:
  GPUSolverDirectPipeline(yavf::Device* device, yavf::ComputeTask* task, const InternalGPUSolverData* data = nullptr);
  virtual ~GPUSolverDirectPipeline();

  void calculateData() override;
  void calculateRayData() override;
  void solve() override;
};

#endif // !GPU_SOLVER_H
