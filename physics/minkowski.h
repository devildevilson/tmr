#ifndef MINKOWSKI_H
#define MINKOWSKI_H

// #define BT_DEBUG_MPR1

#include "transform.h"
#include <cmath>
#include <iostream>
// #include "LinearMath/btAlignedObjectArray.h"

//#define MPR_AVERAGE_CONTACT_POSITIONS
// #define DEBUG_MPR

#ifdef DEBUG_MPR
#include <iostream>
#endif

namespace devils_engine {
  namespace physics {
    namespace mpr {
      using core::vec4;
      using core::scalar;
      using glm::sqrt;
      using std::min;
      using glm::abs;
      using simd::dot;
      using simd::cross;
      
      const uint32_t max_iterations = 1000;
      const scalar tolerance = 1E-6f;
      
      struct collision_description {
        vec4 first_dir;
        int32_t max_gjk_iterations;
        scalar maximum_distance2;
        scalar gjk_rel_error2;
      
        collision_description() : 
          first_dir(0.0f, 1.0f, 0.0f, 0.0f),
          max_gjk_iterations(1000),
          maximum_distance2(1e30f),
          gjk_rel_error2(1.0e-6) {}
          
        virtual ~collision_description() {}
      };

      struct distance_info {
        vec4 point_a;
        vec4 point_b;
        vec4 normal; // b to a
        //vec4 m_distance;
        scalar distance;
      };

      struct _support_t {
        vec4 v;  //!< Support point in minkowski sum
        vec4 v1; //!< Support point in obj1
        vec4 v2; //!< Support point in obj2
      };
      using support_t = _support_t;

      struct _simplex_t {
        support_t ps[4];
        int last; //!< index of last added point
      };
      using simplex_t = _simplex_t;

      inline support_t* simplex_point_w(simplex_t *s, int idx) {
        return &s->ps[idx];
      }

      inline void simplex_set_size(simplex_t *s, int size) {
        s->last = size - 1;
      }
      
      inline void print_vec(const char* name, const vec4 &vec) {
        if (name != nullptr) {
          std::cout << name << " ";
        }
        
        std::cout << "x: " << vec.x << " y: " << vec.y << " z: " << vec.z << " w: " << vec.w << "\n";
      }

      #ifdef DEBUG_MPR
      inline void print_portal_vertex(simplex_t* portal, int index) {
        float arr1[4];
        float arr2[4];
        float arr3[4];
        portal->ps[index].v.storeu(arr1);
        portal->ps[index].v1.storeu(arr2);
        portal->ps[index].v2.storeu(arr3);
        
        std::cout << "portal[" << index << 
          "].v = " << arr1[0] << ", " << arr1[1] << ", " << arr1[2] << 
          ", v1 = " << arr2[0] << ", " << arr2[1] << ", " << arr2[2] << 
          ", v2 = " << arr3[0] << ", " << arr3[1] << ", " << arr3[2] << "\n";
      }
      #endif //DEBUG_MPR

      inline int simplex_size(const simplex_t *s) {
        return s->last + 1;
      }

      inline const support_t* simplex_point(const simplex_t* s, int idx) {
        // here is no check on boundaries
        return &s->ps[idx];
      }

      inline void support_copy(support_t *d, const support_t *s) {
        *d = *s;
      }

      inline void simplex_set(simplex_t *s, size_t pos, const support_t *a) {
        support_copy(s->ps + pos, a);
      }


      inline void simplex_swap(simplex_t *s, size_t pos1, size_t pos2) {
        support_t supp;

        support_copy(&supp, &s->ps[pos1]);
        support_copy(&s->ps[pos1], &s->ps[pos2]);
        support_copy(&s->ps[pos2], &supp);
      }


      inline int is_zero(float val) {
        return abs(val) < FLT_EPSILON;
      }

      inline int eqf(float _a, float _b) {
        float ab;
        float a, b;

        ab = abs(_a - _b);
        if (abs(ab) < FLT_EPSILON) return 1;

        a = abs(_a);
        b = abs(_b);
        
        if (b > a) return ab < FLT_EPSILON * b;
        else return ab < FLT_EPSILON * a;
      }


      inline int vec_eq(const vec4* a, const vec4* b) {
        return simd::all_xyz(simd::equal(*a, *b, EPSILON));
  //       return btMprEq((*a).x(), (*b).x())
  //               && btMprEq((*a).y(), (*b).y())
  //               && btMprEq((*a).z(), (*b).z());
      }

