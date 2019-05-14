#version 450

in gl_PerVertex
{
  vec4 gl_Position;
  //float gl_PointSize;
  //float gl_ClipDistance[];
} gl_in[];

out gl_PerVertex
{
  vec4 gl_Position;
  //float gl_PointSize;
  //float gl_ClipDistance[];
};

layout(set = 0, binding = 0) uniform Camera {
  mat4 viewproj;
  mat4 view;
  vec4 pos;
  vec4 dir;
  uint width;
  uint height;
} camera;

layout(set = 1, binding = 0) uniform Uniform {
  vec4 gravity;
  vec4 planes[6];
  uvec4 data
} u;

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

layout(location = 0) in vec4 vel[];
layout(location = 1) in vec4 col[];
layout(location = 2) in uvec4 uData[];
layout(location = 3) in vec4 fData[];
layout(location = 4) in uvec4 texture[];

layout(location = 0) out vec2 texCoord;
layout(location = 1) out vec4 outCol;
layout(location = 2) out uvec4 outTexture;

bool isGravityAffect(const uint type);
bool changingSizeFromTime(const uint type);
bool deleteParticleWithMinVelocity(const uint type);
bool stopParticleWithMinVelocity(const uint type);

#define OUTSIDE 0
#define INSIDE 1
#define INTERSECT 2
uint testFrustumAABB(const vec4 planes[6], const vec4 pos, const vec4 extents);

void main() {
  const uint particleType = uData[0].z;
  const bool gravityAffect = isGravityAffect(particleType);
  const bool sizeChangingTime = changingSizeFromTime(particleType);
  const bool minVelDeleting = deleteParticleWithMinVelocity(particleType);
  const bool minVelStopping = stopParticleWithMinVelocity(particleType);

  const float timeDiv = float(uData[0].x) / float(uData[0].y);
  const float finalCoef = 1.0f - timeDiv;
  const float radius = sizeChangingTime ? abs(mix(fData[0].x, fData[0].y, finalCoef)) : fData[0].x;

  const vec4 pos = gl_in[0].gl_Position;
  const float finalRadius = radius; //  / pos.w

  // тут нужно проверить точку, попадает ли она в пирамиду видимости
  const uint res = testFrustumAABB(u.planes, pos, vec4(finalRadius, finalRadius, finalRadius, 0.0f));
  if (res == OUTSIDE) return;

  const vec4 up = vec4(0.0f, 1.0f, 0.0f, 0.0f);
  const vec4 right = vec4(1.0f, 0.0f, 0.0f, 0.0f);

  // как вытянуть объект по направлению движения??
  // понятно как, нужно сделать матрицу поворота

  const vec4 projPos = camera.viewproj * pos;
  const vec4 projRadius = finalRadius / pos.w;

  //gl_Position = camera.viewproj * (pos + up * finalRadius - right * finalRadius);
  gl_Position = projPos + up * projRadius - right * projRadius;
  texCoord = vec2(0.0f, 0.0f);
  outCol = col[0];
  outTexture = texture[0];
  EmitVertex();

  //gl_Position = camera.viewproj * (pos + up * finalRadius + right * finalRadius);
  gl_Position = projPos + up * projRadius + right * projRadius;
  texCoord = vec2(0.0f, 1.0f);
  outCol = col[0];
  outTexture = texture[0];
  EmitVertex();

  //gl_Position = camera.viewproj * (pos - up * finalRadius - right * finalRadius);
  gl_Position = projPos - up * projRadius - right * projRadius;
  texCoord = vec2(1.0f, 0.0f);
  outCol = col[0];
  outTexture = texture[0];
  EmitVertex();

  //gl_Position = camera.viewproj * (pos - up * finalRadius + right * finalRadius);
  gl_Position = projPos - up * projRadius + right * projRadius;
  texCoord = vec2(1.0f, 1.0f);
  outCol = col[0];
  outTexture = texture[0];
  EmitVertex();

  EndPrimitive();
}

#define GRAVITY_AFFECT (1<<0)
#define CHANGING_SIZE_FROM_TIME (1<<1)
#define DELETE_PARTICLE_MIN_VEL (1<<2)
#define STOP_PARTICLE_MIN_VEL (1<<3)

bool isGravityAffect(const uint type) {
  return (type & GRAVITY_AFFECT) == GRAVITY_AFFECT;
}

bool changingSizeFromTime(const uint type) {
  return (type & CHANGING_SIZE_FROM_TIME) == CHANGING_SIZE_FROM_TIME;
}

bool deleteParticleWithMinVelocity(const uint type) {
  return (type & DELETE_PARTICLE_MIN_VEL) == DELETE_PARTICLE_MIN_VEL;
}

bool stopParticleWithMinVelocity(const uint type) {
  return (type & STOP_PARTICLE_MIN_VEL) == STOP_PARTICLE_MIN_VEL;
}

uint testFrustumAABB(const vec4 planes[6], const vec4 center, const vec4 extents) {
  uint result = INSIDE;

  for (uint i = 0; i < 6; ++i) {
    const vec4 frustumPlane = vec4(planes[i].xyz, 0.0f);
    const float dist = planes[i].w;

    const float d = dot(center,     frustumPlane);
    const float r = dot(extent, abs(frustumPlane));

    const float d_p_r = d + r;
    const float d_m_r = d - r;

    if (d_p_r < -dist) {
      result = OUTSIDE;
      break;
    } else if (d_m_r < -dist) result = INTERSECT;
  }

  return result;
}
