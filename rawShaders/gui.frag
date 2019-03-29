#version 450 core

layout(set = 1, binding = 0) uniform sampler2D sTexture;

layout(location = 0) in vec4 inColor;
layout(location = 1) in vec2 inUV;

layout(location = 0) out vec4 fColor;

void main() {
  fColor = inColor * texture(sTexture, inUV);
}