      template <typename T>
      inline void find_origin(const T& a, const T& b, const collision_description& colDesc, support_t *center) {
        center->v1 = a.get_object_world_center();
        center->v2 = b.get_object_world_center();
        center->v = center->v1 - center->v2;
        (void)colDesc;
      }

      inline void vec_set(vec4 *v, float x, float y, float z) {
        *v = vec4(x,y,z,0.0f);
      }

      inline void vec_add(vec4 *v, const vec4 *w) {
        *v += *w;
      }

      inline void vec_copy(vec4 *v, const vec4 *w) {
        *v = *w;
      }

      inline void vec_scale(vec4 *d, float k) {
        *d *= k;
      }

      inline float vec_dot(const vec4 *a, const vec4 *b) {
        return dot(*a,*b);
      }


      inline float vec_len2(const vec4 *v) {
        return vec_dot(v, v);
      }

      inline void vec_normalize(vec4 *d) {
        float k = 1.f / sqrt(vec_len2(d));
        vec_scale(d, k);
      }

      inline void vec_cross(vec4 *d, const vec4 *a, const vec4 *b) {
        *d = cross(*a,*b);
      }


      inline void vec_sub2(vec4 *d, const vec4 *v, const vec4 *w) {
        *d = *v - *w;
      }

      inline void portal_dir(const simplex_t *portal, vec4 *dir) {
        vec4 v2v1, v3v1;
        v2v1 = simplex_point(portal, 2)->v - simplex_point(portal, 1)->v;
        v3v1 = simplex_point(portal, 3)->v - simplex_point(portal, 1)->v;
        *dir = cross(v2v1, v3v1);
        *dir = simd::normalize(*dir);
  //       vec_sub2(&v2v1, &simplex_point(portal, 2)->v, &simplex_point(portal, 1)->v);
  //       vec_sub2(&v3v1, &simplex_point(portal, 3)->v, &simplex_point(portal, 1)->v);
  //       vec_cross(dir, &v2v1, &v3v1);
  //       vec_normalize(dir);
      }

      inline int portal_encapsules_origin(const simplex_t *portal, const vec4 *dir) {
        float d = dot(*dir, simplex_point(portal, 1)->v);
        return is_zero(d) || d > 0.f;
      }

      inline int portal_reach_tolerance(const simplex_t *portal, const support_t *v4, const vec4 *dir) {
        float dv1, dv2, dv3, dv4;
        float dot1, dot2, dot3;

        // find the smallest dot product of dir and {v1-v4, v2-v4, v3-v4}

        dv1 = dot(simplex_point(portal, 1)->v, *dir);
        dv2 = dot(simplex_point(portal, 2)->v, *dir);
        dv3 = dot(simplex_point(portal, 3)->v, *dir);
        dv4 = dot(v4->v, *dir);

        dot1 = dv4 - dv1;
        dot2 = dv4 - dv2;
        dot3 = dv4 - dv3;

        dot1 = min(dot1, dot2);
        dot1 = min(dot1, dot3);

        return eqf(dot1, tolerance) || dot1 < tolerance;
      }

      inline int portal_can_encapsule_origin(const simplex_t *portal, const support_t *v4, const vec4 *dir) {
        float d = dot(v4->v, *dir);
        return is_zero(d) || d > 0.f;
      }

      inline void expand_portal(simplex_t *portal, const support_t *v4) {
        vec4 v4v0 = cross(v4->v, simplex_point(portal, 0)->v);
        float d = dot(simplex_point(portal, 1)->v, v4v0);
        if (d > 0.f) {
          d = dot(simplex_point(portal, 2)->v, v4v0);
          
          if (d > 0.f) simplex_set(portal, 1, v4);
          else simplex_set(portal, 3, v4);
        } else {
          d = dot(simplex_point(portal, 3)->v, v4v0);
          
          if (d > 0.f) simplex_set(portal, 2, v4);
          else simplex_set(portal, 1, v4);
        }
      }
      
