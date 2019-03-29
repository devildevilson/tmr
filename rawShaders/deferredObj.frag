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

layout(location = 0) in flat uint inImageIndex;
layout(location = 1) in flat uint inSamplerIndex;
//layout(location = 2) in flat float inMirroredU;
//layout(location = 3) in flat float inMirroredV;
layout(location = 2) in flat float inMovementU;
layout(location = 3) in flat float inMovementV;
//layout(location = 6) in vec4 inColor;
layout(location = 4) in vec3 inUV;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec2 outNormal;

vec2 packNormal(const vec3 normal) {
  const float p = sqrt(normal.z * 8 + 8);
  return vec2(normal.xy / p + 0.5);
}

void main() {
  // const float inMovementU = 0.0f;
  // const float inMovementV = 0.0f;
  const float mirrorU = inMovementU >= 0.0f ? 1.0f : -1.0f;
  const float mirrorV = inMovementV >= 0.0f ? 1.0f : -1.0f;
  const vec3 finalUV = vec3(inUV.x * mirrorU + abs(inMovementU), inUV.y * mirrorV + abs(inMovementV), inUV.z);
  vec4 color = texture(sampler2DArray(textures[inImageIndex], samplers[inSamplerIndex]), finalUV);

  if (color.w < 0.5f) discard;

  // в альфа канал нужно записать отличие пикселя монстра от пикселя окружения
  // для того чтобы правильно вычислить свет (свет должен равномерно падать на моснстра вне зависимости от положения наблюдателя)

  color.w = 0.9f;
  outColor = color;
  const vec3 norm = mat3(camera.view) * camera.dir.xyz;
  outNormal = packNormal(-norm);
}
