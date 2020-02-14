#ifndef UTILITY_H
#define UTILITY_H

#include <cstdint>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
// #define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
// #define GLM_FORCE_SSE42
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/norm.hpp>

#include "simd.h"

#include "Logging.h"

#include "shared_time_constant.h"
#include "shared_mathematical_constant.h"
#include "shared_application_constant.h"

#ifdef _DEBUG
  #include <cassert>
  #define ASSERT(expr) assert(expr)
#else
  #define ASSERT(expr)
#endif

#define PRINT_VEC4(name, vec) std::cout << name << " x: " << vec.x << " y: " << vec.y << " z: " << vec.z << " w: " << vec.w << "\n";
#define PRINT_VEC3(name, vec) std::cout << name << " x: " << vec.x << " y: " << vec.y << " z: " << vec.z << "\n";
#define PRINT_VEC2(name, vec) std::cout << name << " x: " << vec.x << " y: " << vec.y << "\n";
#define PRINT_VAR(name, var) std::cout << name << ": " << var << "\n";
#define PRINT(var) std::cout << var << "\n";

class TimeLogDestructor {
public:
  explicit TimeLogDestructor(const std::string &desc);
  ~TimeLogDestructor();

  void updateTime();
private:
  std::string desc;
  std::chrono::steady_clock::time_point point;
};

float fast_fabsf(const float &f);

float sideOf(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &point, const glm::vec3 &normal);
float sideOf(const glm::vec4 &a, const glm::vec4 &b, const glm::vec4 &point, const glm::vec4 &normal);
float sideOf(const simd::vec4 &a, const simd::vec4 &b, const simd::vec4 &point, const simd::vec4 &normal);

bool intersect(const simd::vec4 &planePoint, const simd::vec4 &planeNormal, const simd::vec4 &a, const simd::vec4 &b, simd::vec4 &point);

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

uint32_t nextPowerOfTwo(const uint32_t &x);

float getAngle(const simd::vec4 &a, const simd::vec4 &b);
float getAngle(const glm::vec4 &a, const glm::vec4 &b);

#endif
