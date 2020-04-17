#include "collision_utils.h"

#include "minkowski.h"
#define CCD_SINGLE
#include <ccd/ccd.h>
#include <stdexcept>
#include "Utility.h"

namespace devils_engine {
  namespace physics {
    namespace collision {
      bool test_aabb(const aabb &box1, const aabb &box2) {
        const vec4 center = simd::abs(box1.center - box2.center);
        const vec4 extent =           box1.extents + box2.extents;

        return simd::all_xyz(extent > center);
      }
      
      bool contain_aabb(const aabb &container, const aabb &box) {
        if (simd::any_xyz(container.extents <= box.extents)) return false;
        return simd::all_xyz(simd::abs(container.center - box.center) < simd::abs(container.extents - box.extents));
      }
      
      bool test_sphere(const sphere &sphere1, const sphere &sphere2) {
        const scalar dist2 = simd::distance2(sphere1.center, sphere2.center);
        scalar arr[4];
        sphere1.extents.storeu(arr);
        const scalar r1 = arr[0];
        sphere2.extents.storeu(arr);
        return dist2 <= (r1 + arr[0])*(r1 + arr[0]);
      }
      
      static const scalar flt_plus_inf = -std::log(0.0f);
      bool test_ray_aabb(const struct ray &ray, const aabb &box, scalar &t) {
        static const vec4  plus_inf = vec4( flt_plus_inf,  flt_plus_inf,  flt_plus_inf,  flt_plus_inf);
        static const vec4 minus_inf = vec4(-flt_plus_inf, -flt_plus_inf, -flt_plus_inf, -flt_plus_inf);
        
        //const vec4 invdir = 1.0f / ray.dir;
        const vec4 invdir = ray.invdir;
        const vec4 min = box.center - box.extents;
        const vec4 max = box.center + box.extents;
        
        const vec4 t1 = (min - ray.pos) * invdir;
        const vec4 t2 = (max - ray.pos) * invdir;
        
        const vec4 filter_t1a = simd::min(t1, plus_inf);
        const vec4 filter_t2a = simd::min(t2, plus_inf);
        
        const vec4 filter_t1b = simd::max(t1, minus_inf);
        const vec4 filter_t2b = simd::max(t2, minus_inf);
        
  //       const vec4 tmin_vec = simd::min(t1, t2);
  //       const vec4 tmax_vec = simd::max(t1, t2);
        const vec4 tmax_vec = simd::max(filter_t1a, filter_t2a);
        const vec4 tmin_vec = simd::min(filter_t1b, filter_t2b);
        
        scalar arr1[4];
        scalar arr2[4];
        tmin_vec.storeu(arr1);
        tmax_vec.storeu(arr2);
        
        const scalar tmin = std::max(arr1[0], std::max(arr1[1], arr1[2]));
        const scalar tmax = std::min(arr2[0], std::min(arr2[1], arr2[2]));
        
        t = tmin;
        return tmax >= std::max(0.0f, tmin); // ограничение по дальности  
      }
      
      bool test_ray_sphere(const struct ray &ray, const sphere &sphere, scalar &t) {
        scalar arr[4];
        sphere.extents.storeu(arr);
        const vec4 oc = ray.pos - sphere.center;
        const scalar a = simd::dot(ray.dir, ray.dir);
        const scalar b = 2.0f * simd::dot(oc, ray.dir);
        const scalar c = simd::dot(oc, oc) - arr[0] * arr[0];
        const scalar discriminant = b*b - 4*a*c;
        t = -1.0f;
        if (discriminant >= 0.0f) {
          scalar numerator = -b - std::sqrt(discriminant);
          if (numerator > 0.0f) t = numerator / (2.0f * a);
          numerator = -b + std::sqrt(discriminant);
          if (numerator > 0.0f) t = numerator / (2.0f * a);
        }
        return discriminant >= 0.0f;
      }
      
      uint32_t test_frustum_aabb(const struct frustum &frustum, const aabb &box) {
        uint32_t result = INSIDE_FRUSTUM; // Assume that the aabb will be inside the frustum
        for (uint32_t i = 0; i < 6; ++i) {
          scalar arr[4];
          frustum.planes[i].store(arr);
          const simd::vec4 frustumPlane = simd::vec4(arr[0], arr[1], arr[2], 0.0f); //frustum.planes[i] * simd::vec4(1.0f, 1.0f, 1.0f, 0.0f);

          const scalar d = simd::dot(box.center,            frustumPlane);
          const scalar r = simd::dot(box.extents, simd::abs(frustumPlane));

          const scalar d_p_r = d + r;
          const scalar d_m_r = d - r;
          
          //frustumPlane.w
          if (d_p_r < -arr[3]) {
            result = OUTSIDE_FRUSTUM;
            break;
          } else if (d_m_r < -arr[3]) result = INTERSECT_FRUSTUM;
        }

        return result;
      }
      
      uint32_t test_frustum_sphere(const struct frustum &frustum, const sphere &sphere) {
        uint32_t result = INSIDE_FRUSTUM; // Assume that the aabb will be inside the frustum
        for (uint32_t i = 0; i < 6; ++i) {
          scalar arr[4];
          frustum.planes[i].store(arr);
          const simd::vec4 frustumPlane = simd::vec4(arr[0], arr[1], arr[2], 0.0f); //frustum.planes[i] * simd::vec4(1.0f, 1.0f, 1.0f, 0.0f);

          const scalar d = simd::dot(sphere.center,            frustumPlane);
          const scalar r = simd::dot(sphere.extents, simd::abs(frustumPlane));

          const scalar d_p_r = d + r;
          const scalar d_m_r = d - r;
          
          //frustumPlane.w
          if (d_p_r < -arr[3]) {
            result = OUTSIDE_FRUSTUM;
            break;
          } else if (d_m_r < -arr[3]) result = INTERSECT_FRUSTUM;
        }

        return result;
      }
      
