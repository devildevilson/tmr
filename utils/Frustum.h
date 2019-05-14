#ifndef FRUSTUM_H
#define FRUSTUM_H

#include "Utility.h"

struct Frustum {
  simd::vec4 planes[6];

  Frustum();
  Frustum(const simd::mat4 &matrix);

  void calcFrustum(const simd::mat4 &matrix);
};

#endif
