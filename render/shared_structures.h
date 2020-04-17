#ifndef SHARED_STRUCTURES_H
#define SHARED_STRUCTURES_H

#ifdef __cplusplus
#include <cstdint>
#include "basic_tri.h"

#define DEFAULT_DESCRIPTOR_POOL_NAME "default_descriptor_pool"
#define UNIFORM_BUFFER_LAYOUT_NAME "uniform_layout"
#define MATRICES_BUFFER_LAYOUT_NAME "matrixes_layout"
#define STORAGE_BUFFER_LAYOUT_NAME "storage_layout"
#define SAMPLED_IMAGE_LAYOUT_NAME "sampled_image_layout"

#define INLINE inline
#define INOUT

namespace devils_engine {
  namespace render {
    using glm::floatBitsToUint;
    using glm::uintBitsToFloat;
    
    using uint = uint32_t;
    using mat4 = basic_mat4;
    using vec4 = basic_vec4;
    using vec2 = glm::vec2;
    using vec3 = glm::vec3;
    using uvec4 = glm::uvec4;
#else

#define INLINE
#define INOUT inout
    
#endif

#define GPU_UINT_MAX 0xffffffff

struct image {
  uint container;
};

struct color {
  uint container;
};

struct image_data {
  image img;
  float movementU;
  float movementV;
};

struct vertex {
  vec4 pos;
  vec4 color;
  vec2 texCoord;
};

struct light_data {
  vec4 pos;
  vec4 color;
};

struct camera_data {
  mat4 viewproj;
  mat4 view;
  vec4 pos;
  vec4 dir;
  uint width;
  uint height;
};

struct matrices_data {
  mat4 proj;
  mat4 view;
  mat4 invProj;
  mat4 invView;
  mat4 invViewProj;
};

struct matrices {
  mat4 persp;
  mat4 ortho;
  mat4 view;
  matrices_data matrixes;
  camera_data camera;
};

struct gpu_particle {
  vec4 pos;
  vec4 vel;
  uvec4 int_data;
  vec4 float_data;
  vec4 speed_color;
};

INLINE bool is_image_valid(const image img) {
  return img.container != GPU_UINT_MAX;
}

INLINE uint get_image_index(const image img) {
  const uint mask = 0xffff;
  return (img.container >> 16) & mask;
}

INLINE uint get_image_layer(const image img) {
  const uint mask = 0xff;
  return (img.container >> 8) & mask;
}

INLINE uint get_image_sampler(const image img) {
  const uint mask = 0x3f;
  return (img.container >> 0) & mask;
}

INLINE bool flip_u(const image img) {
  const uint mask = 0x1;
  return bool((img.container >> 7) & mask);
}

INLINE bool flip_v(const image img) {
  const uint mask = 0x1;
  return bool((img.container >> 6) & mask);
}

INLINE float get_color_r(const color col) {
  const float div = 255.0f;
  const uint mask = 0xff;
  const uint comp = (col.container >> 24) & mask;
  return float(comp) / div;
}

INLINE float get_color_g(const color col) {
  const float div = 255.0f;
  const uint mask = 0xff;
  const uint comp = (col.container >> 16) & mask;
  return float(comp) / div;
}

INLINE float get_color_b(const color col) {
  const float div = 255.0f;
  const uint mask = 0xff;
  const uint comp = (col.container >> 8) & mask;
  return float(comp) / div;
}

INLINE float get_color_a(const color col) {
  const float div = 255.0f;
  const uint mask = 0xff;
  const uint comp = (col.container >> 0) & mask;
  return float(comp) / div;
}

INLINE image get_particle_image(const gpu_particle particle) {
  image img;
  img.container = particle.int_data.y;
  return img;
}

INLINE color get_particle_color(const gpu_particle particle) {
  color col;
  col.container = floatBitsToUint(particle.speed_color[2]);
  return col;
}

INLINE uint get_particle_life_time(const gpu_particle particle) {
  const uint mask = 0x7fffffff;
  return (particle.int_data.z & mask);
}

INLINE bool particle_is_on_ground(const gpu_particle particle) {
  const uint mask = 0x7fffffff;
  return bool(particle.int_data.z & ~mask);
}

INLINE uint get_particle_current_time(const gpu_particle particle) {
  return particle.int_data.w;
}

INLINE float get_particle_max_speed(const gpu_particle particle) {
  return particle.speed_color[0];
}

INLINE float get_particle_min_speed(const gpu_particle particle) {
  return particle.speed_color[1];
}

INLINE float get_particle_friction(const gpu_particle particle) {
  return particle.speed_color[3];
}

INLINE float get_particle_max_scale(const gpu_particle particle) {
  return particle.float_data[0];
}

INLINE float get_particle_min_scale(const gpu_particle particle) {
  return particle.float_data[1];
}

INLINE float get_particle_gravity(const gpu_particle particle) {
  return particle.float_data[2];
}

INLINE float get_particle_bounce(const gpu_particle particle) {
  return particle.float_data[3];
}

INLINE void set_particle_pos(INOUT gpu_particle particle, const vec4 pos) {
  particle.pos = pos;
}

INLINE void set_particle_vel(INOUT gpu_particle particle, const vec4 vel) {
  particle.vel = vel;
}

INLINE void set_particle_current_time(INOUT gpu_particle particle, const uint time) {
  particle.int_data.w = time;
#ifdef __cplusplus
  (void)particle;
#endif
}

INLINE void set_particle_is_on_ground(INOUT gpu_particle particle, const bool value) {
  const uint mask = 0x7fffffff;
  particle.int_data.z = value ? particle.int_data.z | ~mask : particle.int_data.z & mask;
}

const uint min_speed_stop      = (1 << 0);
const uint min_speed_remove    = (1 << 1);
const uint max_speed_stop      = (1 << 2);
const uint max_speed_remove    = (1 << 3);
const uint speed_dec_over_time = (1 << 4);
const uint speed_inc_over_time = (1 << 5);
const uint scale_dec_over_time = (1 << 6);
const uint scale_inc_over_time = (1 << 7);
const uint scaling_along_vel   = (1 << 8);
const uint limit_max_speed     = (1 << 9);
const uint limit_min_speed     = (1 << 10);

INLINE bool particle_min_speed_stop(const gpu_particle particle) {
  return (particle.int_data.x & min_speed_stop) == min_speed_stop;
}

INLINE bool particle_min_speed_remove(const gpu_particle particle) {
  return (particle.int_data.x & min_speed_remove) == min_speed_remove;
}

INLINE bool particle_max_speed_stop(const gpu_particle particle) {
  return (particle.int_data.x & max_speed_stop) == max_speed_stop;
}

INLINE bool particle_max_speed_remove(const gpu_particle particle) {
  return (particle.int_data.x & max_speed_remove) == max_speed_remove;
}

INLINE bool particle_speed_dec_over_time(const gpu_particle particle) {
  return (particle.int_data.x & speed_dec_over_time) == speed_dec_over_time;
}

INLINE bool particle_speed_inc_over_time(const gpu_particle particle) {
  return (particle.int_data.x & speed_inc_over_time) == speed_inc_over_time;
}

INLINE bool particle_scale_dec_over_time(const gpu_particle particle) {
  return (particle.int_data.x & scale_dec_over_time) == scale_dec_over_time;
}

INLINE bool particle_scale_inc_over_time(const gpu_particle particle) {
  return (particle.int_data.x & scale_inc_over_time) == scale_inc_over_time;
}

INLINE bool particle_scaling_along_vel(const gpu_particle particle) {
  return (particle.int_data.x & scaling_along_vel) == scaling_along_vel;
}

INLINE bool particle_limit_max_speed(const gpu_particle particle) {
  return (particle.int_data.x & limit_max_speed) == limit_max_speed;
}

INLINE bool particle_limit_min_speed(const gpu_particle particle) {
  return (particle.int_data.x & limit_min_speed) == limit_min_speed;
}

#ifdef __cplusplus
    INLINE image create_image(const uint16_t &index, const uint8_t &layer, const uint8_t &sampler, const bool flip_u, const bool flip_v) {
      ASSERT(sampler < 0x3f);
      return {(uint(index) << 16) | (uint(layer) << 8) | uint(sampler) | (uint(flip_u) << 7) | (uint(flip_v) << 6)};
    }
    
    INLINE image create_image(const uint16_t &index, const uint8_t &layer, const uint8_t &sampler) {
      ASSERT(sampler < 0x3f);
      return {(uint(index) << 16) | (uint(layer) << 8) | uint(sampler)};
    }
    
    INLINE image create_image(const uint16_t &index, const uint8_t &layer) {
      return {(uint(index) << 16) | (uint(layer) << 8)};
    }
  }
}
#endif

#endif