      bool overlap(const scalar &margin, const scalar &min1, const scalar &max1, const scalar &min2, const scalar &max2, const vec4 &axis, vec4 &mtv, scalar &dist) {
        const scalar test1 = min1 - max2;
        const scalar test2 = min2 - max1;

        if (test1 > 0.0f || test2 > 0.0f) return false;

        const scalar d = glm::max(glm::min(glm::abs(test1), glm::abs(test2)) - margin, 0.0f);
        
        if (d < dist) {
          mtv = axis;
          dist = d;
        }

        return true;
      }
      
      vec4 get_local_vertex(const vec4 &extents, const uint32_t &i) {
        scalar arr[4];
        extents.storeu(arr);
        return vec4(
          (i & 1) == 1 ? arr[0] : -arr[0],
          (i & 2) == 2 ? arr[1] : -arr[1],
          (i & 4) == 4 ? arr[2] : -arr[2],
          1.0f
        );
      }
      
      void project(const vec4 &axis, const transform &trans, const vec4 &extents, scalar &min_ret, scalar &max_ret) {
        const vec4 first = trans.transform_vector(get_local_vertex(extents, 0));
        min_ret = max_ret = simd::dot(first, axis);
        for (uint32_t i = 1; i < 8; ++i) {
          const vec4 point = trans.transform_vector(get_local_vertex(extents, i));
          const scalar d = simd::dot(point, axis);
          
          min_ret = glm::min(min_ret, d);
          max_ret = glm::max(max_ret, d);
        }
      }
      
      void project(const vec4 &axis, const transform &trans, const scalar &radius, scalar &min_ret, scalar &max_ret) {
        min_ret = max_ret = simd::dot(trans.pos, axis);
        
        scalar arr[4];
        trans.scale.storeu(arr);
        min_ret -= radius * arr[0];
        max_ret += radius * arr[0];
      }
      
      void project(const vec4 &axis, const vec4* points, const transform &trans, const shape &shape, scalar &min_ret, scalar &max_ret) {
        const vec4 first = trans.transform_vector(points[shape.offset]);
        min_ret = max_ret = simd::dot(first, axis);
        for (uint32_t i = 1; i < shape.points_count; ++i) {
          const vec4 point = trans.transform_vector(points[shape.offset+i]);
          const scalar d = simd::dot(point, axis);
          
          min_ret = glm::min(min_ret, d);
          max_ret = glm::max(max_ret, d);
        }
      }
      
      vec4 get_box_faces(const simd::mat4 &mat, const uint32_t &index) {
        return mat[index];
      }
      
      vec4 get_spere_faces(const vec4 &pos1, const vec4 pos2) {
        return simd::normalize(pos2 - pos1);
      }
      
      vec4 get_polygon_faces(const vec4* points, const shape &shape, const uint32_t &index) {
        return points[shape.offset+1+shape.points_count+index] * vec4(1.0f, 1.0f, 1.0f, 0.0f);
      }
      
      vec4 get_convex_hull_faces(const vec4* points, const shape &shape, const uint32_t &index) {
        return points[shape.offset+1+shape.points_count+index] * vec4(1.0f, 1.0f, 1.0f, 0.0f);
      }
      
      bool test_objects_SAT(const vec4* points, const shape &shape1, const transform &trans1, const shape &shape2, const transform &trans2, vec4 &mtv, scalar &dist) {
        static object_sat_func* funcs[4][4] = {
          {test_box_box_SAT,         test_box_sphere_SAT,         test_box_polygon_SAT,         test_box_convex_hull_SAT},
          {test_box_sphere_SAT,      test_sphere_sphere_SAT,      test_sphere_polygon_SAT,      test_sphere_convex_hull_SAT},
          {test_box_polygon_SAT,     test_sphere_polygon_SAT,     test_polygon_polygon_SAT,     test_polygon_convex_hull_SAT},
          {test_box_convex_hull_SAT, test_sphere_convex_hull_SAT, test_polygon_convex_hull_SAT, test_convex_hull_convex_hull_SAT},
        };
        
        ASSERT(shape1.type < shape::max_type);
        ASSERT(shape2.type < shape::max_type);
        const bool swap = shape1.type > shape2.type;
        const bool ret = funcs[shape1.type][shape2.type](
          points, 
          swap ? shape2 : shape1, 
          swap ? trans2 : trans1, 
          swap ? shape1 : shape2, 
          swap ? trans1 : trans2, 
          mtv, 
          dist
        );
        
        if (swap) mtv = -mtv;
        
        return ret;
      }
      
      bool test_box_box_SAT(const vec4* points, const shape &shape1, const transform &trans1, const shape &shape2, const transform &trans2, vec4 &mtv, scalar &dist) {
        scalar minFirst = 0.0f, maxFirst = 0.0f, minSecond = 0.0f, maxSecond = 0.0f;
        
        const simd::mat4 orn1 = simd::mat4(trans1.rot);
        const simd::mat4 orn2 = simd::mat4(trans2.rot);
        const scalar margin = glm::max(shape1.margin, shape2.margin);
        const vec4 delta = trans1.pos - trans2.pos;
        for (uint32_t i = 0; i < 3; ++i) {
          const vec4 face = get_box_faces(orn1, i);
          project(face, trans1,  points[shape1.offset],  minFirst,  maxFirst);
          project(face, trans2,  points[shape2.offset], minSecond, maxSecond);

          if (!overlap(margin, minFirst, maxFirst, minSecond, maxSecond, face, mtv, dist)) return false;
        }
        
        for (uint32_t i = 0; i < 3; ++i) {
          const vec4 face = get_box_faces(orn2, i);
          project(face, trans1,  points[shape1.offset],  minFirst,  maxFirst);
          project(face, trans2,  points[shape2.offset], minSecond, maxSecond);

          if (!overlap(margin, minFirst, maxFirst, minSecond, maxSecond, face, mtv, dist)) return false;
        }
        
        if (simd::dot(-simd::vec4(delta), mtv) > 0.0f) mtv = -mtv;
        return true;
      }
      
