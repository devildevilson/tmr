#ifndef BASIC_TRI_H
#define BASIC_TRI_H

#include "Utility.h"

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
  float operator[] (const uint32_t &index) const;
};

#endif //BASIC_TRI_H
