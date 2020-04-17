#ifndef COLLISION_UTILS_H
#define COLLISION_UTILS_H

#include "physics_core.h"
#include "collision_shape.h"
#include "transform.h"

namespace devils_engine {
  namespace physics {
    namespace collision {
      using aabb = core::aabb;
      using sphere = core::sphere;
      using vec4 = core::vec4;
      using scalar = core::scalar;
      using core::transform;
      using support_func = vec4(const vec4*, const shape &, const transform &, const vec4 &);
      using ray_test_func = bool(const struct ray &, const vec4*, const shape &, const transform &, float &);
      using object_sat_func = bool(const vec4* points, const shape &shape1, const transform &trans1, const shape &shape2, const transform &trans2, vec4 &mtv, scalar &dist);
      
      struct return_data {
        vec4 point_a;
        vec4 point_b;
        vec4 normal;
        scalar distance;
      };
      
      bool test_aabb(const aabb &box1, const aabb &box2);
      bool contain_aabb(const aabb &container, const aabb &box);
      bool test_sphere(const sphere &sphere1, const sphere &sphere2);
      bool test_ray_aabb(const struct ray &ray, const aabb &box, float &t);
      bool test_ray_sphere(const struct ray &ray, const sphere &sphere, float &t);
      uint32_t test_frustum_aabb(const struct frustum &frustum, const aabb &box);
      uint32_t test_frustum_sphere(const struct frustum &frustum, const sphere &sphere);
      
      bool test_objects_SAT(const vec4* points, const shape &shape1, const transform &trans1, const shape &shape2, const transform &trans2, vec4 &mtv, scalar &dist);
      bool test_box_box_SAT(const vec4* points, const shape &shape1, const transform &trans1, const shape &shape2, const transform &trans2, vec4 &mtv, scalar &dist);
      bool test_box_sphere_SAT(const vec4* points, const shape &shape1, const transform &trans1, const shape &shape2, const transform &trans2, vec4 &mtv, scalar &dist);
      bool test_box_polygon_SAT(const vec4* points, const shape &shape1, const transform &trans1, const shape &shape2, const transform &trans2, vec4 &mtv, scalar &dist);
      bool test_box_convex_hull_SAT(const vec4* points, const shape &shape1, const transform &trans1, const shape &shape2, const transform &trans2, vec4 &mtv, scalar &dist);
      
//       bool test_sphere_box_SAT(const vec4* points, const shape &shape1, const transform &trans1, const shape &shape2, const transform &trans2, vec4 &mtv, scalar &dist);
      bool test_sphere_sphere_SAT(const vec4* points, const shape &shape1, const transform &trans1, const shape &shape2, const transform &trans2, vec4 &mtv, scalar &dist);
      bool test_sphere_polygon_SAT(const vec4* points, const shape &shape1, const transform &trans1, const shape &shape2, const transform &trans2, vec4 &mtv, scalar &dist);
      bool test_sphere_convex_hull_SAT(const vec4* points, const shape &shape1, const transform &trans1, const shape &shape2, const transform &trans2, vec4 &mtv, scalar &dist);
      
//       bool test_polygon_box_SAT(const vec4* points, const shape &shape1, const transform &trans1, const shape &shape2, const transform &trans2, vec4 &mtv, scalar &dist);
//       bool test_polygon_sphere_SAT(const vec4* points, const shape &shape1, const transform &trans1, const shape &shape2, const transform &trans2, vec4 &mtv, scalar &dist);
      bool test_polygon_polygon_SAT(const vec4* points, const shape &shape1, const transform &trans1, const shape &shape2, const transform &trans2, vec4 &mtv, scalar &dist);
      bool test_polygon_convex_hull_SAT(const vec4* points, const shape &shape1, const transform &trans1, const shape &shape2, const transform &trans2, vec4 &mtv, scalar &dist);
      
//       bool test_convex_hull_box_SAT(const vec4* points, const shape &shape1, const transform &trans1, const shape &shape2, const transform &trans2, vec4 &mtv, scalar &dist);
//       bool test_convex_hull_sphere_SAT(const vec4* points, const shape &shape1, const transform &trans1, const shape &shape2, const transform &trans2, vec4 &mtv, scalar &dist);
//       bool test_convex_hull_polygon_SAT(const vec4* points, const shape &shape1, const transform &trans1, const shape &shape2, const transform &trans2, vec4 &mtv, scalar &dist);
      bool test_convex_hull_convex_hull_SAT(const vec4* points, const shape &shape1, const transform &trans1, const shape &shape2, const transform &trans2, vec4 &mtv, scalar &dist);
      
      bool test_objects_mpr(const vec4* points, const shape &shape1, const transform &trans1, const shape &shape2, const transform &trans2, return_data& data);
      
      vec4 box_support_func(const vec4* points, const shape &shape1, const transform &trans1, const vec4 &axis);
      vec4 sphere_support_func(const vec4* points, const shape &shape1, const transform &trans1, const vec4 &axis);
      vec4 polygon_support_func(const vec4* points, const shape &shape1, const transform &trans1, const vec4 &axis);
      vec4 convex_hull_support_func(const vec4* points, const shape &shape1, const transform &trans1, const vec4 &axis);
      
      bool test_ray_object(const struct ray &ray, const vec4* points, const shape &shape1, const transform &trans1, float &t);
      bool test_ray_box(const struct ray &ray, const vec4* points, const shape &shape1, const transform &trans1, float &t);
      bool test_ray_sphere(const struct ray &ray, const vec4* points, const shape &shape1, const transform &trans1, float &t);
      bool test_ray_polygon(const struct ray &ray, const vec4* points, const shape &shape1, const transform &trans1, float &t);
      bool test_ray_convex_hull(const struct ray &ray, const vec4* points, const shape &shape1, const transform &trans1, float &t);
      
      scalar angle(const vec4 &a, const vec4 &b);
      scalar one_axis_SAT(const vec4* points, const shape &shape, const transform &trans, const vec4 &axis, const vec4 &axis_point);
    }
  }
}

#endif
