//#version 450

#include "physic.h"

float distance2(const vec4 first, const vec4 second) {
  const vec4 dir = second - first;
  return dot(dir, dir);
}

float length2(const vec4 first) {
  return dot(first, first);
}

vec4 getVertex(const AABB box, const uint index) {
  vec4 p = vec4(0.0f, 0.0f, 0.0f, 1.0f);
  p.x = (index & 1) > 0 ? box.center.x + box.extent.x : box.center.x - box.extent.x;
  p.y = (index & 2) > 0 ? box.center.y + box.extent.y : box.center.y - box.extent.y;
  p.z = (index & 4) > 0 ? box.center.z + box.extent.z : box.center.z - box.extent.z;

  return p;
}

vec4 getVertex(const OBB box, const uint index) {
  vec4 p = box.volume.center;
  p = (index & 1) > 0 ? p + box.orientation[0]*box.volume.extent.x : p - box.orientation[0]*box.volume.extent.x;
  p = (index & 2) > 0 ? p + box.orientation[0]*box.volume.extent.y : p - box.orientation[0]*box.volume.extent.y;
  p = (index & 4) > 0 ? p + box.orientation[0]*box.volume.extent.z : p - box.orientation[0]*box.volume.extent.z;

  return p;
}

void project(const vec4 axis, const AABB box, inout float minNum, inout float maxNum) {
  minNum = maxNum = dot(axis, getVertex(box, 0));

  for (uint i = 1; i < 8; ++i) {
    float p = dot(axis, getVertex(box, i));

    minNum = min(minNum, p);
    maxNum = max(maxNum, p);
  }
}

void project(const vec4 axis, const OBB box, inout float minNum, inout float maxNum) {
  minNum = maxNum = dot(axis, getVertex(box, 0));

  for (uint i = 1; i < 8; ++i) {
    float p = dot(axis, getVertex(box, i));

    minNum = min(minNum, p);
    maxNum = max(maxNum, p);
  }
}

void project(const vec4 axis, const vec4 sphere, inout float minRet, inout float maxRet) {
  minRet = maxRet = dot(vec4(sphere.xyz, 1.0f), axis);
  minRet -= sphere.w;
  maxRet += sphere.w;
}

vec4 closestPoint(const OBB box, const vec4 p) {
  vec4 result = box.volume.center;
  vec4 dir = p - result;

  for (int i = 0; i < 3; ++i) {
    float dist = dot(dir, box.orientation[i]);

    dist = min(dist,  box.volume.extent[i]);
    dist = max(dist, -box.volume.extent[i]);

    result = result + (box.orientation[i] * dist);
  }

  return result;
}

vec4 getClosestPointToEdge(const vec4 A, const vec4 B, const vec4 point) {
  const vec4 AB = B - A;
  const vec4 AP = point - A;
  const float ab_ab = dot(AB, AB);
  const float ab_ap = dot(AB, AP);
  float t = ab_ap / ab_ab;
  t = clamp(t, 0.0f, 1.0f);

  return A + t * AB;
}

bool overlap(const float min1, const float max1, const float min2, const float max2) {
  const float test1 = min1 - max2;
  const float test2 = min2 - max1;

  return test1 < 0.0f && test2 < 0.0f;
}

bool overlap(const float min1, const float max1, const float min2, const float max2, const vec4 axis, inout float dist, inout vec4 mtv) {
  const float test1 = min1 - max2;
  const float test2 = min2 - max1;

  const float d = min(abs(test1), abs(test2));
  const float half1 = (min1 + max1) / 2.0f;
  const float half2 = (min2 + max2) / 2.0f;
  mtv = half1 < half2 ? axis : -axis;
  dist = d;

  return test1 < 0.0f && test2 < 0.0f;
}

void clipVelocity(const vec4 clipNormal, const float overbounce, inout vec4 vel) {
  float backoff = dot(vel, clipNormal) * overbounce;
  vel = vel - clipNormal * backoff;
}
