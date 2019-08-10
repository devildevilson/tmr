#version 450

layout(constant_id = 0) const uint samplersCount = 1;
layout(constant_id = 1) const uint imagesCount = 2;

layout(set = 0, binding = 0) uniform Camera {
  mat4 viewproj;
  mat4 view;
  vec4 pos;
  vec4 dir;
  uvec2 dim;
} camera;

layout(set = 1, binding = 0) uniform sampler samplers[samplersCount];
layout(set = 2, binding = 0) uniform texture2DArray textures[imagesCount];

struct TextureData {
  uint imageArrayIndex;
  uint imageArrayLayer;
  uint samplerIndex;
  //float mirroredU;
  //float mirroredV;
  float movementU;
  float movementV;
};

layout(std430, set = 3, binding = 0) readonly buffer Textures {
  // x - imageIndex, y - imageLayer, z - samplerIndex, w - может быть материалом
  //uvec4 texturesData[];
  TextureData texturesData[];
};

// layout(location = 0) in flat uint inImageIndex;
// layout(location = 1) in flat uint inSamplerIndex;
layout(location = 0) in vec4 inNormal;
layout(location = 1) in vec2 inUV;
layout(location = 2) in flat uint faceIndex;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec2 outNormal;

vec2 packNormal(const vec3 normal) {
  float p = sqrt(normal.z * 8 + 8);
  return vec2(normal.xy / p + 0.5);
}

void main() {
  //const uint index = floatBitsToUint(inNormal.w);
  const uint index = faceIndex;
  const uint imageIndex   = texturesData[index].imageArrayIndex;
  const uint imageLayer   = texturesData[index].imageArrayLayer;
  const uint samplerIndex = texturesData[index].samplerIndex;
  const float movementU   = texturesData[index].movementU;
  const float movementV   = texturesData[index].movementV;
  // const float movementU   = 0.0f;
  // const float movementV   = 0.0f;
  const float mirroredU   = movementU >= 0.0f ? 1.0f : -1.0f;
  const float mirroredV   = movementV >= 0.0f ? 1.0f : -1.0f;
  const vec2 finalUV = vec2(inUV.x * mirroredU + abs(movementU), inUV.y * mirroredV + abs(movementV));
  const vec4 color = texture(sampler2DArray(textures[imageIndex], samplers[samplerIndex]), vec3(finalUV, float(imageLayer)));
  // vec4 color = texture(sampler2DArray(textures[inImageIndex], samplers[inSamplerIndex]), inUV);

  if (color.w < 0.5) discard;

  // также я могу в альфа канал записать отражение
  // так как он мне больше не потребуется
  outColor = color;
  //outColor = vec4(color.xyz, textures[index].w);

  const vec3 N = mat3(camera.view) * normalize(inNormal.xyz);
  outNormal = packNormal(N);
}
