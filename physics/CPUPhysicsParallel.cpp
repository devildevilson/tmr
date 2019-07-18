#include "CPUPhysicsParallel.h"

#include "Globals.h"

#include <stdexcept>

// #include <glm/gtx/norm.hpp>
// #include <glm/gtx/rotate_vector.hpp>

#include <iostream>
#include <cstring>
#include <chrono>

#define PRINT_VEC4(name, vec) std::cout << name << " x: " << vec.x << " y: " << vec.y << " z: " << vec.z << " w: " << vec.w << "\n";
#define PRINT_VEC3(name, vec) std::cout << name << " x: " << vec.x << " y: " << vec.y << " z: " << vec.z << "\n";

CPUPhysicsParallel::CPUPhysicsParallel(const CreateInfo &info) : updateDelta(info.updateDelta), accumulator(0) {
  TimeLogDestructor physics("Physics system initialization");
  
  this->pool = info.pool;
  this->broad = info.b;
  this->narrow = info.n;
  this->solver = info.s;
  this->sorter = info.sorter;

  verts.vector().reserve(3000);
  verts.update();

  objects.vector().reserve(3000);
  objects.update();

  physicsDatas.vector().reserve(3000);
  physicsDatas.update();

  staticPhysicDatas.vector().reserve(3000);
  staticPhysicDatas.update();

  indices.resize(3000);

  globalVel.vector().reserve(3000);
  globalVel.update();

  memset(dataIndices.data(), 0, sizeof(DataIndices));
  memset(raysIndices.data(), 0, sizeof(DataIndices));

  this->defaultStaticMatrixIndex = info.buffers->defaultStaticMatrixIndex;
  this->defaultDynamicMatrixIndex = info.buffers->defaultDynamicMatrixIndex;

  setBuffers(*info.buffers);

  matrices->at(defaultStaticMatrixIndex) = simd::mat4(1.0f);
  matrices->at(defaultDynamicMatrixIndex) = simd::mat4(1.0f);

  updateInputOutput();
}

CPUPhysicsParallel::~CPUPhysicsParallel() {}

void CPUPhysicsParallel::update(const uint64_t &time) {
  Gravity* data = gravityBuffer.data();
  data->objCount = physicsDatas.size();
  data->time = updateDelta;
  //data->time = time;
  data->gravity = gravity;
  data->gravityNormal = gravityNorm;
  data->length2 = gravLength2;
  data->length = gravLength;
  
  // по идее обновление буферов бы тоже разнести, но думаю что можно их просто продублировать
  updateBuffers();
  broad->updateBuffers(objects.size(), physicsDatas.size(), rays.size(), frustums.size());
  narrow->updateBuffers(overlappingPairCache.size(), staticOverlappingPairCache.size());
  solver->updateBuffers(overlappingPairCache.size() + staticOverlappingPairCache.size(), rayPairCache.size());

  // RegionLog rl("Physics", true);

  // какое тут должно быть максимальное время? нафига оно вообще мне здесь?
  const size_t frameTime = std::min(time, size_t(250000));
  accumulator += frameTime;
  
  if (accumulator > ACCUMULATOR_MAX_CONSTANT) {
    accumulator = accumulator % ACCUMULATOR_MAX_CONSTANT;
    
    Global::console()->printW("Physics lags detected. Check your PC suitability for the game minimal requirements or remove some reundant mods");
  }
  
  while (accumulator >= updateDelta) {
    memcpy(prevState.data(), currState.data(), currState.size()*sizeof(currState[0]));
    
    // тут наверное можно добавить еще цикл
    // для большей точности вычислений
    
    {
      // RegionLog rl("update velocities");
      updateVelocities();
    }

//     PRINT_VEC3("velocity", physicsDatas[0].velocity)

    broad->update();

    broad->calculateOverlappingPairs();

    sorter->sort(&overlappingPairCache, 1);

    narrow->checkIdenticalPairs();

    pool->submitnr([&] () {
      sorter->sort(&overlappingPairCache);
    });

    pool->submitnr([&] () {
      sorter->sort(&staticOverlappingPairCache);
    });

    pool->compute();
    pool->wait();

    narrow->postCalculation();
    solver->calculateData();
    solver->solve();
    
    overlappingDataSize = dataIndices.data()->count;
    triggerPairsIndicesSize = dataIndices.data()->triggerIndicesCount;

    accumulator -= updateDelta;
  }

  const float alpha = float(accumulator) / float(updateDelta);

  interpolate(alpha);

  broad->update();
  broad->calculateRayTests();
  broad->calculateFrustumTests();

  solver->calculateRayData();

  pool->submitnr([&] () {
    sorter->sort(&frustumTestsResult);
  });

  pool->submitnr([&] () {
    sorter->sort(&overlappingData, &dataIndices, 0);
  });

  pool->submitnr([&] () {
    sorter->sort(&raysData, &raysIndices, 1);
  });

  pool->compute();
  pool->wait();

//   overlappingDataSize = dataIndices.data()->count;
//   triggerPairsIndicesSize = dataIndices.data()->triggerIndicesCount;
  rayTracingSize = raysIndices.data()->temporaryCount;
  frustumTestSize = frustumTestsResult[0].firstIndex;

//   memset(rays.data(), UINT32_MAX, sizeof(RayData));
//   memset(frustums.data(), UINT32_MAX, sizeof(FrustumStruct));
  memset(frustumTestsResult.data(), 0, sizeof(BroadphasePair));

  indices[0] = 0;

  rays.vector().clear();
  rays.update();
  frustums.vector().clear();
  frustums.update();
  frustumPoses.vector().clear();
  frustumPoses.update();
}

