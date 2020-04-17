#ifndef SHARED_PHYSICS_STRUCTURES_H
#define SHARED_PHYSICS_STRUCTURES_H

#ifdef __cplusplus

#include "Utility.h"

#define INLINE inline
#define INOUT

namespace devils_engine {
  namespace physics {
    using vec4 = simd::vec4;
    using quat = simd::quat;
    using mat4 = simd::mat4;
    using uint = uint32_t;
#else
    
#define INLINE
#define INOUT inout

#endif
    
const uint shape_type_box = 0;
const uint shape_type_sphere = shape_type_box+1;
const uint shape_type_polygon = shape_type_sphere+1;
const uint shape_type_convex_hull = shape_type_polygon+1;
const uint shape_type_max = shape_type_convex_hull+1;

struct transform {
  vec4 pos;
  quat rot;
  vec4 scale;
};

struct rigid_body {
  mat4 inv_inertia_tensor;
  vec4 linear_velocity;
  vec4 angular_velocity;

  vec4 force;
  vec4 torque;
  
  vec4 inv_local_inertia;

  vec4 float_data[2];
  
#ifdef __cplusplus
  float friction() const;
  float restituition() const;
  float inv_mass() const;
  float linear_damping() const;
  float angular_damping() const;
  float stair_height() const;
#endif
};

struct object_flags {
  uint container;
};

struct collision_shape {
  uint type;
  uint points_offset;
  uint points_count;
  uint normals_count;
};

struct object {
  uint transform_index;
  uint rigid_body_index;
  uint collision_shape_index;
  uint proxy_index;
};

struct aabb {
  vec4 center;
  vec4 extents;
};

// const uint object_solid = (1 << 2);
// const uint object_visible = (1 << 3);
// const uint object_collide_rays = (1 << 4);
const uint object_on_ground = (1 << 2);
const uint object_was_on_ground = (1 << 3);

INLINE uint get_object_type(const object_flags flags) {
  const uint mask = 0b11;
  return flags.container & mask;
}

INLINE bool is_object_on_ground(const object_flags flags) {
  return (flags.container & object_on_ground) == object_on_ground;
}

INLINE bool was_object_on_ground(const object_flags flags) {
  return (flags.container & object_was_on_ground) == object_was_on_ground;
}

#ifdef __cplusplus 

INLINE void set_object_on_ground(object_flags &flags, const bool value) {
  flags.container = value ? flags.container | object_on_ground : flags.container & ~object_on_ground;
}

INLINE void set_object_was_on_ground(object_flags &flags, const bool value) {
  flags.container = value ? flags.container | object_was_on_ground : flags.container & ~object_was_on_ground;
}

#else

INLINE void set_object_on_ground(INOUT object_flags flags, const bool value) {
  flags.container = value ? flags.container | object_on_ground : flags.container & ~object_on_ground;
}

INLINE void set_object_was_on_ground(INOUT object_flags flags, const bool value) {
  flags.container = value ? flags.container | object_was_on_ground : flags.container & ~object_was_on_ground;
}

#endif
    
#ifdef __cplusplus
  }
}
#endif
    
#endif