      template <typename T>
      inline void support(const T& a, const T& b, const collision_description& colDesc, const vec4& dir, support_t *supp) {
        const vec4 final_vec = simd::normalize(dir);
        vec4 seperatingAxisInA =  final_vec * a.transform().rot;
        vec4 seperatingAxisInB = -final_vec * b.transform().rot;

        vec4 pInA = a.get_local_support_with_margin(seperatingAxisInA);
        vec4 qInB = b.get_local_support_with_margin(seperatingAxisInB);

        supp->v1 = a.transform().transform_vector(pInA);
        supp->v2 = b.transform().transform_vector(qInB);
        supp->v = supp->v1 - supp->v2;
        
//         print_vec("dir", dir);
//         print_vec("seperatingAxisInA", seperatingAxisInA);
//         print_vec("seperatingAxisInB", seperatingAxisInB);
//         print_vec("pInA    ", pInA);
//         print_vec("qInB    ", qInB);
//         print_vec("supp->v1", supp->v1);
//         print_vec("supp->v2", supp->v2);
//         print_vec("supp->v ", supp->v);
        
//         ASSERT(supp->v.w == 0.0f);
        (void)colDesc;
      }


      template <typename T>
      static int discover_portal(const T& a, const T& b, const collision_description& colDesc, simplex_t *portal) {
        vec4 dir, va, vb;
        float d;
        int cont;
      
        // vertex 0 is center of portal
        find_origin(a,b,colDesc, simplex_point_w(portal, 0));

        // vertex 0 is center of portal
        simplex_set_size(portal, 1);
        
        vec4 zero = vec4(0,0,0, 0.0f);
        vec4* org = &zero;

        if (vec_eq(&simplex_point(portal, 0)->v, org)){
            // Portal's center lies on origin (0,0,0) => we know that objects
            // intersect but we would need to know penetration info.
            // So move center little bit...
            vec_set(&va, FLT_EPSILON * 10.f, 0.f, 0.f);
            simplex_point_w(portal, 0)->v += va;
        }

        // vertex 1 = support in direction of origin
        vec_copy(&dir, &simplex_point(portal, 0)->v);
        vec_scale(&dir, -1.f);
        vec_normalize(&dir);

        support(a,b,colDesc, dir, simplex_point_w(portal, 1));
    
        simplex_set_size(portal, 2);

        // test if origin isn't outside of v1
        d = dot(simplex_point(portal, 1)->v, dir);

        if (is_zero(d) || d < 0.f) return -1;

        // vertex 2
        dir = cross(simplex_point(portal, 0)->v, simplex_point(portal, 1)->v);
        if (iszero(vec_len2(&dir))) {
          // origin lies on v1
          if (vec_eq(&simplex_point(portal, 1)->v, org)) return 1;
          // origin lies on v0-v1 segment
          else return 2;
        }

        vec_normalize(&dir);
        support(a,b,colDesc, dir, simplex_point_w(portal, 2));

        d = dot(simplex_point(portal, 2)->v, dir);
        if (is_zero(d) || d < 0.f) return -1;

        simplex_set_size(portal, 3);

        // vertex 3 direction
        va = simplex_point(portal, 1)->v - simplex_point(portal, 0)->v;
        vb = simplex_point(portal, 2)->v - simplex_point(portal, 0)->v;
        
        dir = cross(va, vb);
        dir = simd::normalize(dir);

        // it is better to form portal faces to be oriented "outside" origin
        d = dot(dir, simplex_point(portal, 0)->v);
        if (d > 0.f){
          simplex_swap(portal, 1, 2);
          vec_scale(&dir, -1.f);
        }

        while (simplex_size(portal) < 4) {
          support(a,b,colDesc, dir, simplex_point_w(portal, 3));
              
          d = dot(simplex_point(portal, 3)->v, dir);
          if (is_zero(d) || d < 0.f) return -1;

          cont = 0;

          // test if origin is outside (v1, v0, v3) - set v2 as v3 and
          // continue
          va = cross(simplex_point(portal, 1)->v, simplex_point(portal, 3)->v);
          d = dot(va, simplex_point(portal, 0)->v);
          if (d < 0.f && !is_zero(d)) {
            simplex_set(portal, 2, simplex_point(portal, 3));
            cont = 1;
          }

          if (!cont) {
            // test if origin is outside (v3, v0, v2) - set v1 as v3 and
            // continue
            va = cross(simplex_point(portal, 3)->v, simplex_point(portal, 2)->v);
            d = dot(va, simplex_point(portal, 0)->v);
            if (d < 0.f && !is_zero(d)){
              simplex_set(portal, 1, simplex_point(portal, 3));
              cont = 1;
            }
          }

          if (cont) {
            va = simplex_point(portal, 1)->v - simplex_point(portal, 0)->v;
            vb = simplex_point(portal, 2)->v - simplex_point(portal, 0)->v;
            vec_cross(&dir, &va, &vb);
            vec_normalize(&dir);
          } else {
            simplex_set_size(portal, 4);
          }
        }

        return 0;
      }