// void CPUPhysicsParallel::decoupledUpdate(const uint64_t &time) {
//   updateBuffers();
//   broad->updateBuffers(objects.size(), physicsDatas.size(), rays.size(), frustums.size());
//   narrow->updateBuffers(overlappingPairCache.size(), staticOverlappingPairCache.size());
//   solver->updateBuffers(overlappingPairCache.size() + staticOverlappingPairCache.size(), rayPairCache.size());
//   
//   broad->update();
//   broad->calculateRayTests();
//   broad->calculateFrustumTests();
// 
//   solver->calculateRayData();
// 
//   pool->submitnr([&] () {
//     sorter->sort(&frustumTestsResult);
//   });
// 
//   pool->submitnr([&] () {
//     sorter->sort(&overlappingData, &dataIndices, 0);
//   });
// 
//   pool->submitnr([&] () {
//     sorter->sort(&raysData, &raysIndices, 1);
//   });
// 
//   pool->compute();
//   pool->wait();
// 
//   rayTracingSize = raysIndices.data()->temporaryCount;
//   frustumTestSize = frustumTestsResult[0].firstIndex;
// 
// //   memset(rays.data(), UINT32_MAX, sizeof(RayData));
// //   memset(frustums.data(), UINT32_MAX, sizeof(FrustumStruct));
//   memset(frustumTestsResult.data(), 0, sizeof(BroadphasePair));
// 
//   indices[0] = 0;
// 
//   rays.vector().clear();
//   rays.update();
//   frustums.vector().clear();
//   frustums.update();
//   frustumPoses.vector().clear();
//   frustumPoses.update();
// }

void CPUPhysicsParallel::setBuffers(const PhysicsExternalBuffers &buffers) {
  this->defaultStaticMatrixIndex = buffers.defaultStaticMatrixIndex;
  this->defaultDynamicMatrixIndex = buffers.defaultDynamicMatrixIndex;

  this->inputs = buffers.inputs;
  this->transforms = buffers.transforms;
  this->matrices = buffers.matrices;
  this->rotationDatasCount = buffers.rotationDatasCount;
  this->rotationDatas = buffers.rotationDatas;
  this->externalDatas = buffers.externalDatas;
}

void CPUPhysicsParallel::registerShape(const Type &type, const uint32_t shapeType, const RegisterNewShapeInfo &info) {
  auto itr = shapes.find(type);
  if (itr != shapes.end()) throw std::runtime_error("Shape type " + type.getName() + " is already exist!");

  if (shapeType == SPHERE_TYPE) throw std::runtime_error("Dont need to register sphere");
  if (!(shapeType == BBOX_TYPE || shapeType == POLYGON_TYPE)) throw std::runtime_error("This type is not supported yet");

  if (info.faces.empty()) throw std::runtime_error("Wrong creation data");

  uint32_t additionalFaceOffset = 0;
  uint32_t controlCount = 0;
  if (shapeType == BBOX_TYPE) {
    if (info.faces.empty() || info.faces.size() > 1) throw std::runtime_error("Wrong box data");

    controlCount = 1;
  } else {
    additionalFaceOffset = 1;
    controlCount = info.points.size() + info.faces.size() + 1;

    if (info.points.empty() || info.faces.empty() || controlCount < 6) throw std::runtime_error("Wrong polygon data");
  }

  ShapeInfo s{
    shapeType,
    UINT32_MAX,
    0,
    0
  };

  uint32_t oldIndex = UINT32_MAX;
  s.offset = freeVert;
  while (s.offset != UINT32_MAX && glm::floatBitsToUint(verts[s.offset].y) != controlCount) {
    oldIndex = s.offset;
    s.offset = glm::floatBitsToUint(verts[s.offset].x);
  }

  if (s.offset != UINT32_MAX) {
    if (oldIndex == UINT32_MAX) {
      freeVert = glm::floatBitsToUint(verts[freeVert].x);
    } else {
      verts[oldIndex].x = verts[s.offset].x;
    }
  } else {
    // TODO: может быть ненужно ресайзить каждый раз?
    s.offset = verts.size();
    verts.resize(verts.size() + controlCount);
  }

  s.pointCount = info.points.size();
  s.faceCount = info.faces.size();

  simd::vec4 localCenter = simd::vec4(0.0f, 0.0f, 0.0f, 0.0f);
  for (uint32_t i = 0; i < info.points.size(); ++i) {
    verts[s.offset + i] = info.points[i];

    //if (info.points[i].w != 1.0f) throw std::runtime_error("bad vertex");

    localCenter += info.points[i];
  }
  localCenter /= float(info.points.size());

  verts[s.offset + s.pointCount] = localCenter;

  for (uint32_t i = 0; i < info.faces.size(); ++i) {
    verts[s.offset + s.pointCount + additionalFaceOffset + i] = info.faces[i];
    //if (info.faces[i].w == 6.0f) throw std::runtime_error("bad normal");
  }

  shapes[type] = s;
}

void CPUPhysicsParallel::removeShape(const Type &type) {
  auto itr = shapes.find(type);
  if (itr == shapes.end()) return;
  
  shapes.erase(itr);
}

