#include "PhysicsComponent.h"

#include "Globals.h"

void PhysicsComponent::setContainer(Container<ExternalData>* externalDatas) {
  PhysicsComponent::externalDatas = externalDatas;
}

PhysicsComponent::PhysicsComponent(const CreateInfo &info) : externalDataIndex(externalDatas->insert(info.externalData)), container{UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, info.userData} {
  Global::physics()->add({
    info.physInfo.rotation,
    info.physInfo.type,
    info.physInfo.collisionGroup,
    info.physInfo.collisionFilter,
    info.physInfo.stairHeight,
    info.physInfo.overbounce,
    info.physInfo.groundFricion,
    info.physInfo.radius,
    info.physInfo.inputIndex,
    info.physInfo.transformIndex,
    externalDataIndex,
    info.physInfo.matrixIndex,
    info.physInfo.rotationIndex,
    info.physInfo.shapeType
    }, &container);
}

PhysicsComponent::~PhysicsComponent() {
  externalDatas->erase(externalDataIndex);
  Global::physics()->remove(&container);
}

const PhysicsIndexContainer & PhysicsComponent::getIndexContainer() const {
  return container;
}

simd::vec4 PhysicsComponent::getVelocity() const {
  if (container.physicDataIndex == UINT32_MAX) return simd::vec4(0.0f, 0.0f, 0.0f, 0.0f);

  const glm::vec3 &vel = Global::physics()->getPhysicData(&container).velocity;
  return simd::vec4(vel.x, vel.y, vel.z, 0.0f);
}

float PhysicsComponent::getSpeed() const {
  if (container.physicDataIndex == UINT32_MAX) return 0.0f;

  const float &speed = Global::physics()->getPhysicData(&container).scalar;
  return speed;
}

uint32_t PhysicsComponent::getObjectShapePointsSize() const {
  return Global::physics()->getObjectShapePointsSize(&container);
}

const simd::vec4* PhysicsComponent::getObjectShapePoints() const {
  return Global::physics()->getObjectShapePoints(&container);
}

uint32_t PhysicsComponent::getObjectShapeFacesSize() const {
  return Global::physics()->getObjectShapeFacesSize(&container);
}

const simd::vec4* PhysicsComponent::getObjectShapeFaces() const {
  return Global::physics()->getObjectShapeFaces(&container);
}

PhysicsIndexContainer* PhysicsComponent::getGround() const {
  const Object* obj = &Global::physics()->getObjectData(&container);
  PhysicsIndexContainer* ret = nullptr;

  while (obj->groundObjIndex != UINT32_MAX) {
    ret = Global::physics()->getIndexContainer(obj->groundObjIndex);
    obj = &Global::physics()->getObjectData(ret);
  }

  return ret;
}

uint32_t PhysicsComponent::getExternalDataIndex() const {
  return externalDataIndex;
}

void PhysicsComponent::setUserData(void* ptr) {
  container.userData = ptr;
}

Container<ExternalData>* PhysicsComponent::externalDatas = nullptr;