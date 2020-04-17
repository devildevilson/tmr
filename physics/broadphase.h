#ifndef BROADPHASE_H
#define BROADPHASE_H

#include <vector>
#include "typeless_container.h"
#include "simulation_part.h"
#include "physics_context.h"
#include "phase.h"

namespace devils_engine {
  namespace physics {
    namespace broadphase {
      struct octree_context;
      
//       class group : core::phase<core::context, octree_context> {
//       public:
//         group(const size_t &container_size);
//         ~group();
//         
//         void begin(core::context* ctx) override;
//         void process(core::context* ctx, const size_t &delta_time) override;
//         void clear() override;
//       private:
//         
//       };
    }
  }
}

#endif
