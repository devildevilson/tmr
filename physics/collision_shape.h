#ifndef COLLISION_SHAPE_H
#define COLLISION_SHAPE_H

#include "physics_core.h"
#include "transform.h"

// делать ли форму чисто виртуальным классом
// так и сяк скорее всего придется как то дополнять класс
// для формы мне нужны: точки, плоскости, чтобы к плоскости соотносилась хотя бы одна точка
// это позволит как я понял сделать аж: бокс, сфера, полигон, конвекс хал
// более сложные поди требуют еще ребер
// где требуются ребра?

#define OUTSIDE_FRUSTUM 0
#define INSIDE_FRUSTUM 1
#define INTERSECT_FRUSTUM 2

#define CONVEX_SHAPE_COLLISION_MARGIN 0.04f

namespace devils_engine {
  namespace physics {
    namespace collision {
      struct shape {
        enum {
          box,
          sphere,
          polygon,
          convex_hull,
          max_type
        };
        
        uint32_t type;
        uint32_t offset;
        uint32_t points_count; // для всех типов объектов потребуется центр
        uint32_t faces_count;
        core::scalar margin;
        
        core::aabb get_aabb(const core::vec4* points, const core::transform &t) const;
        core::aabb get_temporal_aabb(const core::vec4* points, const core::transform &t, const core::vec4 &linvel, const core::vec4 &angvel, const size_t &time) const;
        core::sphere get_sphere(const core::vec4* points, const core::transform &t) const;
        uint32_t all_points_count() const;
        uint32_t points_count_considerable() const;
        core::vec4 get_point(const core::vec4* points, const core::transform &t, const uint32_t &index) const;
        core::vec4 calculate_local_inertia(const core::vec4* points, const core::scalar &mass) const; // для полигона и хула нужно самостоятельно расчитать
        core::scalar get_contact_breaking_threshold(const core::vec4* points, const core::scalar &default_contact_threshold) const;
      };
      
      struct ray {
        struct test_data {
          uint32_t ray_index;
          uint32_t body_index;
          float dist;
          uint32_t dummy[1];
        };
        
        core::vec4 pos;
        core::vec4 dir;
        core::vec4 invdir;
        core::vec4 data;
        
        ray();
        ray(const core::vec4 &pos, const core::vec4 &dir);
        ray(const core::vec4 &pos, const core::vec4 &dir, const float &max);
        ray(const core::vec4 &pos, const core::vec4 &dir, const float &max, const uint32_t &ignore_obj, const uint32_t &collision_filter);
        
        uint32_t ignore_obj() const;
        uint32_t collision_filter() const;
        float max() const;
        uint32_t ret_index() const;
        void set_ret_index(const uint32_t &index);
        // min, ???
      };
      
      struct frustum {
        struct test_data {
          uint32_t frustum_index;
          uint32_t body_index;
          float dist;
          uint32_t dummy[1];
        };
        
        core::vec4 planes[6];
        
        frustum();
        frustum(const core::mat4 &mat);
      };
    }
  }
}

#endif
