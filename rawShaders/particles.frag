#version 450

layout(constant_id = 0) const uint samplersCount = 1;
layout(constant_id = 1) const uint imagesCount = 2;

layout(set = 2, binding = 0) uniform sampler samplers[samplersCount];
layout(set = 3, binding = 0) uniform texture2DArray textures[imagesCount];

layout(location = 0) in vec2 texCoord;
layout(location = 1) in vec4 col;
layout(location = 2) flat in uvec4 inTexture;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec2 outNormal;

vec2 packNormal(const vec3 normal) {
  float p = sqrt(normal.z * 8 + 8);
  return vec2(normal.xy / p + 0.5);
}

void main() {
  const uint imageIndex = inTexture.x;
  const uint samplerIndex = inTexture.z;
  const vec3 finalUV = vec3(texCoord.x, texCoord.y, float(inTexture.y));
  vec4 color = texture(sampler2DArray(textures[imageIndex], samplers[samplerIndex]), finalUV);

  if (color.w < 0.5f) discard;

  color.xyz += col.xyz;
  color.w = 0.9f;
  outColor = color;
  const vec3 norm = vec3(0.0f, 0.0f, 0.0f);
  outNormal = packNormal(-norm);
}
