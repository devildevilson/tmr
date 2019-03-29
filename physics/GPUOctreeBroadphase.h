#ifndef GPU_OCTREE_BROADPHASE_H
#define GPU_OCTREE_BROADPHASE_H

#include <vector>

#include "BroadphaseInterface.h"
#include "GPUArray.h"
#include "GPUBuffer.h"

#include "yavf.h"

struct GPUOctreeNode {
  //glm::uvec4 data;
  uint32_t count;
  uint32_t offset;
  uint32_t childIndex;
  uint32_t proxyIndex; // прокси ли? или лучше просто аабб держать?
};

class GPUOctreeProxy : public BroadphaseProxy {
  friend class GPUOctreeBroadphase;
public:
  GPUOctreeProxy(const uint32_t &objIndex, const PhysicsType &type, const uint32_t &collisionGroup, const uint32_t &collisionFilter);
  ~GPUOctreeProxy();
  
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

struct InternalGPUBroadphaseData {
  uint32_t workGroupSizeCalcProxy;
  uint32_t workGroupSizeCalcChanges;
  uint32_t workGroupSizePreUpdate;
  uint32_t workGroupSizeUpdate;
  uint32_t workGroupSizeCalcPairs;
  uint32_t sharedDataSize;
};

class GPUOctreeBroadphase : public Broadphase {
public:
  struct OctreeCreateInfo {
    glm::vec4 center;
    glm::vec4 extent;
    uint32_t depth;
  };

  struct GPUOctreeBroadphaseCreateInfo {
    OctreeCreateInfo octreeInfo;
    yavf::Device* device;
    yavf::ComputeTask* task;
    const InternalGPUBroadphaseData* internal;
  };

  GPUOctreeBroadphase(const GPUOctreeBroadphaseCreateInfo &info);
  virtual ~GPUOctreeBroadphase();
  
//   void setCollisionTestBuffer(ArrayInterface<uint32_t>* indexBuffer) override;
//   void setRayTestBuffer(ArrayInterface<RayData>* rayBuffer) override;
//   void setUpdateBuffers(const UpdateBuffers &buffers) override;
  
  void setInputBuffers(const InputBuffers &buffers) override;
  void setOutputBuffers(const OutputBuffers &buffers, void* indirectPairBuffer = nullptr) override;
    
  //BroadphaseProxy* createProxy(const AABB &box, uint32_t type, void* obj, const uint32_t &collisionGroup, const uint32_t &collisionFilter) override;
  uint32_t createProxy(const FastAABB &box, const uint32_t &objIndex, const PhysicsType &type, const uint32_t &collisionGroup, const uint32_t &collisionFilter) override;
  BroadphaseProxy* getProxy(const uint32_t &index) override;
  void destroyProxy(BroadphaseProxy* proxy) override;
  void destroyProxy(const uint32_t &index) override;
  
  void updateBuffers(const uint32_t &objCount, const uint32_t &dynObjCount, const uint32_t &raysCount, const uint32_t &frustumsCount) override;
  
  // запускает тесты (или наверное только добавляет их в командный буфер вулкана)
  void update() override;
  void calculateOverlappingPairs() override;
  void calculateRayTests() override;
  void calculateFrustumTests() override;

  void postlude() override;
  
//   ArrayInterface<BroadphasePair>* getOverlappingPairCache() override;
//   const ArrayInterface<BroadphasePair>* getOverlappingPairCache() const override;
//   
//   ArrayInterface<BroadphasePair>* getRayPairCache() override;
//   const ArrayInterface<BroadphasePair>* getRayPairCache() const override;
//   
//   ArrayInterface<BroadphasePair>* getFrustumTestsResult() override;
//   const ArrayInterface<BroadphasePair>* getFrustumTestsResult() const override;
// 
//   void* getIndirectPairBuffer() override;
  
