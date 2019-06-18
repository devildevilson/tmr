#ifndef CPU_OCTREE_BROADPHASE_PARALLEL_H
#define CPU_OCTREE_BROADPHASE_PARALLEL_H

#include <vector>

#include "BroadphaseInterface.h"
#include "CPUArray.h"
#include "ThreadPool.h"

class CPUOctreeProxyParallel : public BroadphaseProxy {
  friend class CPUOctreeBroadphaseParallel;
public:
  CPUOctreeProxyParallel(const uint32_t &objIndex, const PhysicsType &type, const uint32_t &collisionGroup, const uint32_t &collisionFilter);
  ~CPUOctreeProxyParallel();

  uint32_t getProxyIndex() const;
  uint32_t getNodeIndex() const;
  uint32_t getContainerIndex() const;

  void setNodeIndex(const uint32_t &index);
  void setContainerIndex(const uint32_t &index);

  void updateAABB();
private:
  uint32_t nodeIndex;
  uint32_t containerIndex;

  uint32_t proxyIndex; // objType
  uint32_t dummy2;
};

class CPUOctreeBroadphaseParallel : public Broadphase {
  friend class CPUParallelOctreeNode;
public:
  struct CPUParallelOctreeNode {
    //FastAABB box;
    
    uint32_t nodeIndex;
    uint32_t childIndex;
    
    std::mutex mutex;
    std::vector<uint32_t> proxies;
    std::vector<uint32_t> freeIndices;
    //CPUOctreeBroadphaseParallel* basePtr;

    CPUParallelOctreeNode() = default;
    ~CPUParallelOctreeNode() = default;
    CPUParallelOctreeNode(const CPUParallelOctreeNode &node) = default;
    CPUParallelOctreeNode & operator=(const CPUParallelOctreeNode &node) = default;

//     FastAABB getAABB() const;
    void add(CPUOctreeProxyParallel* proxy);
    void remove(CPUOctreeProxyParallel* proxy);

    size_t size() const;
  };

  struct OctreeCreateInfo {
    simd::vec4 center;
    simd::vec4 extent;
    uint32_t depth;
  };

  CPUOctreeBroadphaseParallel(dt::thread_pool* pool, const OctreeCreateInfo &octreeInfo);
  ~CPUOctreeBroadphaseParallel();

//   void setCollisionTestBuffer(ArrayInterface<uint32_t>* indexBuffer) override;
//   void setRayTestBuffer(ArrayInterface<RayData>* rayBuffer) override;
//   //virtual void setRayTestBuffer(ArrayInterface<glm::mat4>* frustumBuffer) = 0;
//   void setUpdateBuffers(const UpdateBuffers &buffers) override;

  void setInputBuffers(const InputBuffers &buffers) override;
  void setOutputBuffers(const OutputBuffers &buffers, void* indirectPairBuffer = nullptr) override;

  //virtual BroadphaseProxy* createProxy(const AABB &box, uint32_t type, void* obj, const uint32_t &collisionGroup, const uint32_t &collisionFilter) = 0;
  uint32_t createProxy(const FastAABB &box, const uint32_t &objIndex, const PhysicsType &type, const uint32_t &collisionGroup, const uint32_t &collisionFilter) override;
  BroadphaseProxy* getProxy(const uint32_t &index) override;
  void destroyProxy(BroadphaseProxy* proxy) override;
  void destroyProxy(const uint32_t &index) override;

  void updateBuffers(const uint32_t &objCount, const uint32_t &dynObjCount, const uint32_t &raysCount, const uint32_t &frustumsCount) override;

  void update() override;
  void calculateOverlappingPairs() override;
  void calculateRayTests() override;
  void calculateFrustumTests() override;

  void postlude() override;

//   ArrayInterface<BroadphasePair>* getOverlappingPairCache() override;
//   const ArrayInterface<BroadphasePair>* getOverlappingPairCache() const override;
//
//   ArrayInterface<BroadphasePair>* getStaticOverlappingPairCache() override;
//   const ArrayInterface<BroadphasePair>* getStaticOverlappingPairCache() const override;
//
//   ArrayInterface<BroadphasePair>* getRayPairCache() override;
//   const ArrayInterface<BroadphasePair>* getRayPairCache() const override;
//
//   ArrayInterface<BroadphasePair>* getFrustumTestsResult() override;
//   const ArrayInterface<BroadphasePair>* getFrustumTestsResult() const override;

  void printStats() override;
protected:
  dt::thread_pool* pool = nullptr;

  ArrayInterface<uint32_t>* indexBuffer = nullptr;
  ArrayInterface<simd::vec4>* verticies = nullptr;
  ArrayInterface<Object>* objects = nullptr;
  ArrayInterface<simd::mat4>* systems = nullptr;
  ArrayInterface<Transform>* transforms = nullptr;
  ArrayInterface<RotationData>* rotationDatas = nullptr;

  ArrayInterface<RayData>* rays = nullptr;
  ArrayInterface<Frustum>* frustums = nullptr;
  ArrayInterface<simd::vec4>* frustumPoses = nullptr;

//   CPUArray<BroadphasePair> objPairs;
//   CPUArray<BroadphasePair> staticObjPairs;
//   CPUArray<BroadphasePair> rayPairs;
//   CPUArray<BroadphasePair> frustumPairs;

  ArrayInterface<BroadphasePair>* overlappingPairCache = nullptr;
  ArrayInterface<BroadphasePair>* staticOverlappingPairCache = nullptr;
  ArrayInterface<BroadphasePair>* rayPairCache = nullptr;
  ArrayInterface<BroadphasePair>* frustumTestsResult = nullptr;

  uint32_t raysCount = 0;
  uint32_t frustumCount = 0;
  uint32_t frustumDispatchNodeCount = 64;
  uint32_t frustumIterativeNodeCount = 9;

//   bool toggleIndiciesBuffer = true;
  uint32_t freeProxy = UINT32_MAX;

  size_t nodesCount = 0;
  // максимальная глубина 10 при 32 битных переменных
  // нужно будет почекать какую величину октодерева можно запилить таким образом
  // примерно 3к единиц по каждому направлению
  size_t depth = 0;
  std::vector<CPUOctreeProxyParallel> proxies;
  std::vector<FastAABB> nodeBoxes;
  std::vector<CPUParallelOctreeNode> nodes;
  //std::vector<std::mutex> mutexes;
  std::vector<glm::uvec4> changes;
  std::vector<uint32_t> indices1;
  std::vector<uint32_t> indices2;

  std::vector<uint32_t> proxyIndices;
  std::vector<uint32_t> toPairsCalculate;

  void addEveryObj(const uint32_t &mainNodeIndex, const uint32_t &depth, const uint32_t &frustumIndex, const simd::vec4 &frustumPos);
  void addEveryObjIterative(const uint32_t &mainNodeIndex, const uint32_t &depth, const uint32_t &frustumIndex, const simd::vec4 &frustumPos);
};

#endif
