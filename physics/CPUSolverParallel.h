#ifndef CPU_SOLVER_PARALLEL_H
#define CPU_SOLVER_PARALLEL_H

#include "Solver.h"
#include "CPUArray.h"
#include "ThreadPool.h"

// почему бы здесь не запомнить количество overlappingData, raysData и triggerIndices?
// так мы можем не использовать для количества первый элемент и соотвественно 
// массив будет "чистым", у нас в эти данные уходят в dataIndices и raysIndices
// а для triggerIndices ничего нет, но тип triggerIndices у нас существует вместе с dataIndices
// почему не использовать его
class CPUSolverParallel : public Solver {
public:
  struct OverlappingDataForSolver {
    glm::uvec4 pairData; // w == mtvAngle
    simd::vec4 mtvDist;

    simd::vec4 normals[2];
    simd::vec4 satAngleStair;
    glm::uvec4 stairsMoves;
  };

  CPUSolverParallel(dt::thread_pool* pool, const float &threshold = 0.004f/*, const uint32_t &iterationCount = 10*/);
  ~CPUSolverParallel();
  
  void setInputBuffers(const InputBuffers &buffers, void* indirectIslandCount = nullptr, void* indirectPairCount = nullptr) override;
  void setOutputBuffers(const OutputBuffers &buffers) override;

  void updateBuffers(const uint32_t &pairsCount, const uint32_t &rayPairs) override;

  void calculateData() override;
  void calculateRayData() override;
  void solve() override;

  void printStats() override;
  
  bool intersect(const RayData &ray, const uint32_t &objIndex, const uint32_t &transformIndex, simd::vec4 &point) const override;
  bool intersect(const uint32_t &rayIndex, const uint32_t &objIndex, const uint32_t &transformIndex, simd::vec4 &point) const override;
protected:
  dt::thread_pool* pool = nullptr;
  
  // input buffers
  ArrayInterface<Object>* objects = nullptr;
  ArrayInterface<simd::vec4>* verts = nullptr;
  ArrayInterface<simd::mat4>* systems = nullptr;
  ArrayInterface<PhysData2>* datas = nullptr;
  ArrayInterface<Transform>* transforms = nullptr;
  ArrayInterface<Constants>* staticPhysDatas = nullptr;
  ArrayInterface<RotationData>* rotationDatas = nullptr;
  ArrayInterface<BroadphasePair>* pairs = nullptr;
  ArrayInterface<BroadphasePair>* staticPairs = nullptr;
  ArrayInterface<IslandData>* islands = nullptr;
  ArrayInterface<IslandData>* staticIslands = nullptr;
  ArrayInterface<uint32_t>* indicies = nullptr;
  ArrayInterface<simd::vec4>* velocities = nullptr;
  ArrayInterface<Gravity>* gravity = nullptr;
  ArrayInterface<RayData>* rays = nullptr;
  ArrayInterface<BroadphasePair>* rayPairs = nullptr;
  
  // output buffers
  ArrayInterface<OverlappingData>* overlappingData = nullptr;
  ArrayInterface<DataIndices>* dataIndices = nullptr;
  ArrayInterface<OverlappingData>* raysData = nullptr;
  ArrayInterface<DataIndices>* raysIndices = nullptr;
  ArrayInterface<uint32_t>* triggerIndices = nullptr;
  
  float threshold = 0.04f;
  std::atomic<size_t> counter;
  std::atomic<size_t> free_slots_count;
  std::vector<std::pair<size_t, OverlappingData>> temp_vector;
//   uint32_t iterationCount = 10;

  simd::vec4 getNormalCloseToGravity(const simd::mat4 &orn, const simd::vec4 &gravityNorm) const;
  simd::vec4 getNormalCloseToGravity(const simd::mat4 &orn, const uint32_t &offset, const uint32_t &facesCount, const simd::vec4 &gravityNorm, uint32_t &retIndex) const;

  void project(const simd::vec4 &axis, const uint32_t &offset, const uint32_t &size, const simd::vec4 &pos, const simd::mat4 &invOrn, float &minRet, float &maxRet) const;
  void project(const simd::vec4 &axis, const simd::vec4 &pos, const simd::vec4 &ext, const simd::mat4 &orn, float &minRet, float &maxRet) const;
  void project(const simd::vec4 &axis, const simd::vec4 &pos, const float &radius, float &minRet, float &maxRet) const;

  bool BoxBoxSAT(const float &treshold, const Object &first,  const uint32_t &transFirst,
                 const Object &second, const uint32_t &transSecond, simd::vec4 &mtv, float &dist) const;
  bool BoxSphereSAT(const float &treshold, const Object &first,  const uint32_t &transFirst,
                    const Object &second, const uint32_t &transSecond, simd::vec4 &mtv, float &dist) const;
  bool BoxPolySAT(const float &treshold, const Object &first,  const uint32_t &transFirst,
                  const Object &second, const uint32_t &transSecond, simd::vec4 &mtv, float &dist) const;
  bool SphereSphereSAT(const float &treshold, const Object &first,  const uint32_t &transFirst,
                       const Object &second, const uint32_t &transSecond, simd::vec4 &mtv, float &dist) const;
  bool PolySphereSAT(const float &treshold, const Object &first,  const uint32_t &transFirst,
                     const Object &second, const uint32_t &transSecond, simd::vec4 &mtv, float &dist) const;
  bool PolyPolySAT(const float &treshold, const Object &first,  const uint32_t &transFirst,
                   const Object &second, const uint32_t &transSecond, simd::vec4 &mtv, float &dist) const;
  bool SAT(const float &treshold, const uint32_t &objectIndexFirst,  const uint32_t &transformIndexFirst, 
           const uint32_t &objectIndexSecond, const uint32_t &transformIndexSecond, simd::vec4 &mtv, float &dist) const;
  float SATOneAxis(const uint32_t &objectIndexFirst,  const uint32_t &transformIndexFirst, 
                   const uint32_t &objectIndexSecond, const uint32_t &transformIndexSecond, const simd::vec4 &axis) const;
                   
  void computePair(const BroadphasePair &pair);
  void computePairWithGround(const BroadphasePair &pair, const simd::vec4 &normal);
  void applyChanges(const OverlappingDataForSolver &data);
};

#endif // !CPU_SOLVER_PARALLEL_H
