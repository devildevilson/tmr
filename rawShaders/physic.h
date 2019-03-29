#ifndef GLSL_PHYSIC_H
#define GLSL_PHYSIC_H

#ifdef __cplusplus
#include "glm/glm.hpp"
using glm::vec4;
using glm::mat4;
using glm::uvec4;
using glm::dot;
using glm::min;
using glm::max;
typedef uint32_t uint;
#define inout
//#else
//#version 450
#endif

#define EPSILON 0.000001f
#define PI   3.1415926535897932384626433832795
#define PI_2 6.28318530717958647693
#define PI_H 1.57079632679489661923
#define PI_Q 0.78539816339744830962

#define MCS_TO_SEC(dt) float(dt) / 1000000.0f

#define DEG_TO_RAD(deg) (deg * PI) / 180.0f
#define RAD_TO_DEG(rad) (rad * 180.0f) / PI

#ifndef __cplusplus
struct AABB {
  vec4 center;
  vec4 extent;
};

struct OBB {
  AABB volume;
  mat4 orientation;
};
#endif

struct Object {
  // x == entityId, y == objectId, z == objectType, w == physicDataIndex
  uvec4 worldInfo;
  // x == vertexOffset, y == vertexSize, z == coordinateSystemIndex, w == staticOrDynamic
  uvec4 objectInfo;
  // x == staticCollisionCount, y == dynamicCollisionCount, z == wasOnGround, w == AABBindex
  uvec4 additionalData;
  // x == nodeIndex, y == containerIndex
  uvec4 nodePos;
};

struct Object2 {
  // x == entityId, y == objectId, z == proxyIndex, w == physicDataIndex
  uvec4 worldInfo;
  // x == vertexOffset, y == vert, z == faces, w == objType
  uvec4 objectInfo;
  // x == transformIndex, y == coordinateSystemIndex  (если это == 0xFFFFFFFF то берем transformIndex), z == wasOnGround, w == groundObjIndex
  uvec4 additionalData;
};

struct Object3 {
  // x == objectId, y == proxyIndex, z == staticPhysicDataIndex, w == transformIndex
  uvec4 worldInfo;
  // x == vertexOffset, y == vert, z == faces, w == objType
  uvec4 objectInfo;
  // x == coordinateSystemIndex, y == groundObjIndex, z == rotationData
  uvec4 additionalData;
};

struct PhysicData {
  vec4 velocity;
  vec4 pos;
  vec4 additionalForce;
  // x == maxSpeed, y == groundFriction, z == airFriction, w == velocityScalar (айр фриктион наверное не понадобится)
  vec4 additionalData;
  // x == isOnGround, y == inputIndex, z == groundIndex, w == blockingIndex
  uvec4 constants;
  // x == overbounce, y == maxStairHeight
  vec4 bounceAndHeight;
};

struct PhysicData2 {
  vec4 velocity;
  vec4 additionalForce;
  vec4 oldPos;
  //x == maxSpeed, y == groundFriction, z == airFriction, w == scalar
  vec4 additionalData;
  //x == maxSpeed, y == acceleration, z == stairHeight, w == scalar
  //vec4 constants;
  //x == objectIndex, y == inputIndex, z == groundIndex, w == blockingIndex
  uvec4 indexies;
  //x == overbounce, y == stairHeight, z == transformIndex, w == isOnGround
  vec4 constants;
  //x == transformIndex, y == staticPhysicDataIndex, z == isOnGround, w == wasOnGround
  //uvec4 additionalData;
};

struct PhysicData3 {
  vec4 velocity;
  vec4 additionalForce;
  vec4 oldPos;
  //x == maxSpeed, y == acceleration, z == stairHeight, w == scalar
  vec4 constants;
  //x == objectIndex, y == inputIndex, z == groundIndex, w == blockingIndex
  uvec4 indexies;
  //x == transformIndex, y == staticPhysicDataIndex, z == isOnGround, w == wasOnGround
  uvec4 additionalData;
};

struct PhysicData4 {
  vec4 velocity; // w == scalar
  vec4 oldPos;

  // x == objectIndex, y == inputIndex, z == groundIndex, w == blockingIndex
  uvec4 extIndices;
  // x == externalDataIndex, y == transformIndex, z == constantsIndex, w == onGroundBits
  uvec4 intIndices;
};

struct ExternalData {
  vec4 additionalForce;
  // x == maxSpeed, y == acceleration
  vec4 speedData;
};

struct Constants {
  // x == groundFriction, y == overbounce, z == stairHeight, w == physDataIndex
  vec4 data;
};

struct StaticPhysicData {
  // x == groundFriction, y == overbounce, z == objectIndex, w == physicDataIndex
  uvec4 data;
};

struct IslandData {
  // x == islandId, y == offset, z == size
  uvec4 data;
};

struct Pair {
  // x == first, y == second, z == dist2, w == islandId
  uvec4 data;
};

struct NodeData {
  // x == count, y == offset, z == childIndex, w == vertexOffset (AABB index)
  uvec4 data;
};

struct Transform {
  vec4 pos;
  //mat4 orn;
  vec4 rot;
  vec4 scale;
};

#endif
