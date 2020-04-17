#include "broadphase_parts.h"

#include "broadphase_context.h"
#include "physics_context.h"
#include "physics_core.h"
#include "collision_shape.h"
#include "collision_utils.h"
#include "works_utils.h"
#include <stdexcept>

namespace devils_engine {
  namespace physics {
    namespace broadphase {
      compute_aabb_parallel::compute_aabb_parallel(dt::thread_pool* pool) : pool(pool) {}
      void compute_aabb_parallel::begin(octree_context* ctx) { (void)ctx; }
      void compute_aabb_parallel::process(octree_context* ctx, const size_t &delta_time) {
        static const auto func = [] (const size_t &start, const size_t &count, octree_context* ctx) {
          for (size_t i = start; i < start+count; ++i) {
            auto &proxy = ctx->proxies[i];
            if (proxy.object_index == UINT32_MAX) continue;
            if (proxy.proxy_flags.static_object() && !proxy.proxy_flags.need_updation()) continue;
            
            const auto &body = ctx->context->bodies.at(proxy.object_index);
//             const auto &trans = ctx->context->transforms->at(body.transform_index);
            const auto &trans = ctx->context->new_transforms[body.transform_index];
            const auto &shape = ctx->context->shapes.at(body.shape_index);
            
            const core::aabb box = shape.get_aabb(ctx->context->points.data(), trans);
            proxy.box = {box.center, box.extents};
          }
        };
        
        (void)delta_time;
        utils::submit_works(pool, ctx->proxies.size(), func, ctx); 
      }
      
      void compute_aabb_parallel::clear() {}
      
      update_octree_parallel::update_octree_parallel(dt::thread_pool* pool) : pool(pool) {}
      void update_octree_parallel::begin(octree_context* ctx) { (void)ctx; }
      void update_octree_parallel::process(octree_context* ctx, const size_t &delta_time) {
        static const auto func = [] (const size_t &start, const size_t &count, octree_context* ctx) {
          for (size_t i = start; i < start+count; ++i) {
            auto &proxy = ctx->proxies[i];
            if (proxy.object_index == UINT32_MAX) continue;
            if (proxy.proxy_flags.static_object() && !proxy.proxy_flags.need_updation()) continue;
            
            const collision::aabb proxy_box = {proxy.box.center.get_simd(), proxy.box.extents.get_simd()};
            bool in_node = true;
            size_t current_node = 0;
            const collision::aabb node_box = {ctx->nodes[0].box.center.get_simd(), ctx->nodes[0].box.extents.get_simd()};
            if (!collision::contain_aabb(node_box, proxy_box)) throw std::runtime_error("Object "+std::to_string(proxy.object_index)+" not in octree");
            
            while (in_node) {
              bool child_node = false;
              if (ctx->nodes[current_node].next_level_index == UINT32_MAX) break;
              for (uint32_t j = ctx->nodes[current_node].next_level_index; j < ctx->nodes[current_node].next_level_index+8; ++j) {
                const collision::aabb node_box = {ctx->nodes[j].box.center.get_simd(), ctx->nodes[j].box.extents.get_simd()};
                if (collision::contain_aabb(proxy_box, node_box)) {
                  current_node = j;
                  child_node = true;
                  break;
                }
              }
              
              in_node = in_node && child_node;
            }
            
            if (proxy.node_index != UINT32_MAX) ctx->nodes[proxy.node_index].remove_proxy(proxy.node_array_index);
            proxy.node_array_index = ctx->nodes[current_node].add_proxy(i);
            proxy.node_index = current_node;
            
            if (proxy.proxy_flags.static_object()) proxy.proxy_flags.need_updation(false);
          }
        };
        
        (void)delta_time;
        utils::submit_works(pool, ctx->proxies.size(), func, ctx); 
      }
      
      void update_octree_parallel::clear() {}
      
