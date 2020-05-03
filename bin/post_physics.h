#ifndef POST_PHYSICS_H
#define POST_PHYSICS_H

#include <cstddef>

namespace yacs {
  class entity;
}

namespace dt {
  class thread_pool;
}

namespace devils_engine {
  namespace post_physics {
    void update(dt::thread_pool* pool); // , yacs::entity* player
  }
}

#endif