      bool test_box_sphere_SAT(const vec4* points, const shape &shape1, const transform &trans1, const shape &shape2, const transform &trans2, vec4 &mtv, scalar &dist) {
        scalar minFirst = 0.0f, maxFirst = 0.0f, minSecond = 0.0f, maxSecond = 0.0f;
        
        const simd::mat4 orn1 = simd::mat4(trans1.rot);
        const scalar margin = glm::max(shape1.margin, shape2.margin);
        const vec4 delta = trans1.pos - trans2.pos;
        
        for (uint32_t i = 0; i < 3; ++i) {
          const vec4 face = get_box_faces(orn1, i);
          project(face, trans1,  points[shape1.offset],  minFirst,  maxFirst);
          project(face, trans2,  glm::floatBitsToUint(shape2.faces_count), minSecond, maxSecond);

          if (!overlap(margin, minFirst, maxFirst, minSecond, maxSecond, face, mtv, dist)) return false;
        }
        
        const vec4 face = get_spere_faces(trans1.pos, trans2.pos);
        project(face, trans1,  points[shape1.offset],  minFirst,  maxFirst);
        project(face, trans2,  glm::floatBitsToUint(shape2.faces_count), minSecond, maxSecond);

        if (!overlap(margin, minFirst, maxFirst, minSecond, maxSecond, face, mtv, dist)) return false;
        
        if (simd::dot(-simd::vec4(delta), mtv) > 0.0f) mtv = -mtv;
        return true;
      }
      
      bool test_box_polygon_SAT(const vec4* points, const shape &shape1, const transform &trans1, const shape &shape2, const transform &trans2, vec4 &mtv, scalar &dist) {
        scalar minFirst = 0.0f, maxFirst = 0.0f, minSecond = 0.0f, maxSecond = 0.0f;
        
        const simd::mat4 orn1 = simd::mat4(trans1.rot);
        const simd::mat4 orn2 = simd::mat4(trans2.rot);
        const scalar margin = glm::max(shape1.margin, shape2.margin);
        const vec4 delta = trans1.pos - trans2.transform_vector(points[shape2.offset+shape2.points_count+1]);
        
        for (uint32_t i = 0; i < 3; ++i) {
          const vec4 face = get_box_faces(orn1, i);
          project(face, trans1, points[shape1.offset],  minFirst,  maxFirst);
          project(face, points, trans2, shape2, minSecond, maxSecond);

          if (!overlap(margin, minFirst, maxFirst, minSecond, maxSecond, face, mtv, dist)) return false;
        }
        
        for (uint32_t i = 0; i < shape2.faces_count; ++i) {
          const vec4 face = orn2 * get_polygon_faces(points, shape2, i);
          project(face, trans1, points[shape1.offset], minFirst, maxFirst);
          project(face, points, trans2, shape2, minSecond, maxSecond);

          if (!overlap(margin, minFirst, maxFirst, minSecond, maxSecond, face, mtv, dist)) return false;
        }
        
        if (simd::dot(-simd::vec4(delta), mtv) > 0.0f) mtv = -mtv;
        return true;
      }
      
      bool test_box_convex_hull_SAT(const vec4* points, const shape &shape1, const transform &trans1, const shape &shape2, const transform &trans2, vec4 &mtv, scalar &dist) {
        scalar minFirst = 0.0f, maxFirst = 0.0f, minSecond = 0.0f, maxSecond = 0.0f;
        
        const simd::mat4 orn1 = simd::mat4(trans1.rot);
        const simd::mat4 orn2 = simd::mat4(trans2.rot);
        const scalar margin = glm::max(shape1.margin, shape2.margin);
        const vec4 delta = trans1.pos - trans2.transform_vector(points[shape2.offset+shape2.points_count+1]);
        
        for (uint32_t i = 0; i < 3; ++i) {
          const vec4 face = get_box_faces(orn1, i);
          project(face, trans1, points[shape1.offset],  minFirst,  maxFirst);
          project(face, points, trans2, shape2, minSecond, maxSecond);

          if (!overlap(margin, minFirst, maxFirst, minSecond, maxSecond, face, mtv, dist)) return false;
        }
        
        for (uint32_t i = 0; i < shape2.faces_count; ++i) {
          const vec4 face = orn2 * get_convex_hull_faces(points, shape2, i);
          project(face, trans1, points[shape1.offset], minFirst, maxFirst);
          project(face, points, trans2, shape2, minSecond, maxSecond);

          if (!overlap(margin, minFirst, maxFirst, minSecond, maxSecond, face, mtv, dist)) return false;
        }
        
        if (simd::dot(-simd::vec4(delta), mtv) > 0.0f) mtv = -mtv;
        return true;
      }
      
