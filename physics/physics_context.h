#ifndef PHYSICS_CONTEXT_H
#define PHYSICS_CONTEXT_H

// получается что придется сделать опять на индексах
// доступ к данным, удобно не копировать все трансформы

#include <vector>
#include <unordered_map>
#include <mutex>
#include "id.h"
#include "array_interface.h"
#include "cpu_array.h"
#include "physics_core.h"
#include "transform.h"
#include "rigid_body.h"
#include "collision_shape.h"
#include "joint_interface.h"
#include "core_context.h"

namespace devils_engine {
  namespace physics {
    namespace broadphase {
      struct octree_context;
    }
    
    namespace core {
      using devils_engine::utils::cpu_array;
      using devils_engine::utils::array_interface;
      using devils_engine::utils::id;
//       using namespace devils_engine;
      
      struct trigger_pair {
        uint32_t obj1;
        uint32_t obj2;
        scalar dist;
        scalar mtv[4];
      };
      
      struct context : public context_interface {
        struct shape_creation_data {
          std::vector<vec4> points;
          std::vector<vec4> faces;
          std::vector<uint32_t> faces_points;
        };
        
        struct gravity_data {
          mat4 orientation;
          vec4 gravity;
          vec4 gravity_norm;
          vec4 gravity_scalars;
          
          void set_gravity(const vec4 &gravity);
          scalar length() const;
          scalar length2() const;
        };
        
        mutable std::mutex mutex;
        
        cpu_array<transform> old_transforms;
        cpu_array<transform> new_transforms;
        array_interface<transform>* transforms; // только трансформы внешние? 
        
        cpu_array<rigid_body> bodies;
        cpu_array<collision::shape> shapes;
        cpu_array<vec4> points;
        
        // еще добавляютя указатели на констраинты 
        std::vector<joint_interface*> joints;
        
        // надо как то разделить лучи на точные и не точные
        // неточные лучи проверяются только с прокси
        // искать один ближайший объект достаточно? для абсолютного большинства случаев да
        cpu_array<collision::ray> precision_rays;
        cpu_array<collision::ray> rays;
        cpu_array<collision::frustum> frustums;
        cpu_array<vec4> frustums_pos;
        cpu_array<collision::ray::test_data> ray_test_data;
        std::atomic<uint32_t> frustum_pairs_count;
        cpu_array<collision::frustum::test_data> frustum_test_data;
        
        std::unordered_map<id, uint32_t> shapes_indices;
        
        struct gravity_data* gravity_data;
        
        std::atomic<uint32_t> pairs_count;
        cpu_array<trigger_pair> trigger_pairs;
        
        std::vector<void*> user_datas;
        
        uint32_t free_body;
        
        broadphase::octree_context* broadphase;
        
        struct create_info {
          vec4 initial_gravity;
          array_interface<transform>* transforms;
        };
        context(const create_info &info);
        ~context();
        
        uint32_t add_body(const rigid_body::create_info &info, void* user_data);
        void remove_body(const uint32_t &id);
        void* get_user_data(const uint32_t &id) const;
        void add_joint(joint_interface* joint);
        void remove_joint(joint_interface* joint);
        uint32_t create_collision_shape(const id &shape_id, const uint32_t &type, const shape_creation_data &info, const scalar &margin = CONVEX_SHAPE_COLLISION_MARGIN);
        uint32_t collision_shape_index(const id &shape_id);
        uint32_t add_precision_ray(const collision::ray &ray);
        uint32_t add_ray(const collision::ray &ray);
        uint32_t add_frustum(const collision::frustum &frustum, const vec4 &pos);
        const collision::ray::test_data & get_precision_ray_intersection(const uint32_t &index) const;
        const collision::ray::test_data & get_ray_intersection(const uint32_t &index) const;
        uint32_t frustum_objects_count() const;
        const collision::frustum::test_data & get_frustum_intersection(const uint32_t &index) const;
        void set_gravity(const vec4 &gravity);
        void begin();
        void clear();
        
        size_t memory() const;
      };
    }
  }
}

#endif
