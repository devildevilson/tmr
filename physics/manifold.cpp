#include "manifold.h"

#include "physics_context.h"
#include "Utility.h"
#include <cstring>

namespace devils_engine {
  namespace physics {
    namespace narrowphase {
      manifold_point::flags::flags() : container(0) {}
      manifold_point::flags::flags(const uint32_t &flags) : container(flags) {}
      
      bool manifold_point::flags::lateral_friction_initialized() const {
        return (container & LATERAL_FRICTION_INITIALIZED) == LATERAL_FRICTION_INITIALIZED;
      }
      
      bool manifold_point::flags::has_contact_cfm() const {
        return (container & HAS_CONTACT_CFM) == HAS_CONTACT_CFM;
      }
      
      bool manifold_point::flags::has_contact_erp() const {
        return (container & HAS_CONTACT_ERP) == HAS_CONTACT_ERP;
      }
      
      bool manifold_point::flags::contact_stiffness_damping() const {
        return (container & CONTACT_STIFFNESS_DAMPING) == CONTACT_STIFFNESS_DAMPING;
      }
      
      bool manifold_point::flags::friction_anchor() const {
        return (container & FRICTION_ANCHOR) == FRICTION_ANCHOR;
      }
      
      void manifold_point::flags::lateral_friction_initialized(const bool value) {
        container = value ? container | LATERAL_FRICTION_INITIALIZED : container & ~LATERAL_FRICTION_INITIALIZED;
      }
      
      void manifold_point::flags::has_contact_cfm(const bool value) {
        container = value ? container | HAS_CONTACT_CFM : container & ~HAS_CONTACT_CFM;
      }
      
      void manifold_point::flags::has_contact_erp(const bool value) {
        container = value ? container | HAS_CONTACT_ERP : container & ~HAS_CONTACT_ERP;
      }
      
      void manifold_point::flags::contact_stiffness_damping(const bool value) {
        container = value ? container | CONTACT_STIFFNESS_DAMPING : container & ~CONTACT_STIFFNESS_DAMPING;
      }
      
      void manifold_point::flags::friction_anchor(const bool value) {
        container = value ? container | FRICTION_ANCHOR : container & ~FRICTION_ANCHOR;
      }
      
      manifold_point::manifold_point() : 
        distance1(0.0f),
        combined_friction(0.0f),
        combined_rolling_friction(0.0f),
        combined_spinning_friction(0.0f),
        combined_restitution(0.0f),
        applied_impulse(0.0f),
        applied_impulse_lateral1(0.0f),
        applied_impulse_lateral2(0.0f),
        contact_motion1(0.0f),
        contact_motion2(0.0f),
        contact_cfm(0.0f),
        contact_erp(0.0f),
        friction_cfm(0.0f),
        part_id0(0),
        part_id1(0),
        index0(0),
        index1(0),
        contact_point_flags(0),
        life_time(0),
        user_data(nullptr)
      {}
      
      manifold_point::manifold_point(const vec4 &point_a, const vec4 &point_b, const vec4 &normal, const scalar &dist) : 
        local_point_a(point_a),
        local_point_b(point_b),
        world_normal_b(normal),
        distance1(dist),
        combined_friction(0.0f),
        combined_rolling_friction(0.0f),
        combined_spinning_friction(0.0f),
        combined_restitution(0.0f),
        applied_impulse(0.0f),
        applied_impulse_lateral1(0.0f),
        applied_impulse_lateral2(0.0f),
        contact_motion1(0.0f),
        contact_motion2(0.0f),
        contact_cfm(0.0f),
        contact_erp(0.0f),
        friction_cfm(0.0f),
        part_id0(0),
        part_id1(0),
        index0(0),
        index1(0),
        contact_point_flags(0),
        life_time(0),
        user_data(nullptr)
      {}
      
      vec4 manifold_point::world_pos_b() const {
        return world_position_b + world_normal_b * distance1;
      }
      
      struct manifold_callbacks* persistent_manifold::callbacks = nullptr;
      persistent_manifold::persistent_manifold() : points_count(0), body0(UINT32_MAX), body1(UINT32_MAX), batch_id(UINT32_MAX), contact_breaking_threshold(0.0f), contact_processing_threshold(0.0f) {}
      persistent_manifold::persistent_manifold(const uint32_t &body0, const uint32_t &body1, const scalar &contact_breaking_threshold, const scalar &contact_processing_threshold) : 
        points_count(0), body0(body0), body1(body1), batch_id(UINT32_MAX), contact_breaking_threshold(contact_breaking_threshold), contact_processing_threshold(contact_processing_threshold)
      {}
      
