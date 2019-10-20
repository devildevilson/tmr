#ifndef RENDER_STRUCTURES_H
#define RENDER_STRUCTURES_H

#ifdef __cplusplus
#include <cstdint>
#include "basic_tri.h"

#define DEFAULT_DESCRIPTOR_POOL_NAME "default_descriptor_pool"
#define UNIFORM_BUFFER_LAYOUT_NAME "uniform_layout"
#define MATRICES_BUFFER_LAYOUT_NAME "matrixes_layout"
#define STORAGE_BUFFER_LAYOUT_NAME "storage_layout"
#define SAMPLED_IMAGE_LAYOUT_NAME "sampled_image_layout"

typedef uint32_t uint;
typedef basic_mat4 mat4;
typedef basic_vec4 vec4;
typedef glm::vec2 vec2;
typedef glm::vec3 vec3;
#else

#endif

// в интерфейсе нужно будет сразу взять несколько структур с текстурами
// и мы можем передавать указатели на них nuklear
// сэмплер хоть и не относится к картинке, но всюду ее сопровождает
// что мне вообще в текстуре потребуется?
// изображение, сэмлер, корректировка uv (например для перемещения текстуры), ???
// текстура будет составляться на месте, например, в компоненте

struct Image {
  uint index;
  uint layer;
};

struct Texture {
  Image image;
  uint samplerIndex;
  float movementU;
  float movementV;
};

//struct Texture {
//  uint imageArrayIndex;
//  uint imageArrayLayer;
//  uint samplerIndex;
//};
//
//struct TextureData {
//  Texture t;
//  //uint32_t sampler;
//  float movementU;
//  float movementV;
//};

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
