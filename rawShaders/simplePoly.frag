#version 450

layout(std140, set = 1, binding = 0) readonly buffer Colors {
  vec4 colors[];
};

layout(location = 0) in flat uint index;
layout(location = 0) out vec4 outColor;

void main() {
  outColor = colors[index];
}
