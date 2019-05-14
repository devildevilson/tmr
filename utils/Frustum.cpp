#include "Frustum.h"

Frustum::Frustum() {}
Frustum::Frustum(const simd::mat4 &matrix) {
  calcFrustum(matrix);
}

// #include <iostream>

// #define PRINT_VEC(name, vec) std::cout << name << "(" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ")\n";

void Frustum::calcFrustum(const simd::mat4 &matrix) {
  const simd::mat4 mat = simd::transpose(matrix);
  
  this->planes[0] = mat[3] + mat[0];
  this->planes[1] = mat[3] - mat[0];
  this->planes[2] = mat[3] - mat[1];
  this->planes[3] = mat[3] + mat[1];
  this->planes[4] = mat[3] + mat[2];
  this->planes[5] = mat[3] - mat[2];

  for (uint32_t i = 0; i < 6; ++i) {
    float arr[4];
    planes[i].storeu(arr);
    const float mag = simd::length(simd::vec4(arr[0], arr[1], arr[2], 0.0f));
    planes[i] /= mag;
  }
}
