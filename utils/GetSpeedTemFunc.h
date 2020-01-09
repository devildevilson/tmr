#ifndef GET_SPEED_TEMP_FUNC_H
#define GET_SPEED_TEMP_FUNC_H

#include <functional>

namespace yacs {
  class entity;
}

struct GetSpeedFunc {
  std::function<float(yacs::entity*)> func;
};

#endif