      compute_pairs_parallel::compute_pairs_parallel(dt::thread_pool* pool) : pool(pool) {}
      void compute_pairs_parallel::begin(octree_context* ctx) { (void)ctx; }
      void compute_pairs_parallel::process(octree_context* ctx, const size_t &delta_time) {
        (void)delta_time;
        
        const size_t pairs_count_prediction = ctx->context->bodies.size()*2;
        if (ctx->pairs.size() < pairs_count_prediction) {
          ctx->pairs.resize(pairs_count_prediction);
        }
        
        std::atomic<uint32_t> counter(0);
//         std::mutex mutex;
        utils::submit_works(pool, ctx->proxies.size(), calc_pairs, ctx, std::ref(counter));
        ctx->pairs_count = counter;
      }
      
      void compute_pairs_parallel::clear() {}
      
      void compute_pairs_parallel::calc_pairs(const size_t &start, const size_t &count, octree_context* ctx, std::atomic<uint32_t> &pair_counter) {
        for (size_t i = start; i < start+count; ++i) {
          const auto &proxy = ctx->proxies[i];
          if (proxy.object_index == UINT32_MAX) continue;
          if (proxy.proxy_flags.static_object()) continue;
          // некоторые объекты не обязательно обходить (спящие объекты)
          if (proxy.proxy_flags.is_sleeping()) continue;
          
          const auto &node = ctx->nodes[0];
          const collision::aabb proxy_box = {proxy.box.center.get_simd(), proxy.box.extents.get_simd()};
          const collision::aabb node_box = {ctx->nodes[0].box.center.get_simd(), ctx->nodes[0].box.extents.get_simd()};
          if (!collision::contain_aabb(node_box, proxy_box)) throw std::runtime_error("Object "+std::to_string(proxy.object_index)+" not in octree");
          
          for (size_t j = 0; j < node.proxy_indices.size(); ++j) {
            const uint32_t index = node.proxy_indices[j];
            if (index == UINT32_MAX || index == i) continue;
            check_and_add(i, index, ctx, pair_counter);
          }
          
          // здесь разбиение задач добавит просто миллион задач (у каждого объекта +8)
          for (size_t j = node.next_level_index; j < node.next_level_index+8; ++j) {
            const collision::aabb node_box = {ctx->nodes[j].box.center.get_simd(), ctx->nodes[j].box.extents.get_simd()};
            if (collision::test_aabb(proxy_box, node_box)) calc_pairs_rec(ctx, i, j, pair_counter);
          }
        }
      }
      
      void compute_pairs_parallel::calc_pairs_rec(octree_context* ctx, const size_t &proxy_index, const uint32_t &node_index, std::atomic<uint32_t> &pair_counter) {
        const auto &proxy = ctx->proxies[proxy_index];
        const auto &node = ctx->nodes[node_index];
        
        for (size_t j = 0; j < node.proxy_indices.size(); ++j) {
          const uint32_t index = node.proxy_indices[j];
          if (index == UINT32_MAX || index == proxy_index) continue;
          check_and_add(proxy_index, index, ctx, pair_counter);
        }
        
        if (node.next_level_index == UINT32_MAX) return;
        
        const collision::aabb proxy_box = {proxy.box.center.get_simd(), proxy.box.extents.get_simd()};
        for (size_t j = node.next_level_index; j < node.next_level_index+8; ++j) {
          const collision::aabb node_box = {ctx->nodes[j].box.center.get_simd(), ctx->nodes[j].box.extents.get_simd()};
          if (collision::test_aabb(proxy_box, node_box)) calc_pairs_rec(ctx, proxy_index, j, pair_counter);
        }
      }
      
