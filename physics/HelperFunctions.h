#ifndef HELPER_FUNCTIONS_H
#define HELPER_FUNCTIONS_H

#include "PhysicsUtils.h"

#include "ArrayInterface.h"

#include "Frustum.h"

// float getAngle(const simd::vec4 &first, const simd::vec4 &second);
void clipVelocity(const simd::vec4 &clipNormal, const float &bounce, simd::vec4 &vel);
simd::vec4 transform(const simd::vec4 &p, const simd::vec4 &translation, const simd::mat4 &orientation);
bool isNormalAdequate(const ArrayInterface<simd::vec4>* verts, const uint32_t &offset, const uint32_t &normalIndex, const simd::vec4 &normal);
simd::vec4 getPolyPolyFace(const ArrayInterface<simd::vec4>* verts,
                          const uint32_t &firstFace,  const uint32_t &firstFaceSize,  const simd::mat4 &firstOrn,
                          const uint32_t &secondFace, const uint32_t &secondFaceSize, const simd::mat4 &secondOrn,
                          const uint32_t &index);
simd::vec4 getPolySphereFace(const ArrayInterface<simd::vec4>* verts, const uint32_t &face, const uint32_t &faceSize, const simd::mat4 &ornFirst, const simd::vec4 &pos1, const simd::vec4 &pos2, const uint32_t &index);
simd::vec4 getBoxPolyFace(const ArrayInterface<simd::vec4>* verts, const uint32_t &face, const uint32_t &faceSize, const simd::mat4 &polyOrn, const simd::mat4 &orn, const uint32_t &index);
simd::vec4 getBoxSphereFace(const simd::mat4 &orn, const simd::vec4 &pos1, const simd::vec4 &pos2, const uint32_t &index);
simd::vec4 getBoxBoxFace(const simd::mat4 &orn1, const simd::mat4 &orn2, const uint32_t &index);
bool overlap(const float &treshold, const float &min1, const float &max1, const float &min2, const float &max2, const simd::vec4 &axis, simd::vec4 &mtv, float &dist);
bool overlap(const float &min1, const float &max1, const float &min2, const float &max2, const simd::vec4 &axis, simd::vec4 &mtv, float &dist);
FastAABB recalcAABB(const simd::vec4 &pos, const simd::vec4 &ext, const simd::mat4 &orn);
FastAABB recalcAABB(const ArrayInterface<simd::vec4>* verticies, const simd::vec4 &pos, const uint32_t &vertexOffset, const uint32_t &vertexSize, const simd::mat4 &orn);
bool intersection(const FastAABB &box, const RayData &ray);
uint32_t testFrustumAABB(const Frustum &frustum, const FastAABB &box);
uint32_t nextPowerOfTwo(const uint32_t &number);

#endif
