#include "PhysicsComponent.h"

#include "Globals.h"

void PhysicsComponent::setContainer(Container<ExternalData>* externalDatas) {
  PhysicsComponent::externalDatas = externalDatas;
}

// PhysicsComponent::PhysicsComponent() : externalDataIndex(externalDatas->insert({})), container{UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, nullptr} {}
PhysicsComponent::PhysicsComponent(const CreateInfo &info) : externalDataIndex(info.physInfo.type.isDynamic() ? externalDatas->insert(info.externalData) : UINT32_MAX), container{UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, info.userData} {
  Global::get<PhysicsEngine>()->add({
    info.physInfo.type,
    info.physInfo.collisionGroup,
    info.physInfo.collisionFilter,
    info.physInfo.collisionTrigger,
    info.physInfo.stairHeight,
    info.physInfo.overbounce,
    info.physInfo.groundFricion,
    info.physInfo.radius,
    info.physInfo.gravityCoef,
    info.physInfo.inputIndex,
    info.physInfo.transformIndex,
    externalDataIndex,
    info.physInfo.matrixIndex,
    info.physInfo.rotationIndex,
    info.physInfo.shapeType
    }, &container);
}

PhysicsComponent::~PhysicsComponent() {
  if (externalDataIndex != UINT32_MAX) externalDatas->erase(externalDataIndex);
  Global::get<PhysicsEngine>()->remove(&container);
}

// void PhysicsComponent::init(const CreateInfo &info) {
//   ASSERT(container.inputIndex == UINT32_MAX && 
//          container.internalIndex == UINT32_MAX && 
//          container.objectDataIndex == UINT32_MAX && 
//          container.physicDataIndex == UINT32_MAX && 
//          container.rotationIndex == UINT32_MAX && 
//          container.transformIndex == UINT32_MAX && 
//          container.userData == nullptr);
//   
//   externalDatas->at(externalDataIndex) = info.externalData;
//   
//   container.userData = info.userData;
//   
//   Global::physics()->add({
//     info.physInfo.rotation,
//     info.physInfo.type,
//     info.physInfo.collisionGroup,
//     info.physInfo.collisionFilter,
//     info.physInfo.stairHeight,
//     info.physInfo.overbounce,
//     info.physInfo.groundFricion,
//     info.physInfo.radius,
//     info.physInfo.inputIndex,
//     info.physInfo.transformIndex,
//     externalDataIndex,
//     info.physInfo.matrixIndex,
//     info.physInfo.rotationIndex,
//     info.physInfo.shapeType
//     }, &container);
// }

const PhysicsIndexContainer & PhysicsComponent::getIndexContainer() const {
  return container;
}

simd::vec4 PhysicsComponent::getVelocity() const {
  if (container.physicDataIndex == UINT32_MAX) return simd::vec4(0.0f, 0.0f, 0.0f, 0.0f);

  const glm::vec3 &vel = Global::get<PhysicsEngine>()->getPhysicData(&container).velocity;
  return simd::vec4(vel.x, vel.y, vel.z, 0.0f);
}

float PhysicsComponent::getSpeed() const {
  if (container.physicDataIndex == UINT32_MAX) return 0.0f;

  const float &speed = Global::get<PhysicsEngine>()->getPhysicData(&container).scalar;
  return speed;
}

float PhysicsComponent::getMaxSpeed() const {
  if (externalDataIndex == UINT32_MAX) return 0.0f;

  return externalDatas->at(externalDataIndex).maxSpeed;
}

void PhysicsComponent::setVelocity(const simd::vec4 &vel) {
  if (container.physicDataIndex == UINT32_MAX) return;
  
  float arr[4];
  vel.storeu(arr);
  
  Global::get<PhysicsEngine>()->getPhysicData(&container).velocity = glm::vec3(arr[0], arr[1], arr[2]);
  Global::get<PhysicsEngine>()->getPhysicData(&container).scalar = simd::length(vel);
}

uint32_t PhysicsComponent::getObjectShapePointsSize() const {
  return Global::get<PhysicsEngine>()->getObjectShapePointsSize(&container);
}

const simd::vec4* PhysicsComponent::getObjectShapePoints() const {
  return Global::get<PhysicsEngine>()->getObjectShapePoints(&container);
}

uint32_t PhysicsComponent::getObjectShapeFacesSize() const {
  return Global::get<PhysicsEngine>()->getObjectShapeFacesSize(&container);
}

const simd::vec4* PhysicsComponent::getObjectShapeFaces() const {
  return Global::get<PhysicsEngine>()->getObjectShapeFaces(&container);
}

PhysicsIndexContainer* PhysicsComponent::getGround() const {
  const Object* obj = &Global::get<PhysicsEngine>()->getObjectData(&container);
  PhysicsIndexContainer* ret = nullptr;

  while (obj->groundObjIndex != UINT32_MAX) {
    ret = Global::get<PhysicsEngine>()->getIndexContainer(obj->groundObjIndex);
    obj = &Global::get<PhysicsEngine>()->getObjectData(ret);
  }

  return ret;
}

uint32_t PhysicsComponent::getExternalDataIndex() const {
  return externalDataIndex;
}

void PhysicsComponent::setUserData(void* ptr) {
  container.userData = ptr;
}

void* PhysicsComponent::getUserData() const {
  return container.userData;
}

bool PhysicsComponent::collide_ray(const simd::vec4 &pos, const simd::vec4 &dir, simd::vec4 &point) const {
  const RayData ray(pos, dir, 0.0f, 10000.0f, UINT32_MAX, 0);
  return Global::get<PhysicsEngine>()->intersect(ray, &container, point);
}

Container<ExternalData>* PhysicsComponent::externalDatas = nullptr;
