#include "collision_shape.h"
#include <stdexcept>

namespace devils_engine {
  namespace physics {
    namespace collision {
      core::aabb shape::get_aabb(const core::vec4* points, const core::transform &t) const {
        if (type == sphere) {
          const float radius = glm::uintBitsToFloat(faces_count);
          return {
            t.pos,
            core::vec4(radius, radius, radius, 0.0f)
          };
        }
        
        const uint32_t count = type == box ? 8 : points_count_considerable();
        core::vec4 min = get_point(points, t, 0), max = min;
        for (uint32_t i = 1; i < count; ++i) {
          const core::vec4 point = get_point(points, t, i);
          max = simd::max(max, point);
          min = simd::min(min, point);
        }
        
        return {
                   (max + min) / 2.0f,
          simd::abs(max - min) / 2.0f
        };
      }
      
      core::aabb shape::get_temporal_aabb(const core::vec4* points, const core::transform &t, const core::vec4 &linvel, const core::vec4 &angvel, const size_t &time) const {
        (void)points;
        (void)t;
        (void)linvel;
        (void)angvel;
        (void)time;
        throw std::runtime_error("not working");
        return {};
      }
      
      core::sphere shape::get_sphere(const core::vec4* points, const core::transform &t) const {
        if (type == sphere) {
          const float radius = glm::uintBitsToFloat(faces_count);
          return {
            t.pos,
            core::vec4(radius, radius, radius, 0.0f)
          };
        }
        
        const uint32_t count = type == box ? 8 : points_count_considerable();
        const core::vec4 center = get_point(points, t, points_count);
        float rad2max = 0.0f;
        for (uint32_t i = 0; i < count; ++i) {
          const core::vec4 point = get_point(points, t, i);
          rad2max = std::max(rad2max, simd::distance2(point, center));
        }
        
        const float rad = std::sqrt(rad2max);
        return {
          center,
          core::vec4(rad, rad, rad, 0.0f)
        };
      }
      
      uint32_t shape::all_points_count() const {
        return points_count+1;
      }
      
      uint32_t shape::points_count_considerable() const {
        return points_count;
      }
      
      core::vec4 get_aabb_point(const core::transform &t, const core::vec4 &extents, const uint32_t &index) {
        float arr[4];
        (extents).storeu(arr); //  * t.scale
        
        core::vec4 p = core::vec4(
          (index & 1) == 1 ? arr[0] : -arr[0],
          (index & 2) == 2 ? arr[1] : -arr[1],
          (index & 4) == 4 ? arr[2] : -arr[2],
          1.0f
        );

        return t.transform_vector(p);
      }
      
      core::vec4 shape::get_point(const core::vec4* points, const core::transform &t, const uint32_t &index) const {
        ASSERT(index < all_points_count());
        if (index >= all_points_count()) return core::vec4();
        
        ASSERT(type < max_type);
        
        switch (type) {
          case box:
            ASSERT(points_count == 8);
            if (index == points_count) return t.pos;
            return get_aabb_point(t, points[offset], index);
          case sphere:
            ASSERT(points_count == 0);
            return t.pos;
          case polygon:
          case convex_hull:
            return t.transform_vector(points[offset + index]);
        }
        
        return core::vec4();
      }
      
      core::vec4 shape::calculate_local_inertia(const core::vec4* points, const core::scalar &mass) const {
        ASSERT(type < max_type);
        
        switch (type) {
          case box: {
            const core::vec4 extents_with_margin = (points[offset] + core::vec4(margin, margin, margin, 0.0f)) * core::vec4(2.0f);
            float arr[4];
            extents_with_margin.storeu(arr);
            return core::vec4(
              mass / 12.0f * (arr[1]*arr[1] + arr[2]*arr[2]),
              mass / 12.0f * (arr[0]*arr[0] + arr[2]*arr[2]),
              mass / 12.0f * (arr[0]*arr[0] + arr[1]*arr[1]),
              0.0f
            );
          }
            
          case sphere: {
            const float elem = 0.4f * mass * margin * margin;
            return core::vec4(elem, elem, elem, 0.0f);
          }
        }
        
        return core::vec4();
      }
      
      core::scalar shape::get_contact_breaking_threshold(const core::vec4* points, const core::scalar &default_contact_threshold) const {
        core::transform t;
        const auto sphere = get_sphere(points, t);
        float arr[4];
        sphere.extents.storeu(arr);
        return arr[0] * default_contact_threshold;
      }
      
      ray::ray() {}
      ray::ray(const core::vec4 &pos, const core::vec4 &dir) : pos(pos), dir(dir), invdir(1.0f / dir) {}
      ray::ray(const core::vec4 &pos, const core::vec4 &dir, const float &max) : pos(pos), dir(dir), invdir(1.0f / dir), data(max, 0.0f, 0.0f, 0.0f) {}
      ray::ray(const core::vec4 &pos, const core::vec4 &dir, const float &max, const uint32_t &ignore_obj, const uint32_t &collision_filter) : 
        pos(pos), 
        dir(dir), 
        invdir(1.0f / dir), 
        data(max, glm::uintBitsToFloat(ignore_obj), glm::uintBitsToFloat(collision_filter), 0.0f) 
      {}
      
      uint32_t ray::ignore_obj() const {
        float arr[4];
        data.storeu(arr);
        return glm::floatBitsToUint(arr[1]);
      }
      
      uint32_t ray::collision_filter() const {
        float arr[4];
        data.storeu(arr);
        return glm::floatBitsToUint(arr[2]);
      }
      
      float ray::max() const {
        float arr[4];
        data.storeu(arr);
        return arr[0];
      }
      
      uint32_t ray::ret_index() const {
        float arr[4];
        data.storeu(arr);
        return glm::floatBitsToUint(arr[3]);
      }
      
      void ray::set_ret_index(const uint32_t &index) {
        float arr[4];
        data.storeu(arr);
        arr[3] = glm::uintBitsToFloat(index);
        data.loadu(arr);
      }
      
      frustum::frustum() {}
      frustum::frustum(const core::mat4 &mat) {
        const simd::mat4 mat1 = simd::transpose(mat);
  
        this->planes[0] = mat1[3] + mat1[0];
        this->planes[1] = mat1[3] - mat1[0];
        this->planes[2] = mat1[3] - mat1[1];
        this->planes[3] = mat1[3] + mat1[1];
        this->planes[4] = mat1[3] + mat1[2];
        this->planes[5] = mat1[3] - mat1[2];

        for (uint32_t i = 0; i < 6; ++i) {
          float arr[4];
          planes[i].storeu(arr);
          const float mag = simd::length(simd::vec4(arr[0], arr[1], arr[2], 0.0f));
          planes[i] /= mag;
        }
      }
    }
  }
}
