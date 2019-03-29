#include "CPUPhysics.h"
#include <stdexcept>

#include <glm/gtx/norm.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <iostream>
#include <cstring>
#include <chrono>

CPUPhysics::CPUPhysics(const CreateInfo &info) {
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
  
  this->defaultStaticMatrixIndex = info.buffers->defaultStaticMatrixIndex;
  this->defaultDynamicMatrixIndex = info.buffers->defaultDynamicMatrixIndex;
  
  setBuffers(*info.buffers);
  
  matrices->at(defaultStaticMatrixIndex) = glm::mat4(1.0f);
  matrices->at(defaultDynamicMatrixIndex) = glm::mat4(1.0f);

  updateInputOutput();
}

CPUPhysics::~CPUPhysics() {}

void CPUPhysics::update(const uint64_t &time) {
  Gravity* data = gravityBuffer.data();
  data->objCount = physicsDatas.size();
  data->time = time;
  data->gravity = gravity;
  data->gravityNormal = gravityNorm;
  data->length2 = gravLength2;
  data->length = gravLength;

  updateBuffers();
  broad->updateBuffers(objects.size(), physicsDatas.size(), rays.size(), frustums.size());
  narrow->updateBuffers(overlappingPairCache.size(), staticOverlappingPairCache.size());
  solver->updateBuffers(overlappingPairCache.size() + staticOverlappingPairCache.size(), rayPairCache.size());
  
  auto start = std::chrono::steady_clock::now();
  updateRotationDatas();
  auto end = std::chrono::steady_clock::now() - start;
  auto ns = std::chrono::duration_cast<std::chrono::microseconds>(end).count();  
  std::cout << "Rotation data time: " << ns << " mcs" << "\n";
  
  start = std::chrono::steady_clock::now();
  updateVelocities();
  end = std::chrono::steady_clock::now() - start;
  ns = std::chrono::duration_cast<std::chrono::microseconds>(end).count();  
  std::cout << "Velocity time: " << ns << " mcs" << "\n";

  broad->update();

  broad->calculateOverlappingPairs();
  
  sorter->sort(&overlappingPairCache, 1);
  
  narrow->checkIdenticalPairs();
  
  sorter->sort(&overlappingPairCache);
  sorter->sort(&staticOverlappingPairCache);
  narrow->postCalculation();

  solver->calculateData();

  solver->solve();

  broad->update();

  broad->calculateRayTests();
  broad->calculateFrustumTests();
  
  solver->calculateRayData();

  sorter->sort(&frustumTestsResult);
  sorter->sort(&overlappingData, &dataIndices, 0);
  sorter->sort(&raysData, &raysIndices, 1);

  overlappingDataSize = dataIndices.data()->count;
  rayTracingSize = raysIndices.data()->temporaryCount;
  frustumTestSize = frustumTestsResult[0].firstIndex;

  memset(rays.data(), UINT32_MAX, sizeof(RayData));
  memset(frustums.data(), UINT32_MAX, sizeof(FrustumStruct));
  memset(frustumTestsResult.data(), 0, sizeof(BroadphasePair));
  
  indices[0] = 0;

  end = std::chrono::steady_clock::now() - start;
  ns = std::chrono::duration_cast<std::chrono::microseconds>(end).count();
  std::cout << "Physics takes " << ns << " mcs" << "\n";
}

void CPUPhysics::setBuffers(const PhysicsExternalBuffers &buffers) {
  this->defaultStaticMatrixIndex = buffers.defaultStaticMatrixIndex;
  this->defaultDynamicMatrixIndex = buffers.defaultDynamicMatrixIndex;

  this->inputs = buffers.inputs;
  this->transforms = buffers.transforms;
  this->matrices = buffers.matrices;
  this->rotationDatasCount = buffers.rotationDatasCount;
  this->rotationDatas = buffers.rotationDatas;
  this->externalDatas = buffers.externalDatas;
}

