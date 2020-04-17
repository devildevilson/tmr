#ifndef MANIFOLD_H
#define MANIFOLD_H

#include "physics_core.h"
#include "transform.h"
#include <functional>

#define MANIFOLD_CACHE_SIZE 4

namespace devils_engine {
  namespace physics {
    namespace narrowphase {
      using core::vec4;
      using core::scalar;
      
      struct persistent_manifold;
      struct manifold_point;
      using contact_destroyed_callback = std::function<void(void*)>;
      using contact_processed_callback = std::function<void(manifold_point*, const uint32_t &, const uint32_t &)>;
      using contact_started_callback = std::function<void(const persistent_manifold*)>;
      using contact_ended_callback = std::function<void(const persistent_manifold*)>;
      
      struct manifold_callbacks {
        contact_destroyed_callback destroyed;
        contact_processed_callback processed;
        contact_started_callback started;
        contact_ended_callback ended;
      };
      
      struct manifold_point {
        struct flags {
          enum {
            LATERAL_FRICTION_INITIALIZED = (1 << 0),
            HAS_CONTACT_CFM              = (1 << 1),
            HAS_CONTACT_ERP              = (1 << 2),
            CONTACT_STIFFNESS_DAMPING    = (1 << 3),
            FRICTION_ANCHOR              = (1 << 4)
          };
          
          uint32_t container;
          
          flags();
          flags(const uint32_t &flags);
          
          bool lateral_friction_initialized() const;
          bool has_contact_cfm() const;
          bool has_contact_erp() const;
          bool contact_stiffness_damping() const;
          bool friction_anchor() const;
          
          void lateral_friction_initialized(const bool value);
          void has_contact_cfm(const bool value);
          void has_contact_erp(const bool value);
          void contact_stiffness_damping(const bool value);
          void friction_anchor(const bool value);
        };
        
        vec4 local_point_a;
        vec4 local_point_b;
        vec4 world_position_b;
        // по идее это можно вычислить
        vec4 world_position_a;
        vec4 world_normal_b;
        vec4 lateral_friction_dir1;
        vec4 lateral_friction_dir2;

        scalar distance1;
        scalar combined_friction;
        scalar combined_rolling_friction;   // torsional friction orthogonal to contact normal, useful to make spheres stop rolling forever
        scalar combined_spinning_friction;  // torsional friction around contact normal, useful for grasping objects
        scalar combined_restitution;
        
        scalar applied_impulse;
        scalar applied_impulse_lateral1;
        scalar applied_impulse_lateral2;
        scalar contact_motion1;
        scalar contact_motion2;
        
        union {
          scalar contact_cfm;
          scalar combined_contact_stiffness1;
        };

        union {
          scalar contact_erp;
          scalar combined_contact_damping1;
        };

        scalar friction_cfm;
        
        // возможно пригодится в будущем
        uint32_t part_id0;
        uint32_t part_id1;
        uint32_t index0;
        uint32_t index1;

        // bool m_lateralFrictionInitialized;
        flags contact_point_flags;

        uint32_t life_time; // время жизни в кадрах (или лучше микросекунды)
        
        mutable void* user_data;
        
        manifold_point();
        manifold_point(const vec4 &point_a, const vec4 &point_b, const vec4 &normal, const scalar &dist);
        vec4 world_pos_b() const;
      };
      
      struct persistent_manifold {
        static struct manifold_callbacks* callbacks;
        
        manifold_point point_cache[MANIFOLD_CACHE_SIZE];
        uint32_t points_count;
        
        uint32_t body0;
        uint32_t body1;
        uint32_t batch_id;
        
        scalar contact_breaking_threshold;
        scalar contact_processing_threshold;
        
        persistent_manifold();
        persistent_manifold(const uint32_t &body0, const uint32_t &body1, const scalar &contact_breaking_threshold, const scalar &contact_processing_threshold);
        uint32_t sort_points(const manifold_point &p);
        uint32_t get_nearest_point(const manifold_point &p);
        uint32_t add_manifold_point(const manifold_point &p, const bool is_predictive = false);
        void remove_contact_point(const uint32_t &index);
        void replace_contact_point(const manifold_point &p, const uint32_t &index);
        bool valid_distance(const manifold_point &p);
        void refresh(const core::transform &transform_a, const core::transform &transform_b);
        void clear();
        void destroy_callback(const uint32_t &index);
      };
    }
  }
}

#endif