      void compute_pairs_parallel::check_and_add(const uint32_t &proxy1_index, const uint32_t &proxy2_index, octree_context* ctx, std::atomic<uint32_t> &pair_counter) {
        const auto &proxy1 = ctx->proxies[proxy1_index];
        const auto &proxy2 = ctx->proxies[proxy2_index];
        
        const bool collide = (proxy1.collision_group & proxy2.collision_filter) != 0 &&
                             (proxy2.collision_group & proxy1.collision_filter) != 0;
                         
        const bool trigger = (proxy1.collision_group & proxy2.collision_trigger) != 0 &&
                             (proxy2.collision_group & proxy1.collision_trigger) != 0;
        
        if (proxy1.proxy_flags.static_object() && proxy1.proxy_flags.static_object()) return;
        if (!collide && !trigger) return;
        
        const collision::aabb box1 = {proxy1.box.center.get_simd(), proxy1.box.extents.get_simd()};
        const collision::aabb box2 = {proxy2.box.center.get_simd(), proxy2.box.extents.get_simd()};
        
        if (collision::test_aabb(box1, box2)) {
          const uint32_t &index = pair_counter.fetch_add(1);
          
//           std::cout << "proxy1 obj " << proxy1.object_index << "\n";
//           std::cout << "proxy2 obj " << proxy2.object_index << "\n";
          
          ASSERT(index < ctx->pairs.size());
          ctx->pairs[index].obj1 = proxy1.object_index;
          ctx->pairs[index].obj2 = proxy2.object_index;
          ctx->pairs[index].dist2 = simd::distance2(box1.center, box2.center);
          ctx->pairs[index].pair_flags = octree_context::pair::flags(
            (uint32_t(!collide && trigger) * octree_context::pair::flags::ONLY_TRIGGER_PAIR) | 
            (uint32_t(proxy1.proxy_flags.static_object() || proxy1.proxy_flags.static_object()) * octree_context::pair::flags::STATIC_PAIR)
          );
          
          ASSERT((!collide && trigger) == ctx->pairs[index].pair_flags.trigger_only());
          ASSERT((proxy1.proxy_flags.static_object() || proxy1.proxy_flags.static_object()) == ctx->pairs[index].pair_flags.static_pair());
        }
      }
      
      cast_rays_parallel::cast_rays_parallel(dt::thread_pool* pool) : pool(pool) {}
      void cast_rays_parallel::begin(octree_context* ctx) { (void)ctx; }
      void cast_rays_parallel::process(octree_context* ctx, const size_t &delta_time) {
        (void)delta_time;
        const size_t rays_count = ctx->context->precision_rays.size() + ctx->context->rays.size();
        utils::submit_works(pool, rays_count, calc_rays, ctx, pool);
      }
      
      void cast_rays_parallel::clear() {}
      void cast_rays_parallel::calc_rays(const size_t &start, const size_t &count, octree_context* ctx, dt::thread_pool* pool) {
        for (size_t i = start; i < start+count; ++i) {
          const size_t final_index = i >= ctx->context->precision_rays.size() ? i - ctx->context->precision_rays.size() : i;
          const auto &ray = i >= ctx->context->precision_rays.size() ? ctx->context->rays[final_index] : ctx->context->precision_rays[final_index];
          const auto &node = ctx->nodes[0];
          const collision::aabb node_box = {ctx->nodes[0].box.center.get_simd(), ctx->nodes[0].box.extents.get_simd()};
          float t;
          if (!collision::test_ray_aabb(ray, node_box, t)) throw std::runtime_error("Ray does not collide octree");
          
          std::mutex mutex;
          for (size_t j = 0; j < node.proxy_indices.size(); ++j) {
            const uint32_t index = node.proxy_indices[j];
            if (index == UINT32_MAX) continue;
            check_and_add(ctx, i, index, mutex);
          }
          
          for (size_t j = node.next_level_index; j < node.next_level_index+8; ++j) {
            const collision::aabb node_box = {ctx->nodes[j].box.center.get_simd(), ctx->nodes[j].box.extents.get_simd()};
            float t;
            if (collision::test_ray_aabb(ray, node_box, t)) {
              pool->submitbase([ctx, i, j, &mutex] () {
                calc_rays_rec(ctx, i, j, mutex);
              });
            }
          }
        }
      }
      
      void cast_rays_parallel::calc_rays_rec(octree_context* ctx, const uint32_t &ray_index, const uint32_t &node_index, std::mutex &mutex) {
        const size_t final_index = ray_index >= ctx->context->precision_rays.size() ? ray_index - ctx->context->precision_rays.size() : ray_index;
        const auto &ray = ray_index >= ctx->context->precision_rays.size() ? ctx->context->rays[final_index] : ctx->context->precision_rays[final_index];
        const auto &node = ctx->nodes[node_index];
        
        for (size_t j = 0; j < node.proxy_indices.size(); ++j) {
          const uint32_t index = node.proxy_indices[j];
          if (index == UINT32_MAX) continue;
          check_and_add(ctx, ray_index, index, mutex);
        }
        
        if (node.next_level_index == UINT32_MAX) return;
        
        for (size_t j = node.next_level_index; j < node.next_level_index+8; ++j) {
          const collision::aabb node_box = {ctx->nodes[j].box.center.get_simd(), ctx->nodes[j].box.extents.get_simd()};
          float t;
          if (collision::test_ray_aabb(ray, node_box, t)) calc_rays_rec(ctx, ray_index, j, mutex);
        }
      }
      
