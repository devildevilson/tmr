#version 450

layout(set = 0, binding = 0) uniform Matrix {
  mat4 viewproj;
} m;

// per instance
layout(location = 0) in uint imageIndex;
layout(location = 1) in uint imageLayer;
layout(location = 2) in uint samplerIndex;
// per vertex
layout(location = 3) in vec3 pos;
layout(location = 4) in vec4 color;
layout(location = 5) in vec2 uv;

layout(location = 0) out flat uint outImageIndex;
layout(location = 1) out flat uint outSamplerIndex;
layout(location = 2) out vec4 outColor;
layout(location = 3) out vec3 outUV;

out gl_PerVertex {
  vec4 gl_Position;
};

void main() {
  gl_Position = m.viewproj * vec4(pos, 1.0);
  
  outImageIndex = imageIndex;
  outSamplerIndex = samplerIndex;
  outColor = color;
  outUV = vec3(uv, imageLayer);
}
