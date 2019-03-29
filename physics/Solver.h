#ifndef SOLVER_H
#define SOLVER_H

#include "ArrayInterface.h"
#include "PhysicsUtils.h"

struct SolverBuffers {
  ArrayInterface<Object>* objects;
  ArrayInterface<glm::vec4>* verts;
  ArrayInterface<glm::mat4>* systems;
  ArrayInterface<PhysData2>* datas;
  ArrayInterface<Transform>* transforms;
  ArrayInterface<Constants>* staticPhysDatas;
  ArrayInterface<RotationData>* rotationDatas;
  ArrayInterface<BroadphasePair>* pairs;
  ArrayInterface<BroadphasePair>* staticPairs;
  ArrayInterface<IslandData>* islands;
  ArrayInterface<IslandData>* staticIslands;
  ArrayInterface<uint32_t>* indicies;
  ArrayInterface<glm::vec4>* velocities;
  ArrayInterface<Gravity>* gravity;

  ArrayInterface<RayData>* rays;
  ArrayInterface<BroadphasePair>* rayPairs;
};

class Solver {
public:
  struct InputBuffers {
    ArrayInterface<Object>* objects;
    ArrayInterface<glm::vec4>* verts;
    ArrayInterface<glm::mat4>* systems;
    ArrayInterface<PhysData2>* datas;
    ArrayInterface<Transform>* transforms;
    ArrayInterface<Constants>* staticPhysDatas;
    ArrayInterface<RotationData>* rotationDatas;
    ArrayInterface<BroadphasePair>* pairs;
    ArrayInterface<BroadphasePair>* staticPairs;
    ArrayInterface<IslandData>* islands;
    ArrayInterface<IslandData>* staticIslands;
    ArrayInterface<uint32_t>* indicies;
    ArrayInterface<glm::vec4>* velocities;
    ArrayInterface<Gravity>* gravity;

    ArrayInterface<RayData>* rays;
    ArrayInterface<BroadphasePair>* rayPairs;
  };
  
  struct OutputBuffers {
    ArrayInterface<OverlappingData>* overlappingData;
    ArrayInterface<DataIndices>* dataIndices;

    ArrayInterface<OverlappingData>* raysData;
    ArrayInterface<DataIndices>* raysIndices;

    ArrayInterface<uint32_t>* triggerIndices;
  };
  
  virtual ~Solver() {}
  
  //void setBuffers(const SolverBuffers &buffers, void* indirectIslandCount = nullptr, void* indirectPairCount = nullptr) override;
  virtual void setInputBuffers(const InputBuffers &buffers, void* indirectIslandCount = nullptr, void* indirectPairCount = nullptr) = 0;
  virtual void setOutputBuffers(const OutputBuffers &buffers) = 0;

  virtual void updateBuffers(const uint32_t &pairsCount, const uint32_t &rayPairs) = 0; 

  virtual void calculateData() = 0;
  virtual void calculateRayData() = 0;
  virtual void solve() = 0;
  //virtual void solve2() = 0;

//   virtual ArrayInterface<OverlappingData>* getOverlappingData() = 0;
//   virtual const ArrayInterface<OverlappingData>* getOverlappingData() const = 0;
// 
//   virtual ArrayInterface<DataIndices>* getDataIndices() = 0;
//   virtual const ArrayInterface<DataIndices>* getDataIndices() const = 0;
// 
//   virtual ArrayInterface<OverlappingData>* getRayIntersectData() = 0;
//   virtual const ArrayInterface<OverlappingData>* getRayIntersectData() const = 0;
// 
//   virtual ArrayInterface<DataIndices>* getRayIndices() = 0;
//   virtual const ArrayInterface<DataIndices>* getRayIndices() const = 0;

  virtual void printStats() = 0;
};

#endif // !SOLVER_H