      bool test_sphere_sphere_SAT(const vec4* points, const shape &shape1, const transform &trans1, const shape &shape2, const transform &trans2, vec4 &mtv, scalar &dist) {
        (void)points;
        scalar minFirst = 0.0f, maxFirst = 0.0f, minSecond = 0.0f, maxSecond = 0.0f;
        
        const scalar margin = glm::max(shape1.margin, shape2.margin);
        const vec4 delta = trans1.pos - trans2.pos;
        
        const vec4 face = get_spere_faces(trans1.pos, trans2.pos);
        project(face, trans1,  glm::floatBitsToUint(shape1.faces_count),  minFirst,  maxFirst);
        project(face, trans2,  glm::floatBitsToUint(shape2.faces_count), minSecond, maxSecond);

        if (!overlap(margin, minFirst, maxFirst, minSecond, maxSecond, face, mtv, dist)) return false;
        
        if (simd::dot(-simd::vec4(delta), mtv) > 0.0f) mtv = -mtv;
        return true;
      }
      
      bool test_sphere_polygon_SAT(const vec4* points, const shape &shape1, const transform &trans1, const shape &shape2, const transform &trans2, vec4 &mtv, scalar &dist) {
        scalar minFirst = 0.0f, maxFirst = 0.0f, minSecond = 0.0f, maxSecond = 0.0f;
        
        const simd::mat4 orn2 = simd::mat4(trans2.rot);
        const scalar margin = glm::max(shape1.margin, shape2.margin);
        const vec4 delta = trans1.pos - trans2.pos;
        
        const vec4 face = get_spere_faces(trans1.pos, trans2.pos);
        project(face, trans1, glm::floatBitsToUint(shape1.faces_count), minFirst, maxFirst);
        project(face, points, trans2, shape2, minSecond, maxSecond);
        
        if (!overlap(margin, minFirst, maxFirst, minSecond, maxSecond, face, mtv, dist)) return false;
        
        for (uint32_t i = 0; i < shape2.faces_count; ++i) {
          const vec4 face = orn2 * get_polygon_faces(points, shape2, i);
          project(face, trans1, glm::floatBitsToUint(shape1.faces_count), minFirst, maxFirst);
          project(face, points, trans2, shape2, minSecond, maxSecond);

          if (!overlap(margin, minFirst, maxFirst, minSecond, maxSecond, face, mtv, dist)) return false;
        }
        
        if (simd::dot(-simd::vec4(delta), mtv) > 0.0f) mtv = -mtv;
        return true;
      }
      
      bool test_sphere_convex_hull_SAT(const vec4* points, const shape &shape1, const transform &trans1, const shape &shape2, const transform &trans2, vec4 &mtv, scalar &dist) {
        scalar minFirst = 0.0f, maxFirst = 0.0f, minSecond = 0.0f, maxSecond = 0.0f;
        
        const simd::mat4 orn2 = simd::mat4(trans2.rot);
        const scalar margin = glm::max(shape1.margin, shape2.margin);
        const vec4 delta = trans1.pos - trans2.pos;
        
        const vec4 face = get_spere_faces(trans1.pos, trans2.pos);
        project(face, trans1, glm::floatBitsToUint(shape1.faces_count), minFirst, maxFirst);
        project(face, points, trans2, shape2, minSecond, maxSecond);

        if (!overlap(margin, minFirst, maxFirst, minSecond, maxSecond, face, mtv, dist)) return false;
        
        for (uint32_t i = 0; i < shape2.faces_count; ++i) {
          const vec4 face = orn2 * get_convex_hull_faces(points, shape2, i);
          project(face, trans1, glm::floatBitsToUint(shape1.faces_count), minFirst, maxFirst);
          project(face, points, trans2, shape2, minSecond, maxSecond);

          if (!overlap(margin, minFirst, maxFirst, minSecond, maxSecond, face, mtv, dist)) return false;
        }
        
        if (simd::dot(-simd::vec4(delta), mtv) > 0.0f) mtv = -mtv;
        return true;
      }
      
      bool test_polygon_polygon_SAT(const vec4* points, const shape &shape1, const transform &trans1, const shape &shape2, const transform &trans2, vec4 &mtv, scalar &dist) {
        scalar minFirst = 0.0f, maxFirst = 0.0f, minSecond = 0.0f, maxSecond = 0.0f;
        
        const simd::mat4 orn1 = simd::mat4(trans1.rot);
        const simd::mat4 orn2 = simd::mat4(trans2.rot);
        const scalar margin = glm::max(shape1.margin, shape2.margin);
        const vec4 delta = trans1.pos - trans2.pos;
        
        for (uint32_t i = 0; i < shape1.faces_count; ++i) {
          const vec4 face = orn1 * get_polygon_faces(points, shape1, i);
          project(face, points, trans1, shape1, minFirst, maxFirst);
          project(face, points, trans2, shape2, minSecond, maxSecond);

          if (!overlap(margin, minFirst, maxFirst, minSecond, maxSecond, face, mtv, dist)) return false;
        }
        
        for (uint32_t i = 0; i < shape2.faces_count; ++i) {
          const vec4 face = orn2 * get_polygon_faces(points, shape2, i);
          project(face, points, trans1, shape1, minFirst, maxFirst);
          project(face, points, trans2, shape2, minSecond, maxSecond);

          if (!overlap(margin, minFirst, maxFirst, minSecond, maxSecond, face, mtv, dist)) return false;
        }
        
        if (simd::dot(-simd::vec4(delta), mtv) > 0.0f) mtv = -mtv;
        return true;
      }
      
