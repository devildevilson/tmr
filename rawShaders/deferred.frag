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

layout(std430, set = 2, binding = 0) readonly buffer textures_data {
  // x - imageIndex, y - imageLayer, z - samplerIndex, w - может быть материалом
  //uvec4 texturesData[];
  image_data texturesData[];
};

// layout(location = 0) in flat uint inImageIndex;
// layout(location = 1) in flat uint inSamplerIndex;
layout(location = 0) in vec4 inNormal;
layout(location = 1) in vec2 inUV;
layout(location = 2) in flat uint faceIndex;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec2 outNormal;

vec2 packNormal(const vec3 normal) {
  float p = sqrt(normal.z * 8 + 8);
  return vec2(normal.xy / p + 0.5);
}

void main() {
  //const uint index = floatBitsToUint(inNormal.w);
  const uint index = faceIndex;
  const image img = texturesData[index].img;
  const uint imageIndex   = get_image_index(img); //texturesData[index].image.index;
  const uint imageLayer   = get_image_layer(img); //texturesData[index].image.layer;
  const uint samplerIndex = get_image_sampler(img); //texturesData[index].samplerIndex;
  const float movementU   = texturesData[index].movementU;
  const float movementV   = texturesData[index].movementV;
  // const float movementU   = 0.0f;
  // const float movementV   = 0.0f;
  const float mirroredU   = flip_u(img) ? -1.0f : 1.0f; //movementU >= 0.0f ? 1.0f : -1.0f;
  const float mirroredV   = flip_v(img) ? -1.0f : 1.0f; //movementV >= 0.0f ? 1.0f : -1.0f;
  const vec2 finalUV = vec2(inUV.x * mirroredU + movementU, inUV.y * mirroredV + movementV);
  const vec4 color = texture(sampler2DArray(textures[imageIndex], samplers[samplerIndex]), vec3(finalUV.x, finalUV.y, float(imageLayer)));
  // vec4 color = texture(sampler2DArray(textures[inImageIndex], samplers[inSamplerIndex]), inUV);

  if (color.w < 0.5) discard;

  // также я могу в альфа канал записать отражение
  // так как он мне больше не потребуется
  outColor = color;
  //outColor = vec4(color.xyz, textures[index].w);

  const vec3 N = mat3(camera.view) * normalize(inNormal.xyz);
  outNormal = packNormal(N);
}
