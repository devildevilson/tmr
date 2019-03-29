#version 450

out gl_PerVertex {
  vec4 gl_Position;
};

const vec2 pos[4] = {vec2(0.0, 0.0), vec2(1.0, 0.0), vec2(1.0, 1.0), vec2(0.0, 1.0)};

layout (location = 0) out vec2 outUV;

void main() {
  outUV = pos[gl_VertexIndex];
  gl_Position = vec4(pos[gl_VertexIndex], 0.0, 1.0);
  //outUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
  //gl_Position = vec4(outUV * 2.0f + -1.0f, 0.0f, 1.0f);
}
