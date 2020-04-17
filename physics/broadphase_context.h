#ifndef BROADPHASE_CONTEXT_H
#define BROADPHASE_CONTEXT_H

#include <cstdint>
#include <vector>
#include <mutex>
#include <atomic>

#include "basic_tri.h"
#include "cpu_array.h"
#include "core_context.h"

// должен быть доступ к лучам и фрустумам
// по идее эту информацию мы получаем из основного контекста

namespace devils_engine {
  namespace physics {
    namespace core {
      struct context;
    }
    
    namespace broadphase {
      struct octree_context : public core::context_interface {
        struct aabb {
          basic_vec4 center;
          basic_vec4 extents;
        };
        
        struct node {
          std::mutex mutex;
          aabb box;
          uint32_t free_index;
          uint32_t parent_index;
          uint32_t next_level_index;
          std::vector<uint32_t> proxy_indices;
          
          node();
          uint32_t add_proxy(const uint32_t &index);
          void remove_proxy(const uint32_t &array_index);
        };
        
        struct proxy {
          struct flags {
            enum {
              STATIC_BODY     = (1 << 0),
              NEED_UPDATION   = (1 << 1),
              SLEEPING_OBJECT = (1 << 2),
            };
            
            uint32_t container;
            
            flags();
            flags(const uint32_t &flags);
            
            bool static_object() const;
            bool need_updation() const;
            bool is_sleeping() const; // какие объекты спящие? скорость < константы (гравитация?)
            
            void static_object(const bool value);
            void need_updation(const bool value);
            void sleeping(const bool value);
          };
          
          aabb box;
          uint32_t object_index;
          uint32_t collision_group;
          uint32_t collision_filter;
          uint32_t collision_trigger;
          
          uint32_t node_index;
          uint32_t node_array_index;
          uint32_t obj_type;
          flags proxy_flags;
          
          proxy();
          proxy(const uint32_t &object_index, const uint32_t &collision_group, const uint32_t &collision_filter, const uint32_t &collision_trigger, const uint32_t &obj_type);
        };
        
        struct pair {
          struct flags {
            enum {
              STATIC_PAIR       = (1 << 0),
              ONLY_TRIGGER_PAIR = (1 << 1),
              NOT_UNIQUE_PAIR   = (1 << 2)
            };
            
            uint32_t container;
            
            flags();
            flags(const uint32_t &flags);
            
            bool static_pair() const;
            bool trigger_only() const;
            bool not_unique() const;
            
            void static_pair(const bool value);
            void trigger_only(const bool value);
            void not_unique(const bool value);
          };
          
          uint32_t obj1;
          uint32_t obj2;
          float dist2;
          flags pair_flags;
        };
        
        core::context* context;
        utils::cpu_array<proxy> proxies;
        std::vector<node> nodes;
        utils::cpu_array<pair> pairs;
        uint32_t free_index;
        uint32_t pairs_count;
        std::mutex mutex;
        
        struct create_info {
          basic_vec4 center;
          basic_vec4 extents;
          uint32_t depth;
          core::context* context;
        };
        octree_context(const create_info &info);
        void recreate(const create_info &info);
        
        uint32_t create_proxy(const uint32_t &object_index, const uint32_t &collision_group, const uint32_t &collision_filter, const uint32_t &collision_trigger, const uint32_t &obj_type);
        void destroy_proxy(const uint32_t &proxy_index);
      };
    }
  }
}

#endif
