#version 450

layout(set = 0, binding = 0) uniform sampler   texSampler;
layout(set = 1, binding = 0) uniform texture2D tex;

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
  vec4 color = texture(sampler2D(tex, texSampler), fragTexCoord);
  //color.w = color.w * fragColor.w;
  if (color.w < 0.5) discard;
  outColor = color + fragColor;
  //outColor = vec4(fragColor, 1.0);
}
