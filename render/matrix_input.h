#ifndef MATRIX_INPUT_H
#define MATRIX_INPUT_H

#include "simd.h"

namespace devils_engine {
  namespace render {
    class matrix_input {
    public:
      virtual ~matrix_input() {}
      virtual void set_persp(const simd::mat4 &persp);
      virtual void set_ortho(const simd::mat4 &ortho);
    };
  }
}

#endif
