#version 450

#extension GL_GOOGLE_include_directive : enable

#include "../render/shared_structures.h"

layout(set = 0, binding = 0) uniform Camera {
  mat4 viewproj;
  mat4 view;
  vec4 pos;
  vec4 dir;
  uint width;
  uint height;
} camera;

// per instance
layout(location = 0) in mat4 model;
layout(location = 4) in uint img;
// layout(location = 5) in uint imageLayer;
// layout(location = 6) in uint samplerIndex;
//layout(location = 7)  in float mirroredU;
//layout(location = 8)  in float mirroredV;
layout(location = 5) in float movementU;
layout(location = 6) in float movementV;
// per vertex
layout(location = 7) in vec4 pos;
layout(location = 8) in vec4 color;
layout(location = 9) in vec2 uv;

layout(location = 0) out flat image out_img;
// layout(location = 0) out flat uint outImageIndex;
// layout(location = 1) out flat uint outSamplerIndex;
//layout(location = 2) out flat float outMirroredU;
//layout(location = 3) out flat float outMirroredV;
layout(location = 1) out flat float outMovementU;
layout(location = 2) out flat float outMovementV;
//layout(location = 6) out vec4 outColor;
layout(location = 3) out vec3 outUV;

out gl_PerVertex {
  vec4 gl_Position;
};

void main() {
  gl_Position = camera.viewproj * model * pos;

  out_img.container = img;
  //outImageIndex = get_image_index(img); // imageIndex
  //outSamplerIndex = get_image_sampler(img); // samplerIndex;
  //outMirroredU = mirroredU;
  //outMirroredV = mirroredV;
  outMovementU = movementU;
  outMovementV = movementV;
  //outColor = color;
  outUV = vec3(uv, get_image_layer(out_img)); //imageLayer
}
