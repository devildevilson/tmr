#version 450

layout(set = 0, binding = 0) uniform Camera {
  mat4 viewproj;
  mat4 view;
  vec4 pos;
  vec4 dir;
  uvec2 dim;
} camera;

const mat4 toVulkanSpace = mat4(1.0f, 0.0f, 0.0f, 0.0f,
                                0.0f,-1.0f, 0.0f, 0.0f,
                                0.0f, 0.0f, 1.0f, 0.0f,
                                0.0f, 0.0f, 0.0f, 1.0f);

// per instance
// layout(location = 0) in uint imageIndex;
// layout(location = 1) in uint imageLayer;
// layout(location = 2) in uint samplerIndex;
// per vertex
layout(location = 0) in vec4 pos;
layout(location = 1) in vec4 normal;
layout(location = 2) in vec2 uv;

// layout(location = 0) out flat uint outImageIndex;
// layout(location = 1) out flat uint outSamplerIndex;
layout(location = 0) out vec4 outNormal;
// layout(location = 3) out vec3 outUV;
layout(location = 1) out vec2 outUV;
layout(location = 2) out flat uint faceIndex;

out gl_PerVertex {
  vec4 gl_Position;
};

void main() {
  gl_Position = camera.viewproj * pos;

  outNormal = vec4(normal.xyz, 0.0f);
  outUV = uv;
  faceIndex = floatBitsToUint(normal.w);

  // аккуратнее со значениями передаваемыми в следующие шейдеры
  // нормаль передается фигово ВСЕГДА даже если значения одинаковые у всех вершин
  // необходимо использовать модификатор flat
}
