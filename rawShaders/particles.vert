#version 450

layout(set = 0, binding = 0) uniform Camera {
  mat4 viewproj;
  mat4 view;
  vec4 pos;
  vec4 dir;
  uint width;
  uint height;
} camera;

layout(location = 0) in vec4 pos;
layout(location = 1) in vec4 vel;
layout(location = 2) in vec4 col;
layout(location = 3) in uvec4 uData;
layout(location = 4) in vec4 fData;
layout(location = 5) in uvec4 texture;

layout(location = 0) out vec4 outVel;
layout(location = 1) out vec4 outCol;
layout(location = 2) out uvec4 outUData;
layout(location = 3) out vec4 outFData;
layout(location = 4) out uvec4 outTexture;

out gl_PerVertex {
  vec4 gl_Position;
};

void main() {
  gl_Position = pos; // проще вычислить позицию на следующем шаге
  // camera.viewproj *

  outVel = vel;
  outCol = col;
  outUData = uData;
  outFData = fData;
  outTexture = texture;
}
