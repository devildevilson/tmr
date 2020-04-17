#ifndef TRANSFORM_COMPONENT_NEW_H
#define TRANSFORM_COMPONENT_NEW_H

#include <cstdint>
#include "Utility.h"
#include "transform.h"
#include "array_interface.h"

namespace devils_engine {
  namespace components {
    struct transform {
      static utils::array_interface<physics::core::transform>* transforms;
      uint32_t index;
      
      simd::vec4 & pos();
      simd::quat & rot();
      simd::vec4 & scale();
      const simd::vec4 & pos() const;
      const simd::quat & rot() const;
      const simd::vec4 & scale() const;
    };
  }
}

#endif