void CPUPhysicsParallel::add(const PhysicsObjectCreateInfo &info, PhysicsIndexContainer* container) {
  if (defaultStaticMatrixIndex == UINT32_MAX || defaultDynamicMatrixIndex == UINT32_MAX) throw std::runtime_error("No default matrix");

  container->objectDataIndex = UINT32_MAX;
  container->physicDataIndex = UINT32_MAX;
  container->transformIndex = UINT32_MAX;
  container->inputIndex = UINT32_MAX;
  container->rotationIndex = UINT32_MAX;

  if (info.transformIndex == UINT32_MAX && info.type.getObjType() != POLYGON_TYPE) throw std::runtime_error("Cannot create box or sphere without transform");
  if (info.inputIndex == UINT32_MAX && info.type.isDynamic()) throw std::runtime_error("Cannot create dynamic object without input");
  if (info.externalDataIndex == UINT32_MAX && info.type.isDynamic()) throw std::runtime_error("Cannot create dynamic object without external data");

  if (freeContainer == SIZE_MAX) {
    container->internalIndex = components.size();
    components.push_back(container);
  } else {
    container->internalIndex = freeContainer;
    freeContainer = reinterpret_cast<size_t&>(components[freeContainer]);
    components[container->internalIndex] = container;
  }

  uint32_t staticPhysIndex = 0;
  if (freeObj != UINT32_MAX) {
    container->objectDataIndex = freeObj;
    staticPhysIndex = freeObj;

    freeObj = objects[freeObj].objectId;
  } else {
    container->objectDataIndex = objects.size();
    objects.vector().emplace_back();
    objects.update();

    staticPhysIndex = staticPhysicDatas.size();
    staticPhysicDatas.vector().emplace_back();
    staticPhysicDatas.update();
  }

  if (info.type.isDynamic()) {
    if (freePhys != UINT32_MAX) {
      container->physicDataIndex = freePhys;
      freePhys = glm::floatBitsToUint(physicsDatas[freePhys].velocity.x);
    } else {
      container->physicDataIndex = physicsDatas.size();
      physicsDatas.vector().emplace_back();
      physicsDatas.update();
    }

//     std::cout << "container->objectDataIndex " << container->objectDataIndex << "\n";
  }

  container->transformIndex = info.transformIndex;
  if (info.transformIndex != UINT32_MAX) {
    if (info.transformIndex >= currState.size()) {
      currState.resize(info.transformIndex+1);
      prevState.resize(info.transformIndex+1);
    }

    currState[info.transformIndex] = transforms->at(info.transformIndex);
    prevState[info.transformIndex] = transforms->at(info.transformIndex);
  }

  container->inputIndex = info.inputIndex;

  container->rotationIndex = info.rotationIndex;

//   std::cout << "container->objectDataIndex " << container->objectDataIndex << "\n";
  const uint32_t proxyIndex = broad->createProxy({}, container->objectDataIndex, info.type, info.collisionGroup, info.collisionFilter);

  const Object obj{
    container->internalIndex,
    proxyIndex,
    staticPhysIndex,
    container->transformIndex,

    0,
    0,
    0,
    info.type,

    info.matrixIndex == defaultDynamicMatrixIndex || info.matrixIndex == defaultStaticMatrixIndex || info.matrixIndex == UINT32_MAX ?
                          (info.type.isDynamic() ? defaultDynamicMatrixIndex : defaultStaticMatrixIndex) : info.matrixIndex,
    UINT32_MAX,
    container->rotationIndex,
    0
  };

  objects[container->objectDataIndex] = obj;

  const Constants c{
    info.groundFricion,
    info.overbounce,
    info.stairHeight,
    container->physicDataIndex
  };

  staticPhysicDatas[staticPhysIndex] = c;

  if (container->physicDataIndex != UINT32_MAX) {
    const PhysData2 data{
      glm::vec3(0.0f, 0.0f, 0.0f),
      0.0f,
      simd::vec4(0.0f, 0.0f, 0.0f, 0.0f),

      container->objectDataIndex,
      container->inputIndex,
      0xFFFFFFFF,
      0xFFFFFFFF,

      info.externalDataIndex,
      container->transformIndex,
      staticPhysIndex,
      0
    };

    physicsDatas[container->physicDataIndex] = data;
  }

  auto itr = shapes.find(info.shapeType);
  if (itr == shapes.end()) throw std::runtime_error("Shape " + info.shapeType.getName() + " was not registered");
  if (itr->second.shapeType != info.type.getObjType()) throw std::runtime_error("Wrong shape for object");

  objects[container->objectDataIndex].vertexOffset = itr->second.offset;
  objects[container->objectDataIndex].vertexCount = itr->second.pointCount;
  objects[container->objectDataIndex].faceCount = itr->second.faceCount;

  if (!info.type.isDynamic()) {
    const uint32_t id = indices[0];
    ++indices[0];

    if (id+1 >= indices.size()) indices.resize(indices.size()*1.5f);

    indices[id+1] = container->objectDataIndex;
  }
}

