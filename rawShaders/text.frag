#version 450 core

layout (location = 0) in vec2 inUV;

layout (set = 0, binding = 0) uniform sampler   samplerFont;
layout (set = 1, binding = 0) uniform texture2D sampledTexture;

layout (location = 0) out vec4 outFragColor;

void main(void)
{
  vec4 color = texture(sampler2D(sampledTexture, samplerFont), inUV);
  if ((color.r == 0.0) && (color.g == 0.0) && (color.b == 0.0)) {
    discard;
  }
  outFragColor = color;
}