      bool test_polygon_convex_hull_SAT(const vec4* points, const shape &shape1, const transform &trans1, const shape &shape2, const transform &trans2, vec4 &mtv, scalar &dist) {
        scalar minFirst = 0.0f, maxFirst = 0.0f, minSecond = 0.0f, maxSecond = 0.0f;
        
        const simd::mat4 orn1 = simd::mat4(trans1.rot);
        const simd::mat4 orn2 = simd::mat4(trans2.rot);
        const scalar margin = glm::max(shape1.margin, shape2.margin);
        const vec4 delta = trans1.pos - trans2.pos;
        
        for (uint32_t i = 0; i < shape1.faces_count; ++i) {
          const vec4 face = orn1 * get_polygon_faces(points, shape1, i);
          project(face, points, trans1, shape1, minFirst, maxFirst);
          project(face, points, trans2, shape2, minSecond, maxSecond);

          if (!overlap(margin, minFirst, maxFirst, minSecond, maxSecond, face, mtv, dist)) return false;
        }
        
        for (uint32_t i = 0; i < shape2.faces_count; ++i) {
          const vec4 face = orn2 * get_convex_hull_faces(points, shape2, i);
          project(face, points, trans1, shape1, minFirst, maxFirst);
          project(face, points, trans2, shape2, minSecond, maxSecond);

          if (!overlap(margin, minFirst, maxFirst, minSecond, maxSecond, face, mtv, dist)) return false;
        }
        
        if (simd::dot(-simd::vec4(delta), mtv) > 0.0f) mtv = -mtv;
        return true;
      }
      
      bool test_convex_hull_convex_hull_SAT(const vec4* points, const shape &shape1, const transform &trans1, const shape &shape2, const transform &trans2, vec4 &mtv, scalar &dist) {
        scalar minFirst = 0.0f, maxFirst = 0.0f, minSecond = 0.0f, maxSecond = 0.0f;
        
        const simd::mat4 orn1 = simd::mat4(trans1.rot);
        const simd::mat4 orn2 = simd::mat4(trans2.rot);
        const scalar margin = glm::max(shape1.margin, shape2.margin);
        const vec4 delta = trans1.pos - trans2.pos;
        
        for (uint32_t i = 0; i < shape1.faces_count; ++i) {
          const vec4 face = orn1 * get_convex_hull_faces(points, shape1, i);
          project(face, points, trans1, shape1, minFirst, maxFirst);
          project(face, points, trans2, shape2, minSecond, maxSecond);

          if (!overlap(margin, minFirst, maxFirst, minSecond, maxSecond, face, mtv, dist)) return false;
        }
        
        for (uint32_t i = 0; i < shape2.faces_count; ++i) {
          const vec4 face = orn2 * get_convex_hull_faces(points, shape2, i);
          project(face, points, trans1, shape1, minFirst, maxFirst);
          project(face, points, trans2, shape2, minSecond, maxSecond);

          if (!overlap(margin, minFirst, maxFirst, minSecond, maxSecond, face, mtv, dist)) return false;
        }
        
        if (simd::dot(-simd::vec4(delta), mtv) > 0.0f) mtv = -mtv;
        return true;
      }
      
      struct object_wrapper {
        const vec4* points;
        const struct shape* shape; 
        const struct transform* trans;
        
        object_wrapper(const vec4* points, const struct shape* shape, const transform* trans) : points(points), shape(shape), trans(trans) {}
        const struct transform & transform() const { return *trans; }
        vec4 get_local_support_with_margin(const vec4 &axis) const {
          static support_func* funcs[] = {
            box_support_func,
            sphere_support_func,
            polygon_support_func,
            convex_hull_support_func
          };
          
          ASSERT(shape->type < shape::max_type);
          ASSERT(axis.w == 0.0f);
          return funcs[shape->type](points, *shape, *trans, axis);
        }
        
        vec4 get_object_world_center() const {
          // у меня есть центр, мне нужно его передвинуть
          if (shape->type == shape::box || shape->type == shape::sphere) return trans->pos; //vec4(0.0f, 0.0f, 0.0f, 1.0f);
          return trans->transform_vector(points[shape->offset + shape->points_count]);
        }
      };
      
      void support(const void *obj, const ccd_vec3_t *dir, ccd_vec3_t *vec) {
        auto o = reinterpret_cast<const object_wrapper*>(obj);
        
        const vec4 direction = vec4(dir->v[0], dir->v[1], dir->v[2], 0.0f);
        const vec4 final_dir = simd::normalize(direction * o->transform().rot);
        
        const vec4 sup = o->get_local_support_with_margin(final_dir);
//         PRINT_VEC4("sup",sup);
        
        const vec4 final_sup = o->transform().transform_vector(sup);
//         PRINT_VEC4("o->transform().pos",o->transform().pos);
//         PRINT_VEC4("final_sup",final_sup);
        
        float arr[4];
        final_sup.storeu(arr);
        memcpy(vec->v, arr, sizeof(scalar)*3);
      }
      
      void center(const void *obj, ccd_vec3_t *center) {
        auto o = reinterpret_cast<const object_wrapper*>(obj);
        const vec4 c = o->get_object_world_center();
        float arr[4];
        c.storeu(arr);
        memcpy(center->v, arr, sizeof(scalar)*3);
      }
      
