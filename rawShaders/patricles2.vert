#version 450

#extension GL_GOOGLE_include_directive : enable

#include "../render/shared_structures.h"

layout(set = 0, binding = 0) uniform Camera {
  mat4 viewproj;
  mat4 view;
  vec4 pos;
  vec4 dir;
  uvec2 dim;
} camera;

layout(std140, set = 2, binding = 0) readonly buffer particles_buffer {
  gpu_particle particles[];
};

struct gpu_particles_data {
  uvec4 int_data;
  uvec4 out_data;
  vec4 gravity;
  vec4 frustum[6];
};

layout(std140, set = 2, binding = 2) readonly buffer particles_data_buffer {
  gpu_particles_data particles_data;
};

layout(location = 0) in uint particle_index;

layout(location = 0) out flat uint out_index;

out gl_PerVertex {
  vec4 gl_Position;
};

void main() {
  //gl_Position = camera.viewproj * particles[particle_index].pos;
  gl_Position = particles[particle_index].pos;
  out_index = particle_index;
}
