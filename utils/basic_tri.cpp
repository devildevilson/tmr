#include "basic_tri.h"

#include <cstring>

basic_mat4::basic_mat4()
  : arr{0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f} {}
basic_mat4::basic_mat4(const glm::mat4 &mat)
  : arr{mat[0][0], mat[0][1], mat[0][2], mat[0][3],
        mat[1][0], mat[1][1], mat[1][2], mat[1][3],
        mat[2][0], mat[2][1], mat[2][2], mat[2][3],
        mat[3][0], mat[3][1], mat[3][2], mat[3][3]} {}
basic_mat4::basic_mat4(const simd::mat4 &mat)
  : arr{0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f} {
  mat[0].storeu(&arr[0*4]);
  mat[1].storeu(&arr[1*4]);
  mat[2].storeu(&arr[2*4]);
  mat[3].storeu(&arr[3*4]);
}

basic_mat4::basic_mat4(const float arr[16])
  : arr{0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f} {
  memcpy(this->arr, arr, 16*sizeof(float));
}

basic_mat4::basic_mat4(const float &m11, const float &m12, const float &m13, const float &m14,
                       const float &m21, const float &m22, const float &m23, const float &m24,
                       const float &m31, const float &m32, const float &m33, const float &m34,
                       const float &m41, const float &m42, const float &m43, const float &m44)
  : arr{m11, m12, m13, m14,
        m21, m22, m23, m24,
        m31, m32, m33, m34,
        m41, m42, m43, m44} {}

//basic_mat4::basic_mat4(const basic_mat4 &mat)
//  : arr{0.0f, 0.0f, 0.0f, 0.0f,
//        0.0f, 0.0f, 0.0f, 0.0f,
//        0.0f, 0.0f, 0.0f, 0.0f,
//        0.0f, 0.0f, 0.0f, 0.0f} {
//  memcpy(arr, mat.arr, 16*sizeof(float));
//}

void basic_mat4::set(const glm::mat4 &mat) {
  for (uint8_t i = 0; i < 4; ++i) {
    for (uint8_t j = 0; j < 4; ++j) {
      arr[i*4+j] = mat[i][j];
    }
  }
}

void basic_mat4::set(const simd::mat4 &mat) {
  for (uint8_t i = 0; i < 4; ++i) {
    mat[i].storeu(&arr[i*4]);
  }
}

glm::mat4 basic_mat4::get_glm() const {
  return glm::mat4(
    arr[0*4+0], arr[0*4+1], arr[0*4+2], arr[0*4+3],
    arr[1*4+0], arr[1*4+1], arr[1*4+2], arr[1*4+3],
    arr[2*4+0], arr[2*4+1], arr[2*4+2], arr[2*4+3],
    arr[3*4+0], arr[3*4+1], arr[3*4+2], arr[3*4+3]
  );
}

simd::mat4 basic_mat4::get_simd() const {
  return simd::mat4(
    arr[0*4+0], arr[0*4+1], arr[0*4+2], arr[0*4+3],
    arr[1*4+0], arr[1*4+1], arr[1*4+2], arr[1*4+3],
    arr[2*4+0], arr[2*4+1], arr[2*4+2], arr[2*4+3],
    arr[3*4+0], arr[3*4+1], arr[3*4+2], arr[3*4+3]
  );
}

basic_mat4 & basic_mat4::operator=(const glm::mat4 &mat) {
  for (uint8_t i = 0; i < 4; ++i) {
    for (uint8_t j = 0; j < 4; ++j) {
      arr[i*4+j] = mat[i][j];
    }
  }

  return *this;
}

basic_mat4 & basic_mat4::operator=(const simd::mat4 &mat) {
  for (uint8_t i = 0; i < 4; ++i) {
    mat[i].storeu(&arr[i*4]);
  }

  return *this;
}

//basic_mat4 & basic_mat4::operator=(const basic_mat4 &mat) {
//  memcpy(arr, mat.arr, 16*sizeof(float));
//  return *this;
//}

basic_vec4::basic_vec4() : arr{0.0f, 0.0f, 0.0f, 0.0f} {}
basic_vec4::basic_vec4(const glm::vec4 &vec) : arr{vec[0], vec[1], vec[2], vec[3]} {}
basic_vec4::basic_vec4(const simd::vec4 &vec) : arr{0.0f, 0.0f, 0.0f, 0.0f} {
  vec.storeu(arr);
}

basic_vec4::basic_vec4(const float arr[4]) : arr{arr[0], arr[1], arr[2], arr[3]} {}
basic_vec4::basic_vec4(const float &x, const float &y, const float &z, const float &w) : arr{x, y, z, w} {}
//basic_vec4::basic_vec4(const basic_vec4 &vec) : arr{vec.arr[0], vec.arr[1], vec.arr[2], vec.arr[3]} {}

void basic_vec4::set(const glm::vec4 &vec) {
  for (uint8_t i = 0; i < 4; ++i) {
    arr[i] = vec[i];
  }
}

void basic_vec4::set(const simd::vec4 &vec) {
  vec.storeu(arr);
}

glm::vec4 basic_vec4::get_glm() const {
  return {arr[0], arr[1], arr[2], arr[3]};
}

simd::vec4 basic_vec4::get_simd() const {
  return {arr};
}

basic_vec4 & basic_vec4::operator=(const glm::vec4 &vec) {
  for (uint8_t i = 0; i < 4; ++i) {
    arr[i] = vec[i];
  }

  return *this;
}

basic_vec4 & basic_vec4::operator=(const simd::vec4 &vec) {
  vec.storeu(arr);
  return *this;
}

float basic_vec4::operator[] (const uint32_t &index) const {
  return arr[index];
}

//basic_vec4 & basic_vec4::operator=(const basic_vec4 &vec) {
//  memcpy(arr, vec.arr, 4*sizeof(float));
//  return *this;
//}
