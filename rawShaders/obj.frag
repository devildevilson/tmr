#version 450

layout(constant_id = 0) const uint samplersCount = 1;
layout(constant_id = 1) const uint imagesCount = 2;

layout(set = 1, binding = 0) uniform sampler samplers[samplersCount];
layout(set = 2, binding = 0) uniform texture2DArray textures[imagesCount];

layout(location = 0) in flat uint inImageIndex;
layout(location = 1) in flat uint inImageLayer;
layout(location = 2) in flat uint inSamplerIndex;
layout(location = 3) in vec4 inColor;
layout(location = 4) in vec2 inUV;

layout(location = 0) out vec4 outColor;

void main() {
  //uint layer = 3;
  vec4 color = texture(sampler2DArray(textures[inImageIndex], samplers[inSamplerIndex]), vec3(inUV, inImageLayer));
  
  if (color.w < 0.5) discard;
  
  //outColor = color + inColor;
  outColor = color;
}
