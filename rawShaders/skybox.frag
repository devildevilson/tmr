#version 450

#extension GL_GOOGLE_include_directive : enable

#include "../render/shared_structures.h"
#include "../utils/shared_mathematical_constants.h"

layout(set = 0, binding = 0) uniform Camera {
  mat4 viewproj;
  mat4 view;
  vec4 pos;
  vec4 dir;
  uvec2 dim;
} camera;

layout(set = 1, binding = 0) uniform texture2DArray textures[imagesCount];
layout(set = 1, binding = 1) uniform sampler samplers[samplersCount];

struct instance_data_t {
  mat4 matrix;
  image_data t;
  uint dummy[1];
};

layout(std430, set = 2, binding = 0) readonly buffer textures_data {
  instance_data_t instances[];
};

layout(location = 0) in vec4 normal;
layout(location = 1) in vec2 uv;
layout(location = 2) in flat uint faceIndex;

// че делать с текстуркой заливки?
// в качестве такой текстурки предлагают использовать скайбокс
// то есть кубическая текстурка рисуется вокруг большой области
// и видимо такой подход - это стандартный способ нарисовать окружение
void main() {
  const float _StretchDown = 1.0f; // ???
  const vec2 final_uv = vec2(0.5+atan(camera.dir.z, camera.dir.x) / (PI_2), _StretchDown + camera.dir.y * (1.0f-_StretchDown));
}