      void cast_rays_parallel::check_and_add(octree_context* ctx, const uint32_t &ray_index, const uint32_t &proxy_index, std::mutex &mutex) {
        const size_t final_index = ray_index >= ctx->context->precision_rays.size() ? ray_index - ctx->context->precision_rays.size() : ray_index;
        const auto &ray = ray_index >= ctx->context->precision_rays.size() ? ctx->context->rays[final_index] : ctx->context->precision_rays[final_index];
        const auto &proxy = ctx->proxies[proxy_index];
        
        const uint32_t ignore_obj_index = ray.ignore_obj();
        if (proxy.object_index == ignore_obj_index) return;
        
        const uint32_t collision_filter = ray.collision_filter();
        if ((collision_filter & proxy.collision_group) == 0) return;
        
        //const uint32_t ret_index = ray.ret_index();
        const uint32_t ret_index = ray_index;
        
        const collision::aabb box = {proxy.box.center.get_simd(), proxy.box.extents.get_simd()};
        
        float t;
        if (!collision::test_ray_aabb(ray, box, t)) return;
        
        if (ray_index < ctx->context->precision_rays.size()) {
          const auto &body   = ctx->context->bodies[proxy.object_index];
          const auto &shape  = ctx->context->shapes[body.shape_index];
//           const auto &trans  = ctx->context->transforms->at(body.transform_index);
          const auto &trans = ctx->context->new_transforms[body.transform_index];
          const auto* points = ctx->context->points.data();
          
          if (!collision::test_ray_object(ray, points, shape, trans, t)) return;
        } 
        
        // нужно проверить объект с трансформой на пересечение с лучем
        std::unique_lock<std::mutex> lock(mutex);
        if (ctx->context->ray_test_data[ret_index].dist > t) {
          ctx->context->ray_test_data[ret_index].ray_index = ray_index;
          ctx->context->ray_test_data[ret_index].body_index = proxy.object_index;
          ctx->context->ray_test_data[ret_index].dist = t;
        }
      }
      
      check_frustums_parallel::check_frustums_parallel(dt::thread_pool* pool) : pool(pool) {}
      void check_frustums_parallel::begin(octree_context* ctx) { (void)ctx; }
      void check_frustums_parallel::process(octree_context* ctx, const size_t &delta_time) {
        (void)delta_time;
        //std::atomic<uint32_t> counter(0);
        ctx->context->frustum_pairs_count = 0;
        if (ctx->context->frustum_test_data.size() < ctx->context->bodies.size()) {
          ctx->context->frustum_test_data.resize(ctx->context->bodies.size());
        }
        
        utils::submit_works(pool, ctx->context->frustums.size(), calc_frustums, ctx, pool, std::ref(ctx->context->frustum_pairs_count));
      }
      
      void check_frustums_parallel::clear() {}
      
      void check_frustums_parallel::calc_frustums(const size_t &start, const size_t &count, octree_context* ctx, dt::thread_pool* pool, std::atomic<uint32_t> &pair_counter) {
        for (size_t i = start; i < start+count; ++i) {
          const auto &frustum = ctx->context->frustums[i];
          const auto &node = ctx->nodes[0];
          const collision::aabb node_box = {ctx->nodes[0].box.center.get_simd(), ctx->nodes[0].box.extents.get_simd()};
          if (collision::test_frustum_aabb(frustum, node_box) == OUTSIDE_FRUSTUM) throw std::runtime_error("Frustum does not collide octree");
          
          for (size_t j = 0; j < node.proxy_indices.size(); ++j) {
            const uint32_t index = node.proxy_indices[j];
            if (index == UINT32_MAX) continue;
            check_and_add(ctx, i, index, pair_counter);
          }
          
          for (size_t j = node.next_level_index; j < node.next_level_index+8; ++j) {
            const collision::aabb node_box = {ctx->nodes[j].box.center.get_simd(), ctx->nodes[j].box.extents.get_simd()};
            if (collision::test_frustum_aabb(frustum, node_box) != OUTSIDE_FRUSTUM) {
              pool->submitbase([ctx, i, j, &pair_counter] () {
                calc_frustums_rec(ctx, i, j, pair_counter);
              });
            }
          }
        }
      }
      
