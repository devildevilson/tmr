#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "physics_core.h"

namespace devils_engine {
  namespace physics {
    namespace core {
      struct transform {
        vec4 pos;
        quat rot;
        vec4 scale;
        
        transform();
        transform(const transform &t);
        transform(transform&& t);
        transform(const vec4 &pos, const quat &rot, const vec4 &scale);
        transform & operator=(const transform &t);
        transform & operator=(transform&& t);
        mat4 get_basis() const;
        void indentity();
        transform & operator*=(const transform &t);
        // scale?
        vec4 transform_vector(const vec4 &vec) const;
        vec4 inv_transform(const vec4 &vec) const;
        
        static void integrate(const transform &current, const simd::vec4 &linvel, const simd::vec4 &angvel, const scalar &time_step, transform &predicted);
        static void calculate_diff_axis_angle(const transform& transform0, const transform& transform1, vec4& axis, scalar& angle);
        static void calculate_velocity(const transform& transform0, const transform& transform1, const scalar &time_step, vec4& linvel, vec4& angvel);
      };
      
      transform inverse(const transform &t);
      transform operator*(const transform &t1, const transform &t2);
    }
  }
}

#endif
