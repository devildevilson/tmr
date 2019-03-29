#version 450

layout(set = 0, binding = 0) uniform Matrix {
  mat4 viewproj;
} m;

// per instance
layout(location = 0) in mat4 model;
layout(location = 4) in uint imageIndex;
layout(location = 5) in uint imageLayer;
layout(location = 6) in uint samplerIndex;
// per vertex
layout(location = 7) in vec3 pos;
layout(location = 8) in vec4 color;
layout(location = 9) in vec2 uv;

layout(location = 0) out flat uint outImageIndex;
layout(location = 1) out flat uint outImageLayer;
layout(location = 2) out flat uint outSamplerIndex;
layout(location = 3) out vec4 outColor;
layout(location = 4) out vec2 outUV;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
  
  //if (gl_InstanceIndex > 0) {
  //  gl_Position = m.viewproj * vec4(pos, 1.0);
  //} else {
    gl_Position = m.viewproj * model * vec4(pos, 1.0);
  //}
  
  outImageIndex = imageIndex;
  outImageLayer = imageLayer;
  outSamplerIndex = samplerIndex;
  outColor = color;
  //outUV = vec3(uv, imageLayer);
  outUV = uv;
}
