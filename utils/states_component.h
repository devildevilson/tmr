#ifndef STATES_COMPONENT_H
#define STATES_COMPONENT_H

#include <cstddef>
#include "state.h"
#include "Engine.h"

namespace yacs {
  class entity;
}

namespace dt {
  class thread_pool;
}

namespace devils_engine {
  namespace components {
    struct states {
      const core::state_t* current;
      size_t current_time;
      size_t accumulated_time;
      yacs::entity* ent;
      
      uint32_t counter;
      
      states();
      states(yacs::entity* ent, const core::state_t* current);
      void set(const core::state_t* state);
      void update(const size_t &time);
    };
  }
  
//   namespace systems {
//     class states : public Engine {
//     public:
//       struct create_info {
//         dt::thread_pool* pool;
//       };
//       states(const create_info &info);
//       void update(const size_t &time) override;
//     private:
//       dt::thread_pool* pool;
//     };
//   }
}

// добавится ли здесь что то еще? впринципе здесь есть все что нужно

#endif
