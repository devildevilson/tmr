#version 450

#extension GL_GOOGLE_include_directive : enable

#include "../render/shared_structures.h"

#define FAR_CLIP_PLANE 256.0f
#define EPSILON 0.00001f

const vec4 circle[] = {
  vec4(      0.0f,     -1.0f, 0.0f, 2.0f) / 2.0f,
  vec4(-0.707107f,-0.707107f, 0.0f, 2.0f) / 2.0f,
  vec4( 0.707107f,-0.707107f, 0.0f, 2.0f) / 2.0f,
  vec4(     -1.0f,      0.0f, 0.0f, 2.0f) / 2.0f,
  vec4(      1.0f,      0.0f, 0.0f, 2.0f) / 2.0f,
  vec4(-0.707107f, 0.707107f, 0.0f, 2.0f) / 2.0f,
  vec4( 0.707107f, 0.707107f, 0.0f, 2.0f) / 2.0f,
  vec4(      0.0f,      1.0f, 0.0f, 2.0f) / 2.0f
};

const vec2 circle_uv[] = {
  vec2(     0.5f,      0.0f),
  vec2(0.146447f, 0.146447f),
  vec2(0.853553f, 0.146447f),
  vec2(     0.0f,      0.5f),
  vec2(     1.0f,      0.5f),
  vec2(0.146447f, 0.853553f),
  vec2(0.853553f, 0.853553f),
  vec2(     0.5f,      1.0f)
};

const vec4 box2d[] = {
  vec4( 0.0f,  0.0f, 0.0f, 1.0f),
  vec4(-0.5f, -0.5f, 0.0f, 1.0f),
  vec4( 0.5f, -0.5f, 0.0f, 1.0f),
  vec4(-0.5f,  0.5f, 0.0f, 1.0f),
  vec4( 0.5f,  0.5f, 0.0f, 1.0f)
};

layout(points) in;
layout(triangle_strip, max_vertices = 8) out;

in gl_PerVertex {
    vec4  gl_Position;
    // float gl_PointSize;
    // float gl_ClipDistance[];
} gl_in[];

layout(set = 0, binding = 0) uniform Camera {
  mat4 viewproj;
  mat4 view;
  vec4 pos;
  vec4 dir;
  uvec2 dim;
} camera;

layout(std140, set = 2, binding = 0) readonly buffer particles_buffer {
  gpu_particle particles[];
};

struct gpu_particles_data {
  uvec4 int_data;
  uvec4 out_data;
  vec4 gravity;
  vec4 frustum[6];
};

layout(std140, set = 2, binding = 2) readonly buffer particles_data_buffer {
  gpu_particles_data particles_data;
};

layout(location = 0) in flat uint particle_index[];

layout(location = 0) out flat image out_img;
layout(location = 1) out vec4 out_color;
layout(location = 2) out vec2 out_tex_coords;

#define OUTSIDE 0
#define INSIDE 1
#define INTERSECT 2
uint frustum_test(const vec4 point, const float scale);
mat4 translate(const mat4 mat, const vec4 vec);
mat4 rotate(const mat4 mat, const float angle, const vec4 normal);
mat4 scale(const mat4 mat, const vec4 vec);
vec4 right_vec(const vec4 dir);
vec4 up_vec(const vec4 dir, const vec4 right);
mat4 get_rotation(const bool scaling_along_vel);
mat4 get_mat(const mat4 originalMatrix);
mat4 lookAt(const vec4 eye, const vec4 center, const vec4 up);

void main() {
  const gpu_particle p = particles[particle_index[0]];
  const image img = get_particle_image(p);
  const color col = get_particle_color(p);
  const vec4 final_color = vec4(get_color_r(col), get_color_g(col), get_color_b(col), get_color_a(col));
  const uint lt = get_particle_life_time(p);
  const uint ct = get_particle_current_time(p);
  const float time_ratio = lt == 0 ? 1.0f : float(ct) / float(lt);

  const float max_scale = get_particle_max_scale(p);
  const float min_scale = get_particle_min_scale(p);

  const bool scale_dec_over_time = particle_scale_dec_over_time(p);
  const bool scale_inc_over_time = particle_scale_inc_over_time(p);

  const bool scaling_along_vel = particle_scaling_along_vel(p);

  const float speed = length(p.vel);
  const float scale_var = scale_dec_over_time ? abs(mix(max_scale, min_scale, time_ratio)) : (scale_inc_over_time ? abs(mix(min_scale, max_scale, time_ratio)) : max_scale);

  const float final_scale = scaling_along_vel ? scale_var * max(speed, 1.0f) : scale_var;
  const mat4 trans = translate(mat4(1.0f), gl_in[0].gl_Position);
  const mat4 rot = trans * get_rotation(scaling_along_vel);
  const mat4 scaled_mat = scale(rot, vec4(scale_var, final_scale, scale_var, 0.0f));

  for (uint i = 0; i < 8; ++i) {
    gl_Position = camera.viewproj * scaled_mat * circle[i];
    out_img = img;
    out_color = final_color;
    out_tex_coords = circle_uv[i];
    EmitVertex();
  }

  EndPrimitive();
}

