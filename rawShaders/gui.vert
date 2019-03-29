#version 450 core

struct Texture {
  uint imageIndex;
  uint layerIndex;
  uint samplerIndex;
};

layout(set = 0, binding = 0) uniform UniformBufferObject {
  mat4 proj;
} ubo;

// layout(push_constant) uniform uPushConstant {
//   vec2 uScale;
//   vec2 uTranslate;
// } pc;

layout(push_constant) uniform uPushConstant {
  Texture texture;
} pc;

layout(location = 0) in vec2 inPos;
layout(location = 1) in vec2 inUV;
layout(location = 2) in uvec4 inColor;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec2 outUV;

out gl_PerVertex {
  vec4 gl_Position;
};

void main() {
  outColor = vec4(inColor[0]/255.0, inColor[1]/255.0, inColor[2]/255.0, inColor[3]/255.0);
  //outColor = inColor;
  outUV = inUV; // * pc.uScale + pc.uTranslate
  gl_Position = ubo.proj * vec4(inPos, 0.0f, 1.0f);
}