void CPUPhysics::registerShape(const std::string &name, const uint32_t shapeType, const RegisterNewShapeInfo &info) {
  auto itr = shapes.find(name);
  if (itr != shapes.end()) throw std::runtime_error("Shape type " + name + " is already exist!");

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
  
  glm::vec4 localCenter = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
  for (uint32_t i = 0; i < info.points.size(); ++i) {
    verts[s.offset + i] = info.points[i];
    localCenter += info.points[i];
  }
  localCenter.x /= float(info.points.size());
  localCenter.y /= float(info.points.size());
  localCenter.z /= float(info.points.size());
  localCenter.w = 1.0f;

  verts[s.offset + s.pointCount] = localCenter;
  
  for (uint32_t i = 0; i < info.faces.size(); ++i) {
    verts[s.offset + s.pointCount + additionalFaceOffset + i] = info.faces[i];
  }

  shapes[name] = s;
}

void CPUPhysics::add(const PhysicsObjectCreateInfo &info, PhysicsIndexContainer* container) {
  if (defaultStaticMatrixIndex == UINT32_MAX || defaultDynamicMatrixIndex == UINT32_MAX) throw std::runtime_error("No default matrix");

  container->objectDataIndex = UINT32_MAX;
  container->physicDataIndex = UINT32_MAX;
  container->transformIndex = UINT32_MAX;
  container->inputIndex = UINT32_MAX;
  container->rotationIndex = UINT32_MAX;

  if (info.transformIndex == UINT32_MAX && info.type.getObjType() != POLYGON_TYPE) throw std::runtime_error("Cannot create box or sphere without transform");
  if (info.inputIndex == UINT32_MAX && info.type.isDynamic()) throw std::runtime_error("Cannot create dynamic object without input");
  if (info.externalDataIndex == UINT32_MAX && info.type.isDynamic()) throw std::runtime_error("Cannot create dynamic object without external data");

  container->internalIndex = components.size();
  components.push_back(container);

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
  }

  container->transformIndex = info.transformIndex;
  container->inputIndex = info.inputIndex;

  container->rotationIndex = info.rotationIndex;
  
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
      glm::vec4(0.0f, 0.0f, 0.0f, 0.0f),

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

  auto itr = shapes.find(info.shapeName);
  if (itr == shapes.end()) throw std::runtime_error("Shape " + info.shapeName + " was not registered");
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

void CPUPhysics::remove(PhysicsIndexContainer* comp) {
  components.back()->internalIndex = comp->internalIndex;
  std::swap(components[comp->internalIndex], components.back());
  components.pop_back();
  
  broad->destroyProxy(objects[comp->objectDataIndex].proxyIndex);

  objects[comp->objectDataIndex].objectId = freeObj;
  freeObj = comp->objectDataIndex;
  
  if (comp->physicDataIndex != UINT32_MAX) {
    physicsDatas[comp->physicDataIndex].velocity.x = glm::uintBitsToFloat(freePhys);
    physicsDatas[comp->physicDataIndex].objectIndex = 0xFFFFFFFF;
    freePhys = comp->physicDataIndex;
  }
}

// нужно добавлять заново кадый кадр
uint32_t CPUPhysics::add(const RayData &ray) {
  // эту функцию вполне можно выполнять параллельно

  uint32_t index = rays.vector().size();
  // нормализация??
  rays.vector().push_back(ray);
  rays.update();

  return index;
}

uint32_t CPUPhysics::add(const glm::mat4 &frustum, const glm::vec4 &pos = glm::vec4(10000.0f)) {
  uint32_t index = frustums.vector().size();
  // здесь он будет все пересчитывать
  frustums.vector().emplace_back(frustum);
  frustums.update();

  frustumPoses.vector().push_back(pos);
  frustumPoses.update();

  return index;
}

Object & CPUPhysics::getObjectData(const uint32_t &index) {
  return objects[index];
}

const Object & CPUPhysics::getObjectData(const uint32_t &index) const {
  return objects[index];
}

PhysData2 & CPUPhysics::getPhysicData(const uint32_t &index) {
  return physicsDatas[index];
}

const PhysData2 & CPUPhysics::getPhysicData(const uint32_t &index) const {
  return physicsDatas[index];
}

