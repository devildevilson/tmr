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

struct instance_data_t {
  mat4 matrix;
  image_data t;
  // по каким то причинам плохо работает единичный массив
  // в glsl все бесполезные байты нужно указывать явно
  uint dummy;
};

layout(std430, set = 2, binding = 0) readonly buffer textures_data {
  instance_data_t instances[];
};

const mat4 toVulkanSpace = mat4(1.0f, 0.0f, 0.0f, 0.0f,
                                0.0f,-1.0f, 0.0f, 0.0f,
                                0.0f, 0.0f, 1.0f, 0.0f,
                                0.0f, 0.0f, 0.0f, 1.0f);

layout(location = 0) in vec4 pos;
layout(location = 1) in vec4 normal;
layout(location = 2) in vec2 uv;

layout(location = 0) out vec4 outNormal;
layout(location = 1) out vec2 outUV;
layout(location = 2) out flat uint faceIndex;

out gl_PerVertex {
  vec4 gl_Position;
};

void main() {
  faceIndex = floatBitsToUint(normal.w);
  const mat4 model_matrix = instances[faceIndex].matrix;
  gl_Position = camera.viewproj * model_matrix * pos;

  outNormal = vec4(normal.xyz, 0.0f);
  outUV = uv;
}