      bool test_objects_mpr(const vec4* points, const shape &shape1, const transform &trans1, const shape &shape2, const transform &trans2, return_data& data) {
//         TimeLogDestructor d("test_objects_mpr");
        object_wrapper obj1(points, &shape1, &trans1);
        object_wrapper obj2(points, &shape2, &trans2);
        
//         mpr::collision_description cd;
//         cd.max_gjk_iterations = 1000;
        
        // минковский алгоритм не слишком точный по всей видимости
        // но при этом 1000 итераций происходит сверхбыстро
        // скорее всего теперь проблема не в нем
        // возможно я каждый раз неправильно нахожу направление нормали
        
        // похоже что либссд скомпилированный с оптимизациями
        // работает так же выстро как и моя версия без оптимизаций
        // переделать ли либссд?
        
        // либссд генерит точки гораздо точнеечем моя версия
        // и манифолд теперь обновляется без проблем
        // объект все равно несколько утопает в плоскости
        // но это видимо из-за гравитации
        
        // почему то не работает ccdGJKPenetration
        
//         const int res = compute_penetration(obj1, obj2, cd, &data);
//         return res == 0;
        
        ccd_t ccd;
        CCD_INIT(&ccd); // initialize ccd_t struct
        
        // set up ccd_t struct
        ccd.support1       = support; // support function for first object
        ccd.support2       = support; // support function for second object
        ccd.center1        = center;  // center function for first object
        ccd.center2        = center;  // center function for second object
        ccd.mpr_tolerance  = 0.0001;  // maximal tolerance
        ccd.max_iterations = 100;
        ccd.epa_tolerance  = 0.0001;

        ccd_real_t depth;
        ccd_vec3_t dir, pos;
        int intersect = ccdMPRPenetration(&obj1, &obj2, &ccd, &depth, &dir, &pos);
//         int intersect = ccdGJKPenetration(&obj1, &obj2, &ccd, &depth, &dir, &pos);
        
        if (intersect == 0) {
          data.distance = -depth;
          data.normal = -vec4(dir.v[0], dir.v[1], dir.v[2], 0.0f);
          data.point_a = vec4(pos.v[0], pos.v[1], pos.v[2], 1.0f);
          data.point_b = data.point_a - data.normal * data.distance;
        }
        
        return intersect == 0;
      }
      
      vec4 box_support_func(const vec4* points, const shape &shape1, const transform &trans1, const vec4 &axis) {
        // проблема в том что нам нужно обойти все точки 
        // передвинуть их по трансформе
        // и посчитать дот с направлением
        // сколько раз мы это делаем?
        // очень много раз
        // нужно сначало вычислить все точки
        
        // сюда приходит модифицированый вектор
        // а отсюда уходят точки на которые потом применяется трансформа
        // так что здесь мы по стандарту находим точку
        // как найти точку в конвекс хуле? мы не знаем откуда у нас идет этот вектор
        // так что видимо наилучшим решением будет просто последовательно продотать 
        // все точки и максимальный дот по идее бует интересующей нас точкой
        // у полигона видимо нужно вернуть центр при минимальном отклонении от кросс нормали?
        // наверное нет, небольшая воронка рядом с нормалью скорее
        // нужно возвращать иногда центр
        
        const vec4 extents = points[shape1.offset] + shape1.margin;
        vec4 ret = get_local_vertex(extents, 0);
        scalar dist = simd::dot(axis, ret);
        for (uint32_t i = 1; i < 8; ++i) {
          const vec4 v = get_local_vertex(extents, i);
          const scalar a = simd::dot(axis, v);
          if (dist < a) {
            dist = a;
            ret = v;
          }
        }
        
        (void)trans1;
        return ret;
      }
      
      vec4 sphere_support_func(const vec4* points, const shape &shape1, const transform &trans1, const vec4 &axis) {
        (void)points;
        const scalar radius = glm::uintBitsToFloat(shape1.faces_count) + shape1.margin;
        (void)trans1;
        return axis * radius + vec4(0,0,0,1);
      }
      
#define NORMAL_TOLERANCY (1.0f - 0.04f)
      vec4 polygon_support_func(const vec4* points, const shape &shape1, const transform &trans1, const vec4 &axis) {
        const vec4 normal = points[shape1.offset + shape1.points_count + 1] * vec4(1,1,1,0);
        const scalar normal_dot = simd::dot(normal, simd::normalize(axis));
        if (glm::abs(1.0f - glm::abs(normal_dot)) < NORMAL_TOLERANCY) return points[shape1.offset + shape1.points_count];
        
        const vec4 first = points[shape1.offset];
//         const vec4 final_first = first + (first/first)*vec4(shape1.margin, shape1.margin, shape1.margin, 1.0f);
        float arr[4];
        first.storeu(arr);
        const vec4 final_first = first + vec4(
            arr[0] != 0.0f ? arr[0]/arr[0] : 0.0f,
            arr[1] != 0.0f ? arr[1]/arr[1] : 0.0f,
            arr[2] != 0.0f ? arr[2]/arr[2] : 0.0f,
            1.0f
          )*vec4(shape1.margin, shape1.margin, shape1.margin, 1.0f);
        vec4 ret = final_first;
        scalar dist = simd::dot(axis, ret);
        for (uint32_t i = 1; i < shape1.points_count; ++i) {
          const vec4 v = points[shape1.offset+i];
//           const vec4 final_v = v + (v/v)*vec4(shape1.margin, shape1.margin, shape1.margin, 1.0f);
          float arr[4];
          v.storeu(arr);
          const vec4 final_v = v + vec4(
              arr[0] != 0.0f ? arr[0]/arr[0] : 0.0f,
              arr[1] != 0.0f ? arr[1]/arr[1] : 0.0f,
              arr[2] != 0.0f ? arr[2]/arr[2] : 0.0f,
              1.0f
            )*vec4(shape1.margin, shape1.margin, shape1.margin, 1.0f);
          const scalar a = simd::dot(axis, final_v);
          if (dist < a) {
            dist = a;
            ret = final_v;
          }
        }
        
        (void)trans1;
        return ret;
      }
      
