#version 450

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
layout(location = 4) in uint imageIndex;
layout(location = 5) in uint imageLayer;
layout(location = 6) in uint samplerIndex;
//layout(location = 7)  in float mirroredU;
//layout(location = 8)  in float mirroredV;
layout(location = 7) in float movementU;
layout(location = 8) in float movementV;
// per vertex
layout(location = 9)  in vec4 pos;
layout(location = 10) in vec4 color;
layout(location = 11) in vec2 uv;

layout(location = 0) out flat uint outImageIndex;
layout(location = 1) out flat uint outSamplerIndex;
//layout(location = 2) out flat float outMirroredU;
//layout(location = 3) out flat float outMirroredV;
layout(location = 2) out flat float outMovementU;
layout(location = 3) out flat float outMovementV;
//layout(location = 6) out vec4 outColor;
layout(location = 4) out vec3 outUV;

out gl_PerVertex {
  vec4 gl_Position;
};

void main() {
  gl_Position = camera.viewproj * model * pos;

  outImageIndex = imageIndex;
  outSamplerIndex = samplerIndex;
  //outMirroredU = mirroredU;
  //outMirroredV = mirroredV;
  outMovementU = movementU;
  outMovementV = movementV;
  //outColor = color;
  outUV = vec3(uv, imageLayer);
}
