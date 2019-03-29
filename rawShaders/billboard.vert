#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (std140, push_constant) uniform PushConsts {
  mat4 mvp;
  vec3 vecX;
  vec3 vecY;
  vec3 pos;
} pushConsts;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragTexCoord;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
  vec3 vertex = inPosition.x * pushConsts.vecX + inPosition.y * pushConsts.vecY + pushConsts.pos;
  gl_Position = pushConsts.mvp * vec4(vertex, 1.0);
  fragColor = inColor;
  fragTexCoord = inTexCoord;
}
