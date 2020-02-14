#ifndef CPU_PHYSICS_PARALLEL_H
#define CPU_PHYSICS_PARALLEL_H

#include <unordered_map>

#include "Physics.h"
#include "BroadphaseInterface.h"
#include "NarrowphaseInterface.h"
#include "Solver.h"
#include "PhysicsSorter.h"
#include "ArrayInterface.h"
#include "CPUArray.h"
#include "CPUBuffer.h"
#include "ThreadPool.h"

// typedef BroadphasePair BroadphasePair1;
// typedef BroadphasePair BroadphasePair2;
// typedef BroadphasePair BroadphasePair3;

class CPUPhysicsParallel : public PhysicsEngine {
public:
  struct CreateInfo {
    Broadphase* b;
    Narrowphase* n; 
    Solver* s; 
    PhysicsSorter* sorter;
    
    dt::thread_pool* pool;
    const PhysicsExternalBuffers* buffers;
    
    size_t updateDelta;
  };
  CPUPhysicsParallel(const CreateInfo &info);
  ~CPUPhysicsParallel();

  void update(const uint64_t &time) override;
//   void decoupledUpdate(const uint64_t &time) override;
  
  void setBuffers(const PhysicsExternalBuffers &buffers) override;
  
  void registerShape(const Type &type, const uint32_t shapeType, const RegisterNewShapeInfo &info) override;
  void removeShape(const Type &type) override;

  void add(const PhysicsObjectCreateInfo &info, PhysicsIndexContainer* container) override;
  void remove(PhysicsIndexContainer* comp) override;

  // thread safe, only rays
  uint32_t add(const RayData &ray) override; // лучи и фрустумы нужно передобавлять каждый кадр
  
  uint32_t add(const simd::mat4 &frustum, const simd::vec4 &pos = simd::vec4(10000.0f)) override; // так добавить фрустум, или вычислить его вне?
  
  uint32_t add_ray_poligons(const RayData &ray) override;
  uint32_t add_ray_boxes(const RayData &ray) override;
  
  Object & getObjectData(const PhysicsIndexContainer* container) override;
  const Object & getObjectData(const PhysicsIndexContainer* container) const override;
  
  PhysData2 & getPhysicData(const PhysicsIndexContainer* container) override;
  const PhysData2 & getPhysicData(const PhysicsIndexContainer* container) const override;
  
  const BroadphaseProxy* getObjectBroadphaseProxy(const PhysicsIndexContainer* container) const override;
  
  simd::vec4 getGlobalVelocity(const PhysicsIndexContainer* container) const override;
  
  uint32_t getObjectShapePointsSize(const PhysicsIndexContainer* container) const override;
  const simd::vec4* getObjectShapePoints(const PhysicsIndexContainer* container) const override;
  uint32_t getObjectShapeFacesSize(const PhysicsIndexContainer* container) const override;
  const simd::vec4* getObjectShapeFaces(const PhysicsIndexContainer* container) const override;
  
  bool intersect(const RayData &ray, const PhysicsIndexContainer* container, simd::vec4 &point) const override;
  
//   uint32_t getObjectIndex(const PhysicsIndexContainer* container) const override;
  uint32_t getTransformIndex(const PhysicsIndexContainer* container) const override;
  uint32_t getRotationDataIndex(const PhysicsIndexContainer* container) const override;
  uint32_t getMatrixIndex(const PhysicsIndexContainer* container) const override;
  uint32_t getExternalDataIndex(const PhysicsIndexContainer* container) const override;
  uint32_t getInputDataIndex(const PhysicsIndexContainer* container) const override;
  
  void* getUserData(const uint32_t &objIndex) const override;
  PhysicsIndexContainer* getIndexContainer(const uint32_t &objIndex) const override;
  
  void setGravity(const simd::vec4 &g) override;

  ArrayInterface<OverlappingData>* getOverlappingPairsData() override;
  const ArrayInterface<OverlappingData>* getOverlappingPairsData() const override;
  
  ArrayInterface<uint32_t>* getTriggerPairsIndices() override;
  const ArrayInterface<uint32_t>* getTriggerPairsIndices() const override;

  ArrayInterface<OverlappingData>* getRayTracingData() override;
  const ArrayInterface<OverlappingData>* getRayTracingData() const override;

  ArrayInterface<BroadphasePair>* getFrustumPairs() override;
  const ArrayInterface<BroadphasePair>* getFrustumPairs() const override;
  
