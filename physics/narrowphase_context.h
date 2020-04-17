#ifndef NARROWPHASE_CONTEXT_H
#define NARROWPHASE_CONTEXT_H

#include <vector>
#include <unordered_map>
#include <mutex>
#include "manifold.h"
#include "rigid_body.h"
#include "core_context.h"
#include "memory_pool.h"
#include "core_context.h"

#define MANIFOLDS_POOL_SIZE 1000

namespace devils_engine {
  namespace physics {
    namespace core {
      struct context;
    }
    
    namespace broadphase {
      struct octree_context;
    }
    
    namespace narrowphase {
      struct context : public core::context_interface {
        using calculate_combined_friction_f = std::function<core::scalar(const core::rigid_body &, const core::rigid_body &)>;
        using calculate_combined_restitution_f = std::function<core::scalar(const core::rigid_body &, const core::rigid_body &)>;
        using calculate_combined_rolling_friction_f = std::function<core::scalar(const core::rigid_body &, const core::rigid_body &)>;
        using calculate_combined_spinning_friction_f = std::function<core::scalar(const core::rigid_body &, const core::rigid_body &)>;
        using calculate_combined_contact_damping_f = std::function<core::scalar(const core::rigid_body &, const core::rigid_body &)>;
        using calculate_combined_stiffness_f = std::function<core::scalar(const core::rigid_body &, const core::rigid_body &)>;
        
        using compute_contact_breaking_threshold_f = std::function<core::scalar()>;
        
        struct data_callback {
          calculate_combined_friction_f combined_friction_callback;
          calculate_combined_restitution_f combined_restitution_callback;
          calculate_combined_rolling_friction_f combined_rolling_friction_callback;
          calculate_combined_spinning_friction_f combined_spinning_friction_callback;
          calculate_combined_contact_damping_f combined_contact_damping_callback;
          calculate_combined_stiffness_f combined_stiffness_callback;
        };
        
        core::context* core_context;
        broadphase::octree_context* broadphase_context;
        //std::vector<persistent_manifold> manifolds;
        struct manifold_callbacks manifold_callbacks;
        struct data_callback data_callback;
        std::mutex mutex;
        utils::memory_pool<persistent_manifold, sizeof(persistent_manifold)*MANIFOLDS_POOL_SIZE> manifolds_pool;
        std::unordered_map<size_t, persistent_manifold*> manifolds;
        std::vector<persistent_manifold*> manifolds_array;
        std::vector<size_t> batches_count;
        core::scalar default_contact_breaking_treshold;
        
        struct create_info {
          core::context* core_context;
          broadphase::octree_context* broadphase_context;
//           struct manifold_callbacks manifold_callbacks; // почти все функции дефолтные
//           struct data_callback data_callback;
        };
        context(const create_info &info);
        ~context();
        persistent_manifold* add_manifold(const uint32_t &idx0, const uint32_t &idx1, const compute_contact_breaking_threshold_f &contact_breaking_threshold_f, const core::scalar &contact_processing_threshold);
        void remove_manifold(persistent_manifold* m);
      };
    }
  }
}

#endif