      scalar calc_area4(const vec4 &p0, const vec4 &p1, const vec4 &p2, const vec4 &p3) {
        vec4 a[3], b[3];
        a[0] = p0 - p1;
        a[1] = p0 - p2;
        a[2] = p0 - p3;
        b[0] = p2 - p3;
        b[1] = p1 - p3;
        b[2] = p1 - p2;
        
        const vec4 tmp0 = simd::cross(a[0], b[0]);
        const vec4 tmp1 = simd::cross(a[1], b[1]);
        const vec4 tmp2 = simd::cross(a[2], b[2]);
        
        return std::max(std::max(simd::length2(tmp0), simd::length2(tmp1)), simd::length2(tmp2));
      }
      
      scalar calc_area3(const vec4 &p0, const vec4 &p1, const vec4 &p2, const vec4 &p3) {
        const vec4 a = p0 - p1;
        const vec4 b = p3 - p2;
        const vec4 cross = simd::cross(a, b);
        return simd::length2(cross);
      }
      
      uint32_t persistent_manifold::sort_points(const manifold_point &p) {
        uint32_t max_penetration_index = UINT32_MAX;
        scalar max_penetration = p.distance1;
        for (uint32_t i = 0; i < points_count; ++i) {
          if (point_cache[i].distance1 < max_penetration) {
            max_penetration_index = i;
            max_penetration = point_cache[i].distance1;
          }
        }
        
        scalar res[points_count];
        memset(res, 0, sizeof(scalar)*points_count);
        
        // в булете кажется будто точки не просто так распределены
        // придется много тестить
        for (uint32_t i = 0; i < points_count; ++i) {
          const uint32_t j = (i+1)%points_count;
          const uint32_t k = (j+1)%points_count;
          res[i] = calc_area3(p.local_point_a, point_cache[i].local_point_a, point_cache[j].local_point_a, point_cache[k].local_point_a);
        }
        
        scalar max = std::abs(res[0]);
        uint32_t max_index = 0;
        for (uint32_t i = 1; i < points_count; ++i) {
          if (max < std::abs(res[i])) {
            max = std::abs(res[i]);
            max_index = i;
          }
        }
        
        return max_index;
      }
      
      uint32_t persistent_manifold::get_nearest_point(const manifold_point &p) {
        scalar shortest = contact_breaking_threshold * contact_breaking_threshold;
        uint32_t index = UINT32_MAX;
        for (uint32_t i = 0; i < points_count; ++i) {
          const scalar dist = simd::distance2(point_cache[i].local_point_a, p.local_point_a);
//           PRINT_VEC4("point_cache[i].local_point_a", point_cache[i].local_point_a)
//           PRINT_VEC4("p.local_point_a             ", p.local_point_a)
          if (dist < shortest) {
            shortest = dist;
            index = i;
          }
        }
        return index;
      }
      
      uint32_t persistent_manifold::add_manifold_point(const manifold_point &p, const bool is_predictive) {
        if (!is_predictive) {
          ASSERT(valid_distance(p));
        }
        
        uint32_t insert_index = points_count;
        if (insert_index == MANIFOLD_CACHE_SIZE) {
#if MANIFOLD_CACHE_SIZE >= 4
          insert_index = sort_points(p);
#else
          insert_index = 0;
#endif
          destroy_callback(insert_index);
          point_cache[insert_index].user_data = nullptr;
        } else {
          ++points_count;
        }
        
        point_cache[insert_index] = p;
        return insert_index;
      }
      
      void persistent_manifold::remove_contact_point(const uint32_t &index) {
        if (points_count == 0) return;
        
        destroy_callback(index);
        
        const uint32_t last_index = points_count-1;
        --points_count;
        
        if (last_index != index) point_cache[index] = point_cache[last_index];
        //memset(&point_cache[last_index], 0, sizeof(manifold_point));
        point_cache[last_index] = manifold_point();
        
        if (callbacks != nullptr && callbacks->ended && points_count == 0) {
          callbacks->ended(this);
        }
      }
      
