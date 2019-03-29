#include "PhysicsUtils.h"

FrustumStruct::FrustumStruct() {}
FrustumStruct::FrustumStruct(const glm::mat4 &matrix) {
  calcFrustum(matrix);
}

void FrustumStruct::calcFrustum(const glm::mat4 &matrix) {
  this->planes[0].x = matrix[0][3] + matrix[0][0]; // left
  this->planes[0].y = matrix[1][3] + matrix[1][0];
  this->planes[0].z = matrix[2][3] + matrix[2][0];
  this->planes[0].w = matrix[3][3] + matrix[3][0];

  this->planes[1].x = matrix[0][3] - matrix[0][0]; // right
  this->planes[1].y = matrix[1][3] - matrix[1][0];
  this->planes[1].z = matrix[2][3] - matrix[2][0];
  this->planes[1].w = matrix[3][3] - matrix[3][0];

  this->planes[2].x = matrix[0][3] - matrix[0][1]; // top
  this->planes[2].y = matrix[1][3] - matrix[1][1];
  this->planes[2].z = matrix[2][3] - matrix[2][1];
  this->planes[2].w = matrix[3][3] - matrix[3][1];

  this->planes[3].x = matrix[0][3] + matrix[0][1]; // bottom
  this->planes[3].y = matrix[1][3] + matrix[1][1];
  this->planes[3].z = matrix[2][3] + matrix[2][1];
  this->planes[3].w = matrix[3][3] + matrix[3][1];
  
//   this->planes[3].x = matrix[0][3] - matrix[0][1]; // top
//   this->planes[3].y = matrix[1][3] - matrix[1][1];
//   this->planes[3].z = matrix[2][3] - matrix[2][1];
//   this->planes[3].w = matrix[3][3] - matrix[3][1];
// 
//   this->planes[2].x = matrix[0][3] + matrix[0][1]; // bottom
//   this->planes[2].y = matrix[1][3] + matrix[1][1];
//   this->planes[2].z = matrix[2][3] + matrix[2][1];
//   this->planes[2].w = matrix[3][3] + matrix[3][1];

  this->planes[4].x = matrix[0][3] + matrix[0][2]; // front
  this->planes[4].y = matrix[1][3] + matrix[1][2];
  this->planes[4].z = matrix[2][3] + matrix[2][2];
  this->planes[4].w = matrix[3][3] + matrix[3][2];

  this->planes[5].x = matrix[0][3] - matrix[0][2]; // back
  this->planes[5].y = matrix[1][3] - matrix[1][2];
  this->planes[5].z = matrix[2][3] - matrix[2][2];
  this->planes[5].w = matrix[3][3] - matrix[3][2];

  for (uint32_t i = 0; i < 6; ++i) {
    const float mag = glm::length(glm::vec4(planes[i].x, planes[i].y, planes[i].z, 0.0f));
    planes[i] /= mag;
  }
}

// должно быть синхронизованно с тем что в шейдерах
#define DYNAMIC_BIT_PLACEMENT   0
#define OBJECT_TYPE_PLACEMENT   (DYNAMIC_BIT_PLACEMENT+1)
#define BLOCKING_BIT_PLACEMENT  (OBJECT_TYPE_PLACEMENT+3)
#define TRIGGER_BIT_PLACEMENT   (BLOCKING_BIT_PLACEMENT+1)
#define RAY_BLOCK_BIT_PLACEMENT (TRIGGER_BIT_PLACEMENT+1)
#define VISIBLE_BIT_PLACEMENT   (RAY_BLOCK_BIT_PLACEMENT+1)

#define OBJECT_TYPE_BITS 0x7;

bool testAABB(const FastAABB &first, const FastAABB &second) {
  const glm::vec4 center = glm::abs(first.center - second.center);
  const glm::vec4 extent =          first.extent + second.extent;

  return extent.x > center.x && extent.y > center.y && extent.z > center.z;
  
//   const bool x = glm::abs(first.center[0] - second.center[0]) <= (first.extent[0] + second.extent[0]);
//   const bool y = glm::abs(first.center[1] - second.center[1]) <= (first.extent[1] + second.extent[1]);
//   const bool z = glm::abs(first.center[2] - second.center[2]) <= (first.extent[2] + second.extent[2]);
//   
//   return x && y && z;
}

