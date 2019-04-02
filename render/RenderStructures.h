#ifndef RENDER_STRUCTURES_H
#define RENDER_STRUCTURES_H

#include <cstdint>
#include "Utility.h"

//#include <glm/glm.hpp>

#define DEFAULT_DESCRIPTOR_POOL_NAME "default_descriptor_pool"
#define UNIFORM_BUFFER_LAYOUT_NAME "uniform_layout"
#define MATRICES_BUFFER_LAYOUT_NAME "matrixes_layout"
#define STORAGE_BUFFER_LAYOUT_NAME "storage_layout"
#define SAMPLED_IMAGE_LAYOUT_NAME "sampled_image_layout"

struct Texture {
  uint32_t imageArrayIndex;
  uint32_t imageArrayLayer;
  uint32_t samplerIndex;
};

struct TextureData {
  Texture t;
  //uint32_t sampler;
  float movementU;
  float movementV;
};

struct Vertex {
  simd::vec4 pos;
  simd::vec4 color;
  glm::vec2 texCoord;
};

struct LightData {
  glm::vec3 pos;
  float radius;
  glm::vec3 color;
  float cutoff;
};

struct CameraData {
  simd::mat4 viewproj;
  simd::mat4 view;
  simd::vec4 pos;
  simd::vec4 dir;
  uint32_t width;
  uint32_t height;
};

struct MatBuffer {
  simd::mat4 proj;
  simd::mat4 view;
  simd::mat4 invProj;
  simd::mat4 invView;
  simd::mat4 invViewProj;
};

struct Matrices {
  simd::mat4 persp;
  simd::mat4 ortho;
  simd::mat4 view;
  MatBuffer matrixes;
  CameraData camera;
};

#endif