      template <typename T>
      static int refine_portal(const T& a, const T& b,const collision_description& colDesc, simplex_t *portal) {
        vec4 dir;
        support_t v4;
        
        const uint32_t iters = colDesc.max_gjk_iterations < 1 || uint32_t(abs(colDesc.max_gjk_iterations)) > max_iterations ? max_iterations : colDesc.max_gjk_iterations;
        for (uint32_t i = 0; i < iters; i++) {
          // compute direction outside the portal (from v0 throught v1,v2,v3
          // face)
          portal_dir(portal, &dir);

          // test if origin is inside the portal
          if (portal_encapsules_origin(portal, &dir)) return 0;

          // get next support point
          support(a,b,colDesc, dir, &v4);

          // test if v4 can expand portal to contain origin and if portal
          // expanding doesn't reach given tolerance
          if (!portal_can_encapsule_origin(portal, &v4, &dir) || portal_reach_tolerance(portal, &v4, &dir)) return -1;

          // v1-v2-v3 triangle must be rearranged to face outside Minkowski
          // difference (direction from v0).
          expand_portal(portal, &v4);
        }

        return -1;
      }

      static void find_pos(const simplex_t *portal, vec4 *pos) {
        vec4 zero = vec4(0,0,0,0);
        vec4* origin = &zero;

        vec4 dir;
        size_t i;
        float b[4], sum, inv;
        vec4 vec, p1, p2;

        portal_dir(portal, &dir);

        // use barycentric coordinates of tetrahedron to find origin
        vec = cross(simplex_point(portal, 1)->v, simplex_point(portal, 2)->v);
        b[0] = dot(vec, simplex_point(portal, 3)->v);
        
        vec = cross(simplex_point(portal, 3)->v, simplex_point(portal, 2)->v);
        b[1] = dot(vec, simplex_point(portal, 0)->v);
        
        vec = cross(simplex_point(portal, 0)->v, simplex_point(portal, 1)->v);
        b[2] = dot(vec, simplex_point(portal, 3)->v);
        
        vec = cross(simplex_point(portal, 2)->v, simplex_point(portal, 1)->v);
        b[3] = dot(vec, simplex_point(portal, 0)->v);

        sum = b[0] + b[1] + b[2] + b[3];

        if (is_zero(sum) || sum < 0.f) {
          b[0] = 0.f;
          
          vec = cross(simplex_point(portal, 2)->v, simplex_point(portal, 3)->v);
          b[1] = vec_dot(&vec, &dir);
          
          vec = cross(simplex_point(portal, 3)->v, simplex_point(portal, 1)->v);
          b[2] = vec_dot(&vec, &dir);
          
          vec = cross(simplex_point(portal, 1)->v, simplex_point(portal, 2)->v);
          b[3] = vec_dot(&vec, &dir);

          sum = b[1] + b[2] + b[3];
        }

        inv = 1.f / sum;

        vec_copy(&p1, origin);
        vec_copy(&p2, origin);
        
        for (i = 0; i < 4; i++) {
          vec_copy(&vec, &simplex_point(portal, i)->v1);
          vec_scale(&vec, b[i]);
          vec_add(&p1, &vec);

          vec_copy(&vec, &simplex_point(portal, i)->v2);
          vec_scale(&vec, b[i]);
          vec_add(&p2, &vec);
        }
        
        vec_scale(&p1, inv);
        vec_scale(&p2, inv);
    #ifdef MPR_AVERAGE_CONTACT_POSITIONS
        vec_copy(pos, &p1);
        vec_add(pos, &p2);
        vec_scale(pos, 0.5);
    #else
        vec_copy(pos, &p2);
    #endif//MPR_AVERAGE_CONTACT_POSITIONS
      }

      inline float vec_dist2(const vec4 *a, const vec4 *b) {
        vec4 ab;
        vec_sub2(&ab, a, b);
        return vec_len2(&ab);
      }