uint frustum_test(const vec4 point, const float scale) {
  uint result = INSIDE;
  const vec4 extents = vec4(1.0f, 1.0f, 1.0f, 0.0f) * scale / 2.0f;

  for (uint i = 0; i < 6; ++i) {
    const vec4 frustumPlane = vec4(particles_data.frustum[i].xyz, 0.0f);
    const float dist = particles_data.frustum[i].w;

    const float d = dot(point ,      frustumPlane);
    const float r = dot(extents, abs(frustumPlane));

    const float d_p_r = d + r;
    const float d_m_r = d - r;

    if (d_p_r < -dist) {
      result = OUTSIDE;
      break;
    } else if (d_m_r < -dist) result = INTERSECT;
  }

  return result;
}

mat4 translate(const mat4 mat, const vec4 vec) {
  mat4 ret = mat;
  ret[3] = mat[0] * vec[0] + mat[1] * vec[1] + mat[2] * vec[2] + mat[3];
  return ret;
}

mat4 rotate(const mat4 mat, const float angle, const vec4 normal) {
  const float c = cos(angle);
  const float s = sin(angle);

  const vec4 temp = normalize(normal) * (1.0f - c);

  mat3 rot;

  rot[0][0] = c + temp[0] * normal[0];
  rot[0][1] = temp[0] * normal[1] + s * normal[2];
  rot[0][2] = temp[0] * normal[2] - s * normal[1];

  rot[1][0] = temp[1] * normal[0] - s * normal[2];
  rot[1][1] = c + temp[1] * normal[1];
  rot[1][2] = temp[1] * normal[2] + s * normal[0];

  rot[2][0] = temp[2] * normal[0] + s * normal[1];
  rot[2][1] = temp[2] * normal[1] - s * normal[0];
  rot[2][2] = c + temp[2] * normal[2];

  mat4 result;
  result[0] = mat[0] * rot[0][0] + mat[1] * rot[0][1] + mat[2] * rot[0][2];
  result[1] = mat[0] * rot[1][0] + mat[1] * rot[1][1] + mat[2] * rot[1][2];
  result[2] = mat[0] * rot[2][0] + mat[1] * rot[2][1] + mat[2] * rot[2][2];
  result[3] = mat[3];
  return result;
}

mat4 scale(const mat4 mat, const vec4 vec) {
  mat4 ret = mat;
  ret[0] *= vec[0];
  ret[1] *= vec[1];
  ret[2] *= vec[2];

  return ret;
}

vec4 right_vec(const vec4 dir) {
  const vec4 up = vec4(0.0f, 1.0f, 0.0f, 0.0f);
  return normalize(vec4(cross(vec3(up), vec3(dir)), 0.0f));
}

vec4 up_vec(const vec4 dir, const vec4 right) {
  return normalize(vec4(cross(vec3(right), vec3(dir)), 0.0f));
}

mat4 get_rotation(const bool scaling_along_vel) {
  const float vel_dot = dot(particles[particle_index[0]].vel, particles[particle_index[0]].vel);
  const vec4 z = -camera.dir;
  const vec4 y = scaling_along_vel && vel_dot > EPSILON ? particles[particle_index[0]].vel / sqrt(vel_dot) : vec4(0.0f, 1.0f, 0.0f, 0.0f);
  const vec4 x = normalize(vec4(cross(vec3(z), vec3(y)), 0.0f));
  const vec4 y2 = normalize(vec4(cross(vec3(z), vec3(x)), 0.0f));
  return mat4(
    x,
    y2,
    z,
    vec4(0.0f, 0.0f, 0.0f, 1.0f)
  );
}

mat4 get_mat(const mat4 originalMatrix) {
  mat4 result;
  result[0] = vec4(length(originalMatrix[0].xyz), .0, .0, originalMatrix[0].w);
  result[1] = vec4(.0, length(originalMatrix[1].xyz), .0, originalMatrix[1].w);
  result[2] = vec4(.0, .0, length(originalMatrix[2].xyz), originalMatrix[2].w);
  result[3] = originalMatrix[3];
  return result;
}

vec4 cross(const vec4 a, const vec4 b) {
  return vec4(cross(vec3(a), vec3(b)), 0.0f);
}

mat4 lookAt(const vec4 eye, const vec4 center, const vec4 up) {
  const vec4 ftmp = normalize(center - eye);
  const vec4 stmp = normalize(cross(ftmp, up));
  const vec4 utmp = cross(stmp, ftmp);

  const vec4 f = -ftmp + vec4(0.0f, 0.0f, 0.0f,  dot(ftmp, eye));
  const vec4 s =  stmp + vec4(0.0f, 0.0f, 0.0f, -dot(stmp, eye));
  const vec4 u =  utmp + vec4(0.0f, 0.0f, 0.0f, -dot(utmp, eye));
  const vec4 lastColumn = vec4(0.0f, 0.0f, 0.0f, 1.0f);

  const mat4 res = mat4(
    s,
    u,
    f,
    lastColumn
  );
  return transpose(res);
}