  void printStats() override;
protected:
  yavf::Device* device = nullptr;
  yavf::ComputeTask* task = nullptr;
  
//   ArrayInterface<uint32_t>* indexBuffer = nullptr; // это Индексы Объектов !
//   ArrayInterface<RayData>* rayBuffer = nullptr;
//   
//   ArrayInterface<glm::vec4>* verticies = nullptr; // любыми способами необходимо от этого избавиться
//   ArrayInterface<Object>* objects = nullptr;      // например, обновить все боксы проксей до начала броадфазы
//   ArrayInterface<glm::mat4>* systems = nullptr;   // а для этого достаточно лишь передать буфер проксей
//                                                   // нет это практически невозможно
//                                                   // так как у меня вполне может быть и не octree proxy
//   ArrayInterface<Transform>* transforms = nullptr;
  
  yavf::Buffer* indirectBuffer = nullptr;
  //GPUBuffer<glm::uvec3> indirectPairsCount;
  yavf::Buffer* indirectPairsCount = nullptr;
  
  yavf::DescriptorSet* indexBufferDesc;
  yavf::DescriptorSet* rayBufferDesc;
  yavf::DescriptorSet* objectsDataDesc;
  yavf::DescriptorSet* matricesDesc;
  yavf::DescriptorSet* transformsDesc;
  yavf::DescriptorSet* rotationDatasDesc;

  yavf::DescriptorSet* rayDesc;
  yavf::DescriptorSet* frustumDesc;
  yavf::DescriptorSet* frustumPosesDesc;
                                        
  yavf::Pipeline calcProxy;
  yavf::Pipeline calcChanges;
  yavf::Pipeline preUpdateOctree;
  yavf::Pipeline updateOctree;
  yavf::Pipeline calcPairs;
  
  yavf::Pipeline calcRayPairs;
  yavf::Pipeline iterativeFrustum;
  yavf::Pipeline octreeFrustum;
  yavf::Pipeline pairsToPowerOfTwo;

//   GPUArray<BroadphasePair> objPairs;
//   GPUArray<BroadphasePair> rayPairs;
//   GPUArray<BroadphasePair> frustumPairs;
  yavf::DescriptorSet* overlappingPairCacheDesc;
  yavf::DescriptorSet* staticOverlappingPairCacheDesc;
  yavf::DescriptorSet* rayPairCacheDesc;
  yavf::DescriptorSet* frustumTestsResultDesc;
  ArrayInterface<BroadphasePair>* overlappingPairCache = nullptr;
  ArrayInterface<BroadphasePair>* staticOverlappingPairCache = nullptr;
  ArrayInterface<BroadphasePair>* rayPairCache = nullptr;
  ArrayInterface<BroadphasePair>* frustumTestsResult = nullptr;

  uint32_t raysCount = 0;
  uint32_t frustumCount = 0;
  uint32_t frustumDispatchNodeCount = 64;
  
//   bool toggleIndiciesBuffer = true;
  uint32_t freeProxy = UINT32_MAX;
  
  size_t nodesCount = 0;
  // максимальная глубина 10 при 32 битных переменных
  // нужно будет почекать какую величину октодерева можно запилить таким образом
  // примерно 3к единиц по каждому направлению
  size_t depth = 0;
  yavf::vector<GPUOctreeProxy> proxies;
  yavf::vector<GPUOctreeNode> nodes;
  yavf::vector<glm::uvec4> changes;
  yavf::vector<uint32_t> indices1;
  yavf::vector<uint32_t> indices2;
  
  yavf::vector<uint32_t> proxyIndices;
  yavf::vector<uint32_t> toPairsCalculate;

  void construct(const GPUOctreeBroadphaseCreateInfo &info);
};

class GPUOctreeBroadphaseDirectPipeline : public GPUOctreeBroadphase {
public:
  GPUOctreeBroadphaseDirectPipeline(const GPUOctreeBroadphaseCreateInfo &info);
  virtual ~GPUOctreeBroadphaseDirectPipeline();

  void update() override;
  void calculateOverlappingPairs() override;
  void calculateRayTests() override;
  void calculateFrustumTests() override;
};

#endif

