#version 450

#extension GL_GOOGLE_include_directive : enable

#include "../render/shared_structures.h"

layout(constant_id = 0) const uint imagesCount = 2;
layout(constant_id = 1) const uint samplersCount = 1;

layout(set = 1, binding = 0) uniform texture2DArray textures[imagesCount];
layout(set = 1, binding = 1) uniform sampler samplers[samplersCount];

layout(location = 0) in flat image in_img;
layout(location = 1) in vec4 in_color;
layout(location = 2) in vec2 in_tex_coords;

layout(location = 0) out vec4 out_color;
layout(location = 1) out vec2 out_normal;

vec2 packNormal(const vec3 normal) {
  float p = sqrt(normal.z * 8 + 8);
  return vec2(normal.xy / p + 0.5);
}

void main() {
  const bool valid_image = is_image_valid(in_img);
  const uint index = get_image_index(in_img);
  const uint layer = get_image_layer(in_img);
  const uint sampler_index = get_image_sampler(in_img);
  const vec4 color = valid_image ? in_color * texture(sampler2DArray(textures[index], samplers[sampler_index]), vec3(in_tex_coords, float(layer))) : in_color;

  if (color.w < 0.5f) discard;

  out_color = color;
  out_color.w = 0.9f;
  const vec3 norm = vec3(0.0f, 0.0f, 0.0f);
  out_normal = packNormal(-norm);
}