void CPUPhysicsParallel::remove(PhysicsIndexContainer* comp) {
//   components.back()->internalIndex = comp->internalIndex;
//   std::swap(components[comp->internalIndex], components.back());
//   components.pop_back();

  components[comp->internalIndex] = reinterpret_cast<PhysicsIndexContainer*&>(freeContainer);
  freeContainer = comp->internalIndex;

//   std::cout << "objDataIndex " << comp->objectDataIndex << "\n";
//   std::cout << "proxy index  " << objects[comp->objectDataIndex].proxyIndex << "\n";

  broad->destroyProxy(objects[comp->objectDataIndex].proxyIndex);

  objects[comp->objectDataIndex].objectId = freeObj;
  freeObj = comp->objectDataIndex;

  if (comp->physicDataIndex != UINT32_MAX) {
    physicsDatas[comp->physicDataIndex].velocity.x = glm::uintBitsToFloat(freePhys);
    physicsDatas[comp->physicDataIndex].objectIndex = 0xFFFFFFFF;
    freePhys = comp->physicDataIndex;
  }
  
  comp->inputIndex = UINT32_MAX;
  comp->internalIndex = UINT32_MAX;
  comp->objectDataIndex = UINT32_MAX;
  comp->physicDataIndex = UINT32_MAX;
  comp->rotationIndex = UINT32_MAX;
  comp->transformIndex = UINT32_MAX;
}

// нужно добавлять заново кадый кадр
uint32_t CPUPhysicsParallel::add(const RayData &ray) {
  // эту функцию вполне можно выполнять параллельно

  uint32_t index = rays.vector().size();
  // нормализация??
  rays.vector().push_back(ray);
  rays.update();

  return index;
}

uint32_t CPUPhysicsParallel::add(const simd::mat4 &frustum, const simd::vec4 &pos) {
  uint32_t index = frustums.vector().size();
  // здесь он будет все пересчитывать
  frustums.vector().emplace_back(frustum);
  frustums.update();

  frustumPoses.vector().push_back(pos);
  frustumPoses.update();

  return index;
}

Object & CPUPhysicsParallel::getObjectData(const PhysicsIndexContainer* container) {
  return objects[container->objectDataIndex];
}

const Object & CPUPhysicsParallel::getObjectData(const PhysicsIndexContainer* container) const {
  return objects[container->objectDataIndex];
}

PhysData2 & CPUPhysicsParallel::getPhysicData(const PhysicsIndexContainer* container) {
  return physicsDatas[container->physicDataIndex];
}

const PhysData2 & CPUPhysicsParallel::getPhysicData(const PhysicsIndexContainer* container) const {
  return physicsDatas[container->physicDataIndex];
}

const BroadphaseProxy* CPUPhysicsParallel::getObjectBroadphaseProxy(const PhysicsIndexContainer* container) const {
  const Object &obj = objects[container->objectDataIndex];
  return broad->getProxy(obj.proxyIndex);
}

simd::vec4 CPUPhysicsParallel::getGlobalVelocity(const PhysicsIndexContainer* container) const {
  return globalVel[container->physicDataIndex];
}

uint32_t CPUPhysicsParallel::getObjectShapePointsSize(const PhysicsIndexContainer* container) const {
  const Object &obj = objects[container->objectDataIndex];
  return obj.vertexCount;
}

const simd::vec4* CPUPhysicsParallel::getObjectShapePoints(const PhysicsIndexContainer* container) const {
  const Object &obj = objects[container->objectDataIndex];
  return &verts[obj.vertexOffset];
}

uint32_t CPUPhysicsParallel::getObjectShapeFacesSize(const PhysicsIndexContainer* container) const {
  const Object &obj = objects[container->objectDataIndex];
  return obj.faceCount;
}

const simd::vec4* CPUPhysicsParallel::getObjectShapeFaces(const PhysicsIndexContainer* container) const {
  const Object &obj = objects[container->objectDataIndex];
  return &verts[obj.vertexOffset+obj.vertexCount+1];
}

uint32_t CPUPhysicsParallel::getTransformIndex(const PhysicsIndexContainer* container) const {
  return container->transformIndex;
}

uint32_t CPUPhysicsParallel::getRotationDataIndex(const PhysicsIndexContainer* container) const {
  return container->rotationIndex;
}

uint32_t CPUPhysicsParallel::getMatrixIndex(const PhysicsIndexContainer* container) const {
  return objects[container->objectDataIndex].coordinateSystemIndex;
}

uint32_t CPUPhysicsParallel::getExternalDataIndex(const PhysicsIndexContainer* container) const {
  return physicsDatas[container->physicDataIndex].externalDataIndex;
}

uint32_t CPUPhysicsParallel::getInputDataIndex(const PhysicsIndexContainer* container) const {
  return container->inputIndex;
}

void* CPUPhysicsParallel::getUserData(const uint32_t &objIndex) const {
  const Object &obj = objects[objIndex];
  return components[obj.objectId]->userData;
}

PhysicsIndexContainer* CPUPhysicsParallel::getIndexContainer(const uint32_t &objIndex) const {
  const Object &obj = objects[objIndex];
  return components[obj.objectId];
}

void CPUPhysicsParallel::setGravity(const simd::vec4 &g) {
  gravity = g;
  gravityNorm = simd::normalize(g);
  gravLength2 = simd::length2(g);
  gravLength  = glm::sqrt(gravLength2);
  orientation = simd::orientation(-gravityNorm, simd::vec4(0.0f, 1.0f, 0.0f, 0.0f));

  matrices->at(defaultDynamicMatrixIndex) = orientation;
}

// void CPUPhysicsParallel::updateMaxSpeed(const uint32_t &physicDataIndex, const float &maxSpeed) {
//   (void)physicDataIndex;
//   (void)maxSpeed;
//   throw std::runtime_error("must not be called");
// }
// 
// 
// uint32_t CPUPhysicsParallel::setShapePointsAndFaces(const uint32_t &objectDataIndex, const std::vector<simd::vec4> &points, const std::vector<simd::vec4> &faces) {
//   (void)objectDataIndex;
//   (void)points;
//   (void)faces;
//   throw std::runtime_error("not implemented yet");
// }