void CPUPhysics::setGravity(const glm::vec4 &g) {
  gravity = g;
  gravityNorm = glm::normalize(g);
  gravLength2 = glm::length2(g);
  gravLength  = glm::sqrt(gravLength2);
  orientation = glm::orientation(-glm::vec3(gravityNorm.x, gravityNorm.y, gravityNorm.z), glm::vec3(0.0f, 1.0f, 0.0f));
  
  matrices->at(defaultDynamicMatrixIndex) = orientation;
}

void CPUPhysics::updateMaxSpeed(const uint32_t &physicDataIndex, const float &maxSpeed) {
  throw std::runtime_error("must not be call");
}


uint32_t CPUPhysics::setShapePointsAndFaces(const uint32_t &objectDataIndex, const std::vector<glm::vec4> &points, const std::vector<glm::vec4> &faces) {
  throw std::runtime_error("not implemented yet");
}

ArrayInterface<OverlappingData>* CPUPhysics::getOverlappingPairsData() {
  return &overlappingData;
}

const ArrayInterface<OverlappingData>* CPUPhysics::getOverlappingPairsData() const {
  return &overlappingData;
}

ArrayInterface<OverlappingData>* CPUPhysics::getRayTracingData() {
  return &raysData;
}

const ArrayInterface<OverlappingData>* CPUPhysics::getRayTracingData() const {
  return &raysData;
}

ArrayInterface<BroadphasePair>* CPUPhysics::getFrustumPairs() {
  return &frustumTestsResult;
}

const ArrayInterface<BroadphasePair>* CPUPhysics::getFrustumPairs() const {
  return &frustumTestsResult;
}

void CPUPhysics::printStats() {
  const size_t memoryUsed = verts.vector().size()*sizeof(verts[0]) + 
                            objects.vector().size()*sizeof(objects[0]) + 
                            physicsDatas.vector().size()*sizeof(physicsDatas[0]) + 
                            transforms.vector().size()*sizeof(transforms[0]) + 
                            inputs.vector().size()*sizeof(inputs[0]) + 
                            indices[0]*sizeof(indices[0]) + 
                            coordinateSystems.vector().size()*sizeof(coordinateSystems[0]) + 
                            globalVel.vector().size()*sizeof(globalVel[0]) + sizeof(Gravity);

  const size_t memory = verts.vector().capacity() * sizeof(verts[0]) + 
                        objects.vector().capacity() * sizeof(objects[0]) + 
                        physicsDatas.vector().capacity() * sizeof(physicsDatas[0]) + 
                        transforms.vector().capacity() * sizeof(transforms[0]) + 
                        inputs.vector().capacity() * sizeof(inputs[0]) + 
                        indices.vector().capacity() * sizeof(indices[0]) + 
                        coordinateSystems.vector().capacity() * sizeof(coordinateSystems[0]) + 
                        globalVel.vector().capacity() * sizeof(globalVel[0]) + sizeof(Gravity);

  std::cout << "Physics engine data" << '\n';
  std::cout << "Object count           " << objects.size() << '\n';
  std::cout << "Dynamic object count   " << physicsDatas.size() << '\n';
  std::cout << "Total memory usage     " << memory << " bytes" << "\n";
  std::cout << "Total memory with data " << memoryUsed << " bytes" << "\n";
  std::cout << "Free memory            " << (memory - memoryUsed) << " bytes" << '\n';
  std::cout << "Physics class size     " << sizeof(CPUPhysics) << " bytes" << '\n';
  
  broad->printStats();
  narrow->printStats();
  solver->printStats();
  sorter->printStats();
}

void CPUPhysics::updateBuffers() {
  if (objects.size()+1 > indices.size()) indices.resize(objects.size()+1);
  if (physicsDatas.size() > globalVel.size()) globalVel.resize(physicsDatas.size());
}

