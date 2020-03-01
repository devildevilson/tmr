#ifndef COLLISION_INTERACTION_SYSTEM_H
#define COLLISION_INTERACTION_SYSTEM_H

#include <cstddef>
#include <functional>
#include "id.h"

namespace dt {
  class thread_pool;
}

namespace yacs {
  class entity;
}

namespace devils_engine {
  namespace systems {
    class collision_interaction {
    public:
      struct create_info {
        dt::thread_pool* pool;
      };
      collision_interaction(const create_info &info);
      void update(const size_t &time);
    private:
      dt::thread_pool* pool;
    };
  }
}

#endif
