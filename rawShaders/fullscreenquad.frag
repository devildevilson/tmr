#version 450

layout(set = 0, binding = 0) uniform Camera {
  mat4 viewproj;
  vec4 pos;
  vec4 dir;
  uvec2 dim;
} camera;

layout(set = 1, binding = 0, rgba8) uniform readonly image2D inImage;

layout (location = 0) in vec2 outUV;
layout (location = 0) out vec4 outColor;

void main() {
  ivec2 vec = ivec2(outUV * camera.dim);
  outColor = imageLoad(inImage, vec);
}
