#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 2, binding = 0) uniform Matrix {
	mat4 viewproj;
} m;

layout (std140, push_constant) uniform PushConsts {
  mat4 model;
  vec4 color;
  vec3 normal;
} pushConsts;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec2 inUV;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec2 fragUV;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
  mat4 model = mat4(1.0, 0.0, 0.0, 0.0, /**/ 0.0, 1.0, 0.0, 0.0, /**/ 0.0, 0.0, 1.0, 0.0, /**/ 0.0, 0.0, 0.0, 1.0);
  gl_Position = m.viewproj * pushConsts.model * vec4(inPosition, 1.0);
  fragColor = inColor;
  fragTexCoord = inTexCoord;
  fragUV = inUV;
}