ArrayInterface<OverlappingData>* CPUPhysicsParallel::getOverlappingPairsData() {
  return &overlappingData;
}

const ArrayInterface<OverlappingData>* CPUPhysicsParallel::getOverlappingPairsData() const {
  return &overlappingData;
}

ArrayInterface<uint32_t>* CPUPhysicsParallel::getTriggerPairsIndices() {
  return &triggerIndices;
}

const ArrayInterface<uint32_t>* CPUPhysicsParallel::getTriggerPairsIndices() const {
  return &triggerIndices;
}

ArrayInterface<OverlappingData>* CPUPhysicsParallel::getRayTracingData() {
  return &raysData;
}

const ArrayInterface<OverlappingData>* CPUPhysicsParallel::getRayTracingData() const {
  return &raysData;
}

ArrayInterface<BroadphasePair>* CPUPhysicsParallel::getFrustumPairs() {
  return &frustumTestsResult;
}

const ArrayInterface<BroadphasePair>* CPUPhysicsParallel::getFrustumPairs() const {
  return &frustumTestsResult;
}

void CPUPhysicsParallel::printStats() {
  const size_t memoryUsed = verts.vector().size()*sizeof(verts[0]) +
                            objects.vector().size()*sizeof(objects[0]) +
                            physicsDatas.vector().size()*sizeof(physicsDatas[0]) +
                            //transforms.vector().size()*sizeof(transforms[0]) +
                            //inputs.vector().size()*sizeof(inputs[0]) +
                            indices[0]*sizeof(indices[0]) +
                            //coordinateSystems.vector().size()*sizeof(coordinateSystems[0]) +
                            globalVel.vector().size()*sizeof(globalVel[0]) + sizeof(Gravity);

  const size_t memory = verts.vector().capacity() * sizeof(verts[0]) +
                        objects.vector().capacity() * sizeof(objects[0]) +
                        physicsDatas.vector().capacity() * sizeof(physicsDatas[0]) +
                        //transforms.vector().capacity() * sizeof(transforms[0]) +
                        //inputs.vector().capacity() * sizeof(inputs[0]) +
                        indices.vector().capacity() * sizeof(indices[0]) +
                        //coordinateSystems.vector().capacity() * sizeof(coordinateSystems[0]) +
                        globalVel.vector().capacity() * sizeof(globalVel[0]) + sizeof(Gravity);

  std::cout << "Physics engine data" << '\n';
  std::cout << "Object count           " << objects.size() << '\n';
  std::cout << "Dynamic object count   " << physicsDatas.size() << '\n';
  std::cout << "Total memory usage     " << memory << " bytes" << "\n";
  std::cout << "Total memory with data " << memoryUsed << " bytes" << "\n";
  std::cout << "Free memory            " << (memory - memoryUsed) << " bytes" << '\n';
  std::cout << "Physics class size     " << sizeof(CPUPhysicsParallel) << " bytes" << '\n';

  broad->printStats();
  narrow->printStats();
  solver->printStats();
  sorter->printStats();
}

void CPUPhysicsParallel::syncThreadPool() {
  pool->compute();
  pool->wait();
}

void CPUPhysicsParallel::updateBuffers() {
  if (objects.size()+1 > indices.size()) indices.resize(objects.size()+1);
  if (physicsDatas.size() > globalVel.size()) globalVel.resize(physicsDatas.size());
}

void CPUPhysicsParallel::updateInputOutput() {
  {
    Broadphase::InputBuffers input{
      &indices,
      &verts,
      &objects,
      matrices,
      //transforms,
      &currState,
      rotationDatas,
      &rays,
      &frustums,
      &frustumPoses
    };

    broad->setInputBuffers(input);

    Broadphase::OutputBuffers output{
      &overlappingPairCache,
      &staticOverlappingPairCache,
      &rayPairCache,
      &frustumTestsResult
    };

    // нужно будет сюда добавить индирект буфер
    broad->setOutputBuffers(output);
  }

  {
    Narrowphase::InputBuffers input{
      &overlappingPairCache,
      &staticOverlappingPairCache,
    };

    narrow->setInputBuffers(input);

    Narrowphase::OutputBuffers output{
      &islands,
      &staticIslands
    };

    narrow->setOutputBuffers(output);
  }

  {
    Solver::InputBuffers input{
      &objects,
      &verts,
      matrices,
      &physicsDatas,
      //transforms,
      &currState,
      &staticPhysicDatas,
      rotationDatas,

      &overlappingPairCache,
      &staticOverlappingPairCache,

      &islands,
      &staticIslands,

      &indices,
      &globalVel,
      &gravityBuffer,
      &rays,

      &rayPairCache
    };

    // нужно добавить индирект буферы
    solver->setInputBuffers(input);

    Solver::OutputBuffers output{
      &overlappingData,
      &dataIndices,
      &raysData,
      &raysIndices,
      &triggerIndices
    };

    solver->setOutputBuffers(output);
  }
}

#define AIR_ACCELERATION_DIR 0.0f
#define STOP_SPEED 4.0f
#define AIR_ACCELERATION 5.0f
#define DEFAULT_GROUND_FRICTION 4.0f

