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
  };
  
  CPUPhysicsParallel(const CreateInfo &info);
  virtual ~CPUPhysicsParallel();

  void update(const uint64_t &time) override;
  
  void setBuffers(const PhysicsExternalBuffers &buffers) override;
  
  void registerShape(const std::string &name, const uint32_t shapeType, const RegisterNewShapeInfo &info) override;

  void add(const PhysicsObjectCreateInfo &info, PhysicsIndexContainer* container) override;
  void remove(PhysicsIndexContainer* comp) override;

  uint32_t add(const RayData &ray) override; // лучи и фрустумы нужно передобавлять каждый кадр
  uint32_t add(const glm::mat4 &frustum, const glm::vec4 &pos = glm::vec4(10000.0f)) override; // так добавить фрустум, или вычислить его вне?
  
  Object & getObjectData(const uint32_t &index) override;
  const Object & getObjectData(const uint32_t &index) const override;
  
  PhysData2 & getPhysicData(const uint32_t &index) override;
  const PhysData2 & getPhysicData(const uint32_t &index) const override;
  
  void* getUserData(const uint32_t &objIndex) const override;
  
  void setGravity(const glm::vec4 &g) override;
  
  void updateMaxSpeed(const uint32_t &physicDataIndex, const float &maxSpeed) override;
  uint32_t setShapePointsAndFaces(const uint32_t &objectDataIndex, const std::vector<glm::vec4> &points, const std::vector<glm::vec4> &faces) override;

  ArrayInterface<OverlappingData>* getOverlappingPairsData() override;
  const ArrayInterface<OverlappingData>* getOverlappingPairsData() const override;
  
  ArrayInterface<uint32_t>* getTriggerPairsIndices() override;
  const ArrayInterface<uint32_t>* getTriggerPairsIndices() const override;

  ArrayInterface<OverlappingData>* getRayTracingData() override;
  const ArrayInterface<OverlappingData>* getRayTracingData() const override;

  ArrayInterface<BroadphasePair>* getFrustumPairs() override;
  const ArrayInterface<BroadphasePair>* getFrustumPairs() const override;

  void printStats() override;
protected:
  dt::thread_pool* pool = nullptr;
  Broadphase* broad = nullptr;
  Narrowphase* narrow = nullptr;
  Solver* solver = nullptr;
  PhysicsSorter* sorter = nullptr;
  
  size_t updateDelta;
  size_t accumulator;

  std::vector<PhysicsIndexContainer*> components;
  std::unordered_map<std::string, ShapeInfo> shapes;
  
  uint32_t defaultStaticMatrixIndex = UINT32_MAX;
  uint32_t defaultDynamicMatrixIndex = UINT32_MAX;
  
  ArrayInterface<InputData>* inputs = nullptr;
  ArrayInterface<Transform>* transforms = nullptr;
  ArrayInterface<glm::mat4>* matrices = nullptr;
  ArrayInterface<uint32_t>* rotationDatasCount = nullptr;
  ArrayInterface<RotationData>* rotationDatas = nullptr;
  ArrayInterface<ExternalData>* externalDatas = nullptr;

  size_t freeContainer = SIZE_MAX;
  
  uint32_t freeVert  = UINT32_MAX;
  uint32_t freeObj   = UINT32_MAX;
  uint32_t freePhys  = UINT32_MAX;
  uint32_t freeTrans = UINT32_MAX;
  uint32_t freeInput = UINT32_MAX;
  //uint32_t freeStaticPhys = UINT32_MAX;
  uint32_t freeRotation = UINT32_MAX;

  CPUBuffer<Gravity> gravityBuffer;

  CPUArray<glm::vec4> verts;
  
  CPUArray<Object> objects;
  CPUArray<PhysData2> physicsDatas;
  CPUArray<Constants> staticPhysicDatas;
  CPUArray<Transform> prevState;
  CPUArray<Transform> currState;
  //GPUArray<Transform> transforms;
  
  CPUArray<uint32_t> indices;

  CPUArray<glm::vec4> globalVel;

  CPUArray<RayData> rays; // надо ли их нормализовывать? 
  CPUArray<FrustumStruct> frustums;
  CPUArray<glm::vec4> frustumPoses;
  
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
  void updateRotationDatas();
  
  void interpolate(const float &alpha);
};

#endif // !CPU_PHYSICS_H
