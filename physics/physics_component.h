#ifndef PHYSICS_COMPONENT_NEW_H
#define PHYSICS_COMPONENT_NEW_H

#include "rigid_body.h"

namespace devils_engine {
  namespace components {
    struct physical_body {
      uint32_t index;
      
      physical_body(const physics::core::rigid_body::create_info &info, void* user_data);
      ~physical_body();
      physics::core::rigid_body & get();
      const physics::core::rigid_body & get() const;
      void* user_data() const;
    };
  }
}

#endif
