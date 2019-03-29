#ifndef GPU_PHYSICS_H
#define GPU_PHYSICS_H

#include <unordered_map>

#include "Physics.h"
#include "yavf.h"
#include "BroadphaseInterface.h"
#include "NarrowphaseInterface.h"
#include "Solver.h"
#include "PhysicsSorter.h"
#include "ArrayInterface.h"
#include "GPUArray.h"
#include "GPUBuffer.h"

struct GPUPhysicsCreateInfo {
  Broadphase* b;
  Narrowphase* n;
  Solver* s;
  PhysicsSorter* sorter;

  const PhysicsExternalBuffers* buffers;
};

class GPUPhysics : public PhysicsEngine {
public:
  GPUPhysics(yavf::Device* device, yavf::ComputeTask* task, const GPUPhysicsCreateInfo &info);
  virtual ~GPUPhysics();
  
  void update(const uint64_t &time) override;
  
  // тут возможно что-то еще добавится
  // я как то должен получить указатель (?) для размещения точек и нормалей объектов
  // основной вопрос что делать с динамическим созданием объектов?
  // нужна ли мне такая возможность? самый простой способ сделать как в векторе (собственно сделать просто вектором это дело)
  // при удалении сдвигать все оставшиеся данные, тогда как обновлять индексы?
  // тогда можно сделать примерно как меморипул, но могут остаться огромные дырки, как их сократить?
  // 

  void setBuffers(const PhysicsExternalBuffers &buffers) override;

  // uint32_t registerTransform(const glm::vec4 &initialPos = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)) override;
  // void removeTransform(const uint32_t &index) override;

  void registerShape(const std::string &name, const uint32_t shapeType, const RegisterNewShapeInfo &info) override;
  //uint32_t registerMatrix(const glm::mat4 &matrix) override;
  void add(const PhysicsObjectCreateInfo &info, PhysicsIndexContainer* container) override;
  void remove(PhysicsIndexContainer* comp) override;

  // нужно добавлять заново кадый кадр
  uint32_t add(const RayData &ray) override;
  uint32_t add(const glm::mat4 &frustum, const glm::vec4 &pos = glm::vec4(10000.0f)) override;
  
  Object & getObjectData(const uint32_t &index) override;
  const Object & getObjectData(const uint32_t &index) const override;
  
  // PhysicData & getPhysicData(const uint32_t &index) override;
  // const PhysicData & getPhysicData(const uint32_t &index) const override;

  PhysData2 & getPhysicData(const uint32_t &index) override;
  const PhysData2 & getPhysicData(const uint32_t &index) const override;
  
  // Transform & getTransform(const uint32_t &index) override;
  // const Transform & getTransform(const uint32_t &index) const override;

  // RotationData & getRotationData(const uint32_t &index) override;
  // const RotationData & getRotationData(const uint32_t &index) const override;
  
  void* getUserData(const uint32_t &objIndex) const override;
  
  void setGravity(const glm::vec4 &g) override;
  
//   Object getObjectData(const uint32_t &index) const override;
//   PhysicData getPhysicData(const uint32_t &index) const override;
//   Transform getTransform(const uint32_t &index) const override;
  
  // тут еще может потребоваться изменить кое какие другие данные (фриктион например)
  // думаю что для обновлений таких данных нужно придумать просто что-то другое
  // или обновлять сразу вектора 
  void updateMaxSpeed(const uint32_t &physicDataIndex, const float &maxSpeed) override;
  //void setInput(const uint32_t &index, const InputData &input) override;
  uint32_t setShapePointsAndFaces(const uint32_t &objectDataIndex, const std::vector<glm::vec4> &points, const std::vector<glm::vec4> &faces) override;
  //void removeShapePoints(const uint32_t &index, const uint32_t &count);

  ArrayInterface<OverlappingData>* getOverlappingPairsData() override;
  const ArrayInterface<OverlappingData>* getOverlappingPairsData() const override;

  ArrayInterface<OverlappingData>* getRayTracingData() override;
  const ArrayInterface<OverlappingData>* getRayTracingData() const override;

  ArrayInterface<BroadphasePair>* getFrustumPairs() override;
  const ArrayInterface<BroadphasePair>* getFrustumPairs() const override;

  void printStats() override;
  
  yavf::ComputeTask* getTask() const;
protected:
  Broadphase* broad = nullptr;
  Narrowphase* narrow = nullptr;
  Solver* solver = nullptr;
  PhysicsSorter* sorter = nullptr;

  yavf::Device* device = nullptr;
  yavf::ComputeTask* task = nullptr;
  
  yavf::Pipeline first;

  yavf::DescriptorSet* staticPhysDataDesc;
  
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

  yavf::DescriptorSet* inputDesc;
  yavf::DescriptorSet* transformDesc;
  yavf::DescriptorSet* matrixDesc;
  yavf::DescriptorSet* rotationCountDesc;
  yavf::DescriptorSet* rotationDesc;
  yavf::DescriptorSet* externalsDesc;

  GPUBuffer<Gravity> uniform;
  
  uint32_t freeVert  = UINT32_MAX;
  uint32_t freeObj   = UINT32_MAX;
  uint32_t freePhys  = UINT32_MAX;
  uint32_t freeTrans = UINT32_MAX;
  uint32_t freeInput = UINT32_MAX;
  //uint32_t freeStaticPhys = UINT32_MAX;
  uint32_t freeRotation = UINT32_MAX;
  
  GPUArray<glm::vec4> verts;
  
  GPUArray<Object> objects;
  GPUArray<PhysData2> physicsDatas;
  GPUArray<Constants> staticPhysicDatas;
  //GPUArray<Transform> transforms;
  
  GPUArray<uint32_t> indices;

  GPUArray<glm::vec4> globalVel;

  GPUArray<RayData> rays; // надо ли их нормализовывать? 
  GPUArray<FrustumStruct> frustums;
  GPUArray<glm::vec4> frustumPoses;
  
  // было бы удобно создавать инпут и аутпут буферы вне этого класса
  // это свойство наследовать на все зависимые классы
  // удобно это прежде всего тем что я могу контролировать какую память эти буферы будут использовать
  // тип либо чисто на гпу либо чисто на цпу, но вообще еще удобно какие-нибудь свойства дополнительные задавать буферам
  // но прежде всего это конечно гпу/цпу
  
  // буферы броадфазы
  GPUArray<BroadphasePair> overlappingPairCache;
  GPUArray<BroadphasePair> staticOverlappingPairCache;
  GPUArray<BroadphasePair> rayPairCache;
  GPUArray<BroadphasePair> frustumTestsResult;
  
  // буферы нарров фазы
  GPUArray<IslandData> islands;
  GPUArray<IslandData> staticIslands;
  
  // буферы солвера
  GPUArray<OverlappingData> overlappingData;
  GPUBuffer<DataIndices> dataIndices;
  GPUArray<OverlappingData> raysData;
  GPUBuffer<DataIndices> raysIndices;
  GPUArray<uint32_t> triggerIndices;

  void construct(yavf::Device* device, yavf::ComputeTask* task, Broadphase* b, Narrowphase* n, Solver* s, PhysicsSorter* sorter);
  void updateStaticPhysDataDesc();
  void updateBuffers();
  void updateInputOutput();
};

class GPUPhysicsDirectPipeline : public GPUPhysics {
public:
  GPUPhysicsDirectPipeline(yavf::Device* device, yavf::ComputeTask* task, const GPUPhysicsCreateInfo &info);
  virtual ~GPUPhysicsDirectPipeline();

  void update(const uint64_t &time) override;
};

#endif