  const PhysicsIndexContainer* get_ray_polygons(const uint32_t &index) override;
  const PhysicsIndexContainer* get_ray_boxes(const uint32_t &index) override;

  void printStats() override;
protected:
  struct one_ray_data {
    uint32_t object_index;
    uint32_t next_free_index;
  };
  
  dt::thread_pool* pool = nullptr;
  Broadphase* broad = nullptr;
  Narrowphase* narrow = nullptr;
  Solver* solver = nullptr;
  PhysicsSorter* sorter = nullptr;
  
  size_t updateDelta;
  size_t accumulator;

  std::vector<PhysicsIndexContainer*> components;
  std::unordered_map<Type, ShapeInfo> shapes;
  
  uint32_t defaultStaticMatrixIndex = UINT32_MAX;
  uint32_t defaultDynamicMatrixIndex = UINT32_MAX;
  
  ArrayInterface<InputData>* inputs = nullptr;
  ArrayInterface<Transform>* transforms = nullptr;
  ArrayInterface<simd::mat4>* matrices = nullptr;
  ArrayInterface<uint32_t>* rotationDatasCount = nullptr;
  ArrayInterface<RotationData>* rotationDatas = nullptr;
  ArrayInterface<ExternalData>* externalDatas = nullptr;

  size_t freeContainer = SIZE_MAX;
  
  uint32_t freeVert  = UINT32_MAX;
  uint32_t freeObj   = UINT32_MAX;
  uint32_t freePhys  = UINT32_MAX;
//   uint32_t freeTrans = UINT32_MAX;
//   uint32_t freeInput = UINT32_MAX;
//   uint32_t freeStaticPhys = UINT32_MAX;
//   uint32_t freeRotation = UINT32_MAX;
  
  uint32_t free_polygon = UINT32_MAX;
  uint32_t free_box = UINT32_MAX;

  CPUBuffer<Gravity> gravityBuffer;

  CPUArray<simd::vec4> verts;
  
  CPUArray<Object> objects;
  CPUArray<PhysData2> physicsDatas;
  CPUArray<Constants> staticPhysicDatas;
  CPUArray<Transform> prevState;
  CPUArray<Transform> currState;
  //GPUArray<Transform> transforms;
  
  CPUArray<uint32_t> indices;

  CPUArray<simd::vec4> globalVel;

  std::mutex rayMutex;
  CPUArray<RayData> rays; // надо ли их нормализовывать? скорее всего
  CPUArray<RayData> rays_polygons;
  CPUArray<RayData> rays_boxes;
  CPUArray<one_ray_data> polygons;
  CPUArray<one_ray_data> boxes;
  CPUArray<Frustum> frustums;
  CPUArray<simd::vec4> frustumPoses;
  
  // было бы удобно создавать инпут и аутпут буферы вне этого класса
  // это свойство наследовать на все зависимые классы
  // удобно это прежде всего тем что я могу контролировать какую память эти буферы будут использовать
  // тип либо чисто на гпу либо чисто на цпу, но вообще еще удобно какие-нибудь свойства дополнительные задавать буферам
  // но прежде всего это конечно гпу/цпу
  
  // буферы броадфазы
  CPUArray<BroadphasePair> overlappingPairCache;
  CPUArray<BroadphasePair> staticOverlappingPairCache;
  CPUArray<BroadphasePair> rayPairCache;
  CPUArray<BroadphasePair> frustumTestsResult;
  
  // буферы нарров фазы
  CPUArray<IslandData> islands;
  CPUArray<IslandData> staticIslands;
  
  // буферы солвера
  CPUArray<OverlappingData> overlappingData;
  CPUBuffer<DataIndices> dataIndices;
  CPUArray<OverlappingData> raysData;
  CPUBuffer<DataIndices> raysIndices;
  CPUArray<uint32_t> triggerIndices;

  void syncThreadPool();
  
  void updateBuffers();
  void updateInputOutput();
  
  void updateVelocities();
  void updateVelocities(const uint32_t &start, const uint32_t &count);
  void updatePos(const uint32_t &start, const uint32_t &count, std::atomic<uint32_t> &counter);
  void updateRotationDatas();
  
  void interpolate(const float &alpha);
  void interpolate(const size_t &start, const size_t &count, const float &alpha);
  
  void ray_casting();
};

#endif // !CPU_PHYSICS_H
