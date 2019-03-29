#version 450

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

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragTexCoord;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
  //mat4 model = mat4(1.0, 0.0, 0.0, 0.0, /**/ 0.0, 1.0, 0.0, 0.0, /**/ 0.0, 0.0, 1.0, 0.0, /**/ 0.0, 0.0, 0.0, 1.0);
  gl_Position = m.viewproj * pushConsts.model * vec4(inPosition, 1.0);
  //fragColor = inColor;
  fragColor = pushConsts.color;
  fragTexCoord = inTexCoord;
}