bool AABBcontain(const FastAABB &first, const FastAABB &second) {
//   if (first.extent.x >= second.extent.x || first.extent.y >= second.extent.y || first.extent.z >= second.extent.z) return false;
//   
//   const glm::vec4 one = glm::abs(first.center - second.center);
//   const glm::vec4 two = glm::abs(first.extent - second.extent);
//   
//   return two.x > one.x && two.y > one.y && two.z > one.z;
  
  if ((first.extent.x >= second.extent.x) ||
      (first.extent.y >= second.extent.y) ||
      (first.extent.z >= second.extent.z)) return false;
  
  if (glm::abs(first.center.x - second.center.x) > glm::abs(first.extent.x - second.extent.x)) return false;
  if (glm::abs(first.center.y - second.center.y) > glm::abs(first.extent.y - second.extent.y)) return false;
  if (glm::abs(first.center.z - second.center.z) > glm::abs(first.extent.z - second.extent.z)) return false;

  return true;
}

PhysicsType::PhysicsType() : type(0) {}

PhysicsType::PhysicsType(const bool &dynamic, const uint32_t &objType, const bool &blocking, const bool &trigger, const bool &blockRays, const bool &visible) : type(0) {
  //makeType(dynamic, objType);
  makeType(dynamic, objType, blocking, trigger, blockRays, visible);
}

void PhysicsType::makeType(const bool &dynamic, const uint32_t &objType, const bool &blocking, const bool &trigger, const bool &blockRays, const bool &visible) {
  this->type = (uint32_t(visible)   << VISIBLE_BIT_PLACEMENT)   | 
               (uint32_t(blockRays) << RAY_BLOCK_BIT_PLACEMENT) | 
               (uint32_t(trigger)   << TRIGGER_BIT_PLACEMENT)   | 
               (uint32_t(blocking)  << BLOCKING_BIT_PLACEMENT)  | 
               (uint32_t(objType)   << OBJECT_TYPE_PLACEMENT)   |
               (uint32_t(dynamic)   << DYNAMIC_BIT_PLACEMENT);
  //this->type = (objType << 1) | dynamic;
}

bool PhysicsType::isDynamic() const {
  const uint32_t mask = 1 << DYNAMIC_BIT_PLACEMENT;
  return (type & mask) == mask;
}

uint32_t PhysicsType::getObjType() const {
  return (type >> OBJECT_TYPE_PLACEMENT) & OBJECT_TYPE_BITS;
}

bool PhysicsType::isBlocking() const {
  const uint32_t mask = 1 << BLOCKING_BIT_PLACEMENT;
  return (type & mask) == mask;
}

bool PhysicsType::isTrigger() const {
  const uint32_t mask = 1 << TRIGGER_BIT_PLACEMENT;
  return (type & mask) == mask;
}

bool PhysicsType::canBlockRays() const {
  const uint32_t mask = 1 << RAY_BLOCK_BIT_PLACEMENT;
  return (type & mask) == mask;
}

bool PhysicsType::isVisible() const {
  const uint32_t mask = 1 << VISIBLE_BIT_PLACEMENT;
  return (type & mask) == mask;
}

glm::vec4 getVertex(const glm::vec4 &pos, const glm::vec4 &ext, const glm::mat4 &orn, const uint32_t &index) {
  glm::vec4 p = pos;
  p = (index & 1) == 1 ? p + orn[0]*ext.x : p - orn[0]*ext.x;
  p = (index & 2) == 2 ? p + orn[1]*ext.y : p - orn[1]*ext.y;
  p = (index & 4) == 4 ? p + orn[2]*ext.z : p - orn[2]*ext.z;
  p.w = 1.0f;

  return p;
}

