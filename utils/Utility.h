#ifndef UTILITY_H
#define UTILITY_H

#define APPLICATION_NAME "VulkanTest"
#define ENGINE_NAME "test_engine"

#define MAKE_VERSION(major, minor, patch) \
(major << 22) | (minor << 12) | patch

#define VERSION_TO_STR(ver) \
(std::to_string((ver >> 22) & 0xffc) + "." + \
 std::to_string((ver >> 12) & 0x3ff) + "." + \
 std::to_string(ver & 0xfff))

#define EGINE_VERSION MAKE_VERSION(0, 0, 1)
#define APP_VERSION   MAKE_VERSION(0, 0, 1)

#define ENGINE_VERSION_STR VERSION_TO_STR(EGINE_VERSION)
#define APP_VERSION_STR    VERSION_TO_STR(APP_VERSION)

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
// #define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
// #define GLM_FORCE_SSE42
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "simd.h"

#include "Logging.h"

#ifdef _DEBUG
  #include <cassert>
  #define ASSERT(expr) assert(expr)
#else
  #define ASSERT(expr)
#endif

#define EPSILON 0.000001f
#define MCS_TO_SEC(dt) (float(dt) / 1000000.0f)

float fast_fabsf(const float &f);

float sideOf(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &point, const glm::vec3 &normal);
float sideOf(const glm::vec4 &a, const glm::vec4 &b, const glm::vec4 &point, const glm::vec4 &normal);
float sideOf(const simd::vec4 &a, const simd::vec4 &b, const simd::vec4 &point, const simd::vec4 &normal);

glm::vec3 closestPointOnPlane(const glm::vec3 &normal, const float &dist, const glm::vec3 &point);
glm::vec4 closestPointOnPlane(const glm::vec4 &normal, const float &dist, const glm::vec4 &point);
simd::vec4 closestPointOnPlane(const simd::vec4 &normal, const float &dist, const simd::vec4 &point);

bool f_eq(const float &a, const float &b);
bool vec_eq(const glm::vec3 &a, const glm::vec3 &b);
bool vec_eq(const glm::vec2 &a, const glm::vec2 &b);
bool vec_eq(const simd::vec4 &a, const simd::vec4 &b);

glm::vec3 projectPointOnPlane(const glm::vec3 &normal, const glm::vec3 &origin, const glm::vec3 &point);
simd::vec4 projectPointOnPlane(const simd::vec4 &normal, const simd::vec4 &origin, const simd::vec4 &point);
glm::vec3 projectVectorOnPlane(const glm::vec3 &normal, const glm::vec3 &origin, const glm::vec3 &vector);
glm::vec4 projectVectorOnPlane(const glm::vec4 &normal, const glm::vec4 &origin, const glm::vec4 &vector);
simd::vec4 projectVectorOnPlane(const simd::vec4 &normal, const simd::vec4 &origin, const simd::vec4 &vector);

float atan2Approximation(float y, float x);

void cartesianToSpherical(const simd::vec4 &vec, float &theta, float &phi);

#endif
