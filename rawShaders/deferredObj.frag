#version 450

#extension GL_GOOGLE_include_directive : enable

#include "../render/shared_structures.h"

layout(constant_id = 0) const uint imagesCount = 2;
layout(constant_id = 1) const uint samplersCount = 1;

layout(set = 0, binding = 0) uniform Camera {
  mat4 viewproj;
  mat4 view;
  vec4 pos;
  vec4 dir;
  uvec2 dim;
} camera;

layout(set = 1, binding = 0) uniform texture2DArray textures[imagesCount];
layout(set = 1, binding = 1) uniform sampler samplers[samplersCount];

layout(location = 0) in flat image in_img;
// layout(location = 0) in flat uint inImageIndex;
// layout(location = 1) in flat uint inSamplerIndex;
//layout(location = 2) in flat float inMirroredU;
//layout(location = 3) in flat float inMirroredV;
layout(location = 1) in flat float inMovementU;
layout(location = 2) in flat float inMovementV;
//layout(location = 6) in vec4 inColor;
layout(location = 3) in vec3 inUV;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec2 outNormal;
// видимо нужно будет сделать uint id объекта

vec2 packNormal(const vec3 normal) {
  const float p = sqrt(normal.z * 8 + 8);
  return vec2(normal.xy / p + 0.5);
}

void main() {
  // const float inMovementU = 0.0f;
  // const float inMovementV = 0.0f;
  // const float mirrorU = inMovementU >= 0.0f ? 1.0f : -1.0f;
  // const float mirrorV = inMovementV >= 0.0f ? 1.0f : -1.0f;
  const uint imageIndex   = get_image_index(in_img); //texturesData[index].image.index;
  //const uint imageLayer   = get_image_layer(in_img); //texturesData[index].image.layer;
  const uint samplerIndex = get_image_sampler(in_img); //texturesData[index].samplerIndex;
  const float mirrorU   = flip_u(in_img) ? -1.0f : 1.0f;
  const float mirrorV   = flip_u(in_img) ? -1.0f : 1.0f;
  const vec3 finalUV = vec3(inUV.x * mirrorU + inMovementU, inUV.y * mirrorV + inMovementV, inUV.z);
  vec4 color = texture(sampler2DArray(textures[imageIndex], samplers[samplerIndex]), finalUV);

  if (color.w < 0.5f) discard;

  // в альфа канал нужно записать отличие пикселя монстра от пикселя окружения
  // для того чтобы правильно вычислить свет (свет должен равномерно падать на моснстра вне зависимости от положения наблюдателя)

  color.w = 0.9f;
  outColor = color;
  const vec3 norm = mat3(camera.view) * camera.dir.xyz;
  outNormal = packNormal(-norm);
}
