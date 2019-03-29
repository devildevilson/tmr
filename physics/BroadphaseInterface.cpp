#include "BroadphaseInterface.h"

BroadphaseProxy::BroadphaseProxy(const uint32_t &objIndex, const PhysicsType &type, const uint32_t &collisionGroup, const uint32_t &collisionFilter) {
  this->filter = collisionFilter;
  this->group = collisionGroup;
  this->objType = type;
  this->objIndex = objIndex;
}

BroadphaseProxy::~BroadphaseProxy() {}

void BroadphaseProxy::setAABB(const FastAABB &box) {
  this->box = box;
}

FastAABB BroadphaseProxy::getAABB() const {
  return box;
}

bool BroadphaseProxy::collide(const BroadphaseProxy* proxy) const {
  return testAABB(box, proxy->getAABB());
}

bool BroadphaseProxy::in(const FastAABB &box) const {
  return AABBcontain(this->box, box);
}

uint32_t BroadphaseProxy::collisionGroup() const {
  return group;
}

uint32_t BroadphaseProxy::collisionFilter() const {
  return filter;
}

uint32_t BroadphaseProxy::getIndex() const {
  return UINT32_MAX;
}

uint32_t BroadphaseProxy::getObjectIndex() const {
  return objIndex;
}

PhysicsType BroadphaseProxy::getType() const {
  return objType;
}

Broadphase::~Broadphase() {}
  
void Broadphase::getBroadphaseAabb(FastAABB &box) const {
  box = this->box;
}
