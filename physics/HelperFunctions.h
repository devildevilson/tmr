#ifndef HELPER_FUNCTIONS_H
#define HELPER_FUNCTIONS_H

#include "PhysicsUtils.h"

#include "ArrayInterface.h"

float getAngle(const glm::vec4 &first, const glm::vec4 &second);
void clipVelocity(const glm::vec4 &clipNormal, const float &bounce, glm::vec4 &vel);
glm::vec4 transform(const glm::vec4 &p, const glm::vec4 &translation, const glm::mat4 &orientation);
bool isNormalAdequate(const ArrayInterface<glm::vec4>* verts, const uint32_t &offset, const uint32_t &normalIndex, const glm::vec4 &normal);
glm::vec4 getPolyPolyFace(const ArrayInterface<glm::vec4>* verts,
                          const uint32_t &firstFace,  const uint32_t &firstFaceSize,  const glm::mat4 &firstOrn,
                          const uint32_t &secondFace, const uint32_t &secondFaceSize, const glm::mat4 &secondOrn,
                          const uint32_t &index);
glm::vec4 getPolySphereFace(const ArrayInterface<glm::vec4>* verts, const uint32_t &face, const uint32_t &faceSize, const glm::mat4 &ornFirst, const glm::vec4 &pos1, const glm::vec4 &pos2, const uint32_t &index);
glm::vec4 getBoxPolyFace(const ArrayInterface<glm::vec4>* verts, const uint32_t &face, const uint32_t &faceSize, const glm::mat4 &polyOrn, const glm::mat4 &orn, const uint32_t &index);
glm::vec4 getBoxSphereFace(const glm::mat4 &orn, const glm::vec4 &pos1, const glm::vec4 &pos2, const uint32_t &index);
glm::vec4 getBoxBoxFace(const glm::mat4 &orn1, const glm::mat4 &orn2, const uint32_t &index);
bool overlap(const float &treshold, const float &min1, const float &max1, const float &min2, const float &max2, const glm::vec4 &axis, glm::vec4 &mtv, float &dist);
FastAABB recalcAABB(const glm::vec4 &pos, const glm::vec4 &ext, const glm::mat4 &orn);
FastAABB recalcAABB(const ArrayInterface<glm::vec4>* verticies, const glm::vec4 &pos, const uint32_t &vertexOffset, const uint32_t &vertexSize, const glm::mat4 &orn);
bool intersection(const FastAABB &box, const RayData &ray);
uint32_t testFrustumAABB(const FrustumStruct &frustum, const FastAABB &box);
uint32_t nextPowerOfTwo(const uint32_t &number);

#endif