      void check_frustums_parallel::calc_frustums_rec(octree_context* ctx, const size_t &frustum_index, const uint32_t &node_index, std::atomic<uint32_t> &pair_counter) {
        const auto &frustum = ctx->context->frustums[frustum_index];
        const auto &node = ctx->nodes[node_index];
        
        for (size_t j = 0; j < node.proxy_indices.size(); ++j) {
          const uint32_t index = node.proxy_indices[j];
          if (index == UINT32_MAX) continue;
          check_and_add(ctx, frustum_index, index, pair_counter);
        }
        
        for (size_t j = node.next_level_index; j < node.next_level_index+8; ++j) {
          const collision::aabb node_box = {ctx->nodes[j].box.center.get_simd(), ctx->nodes[j].box.extents.get_simd()};
          if (collision::test_frustum_aabb(frustum, node_box) != OUTSIDE_FRUSTUM) {
            calc_frustums_rec(ctx, frustum_index, j, pair_counter);
          }
        }
      }
      
      void check_frustums_parallel::check_and_add(octree_context* ctx, const uint32_t &frustum_index, const uint32_t &proxy_index, std::atomic<uint32_t> &pair_counter) {
        const auto &frustum = ctx->context->frustums[frustum_index];
        const auto &proxy = ctx->proxies[proxy_index];
        const auto &frustum_pos = ctx->context->frustums_pos[frustum_index];
        
        const collision::aabb box = {proxy.box.center.get_simd(), proxy.box.extents.get_simd()};
        
        if (collision::test_frustum_aabb(frustum, box) != OUTSIDE_FRUSTUM) {
          const uint32_t index = pair_counter.fetch_add(1);
          ASSERT(index < ctx->context->frustum_test_data.size());
          ctx->context->frustum_test_data[index].frustum_index = frustum_index;
          ctx->context->frustum_test_data[index].body_index = proxy.object_index;
          ctx->context->frustum_test_data[index].dist = simd::distance2(box.center, frustum_pos);
          ctx->context->frustum_test_data[index].dummy[0] = 0;
        }
      }
      
//       float solve_single_row(const solver_constraint &constraint, solver_body &body_a, solver_body &body_b);
//       
//       solve_constraints_parallel::solve_constraints_parallel(dt::thread_pool* pool) : pool(pool) {}
//       void solve_constraints_parallel::begin(octree_context* ctx) {}
//       void solve_constraints_parallel::process(octree_context* ctx, const size_t &delta_time) {
//         std::vector<solver_constraint> non_contact_constaints;
//         std::vector<solver_constraint> contact_constraints;
//         std::vector<solver_constraint> friction_constraints;
//         std::vector<solver_constraint> rolling_friction_constraints;
//         std::vector<solver_body> bodies;
//         
//         // все данные выше мы собираем обходя констраинты, делаем мы это потому что хотим чтобы все данные 
//         // как можно лучше использовали кеш, в solver_constraint содержатся данные о единственном
//         // ряду, мы собственно только их и вычисляем
//         std::vector<typed_constraint*> constaints;
//         
//         const uint32_t iterations_count = 10;
//         for (uint32_t iteration = 0; iteration < iterations_count; ++iteration) {
//           float least_squares_residual = 0.0f;
//           for (size_t i = 0; i < non_contact_constaints.size(); ++i) {
//             const solver_constraint &c = non_contact_constaints[i];
//             if (iteration > uint32_t(c.m_overrideNumSolverIterations)) continue;
//             const float res = solve_single_row(c, bodies[0], bodies[1]);
//             least_squares_residual = glm::max(least_squares_residual, res * res);
//           }
//           
//           // это существует не у всех констраинтов (что здесь происходит?)
//           // судя по названию это что то старое
//           for (size_t i = 0; i < constaints.size(); ++i) {
//             constaints[i]->solveConstraintObsolete(bodies[0], bodies[1], MCS_TO_SEC(delta_time));
//           }
//           
//           // у нас тут должна быть опция считать констраинты соприкосновения вместе или по отдельности
//           
//           for (size_t i = 0; i < contact_constraints.size(); ++i) {
//             const solver_constraint &c = contact_constraints[i];
//             const float res = solve_single_row(c, bodies[0], bodies[1]);
//             least_squares_residual = glm::max(least_squares_residual, res * res);
//           }
//           
//           for (size_t i = 0; i < friction_constraints.size(); ++i) {
//             solver_constraint &c = friction_constraints[i];
//             const float total_impulse = contact_constraints[c.m_frictionIndex].m_appliedImpulse[0];
//             if (total_impulse > 0) {
//               c.m_upperLimit = -(c.m_friction * total_impulse);
//               c.m_lowerLimit = c.m_friction * total_impulse;
//               const float res = solve_single_row(c, bodies[0], bodies[1]);
//               least_squares_residual = glm::max(least_squares_residual, res * res);
//             }
//           }
//           
//           for (size_t i = 0; i < rolling_friction_constraints.size(); ++i) {
//             solver_constraint &c = rolling_friction_constraints[i];
//             const float total_impulse = contact_constraints[c.m_frictionIndex].m_appliedImpulse[0];
//             if (total_impulse > 0) {
//               const float rollingFrictionMagnitude = std::min(c.m_friction * total_impulse, c.m_friction);
//               c.m_upperLimit = -(rollingFrictionMagnitude);
//               c.m_lowerLimit = rollingFrictionMagnitude;
//               const float res = solve_single_row(c, bodies[0], bodies[1]);
//               least_squares_residual = glm::max(least_squares_residual, res * res);
//             }
//           }
//           
//         }
//       }
//       
//       void solve_constraints_parallel::clear() {}
//       
//       float solve_single_row(const solver_constraint &constraint, solver_body &body_a, solver_body &body_b) {
//         float deltaImpulse = constraint.m_rhs - constraint.m_appliedImpulse[0] * constraint.m_cfm;
//         const float deltaVel1Dotn = simd::dot(constraint.m_contactNormal1, body_a.m_deltaLinearVelocity) + simd::dot(constraint.m_relpos1CrossNormal, body_a.m_deltaAngularVelocity);
//         const float deltaVel2Dotn = simd::dot(constraint.m_contactNormal2, body_b.m_deltaLinearVelocity) + simd::dot(constraint.m_relpos2CrossNormal, body_b.m_deltaAngularVelocity);
//         
//         // const btScalar delta_rel_vel = deltaVel1Dotn-deltaVel2Dotn;
//         deltaImpulse -= deltaVel1Dotn * constraint.m_jacDiagABInv;
//         deltaImpulse -= deltaVel2Dotn * constraint.m_jacDiagABInv;
// 
//         const float sum = float(constraint.m_appliedImpulse[0]) + deltaImpulse;
//         if (sum < constraint.m_lowerLimit) {
//           deltaImpulse = constraint.m_lowerLimit - constraint.m_appliedImpulse[0];
//           constraint.m_appliedImpulse = simd::vec4(constraint.m_lowerLimit);
//         } else if (sum > constraint.m_upperLimit) {
//           deltaImpulse = constraint.m_upperLimit - constraint.m_appliedImpulse[0];
//           constraint.m_appliedImpulse = simd::vec4(constraint.m_upperLimit);
//         } else {
//           constraint.m_appliedImpulse = simd::vec4(sum);
//         }
// 
//         body_a.apply_impulse(constraint.m_contactNormal1 * body_a.m_invMass, constraint.m_angularComponentA, deltaImpulse);
//         body_b.apply_impulse(constraint.m_contactNormal2 * body_b.m_invMass, constraint.m_angularComponentB, deltaImpulse);
// 
//         return deltaImpulse * (1. / constraint.m_jacDiagABInv);
//       }
    }
  }
}
