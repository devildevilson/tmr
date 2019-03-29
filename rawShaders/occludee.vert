#version 450

layout(set = 0, binding = 0) uniform UBO {
  mat4 viewproj;
  mat4 view;
  vec4 pos;
  vec4 dir;
  uvec2 dim;
} ubo;

// per instance
layout(location = 0) in mat4 inModel;
layout(location = 4) in uint id;

//per vertex
layout(location = 5) in vec3 inPos;

layout(location = 0) out flat uint objId;

out gl_PerVertex {
  vec4 gl_Position;
};

void main() {
  objId = id;
  // обязательно нужно не забывать об этой единичке в vec4
  gl_Position = ubo.viewproj * inModel * vec4(inPos, 1.0);
}