void CPUPhysics::updateInputOutput() {
  {
    Broadphase::InputBuffers input{
      &indices,
      &verts, 
      &objects, 
      matrices, 
      transforms, 
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
      transforms, 
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

void computeGroundVelocity(const glm::vec4 &accelerationDir,
                           const glm::vec4 &oldVelocity,
                           const uint32_t &dt,
                           const bool jump,
                           const glm::vec4 &additionalForce,
                           const glm::vec4 &gravityNorm,
                           const float &maxSpeed,
                           const float &acceleration,
                           const float &groundFriction,
                             glm::vec4 &velocity,
                                 float &velocityScalar) {
  const float dt1 = MCS_TO_SEC(dt);
  
  const glm::vec4 finalAcceleretionDir = accelerationDir * acceleration;
  const glm::vec4 a = finalAcceleretionDir + additionalForce;

  const float stopspeed = velocityScalar < STOP_SPEED ? STOP_SPEED : velocityScalar;
  // фриктион? additionalData.y
  float fr = velocityScalar - stopspeed*groundFriction*dt1;
  fr = glm::max(fr, 0.0f);
  fr = velocityScalar == 0.0f ? fr = 0.0f : fr / velocityScalar;
  
  velocity = oldVelocity*fr + a * dt1;
  
  float newSpeed = glm::length(velocity);

  const float jumpKoef = (4.0f / (1.0f / 60.0f));
  const float jumpAcceleration = jumpKoef * float(jump);
  const glm::vec4 jumpAccelerationDir = -gravityNorm * jumpAcceleration;
  
  if (newSpeed <= EPSILON) {
    velocity = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f) + jumpAccelerationDir*dt1;
    velocityScalar = jumpAcceleration*dt1;
    return;
  }
  
  const glm::vec4 newVelNorm = velocity / newSpeed;

  newSpeed = glm::min(newSpeed, maxSpeed);

  velocity = newVelNorm * newSpeed + jumpAccelerationDir*dt1; // - globalData.gravity*aJump;
  velocityScalar = length(velocity);
}

void computeAirVelocity(const glm::vec4 &accelerationDir,
                        const glm::vec4 &oldVelocity,
                        const uint32_t  &dt,
                        const glm::vec4 &additionalForce,
                        const glm::vec4 &gravity,
                        glm::vec4 &velocity,
                        float &velocityScalar) {
  float dt1 = MCS_TO_SEC(dt);

  glm::vec4 vn = velocityScalar > EPSILON ? oldVelocity / velocityScalar : glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

  const float airFriction = 0.0f;
  glm::vec4 a = -vn*airFriction + additionalForce + gravity + AIR_ACCELERATION_DIR*accelerationDir*AIR_ACCELERATION;

  velocity = oldVelocity + a * dt1;
  velocityScalar = glm::length(velocity);
}

glm::vec4 projectVectorOnPlane(const glm::vec4 normal, const glm::vec4 origin, const glm::vec4 vector) {
  float dist = glm::dot(vector, normal);
  glm::vec4 point = origin + vector - normal*dist;
  return point - origin;
}

bool feq(const float first, const float second) {
  return abs(first - second) < EPSILON;
}

void CPUPhysics::updateVelocities() {
  for (uint i = 0; i < physicsDatas.size(); ++i) {
    const uint32_t index = i;
    if (physicsDatas[index].objectIndex == UINT32_MAX) continue;

    const PhysData2 &current = physicsDatas[index];
    const uint32_t &transIndex = current.transformIndex;
    const Transform &trans = transforms->at(transIndex);
    const uint32_t externalDataIndex = current.externalDataIndex;

    const uint32_t &groundPhysicsIndex = current.groundIndex;

    const InputData &currentInput = inputs->at(current.inputIndex);
    glm::vec4 frontOnGround = currentInput.moves.z == 0.0f ? glm::vec4(0.0f, 0.0f, 0.0f, 0.0f) :
      projectVectorOnPlane(-gravityBuffer.data()->gravityNormal, trans.pos, currentInput.front);

    const float &lengthFront = glm::length(frontOnGround);
    if (lengthFront > EPSILON) frontOnGround /= lengthFront;
    else frontOnGround = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

    const glm::vec4 &acceleration = frontOnGround * currentInput.moves.z + currentInput.right * currentInput.moves.x + currentInput.up * currentInput.moves.y;

    const float accelLenght = glm::length(acceleration);
    glm::vec4 aDir = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    if (accelLenght > EPSILON) aDir = acceleration / accelLenght;
    
    const glm::vec4 additionalForce = externalDatas->at(externalDataIndex).additionalForce;
    const float maxSpeed = externalDatas->at(externalDataIndex).maxSpeed;
    const float accelerationScalar = externalDatas->at(externalDataIndex).acceleration;
    
    const glm::vec4 oldVel = glm::vec4(current.velocity, 0.0f);
    float scalar = glm::length(oldVel);
    glm::vec4 vel = oldVel;

    const float groundFriction = groundPhysicsIndex != UINT32_MAX ?
                                   staticPhysicDatas[groundPhysicsIndex].groundFriction : DEFAULT_GROUND_FRICTION;

    if (bool(physicsDatas[index].onGroundBits & 0x1)) { 
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
   } else {
      computeAirVelocity(aDir,
                         oldVel,
                         gravityBuffer.data()->time,
                         additionalForce,
                         gravityBuffer.data()->gravity,
                         vel,
                         scalar);

      physicsDatas[index].groundIndex = UINT32_MAX;
    }

    physicsDatas[index].scalar = scalar;
    physicsDatas[index].velocity = vel;
    
    physicsDatas[index].onGroundBits = bool(physicsDatas[index].onGroundBits & 0x1) && currentInput.moves.y > 0.0f ? 0 : physicsDatas[index].onGroundBits;
    if (!bool(physicsDatas[index].onGroundBits & 0x1)) physicsDatas[index].groundIndex = UINT32_MAX;
  }

  for (uint i = 0; i < physicsDatas.size(); ++i) {
    const uint index = i;
    if (physicsDatas[index].objectIndex == UINT32_MAX) continue;

    glm::vec4 velocity = glm::vec4(physicsDatas[index].velocity, 0.0f);

    uint32_t groundIndex = physicsDatas[index].groundIndex;
    while (groundIndex != UINT32_MAX) {
      const uint32_t physDataIndex = staticPhysicDatas[groundIndex].physDataIndex;

      if (physDataIndex != UINT32_MAX) {
        const PhysData2 &data = physicsDatas[physDataIndex];
        velocity += data.velocity;
        groundIndex = data.groundIndex;
      } else {
        groundIndex = UINT32_MAX;
      }
    }

    const float globalScalar = glm::length(velocity);

    const uint32_t &transIndex = physicsDatas[index].transformIndex;

    physicsDatas[index].oldPos = transforms->at(transIndex).pos;
    transforms->at(transIndex).pos = transforms->at(transIndex).pos + velocity*MCS_TO_SEC(gravityBuffer.data()->time);

    globalVel[index] = velocity;

    if (globalScalar > EPSILON) {
      // const uint id = atomicAdd(count, 1);
      // indexies[id] = datas[index].objectIndex;
      const uint32_t id = indices[0];
      ++indices[0];
      
      indices[id+1] = physicsDatas[index].objectIndex;
    }
  }
}

void CPUPhysics::updateRotationDatas() {
  for (uint i = 0; i < *rotationDatasCount->data(); ++i) {
    const uint index = i;

    rotationDatas->at(index).currentAngle += rotationDatas->at(index).rotationSpeed;
    rotationDatas->at(index).currentAngle = glm::min(rotationDatas->at(index).currentAngle, rotationDatas->at(index).maxAngle);
    rotationDatas->at(index).currentAngle = glm::max(rotationDatas->at(index).currentAngle, 0.0f);

    const glm::vec4 &anchorDir = glm::vec4(rotationDatas->at(index).anchorDir, 0.0f);
    const float &anchorDist = rotationDatas->at(index).anchorDist;
    
    rotationDatas->at(index).matrix = glm::translate(glm::mat4(1.0f), glm::vec3(anchorDir * anchorDist));
    rotationDatas->at(index).matrix = glm::rotate(rotationDatas->at(index).matrix, rotationDatas->at(index).currentAngle, glm::vec3(rotationDatas->at(index).rotationNormal));
    rotationDatas->at(index).matrix = glm::translate(rotationDatas->at(index).matrix, -glm::vec3(anchorDir * anchorDist));
  }
}