void computeGroundVelocity(const simd::vec4 &accelerationDir,
                           const simd::vec4 &oldVelocity,
                           const uint32_t &dt,
                           const bool jump,
                           const simd::vec4 &additionalForce,
                           const simd::vec4 &gravityNorm,
                           const float &maxSpeed,
                           const float &acceleration,
                           const float &groundFriction,
                             simd::vec4 &velocity,
                                 float &velocityScalar) {
  const float dt1 = MCS_TO_SEC(dt);

  const simd::vec4 finalAcceleretionDir = accelerationDir * acceleration;
  const simd::vec4 a = finalAcceleretionDir + additionalForce;

  const float stopspeed = velocityScalar < STOP_SPEED ? STOP_SPEED : velocityScalar;
  // фриктион? additionalData.y
  float fr = velocityScalar - stopspeed*groundFriction*dt1;
  fr = glm::max(fr, 0.0f);
  fr = velocityScalar == 0.0f ? 0.0f : fr / velocityScalar;

  velocity = oldVelocity*fr + a * dt1;

  float newSpeed = simd::length(velocity);

  const float jumpKoef = (4.0f / dt1);
  const float jumpAcceleration = jumpKoef * float(jump);
  const simd::vec4 jumpAccelerationDir = -gravityNorm * jumpAcceleration;

  if (newSpeed <= EPSILON) {
    velocity = simd::vec4(0.0f, 0.0f, 0.0f, 0.0f) + jumpAccelerationDir*dt1;
    velocityScalar = jumpAcceleration*dt1;
    return;
  }

  const simd::vec4 newVelNorm = projectVectorOnPlane(-gravityNorm, simd::vec4(0.0f, 0.0f, 0.0f, 1.0f), velocity / newSpeed);

  newSpeed = glm::min(newSpeed, maxSpeed);

  velocity = newVelNorm * newSpeed + jumpAccelerationDir*dt1; // - globalData.gravity*aJump;
  velocityScalar = simd::length(velocity);
}

void computeAirVelocity(const simd::vec4 &accelerationDir,
                        const simd::vec4 &oldVelocity,
                        const uint32_t  &dt,
                        const simd::vec4 &additionalForce,
                        const simd::vec4 &gravity,
                        simd::vec4 &velocity,
                        float &velocityScalar) {
  const float dt1 = MCS_TO_SEC(dt);

  const simd::vec4 vn = velocityScalar > EPSILON ? oldVelocity / velocityScalar : simd::vec4(0.0f, 0.0f, 0.0f, 0.0f);

  const float airFriction = 0.0f;
  const simd::vec4 a = -simd::vec4(vn)*airFriction + additionalForce + gravity + AIR_ACCELERATION_DIR*accelerationDir*AIR_ACCELERATION;

  velocity = oldVelocity + a * dt1;
  velocityScalar = simd::length(velocity);
}

// simd::vec4 projectVectorOnPlane(const simd::vec4 normal, const simd::vec4 origin, const simd::vec4 vector) {
//   float dist = glm::dot(vector, normal);
//   simd::vec4 point = origin + vector - normal*dist;
//   return point - origin;
// }

bool feq(const float first, const float second) {
  return abs(first - second) < EPSILON;
}

