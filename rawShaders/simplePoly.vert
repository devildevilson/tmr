#version 450

layout(set = 0, binding = 0) uniform Matrix {
  mat4 viewproj;
} m;

layout(location = 0) in vec4 pos;
layout(location = 1) in vec4 normal;
layout(location = 2) in vec2 uv;
//layout(location = 0) out vec4 colorOut;
layout(location = 0) out flat uint index;

out gl_PerVertex {
  vec4 gl_Position;
};

void main() {
  const vec4 finalPos = pos + vec4(normal.xyz, 0.0f) * (0.1f);
  gl_Position = m.viewproj * finalPos;

  index = floatBitsToUint(normal.w);
}
