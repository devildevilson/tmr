#ifndef OCTREE_BROADPHASE_H
#define OCTREE_BROADPHASE_H

#include <vector>

#include "BroadphaseInterface.h"
#include "CPUArray.h"
#include "MemoryPool.h"

class Collidable;

class CPUOctreeProxy : public BroadphaseProxy {
public:
  CPUOctreeProxy(const uint32_t &collisionGroup, const uint32_t &collisionFilter);
  ~CPUOctreeProxy();
  
  uint32_t getNodeIndex() const;
  uint32_t getContainerIndex() const;
  
  void setNodeIndex(const uint32_t &index);
  void setContainerIndex(const uint32_t &index);
  
  void updateAABB();
private:
  uint32_t nodeIndex;
  uint32_t containerIndex;
  
  Collidable* object;
};

class CPUOctreeBroadphase : public Broadphase {
public:
  CPUOctreeBroadphase();
  virtual ~CPUOctreeBroadphase();
  
  //void setTask();
  
  // индексы прокси
  void setCollisionTestBuffer(ArrayInterface<uint32_t>* indexBuffer) override; // здесь хранятся индексы для проверки на коллизию
  void setRayTestBuffer(ArrayInterface<RayData>* rayBuffer) override; // здесь хранятся данные луча (или тоже индексы)
  //void setOverlappingPairCache(ArrayInterface<BroadphasePair>* cache); // здесь хранятся пары пересекающихся объектов
  //void setRayPairCache(ArrayInterface<BroadphasePair>* cache); // здесь хранятся пары луч-объект
  
  BroadphaseProxy* createProxy(const AABB &box, uint32_t type, void* obj, const uint32_t &collisionGroup, const uint32_t &collisionFilter) override;
  BroadphaseProxy* getProxy(const uint32_t &index) const override;
  void destroyProxy(BroadphaseProxy* proxy) override;
  void destroyProxy(const uint32_t &index) override;
  
  // запускает тесты (или наверное только добавляет их в командный буфер вулкана)
  void calculateOverlappingPairs(const uint32_t &maxPairs) override;
  void calculateRayTests(const uint32_t &maxPairs) override;
  
  ArrayInterface<BroadphasePair>* getOverlappingPairCache() override;
  const ArrayInterface<BroadphasePair>* getOverlappingPairCache() const override;
  
  ArrayInterface<BroadphasePair>* getRayPairCache() override;
  const ArrayInterface<BroadphasePair>* getRayPairCache() const override;
  
  void printStats() override;
protected:
  CPUArray<BroadphasePair> objPairs;
  CPUArray<BroadphasePair> rayPairs;
  
  ArrayInterface<uint32_t>* indexBuffer;
  ArrayInterface<RayData>* rayBuffer;
  
  MemoryPool<CPUOctreeProxy> pool;
  std::vector<CPUOctreeProxy*> pointers;
  uint32_t freePtr;
  // тут собственно октри и будет
};

#endif