void CPUPhysicsParallel::updateVelocities() {
  //static const auto calcVel = [&] (const uint32_t &index) {
  static const auto calcVel = [&] (const uint32_t &start, const uint32_t &count) {
    for (uint32_t index = start; index < start+count; ++index) {
      if (physicsDatas[index].objectIndex == UINT32_MAX) return;

      const PhysData2 &current = physicsDatas[index];
      const uint32_t &transIndex = current.transformIndex;
      //const Transform &trans = transforms->at(transIndex);
      const Transform &trans = currState.at(transIndex);
      const uint32_t externalDataIndex = current.externalDataIndex;

      const uint32_t &groundPhysicsIndex = current.groundIndex;

      const InputData &currentInput = inputs->at(current.inputIndex);
      simd::vec4 frontOnGround = currentInput.moves.z == 0.0f ? simd::vec4(0.0f, 0.0f, 0.0f, 0.0f) :
        projectVectorOnPlane(-gravityBuffer.data()->gravityNormal, trans.pos, currentInput.front);

      const float &lengthFront = simd::length(frontOnGround);
      if (lengthFront > EPSILON) frontOnGround /= lengthFront;
      else frontOnGround = simd::vec4(0.0f, 0.0f, 0.0f, 0.0f);

      const simd::vec4 &acceleration = frontOnGround * currentInput.moves.z + currentInput.right * currentInput.moves.x;// + currentInput.up * currentInput.moves.y;

      const float accelLenght = simd::length(acceleration);
      simd::vec4 aDir = simd::vec4(0.0f, 0.0f, 0.0f, 0.0f);
      if (accelLenght > EPSILON) aDir = acceleration / accelLenght;

      const simd::vec4 additionalForce = externalDatas->at(externalDataIndex).additionalForce;
      const float maxSpeed = externalDatas->at(externalDataIndex).maxSpeed;
      const float accelerationScalar = externalDatas->at(externalDataIndex).acceleration;

      const simd::vec4 oldVel = simd::vec4(current.velocity.x, current.velocity.y, current.velocity.z, 0.0f);
      float scalar = simd::length(oldVel);
      simd::vec4 vel = oldVel;

//       std::cout << "index " << index << " scalar " << scalar << "\n";

      const float groundFriction = groundPhysicsIndex != UINT32_MAX ?
                                    staticPhysicDatas[groundPhysicsIndex].groundFriction : DEFAULT_GROUND_FRICTION;

      if ((physicsDatas[index].onGroundBits & 0x1) == 0x1) {
        computeGroundVelocity(aDir,
                              oldVel,
                              gravityBuffer.data()->time,
                              bool(currentInput.moves.y),
                              additionalForce,
                              gravityBuffer.data()->gravityNormal,
                              maxSpeed,
                              accelerationScalar,
                              groundFriction,
                              vel,
                              scalar);

//         const float dt1 = MCS_TO_SEC(gravityBuffer.data()->time);
//
//         const simd::vec4 finalAcceleretionDir = aDir * accelerationScalar;
//         const simd::vec4 a = finalAcceleretionDir + additionalForce;
//
//         const float stopspeed = scalar < STOP_SPEED ? STOP_SPEED : scalar;
//         // фриктион? additionalData.y
//         float fr = scalar - stopspeed*groundFriction*dt1;
//         fr = glm::max(fr, 0.0f);
//         fr = scalar == 0.0f ? 0.0f : fr / scalar;
//
//         vel = oldVel*fr + a * dt1;
//
//         float newSpeed = glm::length(vel);
//
//         const float jumpKoef = (4.0f / dt1);
//         const float jumpAcceleration = jumpKoef * float(currentInput.moves.y);
//         const simd::vec4 jumpAccelerationDir = -gravityBuffer.data()->gravityNormal * jumpAcceleration;
//
//         if (newSpeed <= EPSILON) {
//           vel = simd::vec4(0.0f, 0.0f, 0.0f, 0.0f) + jumpAccelerationDir*dt1;
//           scalar = jumpAcceleration*dt1;
//           return;
//         }
//
//         const simd::vec4 newVelNorm = vel / newSpeed;
//
//         newSpeed = glm::min(newSpeed, maxSpeed);
//
//         vel = newVelNorm * newSpeed + jumpAccelerationDir*dt1; // - globalData.gravity*aJump;
//         scalar = glm::length(vel);
    } else {
//         computeAirVelocity(aDir,
//                           oldVel,
//                           gravityBuffer.data()->time,
//                           additionalForce,
//                           gravityBuffer.data()->gravity,
//                           vel,
//                           scalar);

        const float dt1 = MCS_TO_SEC(gravityBuffer.data()->time);

        const simd::vec4 vn = scalar > EPSILON ? oldVel / scalar : simd::vec4(0.0f, 0.0f, 0.0f, 0.0f);

        const float airFriction = 0.0f;
        const simd::vec4 a = -simd::vec4(vn)*airFriction + additionalForce + gravity + AIR_ACCELERATION_DIR*aDir*AIR_ACCELERATION;

        vel = oldVel + a * dt1;
        scalar = simd::length(vel);

        physicsDatas[index].groundIndex = UINT32_MAX;
      }

      physicsDatas[index].scalar = scalar;
      //physicsDatas[index].velocity = glm::vec3(vel);
      float arr[4];
      vel.store(arr);
      physicsDatas[index].velocity = glm::vec3(arr[0], arr[1], arr[2]);

      physicsDatas[index].onGroundBits = bool(physicsDatas[index].onGroundBits & 0x1) && currentInput.moves.y > 0.0f ? 0 : physicsDatas[index].onGroundBits;
      if (!bool(physicsDatas[index].onGroundBits & 0x1)) physicsDatas[index].groundIndex = UINT32_MAX;
    }
  };

  //static const auto calcPos = [&] (const uint32_t &index, std::atomic<uint32_t> &counter) {
  static const auto calcPos = [&] (const uint32_t &start, const uint32_t &count, std::atomic<uint32_t> &counter) {
    for (uint32_t index = start; index < start+count; ++index) {
      if (physicsDatas[index].objectIndex == UINT32_MAX) return;
      
      const glm::vec3 &vel = physicsDatas[index].velocity;
      simd::vec4 velocity = simd::vec4(vel.x, vel.y, vel.z, 0.0f);

      uint32_t groundIndex = physicsDatas[index].groundIndex;
      while (groundIndex != UINT32_MAX) {
        const uint32_t physDataIndex = staticPhysicDatas[groundIndex].physDataIndex;

        if (physDataIndex != UINT32_MAX) {
          const PhysData2 &data = physicsDatas[physDataIndex];
          const glm::vec3 &vel = data.velocity;
          velocity += simd::vec4(vel.x, vel.y, vel.z, 0.0f);
          groundIndex = data.groundIndex;
        } else {
          groundIndex = UINT32_MAX;
        }
      }

      const float globalScalar = simd::length(velocity);

      const uint32_t &transIndex = physicsDatas[index].transformIndex;

      //physicsDatas[index].oldPos = transforms->at(transIndex).pos;
      physicsDatas[index].oldPos = currState.at(transIndex).pos;
      //transforms->at(transIndex).pos = transforms->at(transIndex).pos + velocity*MCS_TO_SEC(gravityBuffer.data()->time);
      currState.at(transIndex).pos += velocity*MCS_TO_SEC(gravityBuffer.data()->time);

      globalVel[index] = velocity;

      if (globalScalar > EPSILON) {
        // const uint id = atomicAdd(count, 1);
        // indexies[id] = datas[index].objectIndex;
        const uint32_t id = counter.fetch_add(1);

        indices[id+1] = physicsDatas[index].objectIndex;
      }
    }
  };

//   std::cout << "physicsDatas.size() " << physicsDatas.size() << "\n";
//   if (physicsDatas.size() > 2) throw std::runtime_error("physicsDatas.size() > 2");

  const size_t count = std::ceil(float(physicsDatas.size()) / float(pool->size()+1));
  size_t start = 0;
  for (uint32_t i = 0; i < pool->size()+1; ++i) {
    const size_t jobCount = std::min(count, physicsDatas.size()-start);
    if (jobCount == 0) break;

    pool->submitnr(calcVel, start, jobCount);

    start += jobCount;
  }

//   for (uint32_t i = 0; i < physicsDatas.size(); ++i) {
//     pool->submitnr(calcVel, i);
//   }

  pool->compute();
  pool->wait();

  std::atomic<uint32_t> counter(indices[0]);

  start = 0;
  for (uint32_t i = 0; i < pool->size()+1; ++i) {
    const size_t jobCount = std::min(count, physicsDatas.size()-start);
    if (jobCount == 0) break;

//     pool->submitnr(calcPos, i, std::ref(counter));
    pool->submitnr(calcPos, start, jobCount, std::ref(counter));

    start += jobCount;
  }

  pool->compute();
  pool->wait();

  indices[0] = counter;
}

