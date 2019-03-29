#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform sampler   texSampler;
layout(set = 1, binding = 0) uniform texture2D tex;

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

void main() {
  //vec4 color = texture(sampler2D(tex, texSampler), fragTexCoord);
  vec4 color = texture(sampler2D(tex, texSampler), fragUV);
  //color.w = color.w * fragColor.w;
  if (color.w < 0.5) discard;
  outColor = color;
  //outColor = vec4(fragColor, 1.0);
}