      inline float _vec_point_segment_dist2(const vec4 *P, const vec4 *x0, const vec4 *b, vec4 *witness) {
          // The computation comes from solving equation of segment:
          //      S(t) = x0 + t.d
          //          where - x0 is initial point of segment
          //                - d is direction of segment from x0 (|d| > 0)
          //                - t belongs to <0, 1> interval
          // 
          // Than, distance from a segment to some point P can be expressed:
          //      D(t) = |x0 + t.d - P|^2
          //          which is distance from any point on segment. Minimization
          //          of this function brings distance from P to segment.
          // Minimization of D(t) leads to simple quadratic equation that's
          // solving is straightforward.
          //
          // Bonus of this method is witness point for free.

          float dist, t;
          vec4 d, a;
          const vec4 final_x0 = (*x0)*vec4(1,1,1,0);
          const vec4 final_b = (*b)*vec4(1,1,1,0);

          // direction of segment
          vec_sub2(&d, b, x0);

          // precompute vector from P to x0
          vec_sub2(&a, x0, P);

          t  = -1.f * vec_dot(&a, &d);
          t /= vec_len2(&d);

          if (t < 0.f || is_zero(t)) {
            dist = vec_dist2(x0, P);
            if (witness) vec_copy(witness, &final_x0);
            
          } else if (t > 1.f || eqf(t, 1.f)) {
            dist = vec_dist2(b, P);
            if (witness) vec_copy(witness, &final_b);
            
          } else {
            if (witness) {
              vec_copy(witness, &d);
              vec_scale(witness, t);
              //vec_add(witness, x0);
              vec_add(witness, &final_x0);
              dist = vec_dist2(witness, P);
            } else {
              // recycling variables
              vec_scale(&d, t);
              vec_add(&d, &a);
              dist = vec_len2(&d);
            }
          }

          return dist;
      }

      inline float vec_point_tri_dist2(const vec4 *P, const vec4 *x0, const vec4 *B, const vec4 *C, vec4 *witness) {
        // Computation comes from analytic expression for triangle (x0, B, C)
        //      T(s, t) = x0 + s.d1 + t.d2, where d1 = B - x0 and d2 = C - x0 and
        // Then equation for distance is:
        //      D(s, t) = | T(s, t) - P |^2
        // This leads to minimization of quadratic function of two variables.
        // The solution from is taken only if s is between 0 and 1, t is
        // between 0 and 1 and t + s < 1, otherwise distance from segment is
        // computed.

        vec4 d1, d2, a;
        float u, v, w, p, q, r;
        float s, t, dist, dist2;
        vec4 witness2;

        vec_sub2(&d1, B, x0);
        vec_sub2(&d2, C, x0);
        vec_sub2(&a, x0, P);

        u = vec_dot(&a, &a);
        v = vec_dot(&d1, &d1);
        w = vec_dot(&d2, &d2);
        p = vec_dot(&a, &d1);
        q = vec_dot(&a, &d2);
        r = vec_dot(&d1, &d2);

        scalar div = (w * v - r * r);
        if (is_zero(div)) {
          s=-1;
        } else {
          s = (q * r - w * p) / div;
          t = (-s * r - q) / w;
        }

        if ((is_zero(s) || s > 0.f) && (eqf(s, 1.f) || s < 1.f) && (is_zero(t) || t > 0.f) && (eqf(t, 1.f) || t < 1.f) && (eqf(t + s, 1.f) || t + s < 1.f)) {
          if (witness) {
            vec_scale(&d1, s);
            vec_scale(&d2, t);
            const vec4 final_x0 = (*x0)*vec4(1,1,1,0);
            vec_copy(witness, &final_x0);
            vec_add(witness, &d1);
            vec_add(witness, &d2);

            dist = vec_dist2(witness, P);
          } else {
            dist  = s * s * v;
            dist += t * t * w;
            dist += 2.f * s * t * r;
            dist += 2.f * s * p;
            dist += 2.f * t * q;
            dist += u;
          }
        } else {
          dist = _vec_point_segment_dist2(P, x0, B, witness);
          dist2 = _vec_point_segment_dist2(P, x0, C, &witness2);
          
          if (dist2 < dist){
            dist = dist2;
            if (witness) vec_copy(witness, &witness2);
          }

          dist2 = _vec_point_segment_dist2(P, B, C, &witness2);
          if (dist2 < dist){
            dist = dist2;
            if (witness) vec_copy(witness, &witness2);
          }
        }

        return dist;
      }

