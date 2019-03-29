#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform Matrix {
  mat4 viewproj;
} m;

layout(location = 0) in mat4 matrix;

layout(location = 4) in vec4 color;

layout(location = 5) in vec4 pos;
layout(location = 6) in vec4 normal;
layout(location = 7) in vec2 uv;

layout(location = 0) out vec4 colorOut;

out gl_PerVertex {
  vec4 gl_Position;
};

void main() {
  gl_Position = m.viewproj * matrix * pos;
  colorOut = color;
}
