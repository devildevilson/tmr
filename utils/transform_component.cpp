#include "transform_component.h"

namespace devils_engine {
  namespace components {
    utils::array_interface<physics::core::transform>* transform::transforms = nullptr;
    simd::vec4 & transform::pos() { return transforms->at(index).pos; }
    simd::quat & transform::rot() { return transforms->at(index).rot; }
    simd::vec4 & transform::scale() { return transforms->at(index).scale; }
    const simd::vec4 & transform::pos() const { return transforms->at(index).pos; }
    const simd::quat & transform::rot() const { return transforms->at(index).rot; }
    const simd::vec4 & transform::scale() const { return transforms->at(index).scale; }
  }
}