      vec4 convex_hull_support_func(const vec4* points, const shape &shape1, const transform &trans1, const vec4 &axis) {
        const vec4 first = points[shape1.offset];
        //const vec4 final_first = first + (first/first)*vec4(shape1.margin, shape1.margin, shape1.margin, 1.0f);
        float arr[4];
        first.storeu(arr);
        const vec4 final_first = first + vec4(
            arr[0] != 0.0f ? arr[0]/arr[0] : 0.0f,
            arr[1] != 0.0f ? arr[1]/arr[1] : 0.0f,
            arr[2] != 0.0f ? arr[2]/arr[2] : 0.0f,
            1.0f
          )*vec4(shape1.margin, shape1.margin, shape1.margin, 1.0f);
        vec4 ret = final_first;
        scalar dist = simd::dot(axis, ret);
        for (uint32_t i = 1; i < shape1.points_count; ++i) {
          const vec4 v = points[shape1.offset+i];
          float arr[4];
          v.storeu(arr);
          //const vec4 final_v = v + (v/v)*vec4(shape1.margin, shape1.margin, shape1.margin, 1.0f);
          const vec4 final_v = v + vec4(
            arr[0] != 0.0f ? arr[0]/arr[0] : 0.0f,
            arr[1] != 0.0f ? arr[1]/arr[1] : 0.0f,
            arr[2] != 0.0f ? arr[2]/arr[2] : 0.0f,
            1.0f
          )*vec4(shape1.margin, shape1.margin, shape1.margin, 1.0f);
          const scalar a = simd::dot(axis, final_v);
          if (dist < a) {
            dist = a;
            ret = final_v;
          }
        }
        
        (void)trans1;
        return ret;
      }
      
      bool test_ray_object(const struct ray &ray, const vec4* points, const shape &shape1, const transform &trans1, scalar &t) {
        static ray_test_func* ray_funcs[] = {
          test_ray_box,
          test_ray_sphere,
          test_ray_polygon,
          test_ray_convex_hull
        };
        
        ASSERT(shape1.type < shape::max_type);
        return ray_funcs[shape1.type](ray, points, shape1, trans1, t);
      }
      
      bool feq(const scalar &a, const scalar &b) {
        return std::abs(a-b) < EPSILON;
      }
      
      bool test_ray_box(const struct ray &ray, const vec4* points, const shape &shape1, const transform &trans1, scalar &t) {
        const vec4 p = trans1.pos - ray.pos;
//         const simd::mat4 orn = simd::mat4(trans1.rot);
//         vec4 X(orn[0]);
//         vec4 Y(orn[1]);
//         vec4 Z(orn[2]);
        
        const vec4 f = trans1.rot * ray.dir;
//         const vec4 f(
//           simd::dot(X, ray.dir),
//           simd::dot(Y, ray.dir),
//           simd::dot(Z, ray.dir),
//           0.0f
//         );
        
        const vec4 e = trans1.rot * p;
//         const vec4 e(
//           simd::dot(X, p),
//           simd::dot(Y, p),
//           simd::dot(Z, p),
//           0.0f
//         );
        
        const vec4 eps = vec4(EPSILON, EPSILON, EPSILON, EPSILON);
        const vec4 size_v = (trans1.scale * points[shape1.offset]);
        scalar t_arr[6] = {0, 0, 0, 0, 0, 0};
        
//         scalar f_arr[4];
//         scalar e_arr[4];
//         scalar size[4];
//         f.storeu(f_arr);
//         e.storeu(e_arr);
//         size_v.storeu(size);
//         for (uint32_t i = 0; i < 3; ++i) {
//           if (feq(f[i], 0.0f)) {
//             if (-e_arr[i] - size[i] > 0 || -e_arr[i] + size[i] < 0) return false;
//             f_arr[i] = EPSILON;
//           }
//           
//           t_arr[i * 2 + 0] = (e_arr[i] + size[i]) / f_arr[i];
//           t_arr[i * 2 + 1] = (e_arr[i] - size[i]) / f_arr[i];
//         }
        
        const vec4 zero = vec4();
        const vec4 eq = simd::equal(f, zero, EPSILON);
        const vec4 esubs1 = -e - size_v;
        const vec4 esubs2 = -e + size_v;
        
        if (simd::any_xyz(eq & ((esubs1 > zero) | (esubs2 < zero)))) return false;
        const vec4 f2 = (f + eq) & eps;
        const vec4 t1 = (e + size_v) / f2;
        const vec4 t2 = (e - size_v) / f2;
        
        scalar t1_arr[4];
        scalar t2_arr[4];
        t1.storeu(t1_arr);
        t2.storeu(t2_arr);
        
        t_arr[0] = t1_arr[0]; t_arr[1] = t2_arr[0]; t_arr[2] = t1_arr[1]; t_arr[3] = t2_arr[1]; t_arr[4] = t1_arr[3]; t_arr[5] = t2_arr[3];
        
        scalar tmin = std::max(std::max(std::min(t_arr[0], t_arr[1]), std::min(t_arr[2], t_arr[3])), std::min(t_arr[4], t_arr[5]));
        scalar tmax = std::min(std::min(std::max(t_arr[0], t_arr[1]), std::max(t_arr[2], t_arr[3])), std::max(t_arr[4], t_arr[5]));
        // if tmax < 0, ray is intersecting AABB
        // but entire AABB is behing it's origin
        if (tmax < 0) return false;
        // if tmin > tmax, ray doesn't intersect AABB
        if (tmin > tmax) return false;
        
        t = tmin;
        if (tmin < 0.0f) t = tmax;
        
        return true;
      }
      