      template <typename T>
      static void find_penetr(const T& a, const T& b, const collision_description& colDesc, simplex_t *portal, float *depth, vec4 *pdir, vec4 *pos) {
        vec4 dir;
        support_t v4;
        unsigned long iterations;

        vec4 zero = vec4(0,0,0,0);
        vec4* origin = &zero;


        iterations = 1UL;
        const uint32_t iters = colDesc.max_gjk_iterations < 1 || uint32_t(abs(colDesc.max_gjk_iterations)) > max_iterations ? max_iterations : colDesc.max_gjk_iterations;
        for (uint32_t i = 0; i < iters; i++) {
          // compute portal direction and obtain next support point
          portal_dir(portal, &dir);
              
          support(a,b,colDesc, dir, &v4);

          // reached tolerance -> find penetration info
          if (portal_reach_tolerance(portal, &v4, &dir) || iterations == max_iterations) {
            *depth = vec_point_tri_dist2(origin,&simplex_point(portal, 1)->v,&simplex_point(portal, 2)->v,&simplex_point(portal, 3)->v, pdir);
            *depth = sqrt(*depth);
            
            float arr[4];
            pdir->storeu(arr);
            if (is_zero(arr[0]) && is_zero(arr[1]) && is_zero(arr[2])) *pdir = dir;
            vec_normalize(pdir);
        
            // barycentric coordinates:
            find_pos(portal, pos);
            return;
          }

          expand_portal(portal, &v4);

          iterations++;
        }
      }

      static void find_penetr_touch(simplex_t *portal,float *depth, vec4 *dir, vec4 *pos) {
        // Touching contact on portal's v1 - so depth is zero and direction
        // is unimportant and pos can be guessed
        *depth = 0.f;
        vec4 zero = vec4(0,0,0,0);
        vec4* origin = &zero;

        vec_copy(dir, origin);
  #ifdef MPR_AVERAGE_CONTACT_POSITIONS
        vec_copy(pos, &simplex_point(portal, 1)->v1);
        vec_add(pos, &simplex_point(portal, 1)->v2);
        vec_scale(pos, 0.5);
  #else
        vec_copy(pos, &simplex_point(portal, 1)->v2);
  #endif
      }

      static void find_penetr_segment(simplex_t *portal, float *depth, vec4 *dir, vec4 *pos) {
        // Origin lies on v0-v1 segment.
        // Depth is distance to v1, direction also and position must be
        // computed
  #ifdef MPR_AVERAGE_CONTACT_POSITIONS
        vec_copy(pos, &simplex_point(portal, 1)->v1);
        vec_add(pos, &simplex_point(portal, 1)->v2);
        vec_scale(pos, 0.5f);
  #else
        vec_copy(pos, &simplex_point(portal, 1)->v2);
  #endif//MPR_AVERAGE_CONTACT_POSITIONS

//         const vec4 test = simplex_point(portal, 1)->v;
//         std::cout << "simplex_point(portal, 1)->v x: " << test.x << " y: " << test.y << " z: " << test.z << " w: " << test.w << "\n";
//         ASSERT(simplex_point(portal, 1)->v.w == 0.0f);
        
        vec_copy(dir, &simplex_point(portal, 1)->v);
        *depth = sqrt(vec_len2(dir));
        vec_normalize(dir);
      }


      template <typename T>
      inline int penetration(const T& a, const T& b, const collision_description& colDesc, float *depthOut, vec4* dirOut, vec4* posOut) {
        simplex_t portal;
        // Phase 1: Portal discovery
        int result = discover_portal(a,b,colDesc, &portal);

        //sepAxis[pairIndex] = *pdir;//or -dir?

        switch (result) {
          case 0: {
            // Phase 2: Portal refinement
          
            result = refine_portal(a,b,colDesc, &portal);
            if (result < 0) return -1;

            // Phase 3. Penetration info
            find_penetr(a,b,colDesc, &portal, depthOut, dirOut, posOut);
//             ASSERT(dirOut->w == 0.0f);
            break;
          }
          case 1: {
            // Touching contact on portal's v1.
            find_penetr_touch(&portal, depthOut, dirOut, posOut);
//             ASSERT(dirOut->w == 0.0f);
            result=0;
            break;
          }
          case 2: {
            find_penetr_segment( &portal, depthOut, dirOut, posOut);
//             ASSERT(dirOut->w == 0.0f);
            result=0;
            break;
          }
          default: {
            //if (res < 0) {
            // Origin isn't inside portal - no collision.
              result = -1;
            //}
          }
        };
        
        return result;
      };


      template<typename T, typename D>
      inline int compute_penetration(const T& a, const T& b, const collision_description& col_Desc, D* dist_info) {
        vec4 dir,pos;
        float depth;

        int res = penetration(a,b,col_Desc,&depth, &dir, &pos);
        if (res == 0) {
          dist_info->distance = -depth;
          dist_info->point_b = pos;
          dist_info->normal = -dir;
          dist_info->point_a = pos - dist_info->distance*dir;
          return 0;
        }

        return -1;
      }
    }
  }
}

#endif
