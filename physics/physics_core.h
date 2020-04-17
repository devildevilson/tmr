#ifndef PHYSICS_CORE_H
#define PHYSICS_CORE_H

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

#include <cstdint>
#include <cstddef>
#include <cstring>

#include "shared_mathematical_constant.h"
#include "shared_time_constant.h"

#ifdef _DEBUG
  #include <cassert>
  #define ASSERT(expr) assert(expr)
#else
  #define ASSERT(expr)
#endif

#define ANGULAR_MOTION_THRESHOLD (float(0.5) * PI_H)

#define DE_LARGE_FLOAT 1e18f
#define DE_SQRT12 0.7071067811865475244008443621048490
#define DE_PASSABLE_ANGLE PASSABLE_ANGLE

namespace devils_engine {
  namespace physics {
    namespace core {
      using scalar = float;
      using vec4 = simd::vec4;
      using mat4 = simd::mat4;
      using quat = simd::quat;
      
      struct aabb {
        vec4 center;
        vec4 extents;
      };
      
      struct sphere {
        vec4 center;
        vec4 extents;
      };
      
      inline void plane_space(const vec4 &n, vec4 &p, vec4 &q) {
        float arr_n[4];
        n.storeu(arr_n);
        float arr_p[4];
        float arr_q[4];
        memset(arr_p, 0, sizeof(float)*4);
        memset(arr_q, 0, sizeof(float)*4);
        if (std::abs(arr_n[2]) > DE_SQRT12) {
          scalar a = arr_n[1]*arr_n[1] + arr_n[2]*arr_n[2];
          scalar k = 1.0f / std::sqrt(a);
          arr_p[0] = 0.0f;
          arr_p[1] = -arr_n[2] * k;
          arr_p[2] =  arr_n[1] * k;
          arr_q[0] = a * k;
          arr_q[1] = -arr_n[0] * arr_p[2];
          arr_q[2] =  arr_n[0] * arr_p[1];
        } else {
          scalar a = arr_n[0]*arr_n[0] + arr_n[1]*arr_n[1];
          scalar k = 1.0f / std::sqrt(a);
          arr_p[0] = -arr_n[1] * k;
          arr_p[1] =  arr_n[0] * k;
          arr_p[2] = 0.0f;
          arr_q[0] = -arr_n[2] * arr_p[1];
          arr_q[1] =  arr_n[2] * arr_p[0];
          arr_q[2] = a * k;
        }
        p.loadu(arr_p);
        q.loadu(arr_q);
      }
    }
  }
}

#endif