      bool test_ray_sphere(const struct ray &ray, const vec4* points, const shape &shape1, const transform &trans1, scalar &t) {
        // проблема в том что я не знаю как пересекать сплюснутую сферу
        // по идее мы можем передать просто obb, и будет даже довольно точно
        (void)points;
        scalar arr[4];
        trans1.scale.storeu(arr);
        const scalar radius = glm::uintBitsToFloat(shape1.faces_count) * arr[0];
        return test_ray_sphere(ray, {trans1.pos, vec4(radius, radius, radius, 0.0f)}, t);
      }
      
      bool test_ray_tri(const struct ray &ray, const vec4 &vert1, const vec4 &vert2, const vec4 &vert3, scalar &t) {
        const vec4 v0v1 = vert2 - vert1;
        const vec4 v0v2 = vert3 - vert1;
        const vec4 pvec = simd::cross(ray.dir, v0v2);
        const scalar invDet = 1.0f/simd::dot(v0v1, pvec); // скорее всего этот det не совпадает с тем что я вычисляю от нормали и направления

        const vec4 tvec = ray.pos - vert1;
        const scalar u = simd::dot(tvec, pvec) * invDet;
        if (u < 0.0f || u > 1.0f) return false;

        const vec4 qvec = simd::cross(tvec, v0v1);
        const scalar v = simd::dot(ray.dir, qvec) * invDet;
        if (v < 0.0f || u + v > 1.0f) return false;

        t = simd::dot(v0v2, qvec) * invDet;

        if (t < 0.0f) return false;
        return true;
      }
      
      bool test_ray_polygon(const struct ray &ray, const vec4* points, const shape &shape1, const transform &trans1, scalar &t) {
        const vec4 normal = trans1.rot * (points[shape1.offset+shape1.points_count+1] * vec4(1.0f, 1.0f, 1.0f, 0.0f));
        const scalar det = simd::dot(normal, ray.dir);
        if (glm::abs(det) < EPSILON) return false;
        
        const vec4 ref_p = trans1.transform_vector(points[shape1.offset]);
        for (uint32_t i = shape1.offset+1; i < shape1.points_count-1; ++i) {
          const vec4 b = trans1.transform_vector(points[i]);
          const vec4 c = trans1.transform_vector(points[i+1]);
          if (test_ray_tri(ray, ref_p, b, c, t)) return true;
        }
        
        return false;
      }
      
      bool test_ray_convex_hull(const struct ray &ray, const vec4* points, const shape &shape1, const transform &trans1, scalar &t) {
        scalar tE = 0.0f;
        scalar tL = 0.0f;
        for (uint32_t i = shape1.offset+shape1.points_count+1; i < shape1.offset+shape1.points_count+1+shape1.faces_count; ++i) {
          const simd::vec4 normal = trans1.rot * (points[i] * vec4(1.0f, 1.0f, 1.0f, 0.0f));
          scalar arr[4];
          points[i].storeu(arr);
          const uint32_t vertex_index = glm::floatBitsToUint(arr[3]);
          // тут нужно получить вершину, СКОРЕЕ ВСЕГО любую которая лежит на плоскости
          // судя по всему достаточно одной вершины которая лежит на плоскости
          
          // трансформа тут неправильно накладывается походу
          const simd::vec4 vertex = trans1.transform_vector(points[shape1.offset+vertex_index]);
          const scalar N = -simd::dot(ray.pos - vertex, normal);
          const scalar D =  simd::dot(ray.dir, normal);

          // параллельна ли данная нормаль направлению луча?
          if (glm::abs(D) < EPSILON) {
            if (N < 0.0f) return false; // луч не может пересечь объект
            else continue; // луч параллелен одной из плоскостей, но еще может пересечь объект
          }

          const scalar t = N / D;
          // луч входит в объект минуя эту плоскость
          if (D < 0.0f) {
            tE = glm::max(tE, t);
            if (tE > tL) return false; // входим дальше чем выходим, объект не может быть пересечен
          }
          // луч выходит из объекта минуя эту плоскость
          else if (D > 0.0f) {
            tL = glm::max(tL, t);
            if (tL < tE) return false; // выходит раньше чем входит, объект не может быть пересечен
          }
        }

//         point = ray.pos + ray.dir*tE;
        t = tE;
        return true;
      }
      
      scalar angle(const vec4 &a, const vec4 &b) {
        const scalar dotV = simd::dot(a, b);
        const scalar lenSq1 = simd::length2(a);
        const scalar lenSq2 = simd::length2(b);

        return glm::acos(dotV / glm::sqrt(lenSq1 * lenSq2));
      }
      
      scalar one_axis_SAT(const vec4* points, const shape &shape, const transform &trans, const vec4 &axis, const vec4 &axis_point) {
        scalar minFirst = 0.0f, maxFirst = 0.0f, minSecond = 0.0f, maxSecond = 0.0f;
        minSecond = maxSecond = simd::dot(axis, axis_point);
        
        if (shape.type == shape::box) {
          project(axis, trans, points[shape.offset], minFirst, maxFirst);
        } else if (shape.type == shape::sphere) {
          project(axis, trans, glm::floatBitsToUint(shape.faces_count), minFirst, maxFirst);
        } else {
          project(axis, points, trans, shape, minFirst, maxFirst);
        }
        
        vec4 mtv;
        scalar dist;
        if (!overlap(shape.margin, minFirst, maxFirst, minSecond, maxSecond, axis, mtv, dist)) return 100000.0f;
        return dist;
      }
    }
  }
}
