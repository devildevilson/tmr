#ifndef CPU_SOLVER_H
#define CPU_SOLVER_H

#include "Solver.h"
#include "CPUArray.h"

class CPUSolver : public Solver {
public:
  struct OverlappingDataForSolver {
    glm::uvec4 pairData; // w == mtvAngle
    glm::vec4 mtvDist;

    glm::vec4 normals[2];
    glm::vec4 satAngleStair;
    glm::uvec4 stairsMoves;
  };

  CPUSolver(const float &threshold = 0.04f);
  virtual ~CPUSolver();

  //void setBuffers(const SolverBuffers &buffers, void* indirectIslandCount = nullptr, void* indirectPairCount = nullptr) override;
  void setInputBuffers(const InputBuffers &buffers, void* indirectIslandCount = nullptr, void* indirectPairCount = nullptr) override;
  void setOutputBuffers(const OutputBuffers &buffers) override;

  void updateBuffers(const uint32_t &pairsCount, const uint32_t &rayPairs) override;

  void calculateData() override;
  void calculateRayData() override;
  void solve() override;
  //void solve2() override;

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
  ArrayInterface<Object>* objects = nullptr;
  ArrayInterface<glm::vec4>* verts = nullptr;
  ArrayInterface<glm::mat4>* systems = nullptr;
  ArrayInterface<PhysData2>* datas = nullptr;
  ArrayInterface<Transform>* transforms = nullptr;
  ArrayInterface<Constants>* staticPhysDatas = nullptr;
  ArrayInterface<RotationData>* rotationDatas = nullptr;
  ArrayInterface<BroadphasePair>* pairs = nullptr;
  ArrayInterface<BroadphasePair>* staticPairs = nullptr;
  ArrayInterface<IslandData>* islands = nullptr;
  ArrayInterface<IslandData>* staticIslands = nullptr;
  ArrayInterface<uint32_t>* indicies = nullptr;
  ArrayInterface<glm::vec4>* velocities = nullptr;
  ArrayInterface<Gravity>* gravity = nullptr;

  ArrayInterface<RayData>* rays = nullptr;
  ArrayInterface<BroadphasePair>* rayPairs = nullptr;

  //std::vector<OverlappingDataForSolver> tmpBuffer;

//   CPUArray<OverlappingData> overlappingData;
//   CPUArray<OverlappingData> rayData;
  
  //CPUArray<glm::vec4> lastVelocities;
  
  ArrayInterface<OverlappingData>* overlappingData = nullptr;
  ArrayInterface<DataIndices>* dataIndices = nullptr;

  ArrayInterface<OverlappingData>* raysData = nullptr;
  ArrayInterface<DataIndices>* raysIndices = nullptr;

  ArrayInterface<uint32_t>* triggerIndices = nullptr;
  
  float threshold = 0.04f;

  glm::vec4 getNormalCloseToGravity(const glm::mat4 &orn, const glm::vec4 &gravityNorm) const;
  glm::vec4 getNormalCloseToGravity(const glm::mat4 &orn, const uint32_t &offset, const uint32_t &facesCount, const glm::vec4 &gravityNorm, uint32_t &retIndex) const;

  void project(const glm::vec4 &axis, const uint32_t &offset, const uint32_t &size, const glm::vec4 &pos, const glm::mat4 &invOrn, float &minRet, float &maxRet) const;
  void project(const glm::vec4 &axis, const glm::vec4 &pos, const glm::vec4 &ext, const glm::mat4 &orn, float &minRet, float &maxRet) const;
  void project(const glm::vec4 &axis, const glm::vec4 &pos, const float &radius, float &minRet, float &maxRet) const;

  bool BoxBoxSAT(const Object &first,  const uint32_t &transFirst,
                 const Object &second, const uint32_t &transSecond, glm::vec4 &mtv, float &dist) const;
  bool BoxSphereSAT(const Object &first,  const uint32_t &transFirst,
                    const Object &second, const uint32_t &transSecond, glm::vec4 &mtv, float &dist) const;
  bool BoxPolySAT(const Object &first,  const uint32_t &transFirst,
                  const Object &second, const uint32_t &transSecond, glm::vec4 &mtv, float &dist) const;
  bool SphereSphereSAT(const Object &first,  const uint32_t &transFirst,
                       const Object &second, const uint32_t &transSecond, glm::vec4 &mtv, float &dist) const;
  bool PolySphereSAT(const Object &first,  const uint32_t &transFirst,
                     const Object &second, const uint32_t &transSecond, glm::vec4 &mtv, float &dist) const;
  bool PolyPolySAT(const Object &first,  const uint32_t &transFirst,
                   const Object &second, const uint32_t &transSecond, glm::vec4 &mtv, float &dist) const;
  bool SAT(const uint32_t &objectIndexFirst,  const uint32_t &transformIndexFirst, 
           const uint32_t &objectIndexSecond, const uint32_t &transformIndexSecond, glm::vec4 &mtv, float &dist) const;
  float SATOneAxis(const uint32_t &objectIndexFirst,  const uint32_t &transformIndexFirst, 
                   const uint32_t &objectIndexSecond, const uint32_t &transformIndexSecond, const glm::vec4 &axis) const;
                   
  void computePair(const BroadphasePair &pair);
  void computePairWithGround(const BroadphasePair &pair, const glm::vec4 &normal);
  void applyChanges(const OverlappingDataForSolver &data);
};

#endif // !CPU_SOLVER_H
