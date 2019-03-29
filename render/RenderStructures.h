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
  glm::vec4 pos;
  glm::vec4 color;
  glm::vec2 texCoord;
};

struct LightData {
  glm::vec3 pos;
  float radius;
  glm::vec3 color;
  float cutoff;
};

struct CameraData {
  glm::mat4 viewproj;
  glm::mat4 view;
  glm::vec4 pos;
  glm::vec4 dir;
  uint32_t width;
  uint32_t height;
};

struct MatBuffer {
  glm::mat4 proj;
  glm::mat4 view;
  glm::mat4 invProj;
  glm::mat4 invView;
  glm::mat4 invViewProj;
};

struct Matrices {
  glm::mat4 persp;
  glm::mat4 ortho;
  glm::mat4 view;
  MatBuffer matrixes;
  CameraData camera;
};

#endif
