#include "Utility.h"

#define SIMD_IMPLEMENTATION
#include "simd.h"

float fast_fabsf(const float &f) {
  int i=((*(int*)&f)&0x7fffffff);

  return (*(float*)&i);
}

float sideOf(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &point, const glm::vec3 &normal) {
  glm::vec3 vec = b - a;
  vec = glm::cross(vec, normal);
  return glm::dot(vec, point - a);
}

float sideOf(const glm::vec4 &a, const glm::vec4 &b, const glm::vec4 &point, const glm::vec4 &normal) {
  glm::vec4 vec = b - a;
  vec = glm::vec4(glm::cross(glm::vec3(vec), glm::vec3(normal)), 0.0f);
  return glm::dot(vec, point - a);
}

float sideOf(const simd::vec4 &a, const simd::vec4 &b, const simd::vec4 &point, const simd::vec4 &normal) {
  simd::vec4 vec = b - a;
  // по идее, в 'w' должен остаться 0
  // тут 2 умножения, одно вычитание, w*w - w*w, если вектор будет верен (w == либо 1 либо 0), то это будет приводить к верному результату
  // ко всему прочему в кросс обычно попадает нормаль
  vec = simd::cross(vec, normal);
  return simd::dot(vec, point - a);
}

glm::vec3 closestPointOnPlane(const glm::vec3 &normal, const float &dist, const glm::vec3 &point) {
  float distance = glm::dot(normal, point) - dist;
  return point - distance*normal;
}

glm::vec4 closestPointOnPlane(const glm::vec4 &normal, const float &dist, const glm::vec4 &point) {
  float distance = glm::dot(normal, point) - dist;
  return point - distance*normal;
}

simd::vec4 closestPointOnPlane(const simd::vec4 &normal, const float &dist, const simd::vec4 &point) {
  float distance = simd::dot(normal, point) - dist;
  return point - distance*normal;
}

bool f_eq(const float &a, const float &b) {
  return fast_fabsf(a - b) < EPSILON;
}

bool vec_eq(const glm::vec3 &vec1, const glm::vec3 &vec2) {
  return fast_fabsf(vec1.x - vec2.x) < EPSILON && 
         fast_fabsf(vec1.y - vec2.y) < EPSILON && 
         fast_fabsf(vec1.z - vec2.z) < EPSILON;
}

bool vec_eq(const glm::vec2 &a, const glm::vec2 &b) {
  return fast_fabsf(a.x - b.x) < EPSILON && 
         fast_fabsf(a.y - b.y) < EPSILON;
}

bool vec_eq(const simd::vec4 &a, const simd::vec4 &b) {
  // по идее должно быть так
  return ((simd::abs(a - b) < simd::vec4(EPSILON)).movemask() & 0x1111) == 0x1111;
}

glm::vec3 projectPointOnPlane(const glm::vec3 &normal, const glm::vec3 &origin, const glm::vec3 &point) {
  glm::vec3 vectorOP = point - origin;
  float dist = glm::dot(vectorOP, normal);
  return point - normal*dist;
}

simd::vec4 projectPointOnPlane(const simd::vec4 &normal, const simd::vec4 &origin, const simd::vec4 &point) {
  simd::vec4 vectorOP = point - origin;
  float dist = simd::dot(vectorOP, normal);
  return point - normal*dist;
}

glm::vec3 projectVectorOnPlane(const glm::vec3 &normal, const glm::vec3 &origin, const glm::vec3 &vector) {
  float dist = glm::dot(vector, normal);
  glm::vec3 point2 = origin + vector - normal*dist;
  return point2 - origin;
}

glm::vec4 projectVectorOnPlane(const glm::vec4 &normal, const glm::vec4 &origin, const glm::vec4 &vector) {
  float dist = glm::dot(vector, normal);
  glm::vec4 point2 = origin + vector - normal*dist;
  return point2 - origin;
}

simd::vec4 projectVectorOnPlane(const simd::vec4 &normal, const simd::vec4 &origin, const simd::vec4 &vector) {
  float dist = simd::dot(vector, normal);
  const simd::vec4 point2 = origin + vector - normal*dist;
  return point2 - origin;
}

float atan2Approximation(float y, float x) {
  float t0, t1, t2, t3;
  t2 = fast_fabsf(x);
  t1 = fast_fabsf(y);
  t0 = fmax(t2, t1);
  t1 = fmin(t2, t1);
  t2 = 1.0f / t0;
  t2 = t1 * t2;
  t3 = t2 * t2;
  t0 =         - 0.013480470f;
  t0 = t0 * t3 + 0.057477314f;
  t0 = t0 * t3 - 0.121239071f;
  t0 = t0 * t3 + 0.195635925f;
  t0 = t0 * t3 - 0.332994597f;
  t0 = t0 * t3 + 0.999995630f;
  t2 = t0 * t2;
  t2 = (fast_fabsf(y) > fast_fabsf(x)) ? 1.570796327f-t2 : t2;
  t2 = (x < 0) ? 3.141592654f-t2:t2;
  t2 = (y < 0) ? -t2 : t2;
  return t2;
}

void cartesianToSpherical(const glm::vec3 &vec, float &theta, float &phi) {
  theta = glm::acos(vec.y / 1.0f);
  phi = glm::atan(vec.z, vec.x);
}
