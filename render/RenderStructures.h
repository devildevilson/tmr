#ifndef RENDER_STRUCTURES_H
#define RENDER_STRUCTURES_H

#ifdef __cplusplus
#include <cstdint>
#include <cstring>
#include "Utility.h"

#define DEFAULT_DESCRIPTOR_POOL_NAME "default_descriptor_pool"
#define UNIFORM_BUFFER_LAYOUT_NAME "uniform_layout"
#define MATRICES_BUFFER_LAYOUT_NAME "matrixes_layout"
#define STORAGE_BUFFER_LAYOUT_NAME "storage_layout"
#define SAMPLED_IMAGE_LAYOUT_NAME "sampled_image_layout"

struct basic_mat4 {
  float arr[16];

  basic_mat4();
  basic_mat4(const glm::mat4 &mat);
  basic_mat4(const simd::mat4 &mat);
  basic_mat4(const float arr[16]);
  basic_mat4(const float &m11, const float &m12, const float &m13, const float &m14,
             const float &m21, const float &m22, const float &m23, const float &m24,
             const float &m31, const float &m32, const float &m33, const float &m34,
             const float &m41, const float &m42, const float &m43, const float &m44);
//  basic_mat4(const basic_mat4 &mat);

  void set(const glm::mat4 &mat);
  void set(const simd::mat4 &mat);
  glm::mat4 get_glm() const;
  simd::mat4 get_simd() const;

  basic_mat4 & operator=(const glm::mat4 &mat);
  basic_mat4 & operator=(const simd::mat4 &mat);
//  basic_mat4 & operator=(const basic_mat4 &mat);
};

struct basic_vec4 {
  float arr[4];

  basic_vec4();
  basic_vec4(const glm::vec4 &vec);
  basic_vec4(const simd::vec4 &vec);
  basic_vec4(const float arr[4]);
  basic_vec4(const float &x, const float &y, const float &z, const float &w);
//  basic_vec4(const basic_vec4 &vec);

  void set(const glm::vec4 &vec);
  void set(const simd::vec4 &vec);
  glm::vec4 get_glm() const;
  simd::vec4 get_simd() const;

  basic_vec4 & operator=(const glm::vec4 &vec);
  basic_vec4 & operator=(const simd::vec4 &vec);
//  basic_vec4 & operator=(const basic_vec4 &vec);
};

typedef uint32_t uint;
typedef basic_mat4 mat4;
typedef basic_vec4 vec4;
typedef glm::vec2 vec2;
typedef glm::vec3 vec3;
#else

#endif

struct Texture {
  uint imageArrayIndex;
  uint imageArrayLayer;
  uint samplerIndex;
};

struct TextureData {
  Texture t;
  //uint32_t sampler;
  float movementU;
  float movementV;
};

struct Vertex {
  vec4 pos;
  vec4 color;
  vec2 texCoord;
};

struct LightData {
//  vec3 pos;
//  float radius;
//  vec3 color;
//  float cutoff;
  vec4 pos;
  vec4 color;
};

struct CameraData {
  mat4 viewproj;
  mat4 view;
  vec4 pos;
  vec4 dir;
  uint width;
  uint height;
};

struct MatBuffer {
  mat4 proj;
  mat4 view;
  mat4 invProj;
  mat4 invView;
  mat4 invViewProj;
};

struct Matrices {
  mat4 persp;
  mat4 ortho;
  mat4 view;
  MatBuffer matrixes;
  CameraData camera;
};

#endif
