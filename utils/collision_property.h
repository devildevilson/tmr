#ifndef COLLISION_PROPERTY_H
#define COLLISION_PROPERTY_H

#include <functional>

namespace yacs {
  class entity;
}

namespace devils_engine {
  namespace properties {
    struct collision {
      using func_type = std::function<void(yacs::entity*, yacs::entity*)>;
      const func_type* func;
    };
  }
}

#endif