void CPUPhysicsParallel::updateRotationDatas() {
//   static const auto calcRotationData = [&] (const uint32_t &index, RotationData* data) {
// //     rotationDatas->at(index).currentAngle += rotationDatas->at(index).rotationSpeed;
// //     rotationDatas->at(index).currentAngle = glm::min(rotationDatas->at(index).currentAngle, rotationDatas->at(index).maxAngle);
// //     rotationDatas->at(index).currentAngle = glm::max(rotationDatas->at(index).currentAngle, 0.0f);
// //
// //     const simd::vec4 &anchorDir = simd::vec4(rotationDatas->at(index).anchorDir, 0.0f);
// //     const float &anchorDist = rotationDatas->at(index).anchorDist;
// //
// //     rotationDatas->at(index).matrix = glm::translate(glm::mat4(1.0f), glm::vec3(anchorDir * anchorDist));
// //     rotationDatas->at(index).matrix = glm::rotate(rotationDatas->at(index).matrix, rotationDatas->at(index).currentAngle, glm::vec3(rotationDatas->at(index).rotationNormal));
// //     rotationDatas->at(index).matrix = glm::translate(rotationDatas->at(index).matrix, -glm::vec3(anchorDir * anchorDist));
//
//     data[index].currentAngle += data[index].rotationSpeed;
//     data[index].currentAngle = glm::min(data[index].currentAngle, data[index].maxAngle);
//     data[index].currentAngle = glm::max(data[index].currentAngle, 0.0f);
//
//     const simd::vec4 &anchorDir = simd::vec4(data[index].anchorDir, 0.0f);
//     const float &anchorDist = data[index].anchorDist;
//
//     data[index].matrix = glm::translate(glm::mat4(1.0f), glm::vec3(anchorDir * anchorDist));
//     data[index].matrix = glm::rotate(data[index].matrix, data[index].currentAngle, glm::vec3(data[index].rotationNormal));
//     data[index].matrix = glm::translate(data[index].matrix, -glm::vec3(anchorDir * anchorDist));
//   };
//
//   // вобщем с ротатионДатой у меня проблемы
//   // мне нужно придумать как ее обновлять
//   // почему собственно я ее вообще обновляю в физике??
//   // гораздо лучше если я ее буду обновлять вне,
//   // ко всему прочему так я буду знать сколько всего этой ротатионДаты у меня есть
//
//   // скорее всего это неудачное решение (точнее это неверное решение 250%)
//   // короче, нужно просто вытащить RotationData из физики
//   RotationData* data = rotationDatas->data();
//
//   for (uint32_t i = 0; i < *rotationDatasCount->data(); ++i) {
//     pool->submitnr(calcRotationData, i, data);
//   }
}

void CPUPhysicsParallel::interpolate(const float &alpha) {
//   std::cout << "alpha " << alpha << "\n";
  
  static const auto interpolateFunc = [&] (const size_t &start, const size_t &count, const float &alpha) {
    for (size_t index = start; index < start+count; ++index) {
      if (physicsDatas[index].objectIndex == UINT32_MAX) return;

//       const simd::vec4 vel = globalVel[index];

//       const size_t sixteeFrames = 16667;
      const uint32_t &transIndex = physicsDatas[index].transformIndex;
      //transforms->at(transIndex).pos = transforms->at(transIndex).pos + vel*alpha*MCS_TO_SEC(sixteeFrames);
      transforms->at(transIndex).pos = simd::mix(prevState[transIndex].pos, currState[transIndex].pos, alpha);
      // потом добавится кватернион который мы будем slerp'ать
    }
  };

  const size_t count = std::ceil(float(physicsDatas.size()) / float(pool->size()+1));
  size_t start = 0;
  for (uint32_t i = 0; i < pool->size()+1; ++i) {
    const size_t jobCount = std::min(count, physicsDatas.size()-start);
    if (jobCount == 0) break;

    pool->submitnr(interpolateFunc, start, jobCount, alpha);

    start += jobCount;
  }

//   for (uint32_t i = 0; i < physicsDatas.size(); ++i) {
//     pool->submitnr(interpolateFunc, i, alpha);
//   }

  pool->compute();
  pool->wait();
}
