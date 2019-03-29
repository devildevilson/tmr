#version 450

layout(early_fragment_tests) in;

layout(set = 0, binding = 0) uniform UBO {
  mat4 viewproj;
  mat4 view;
  vec4 pos; // возможно причина кроется в vec3
  vec4 dir;
  uvec2 dim;
} ubo;

layout(std430, set = 1, binding = 0) buffer visibleBuffer {
  //uvec4 counts;
  uint visibles[];
};

layout(location = 0) in flat uint objId;

//layout(location = 0) out vec4 out_Color;

void main() {
  visibles[objId] = 1;

  //out_Color = vec4(0.0, 0.0, 1.0, 1.0);
  //out_Color = unpackUnorm4x8(uint(objId));
}