      void persistent_manifold::replace_contact_point(const manifold_point &p, const uint32_t &index) {
#define MAINTAIN_PERSISTENCY 1
#ifdef MAINTAIN_PERSISTENCY
        const uint32_t life_time = point_cache[index].life_time;
        const scalar applied_impulse = point_cache[index].applied_impulse;
        const scalar applied_impulse_lateral1 = point_cache[index].applied_impulse_lateral1;
        const scalar applied_impulse_lateral2 = point_cache[index].applied_impulse_lateral2;
        
        bool replace = true;
        
        if (p.contact_point_flags.friction_anchor()) {
          scalar mu = point_cache[index].combined_friction;
          scalar eps = 0;  //we could allow to enlarge or shrink the tolerance to check against the friction cone a bit, say 1e-7
          scalar a = applied_impulse_lateral1 * applied_impulse_lateral1 + applied_impulse_lateral2 * applied_impulse_lateral2;
          scalar b = eps + mu * applied_impulse;
          b = b * b;
          replace = (a) > (b);
        }
        
        if (replace) {
          void* cache = point_cache[index].user_data;
          
          point_cache[index] = p;
          point_cache[index].user_data = cache;
          point_cache[index].applied_impulse = applied_impulse;
          point_cache[index].applied_impulse_lateral1 = applied_impulse_lateral1;
          point_cache[index].applied_impulse_lateral2 = applied_impulse_lateral2;
        }
        
        point_cache[index].life_time = life_time;
#else
        destroy_callback(index);
        point_cache[index] = p;
#endif
      }
      
      bool persistent_manifold::valid_distance(const manifold_point &p) {
        return p.distance1 <= contact_breaking_threshold;
      }
      
      void persistent_manifold::refresh(const core::transform &transform_a, const core::transform &transform_b) {
        for (uint32_t i = 0; i < points_count; ++i) {
          point_cache[i].world_position_a = transform_a.transform_vector(point_cache[i].local_point_a);
          point_cache[i].world_position_b = transform_b.transform_vector(point_cache[i].local_point_b);
          point_cache[i].distance1 = simd::dot((point_cache[i].world_position_a - point_cache[i].world_position_b), point_cache[i].world_normal_b);
          ++point_cache[i].life_time;
          
//           PRINT_VEC4("point_cache[i].local_point_a   ",point_cache[i].local_point_a)
//           PRINT_VEC4("point_cache[i].local_point_b   ",point_cache[i].local_point_b)
//           PRINT_VEC4("point_cache[i].world_position_a",point_cache[i].world_position_a)
//           PRINT_VEC4("point_cache[i].world_position_b",point_cache[i].world_position_b)
//           PRINT_VAR("point_cache[i].distance1",point_cache[i].distance1)
//           ASSERT(point_cache[i].distance1 < 0.0f);
        }
        
        for (uint32_t i = 0; i < points_count; ++i) {
          manifold_point &pt = point_cache[points_count-i-1];
          if (!valid_distance(pt)) {
            PRINT_VAR("removed point dist",points_count-i-1)
            remove_contact_point(points_count-i-1);
            continue;
          }
          
          const vec4 proj_point = pt.world_position_a - pt.world_normal_b * pt.distance1;
          const vec4 prof_diff = pt.world_position_b - proj_point;
//           PRINT_VEC4("proj_point",proj_point)
//           PRINT_VEC4("prof_diff",prof_diff)
          const scalar d2 = simd::dot(prof_diff, prof_diff);
//           PRINT_VAR("d2",d2)
//           PRINT_VAR("c2",contact_breaking_threshold * contact_breaking_threshold)
          if (d2 > contact_breaking_threshold * contact_breaking_threshold) {
//             PRINT_VAR("removed point proj",points_count-i-1)
            remove_contact_point(points_count-i-1);
            continue;
          }
          
          if (callbacks != nullptr && callbacks->processed) {
            callbacks->processed(&pt, body0, body1);
          }
        }
      }
      
      void persistent_manifold::clear() {
        for (uint32_t i = 0; i < points_count; ++i) {
          destroy_callback(i);
        }
        
        if (callbacks != nullptr && callbacks->ended && points_count != 0) {
          callbacks->ended(this);
        }
        points_count = 0;
      }
      
      void persistent_manifold::destroy_callback(const uint32_t &index) {
        if (callbacks != nullptr && callbacks->destroyed && point_cache[index].user_data != nullptr) {
          callbacks->destroyed(point_cache[index].user_data);
        }
      }
    }
  }
}
