#ifndef BROADPHASE_INTERFACE_H
#define BROADPHASE_INTERFACE_H

#include "ArrayInterface.h"
#include "PhysicsUtils.h"

#include "Frustum.h"

// алигн 16, могут возникнуть проблемы
class BroadphaseProxy {
public:
  BroadphaseProxy(const uint32_t &objIndex, const PhysicsType &type, const uint32_t &collisionGroup, const uint32_t &collisionFilter);
  ~BroadphaseProxy();
  
  void setAABB(const FastAABB &box);
  FastAABB getAABB() const;
  
  bool collide(const BroadphaseProxy* proxy) const;
  bool in(const FastAABB &box) const;
  
  uint32_t collisionGroup() const;
  uint32_t collisionFilter() const;
  
  uint32_t getIndex() const;
  uint32_t getObjectIndex() const;
  
  PhysicsType getType() const;
protected:
  FastAABB box;
//   float center[4]; 
//   float extent[4];
  uint32_t group;
  uint32_t filter;
  PhysicsType objType; //proxyIndex
  uint32_t objIndex;
};

class Broadphase {
public:
  struct InputBuffers {
    ArrayInterface<uint32_t>* indexBuffer;
    ArrayInterface<simd::vec4>* verticies; // броадфаза по идее не должна знать это
    ArrayInterface<Object>* objects;      // это тоже броадфазе не нужно
    ArrayInterface<simd::mat4>* systems;   // это уже никуда не годится
    ArrayInterface<Transform>* transforms;
    ArrayInterface<RotationData>* rotationDatas;

    ArrayInterface<RayData>* rays;
    ArrayInterface<Frustum>* frustums;
    ArrayInterface<simd::vec4>* frustumPoses;
  };
  
  struct OutputBuffers {
    ArrayInterface<BroadphasePair>* overlappingPairCache;
    ArrayInterface<BroadphasePair>* staticOverlappingPairCache;
    ArrayInterface<BroadphasePair>* rayPairCache;
    ArrayInterface<BroadphasePair>* frustumTestsResult;
  };
  
  virtual ~Broadphase();
  
//   void getBroadphaseAabb(FastAABB &box) const;
  
  virtual void setInputBuffers(const InputBuffers &buffers) = 0;
  virtual void setOutputBuffers(const OutputBuffers &buffers, void* indirectPairBuffer = nullptr) = 0;
  
  //virtual BroadphaseProxy* createProxy(const AABB &box, uint32_t type, void* obj, const uint32_t &collisionGroup, const uint32_t &collisionFilter) = 0;
  virtual uint32_t createProxy(const FastAABB &box, const uint32_t &objIndex, const PhysicsType &type, const uint32_t &collisionGroup, const uint32_t &collisionFilter) = 0;
  virtual BroadphaseProxy* getProxy(const uint32_t &index) = 0;
  virtual void destroyProxy(BroadphaseProxy* proxy) = 0;
  virtual void destroyProxy(const uint32_t &index) = 0;
  
  virtual void updateBuffers(const uint32_t &objCount, const uint32_t &dynObjCount, const uint32_t &raysCount, const uint32_t &frustumsCount) = 0;
  
  // запускает тесты (или наверное только добавляет их в командный буфер вулкана)
  virtual void update() = 0;
  virtual void calculateOverlappingPairs() = 0;
  virtual void calculateRayTests() = 0;
  virtual void calculateFrustumTests() = 0;

  virtual void postlude() = 0;
  
  virtual void traverse(const RayData &ray, const std::function<void(const RayData &ray, const BroadphaseProxy* proxy)> &func) const = 0;
  
//   virtual ArrayInterface<BroadphasePair>* getOverlappingPairCache() = 0;
//   virtual const ArrayInterface<BroadphasePair>* getOverlappingPairCache() const = 0;
//   
//   virtual ArrayInterface<BroadphasePair>* getStaticOverlappingPairCache() = 0;
//   virtual const ArrayInterface<BroadphasePair>* getStaticOverlappingPairCache() const = 0;
//   
//   virtual ArrayInterface<BroadphasePair>* getRayPairCache() = 0;
//   virtual const ArrayInterface<BroadphasePair>* getRayPairCache() const = 0;
//   
//   virtual ArrayInterface<BroadphasePair>* getFrustumTestsResult() = 0;
//   virtual const ArrayInterface<BroadphasePair>* getFrustumTestsResult() const = 0;

  //virtual void* getIndirectPairBuffer() { return nullptr; }
  
  virtual void printStats() = 0;
protected:
//   FastAABB box;
};

#endif
