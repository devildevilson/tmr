#include "narrowphase_parts.h"

#include "physics_context.h"
// #include "collision_utils.h"
#include "broadphase_context.h"
#include "ThreadPool.h"
#include "works_utils.h"
#include "narrowphase_context.h"

namespace devils_engine {
  namespace physics {
    namespace narrowphase {
      unique_pairs_parallel::unique_pairs_parallel(dt::thread_pool* pool) : pool(pool) {}
      void unique_pairs_parallel::begin(context* ctx) { (void)ctx; unique.clear(); }
      void unique_pairs_parallel::process(context* ctx, const size_t &delta_time) {
        static const auto func = [this] (const size_t &start, const size_t &count, context* ctx) {
          for (size_t i = start; i < start+count; ++i) {
            auto &pair = ctx->broadphase_context->pairs[i];
            if (pair.pair_flags.static_pair()) continue;
            if (has_pair(pair.obj1, pair.obj2)) {
              pair.pair_flags.not_unique(true);
            }
          }
        };
        
        (void)delta_time;
        utils::submit_works(pool, ctx->broadphase_context->pairs_count, func, ctx);
      }
      
      void unique_pairs_parallel::clear() {}
      bool unique_pairs_parallel::has_pair(const uint32_t &idx1, const uint32_t &idx2) {
        const uint32_t min_idx = std::min(idx1, idx2);
        const uint32_t max_idx = std::max(idx1, idx2);
        const size_t key = (size_t(min_idx) << 32) | max_idx;
        
        std::unique_lock<std::mutex> lock(mutex);
        auto itr = unique.find(key);
        if (itr != unique.end()) return true;
        unique.insert(key);
        return false;
      }
      
      update_manifolds_parallel::update_manifolds_parallel(dt::thread_pool* pool) : pool(pool) {}
      void update_manifolds_parallel::begin(context* ctx) { (void)ctx; }
      void update_manifolds_parallel::process(context* ctx, const size_t &delta_time) {
        (void)delta_time;
        utils::submit_works(pool, ctx->manifolds_array.size(), update, ctx);
      }
      
      void update_manifolds_parallel::clear() {}
      void update_manifolds_parallel::update(const size_t &start, const size_t &count, context* ctx) {
        for (size_t i = start; i < start+count; ++i) {
          auto m = ctx->manifolds_array[i];
          m->batch_id = UINT32_MAX;
          const auto &body0 = ctx->core_context->bodies[m->body0];
          const auto &body1 = ctx->core_context->bodies[m->body1];
//           const auto &trans0 = ctx->core_context->transforms->at(body0.transform_index);
//           const auto &trans1 = ctx->core_context->transforms->at(body1.transform_index);
          const auto &trans0 = ctx->core_context->new_transforms[body0.transform_index];
          const auto &trans1 = ctx->core_context->new_transforms[body1.transform_index];
          m->refresh(trans0, trans1);
          
          if (m->points_count == 0) {
            // манифолд удалился
            // стоит ли его ждать некоторое время? или сразу удалить?
            std::cout << "remove manifold " << m->body0 << " " << m->body1 << "\n";
            ctx->remove_manifold(m);
          }
        }
      }
      
      struct no_update_bodies {
        std::atomic<size_t>* memory;
        size_t size;
        
        no_update_bodies(std::atomic<size_t>* memory, const size_t &size) : memory(memory), size(size) {
          //memset(memory, 0, sizeof(std::atomic<size_t>)*size);
          for (size_t i = 0; i < size; ++i) {
            memory[i] = 0;
          }
        }
        
        bool has_body(const uint32_t &id) {
          const uint32_t mem_index = id / SIZE_WIDTH;
          const size_t num_mask = 1 << (id % SIZE_WIDTH);
          
          const size_t ret = memory[mem_index].fetch_or(num_mask);
          return (ret & num_mask) == num_mask;
        }
        
        bool check_body(const uint32_t &id) {
          const uint32_t mem_index = id / SIZE_WIDTH;
          const size_t num_mask = 1 << (id % SIZE_WIDTH);
          
          const size_t ret = memory[mem_index];
          return (ret & num_mask) == num_mask;
        }
      };
      
      compute_manifolds_parallel::compute_manifolds_parallel(dt::thread_pool* pool) : pool(pool) {}
      void compute_manifolds_parallel::begin(context* ctx) { (void)ctx; }
      void compute_manifolds_parallel::process(context* ctx, const size_t &delta_time) {
        (void)delta_time;
        const size_t mem_size = ctx->core_context->bodies.size() / SIZE_WIDTH + 1;
        std::atomic<size_t> memory[mem_size];
        no_update_bodies b(memory, mem_size);
        utils::submit_works(pool, ctx->broadphase_context->pairs_count, find_stair, ctx, &b);
        utils::submit_works(pool, ctx->broadphase_context->pairs_count, compute, ctx, &b);
      }
      
      void compute_manifolds_parallel::clear() {}
      bool shape_predicate(const core::rigid_body &b1, const core::rigid_body &b2, const collision::shape &shape1, const collision::shape &shape2) {
        // вполне возможна ситуация когда мы находим больше чем одну подходящую нормаль
        // по идее в этом случае нам нужно найти "правильную" нормаль
        bool pred1 = shape1.type == collision::shape::box || shape1.type == collision::shape::polygon || shape1.type == collision::shape::convex_hull; 
        pred1 = pred1 && (b1.is_static() || b1.is_kinematic());
        pred1 = pred1 && b2.is_kinematic();
//         for (size_t i = 0; i < shape1.faces_count; ++i) {
//           const uint32_t &index = shape1.offset+shape1.points_count+1+i;
//           const vec4 norm = trans1.rot * (points[index] * vec4(1.0f, 1.0f, 1.0f, 0.0f));
//           // угол
//         }
        
        bool pred2 = shape2.type == collision::shape::box || shape2.type == collision::shape::polygon || shape2.type == collision::shape::convex_hull;
        pred2 = pred2 && (b2.is_static() || b2.is_kinematic());
        pred2 = pred2 && b1.is_kinematic();
        
        return pred1 || pred2;
      }
      
      uint32_t get_object_index(const core::rigid_body &b1, const core::rigid_body &b2, const collision::shape &shape1, const collision::shape &shape2, const core::transform &trans1, const core::transform &trans2, const vec4 &n) {
        if (b1.is_kinematic() && shape1.type == collision::shape::box && b2.is_kinematic() && shape2.type == collision::shape::box) {
          const vec4 dir = trans2.pos - trans1.pos;
          const scalar d = simd::dot(dir, n);
          if (d > 0.0f) return 0;
          return 1;
        }
        
        if (b1.is_kinematic() && b2.is_static()) return 0;
        if (b2.is_kinematic() && b1.is_static()) return 1;
        
        // тип один из них кинематик и не бокс
        // вообще по идее это ошибка
        throw std::runtime_error("kinematic && not box");
        return 0;
      }
      
      vec4 find_normal(const vec4* points, const collision::shape &shape, const core::transform &trans, const vec4 &n, uint32_t &point_index) {
        if (shape.type == collision::shape::sphere) return vec4(0, 0, 0, 0);
        
        if (shape.type == collision::shape::box) {
          const core::mat4 basis = trans.get_basis();
          vec4 normal = vec4(0.0f, 0.0f, 0.0f, 0.0f);
          scalar a = DE_PASSABLE_ANGLE;
          for (uint32_t i = 0; i < 3; ++i) {
            const auto tmp = basis[i];
            const auto tmp_a = collision::angle(tmp, n);
            if (a > tmp_a) {
              a = tmp_a;
              normal = tmp;
              point_index = i;
            }
          }
          
          for (uint32_t i = 0; i < 3; ++i) {
            const auto tmp = -basis[i];
            const auto tmp_a = collision::angle(tmp, n);
            if (a > tmp_a) {
              a = tmp_a;
              normal = tmp;
              point_index = i+3;
            }
          }
          
          return normal;
        }
        
        if (shape.type == collision::shape::polygon) {
          const uint32_t index = shape.offset+shape.points_count+1;
          float arr[4];
          points[index].storeu(arr);
          const vec4 tmp = trans.rot * vec4(arr[0], arr[1], arr[2], 0.0f);
          const auto tmp_a = collision::angle(tmp, n);
          if (tmp_a < DE_PASSABLE_ANGLE) {
            point_index = glm::floatBitsToUint(arr[3]);
            return tmp;
          }
          
          return vec4(0,0,0,0);
        }
        
        vec4 normal = vec4(0.0f, 0.0f, 0.0f, 0.0f);
        scalar a = DE_PASSABLE_ANGLE;
        for (uint32_t i = 0; i < shape.faces_count; ++i) {
          const uint32_t index = shape.offset+shape.points_count+1+i;
          float arr[4];
          points[index].storeu(arr);
//           const vec4 tmp = trans.rot * (points[index] * vec4(1.0f, 1.0f, 1.0f, 0.0f));
          const vec4 tmp = trans.rot * vec4(arr[0], arr[1], arr[2], 0.0f);
          const auto tmp_a = collision::angle(tmp, n);
          if (a > tmp_a) {
            a = tmp_a;
            normal = tmp;
            point_index = glm::floatBitsToUint(arr[3]);
          }
        }
        
        return normal;
      }
      
      vec4 get_normal_point(const vec4* points, const collision::shape &shape, const core::transform &trans, const uint32_t &point_index) {
        if (shape.type == collision::shape::sphere) return vec4(0, 0, 0, 0);
        
        if (shape.type == collision::shape::box) {
          const vec4 box_max = trans.transform_vector( points[shape.offset]);
          const vec4 box_min = trans.transform_vector(-points[shape.offset]);
          
          if (point_index < 3) return box_max;
          return box_min;
        }
        
        return points[shape.offset+point_index];
      }
      
      void compute_manifolds_parallel::find_stair(const size_t &start, const size_t &count, context* ctx, void* ptr) {
        for (size_t i = start; i < start+count; ++i) {
          const auto &pair = ctx->broadphase_context->pairs[i];
          if (pair.pair_flags.trigger_only()) continue;
          if (pair.pair_flags.not_unique()) continue;
          
          const auto &body0 = ctx->core_context->bodies[pair.obj1];
          const auto &body1 = ctx->core_context->bodies[pair.obj2];
          const auto &trans0 = ctx->core_context->new_transforms[body0.transform_index];
          const auto &trans1 = ctx->core_context->new_transforms[body1.transform_index];
          const auto &shape0 = ctx->core_context->shapes[body0.shape_index];
          const auto &shape1 = ctx->core_context->shapes[body1.shape_index];
          
          if (!shape_predicate(body0, body1, shape0, shape1)) continue;
          
          collision::return_data rd;
          const bool ret = collision::test_objects_mpr(ctx->core_context->points.data(), shape0, trans0, shape1, trans1, rd);
          if (!ret) continue;
          
          const vec4 neg_gravity = -ctx->core_context->gravity_data->gravity_norm;
//           const scalar angle = collision::angle(rd.normal, ctx->context->gravity_data->gravity_norm);
//           if (angle < DE_PASSABLE_ANGLE - EPSILON) continue;
          
          if (glm::abs(simd::dot(rd.normal, neg_gravity)) > EPSILON) continue;
          
          const uint32_t current_obj = get_object_index(body0, body1, shape0, shape1, trans0, trans1, neg_gravity);
          if (current_obj == 0) {
            uint32_t point_index = 0;
            const vec4 push_normal = find_normal(ctx->core_context->points.data(), shape1, trans1, neg_gravity, point_index);
            if (simd::length2(push_normal) < EPSILON) continue;
            
            const vec4 normal_point = get_normal_point(ctx->core_context->points.data(), shape1, trans1, point_index);
            
            const scalar push_dist = collision::one_axis_SAT(ctx->core_context->points.data(), shape0, trans0, push_normal, normal_point);
            if (push_dist > body0.stair_height) continue;
            
            auto b = reinterpret_cast<no_update_bodies*>(ptr);
            if (b->has_body(pair.obj1)) continue;
            
            rd.normal = push_normal;
            rd.distance = push_dist; // необходимый подъем
            //rd.point_a = 
            rd.point_b = rd.point_a + rd.normal * rd.distance;
          } else {
            uint32_t point_index = 0;
            const vec4 push_normal = find_normal(ctx->core_context->points.data(), shape0, trans0, neg_gravity, point_index);
            if (simd::length2(push_normal) < EPSILON) continue;
            
            const vec4 normal_point = get_normal_point(ctx->core_context->points.data(), shape0, trans0, point_index);
            
            const scalar push_dist = collision::one_axis_SAT(ctx->core_context->points.data(), shape1, trans1, push_normal, normal_point);
            if (push_dist > body1.stair_height) continue;
            
            auto b = reinterpret_cast<no_update_bodies*>(ptr);
            if (b->has_body(pair.obj2)) continue;
            
            rd.normal = push_normal;
            rd.distance = push_dist; // необходимый подъем
            rd.point_a = rd.point_b + rd.normal * rd.distance;
            //rd.point_b = rd.point_a + rd.normal * rd.distance;
          }
          
          add_manifold(ctx, i, rd);
        }
      }
      
      void compute_manifolds_parallel::compute(const size_t &start, const size_t &count, context* ctx, void* ptr) {
        for (size_t i = start; i < start+count; ++i) {
          const auto &pair = ctx->broadphase_context->pairs[i];
          if (pair.pair_flags.trigger_only()) continue;
          if (pair.pair_flags.not_unique()) continue;
          
          auto b = reinterpret_cast<no_update_bodies*>(ptr);
          if (b->check_body(pair.obj1)) continue;
          if (b->check_body(pair.obj2)) continue;
          
          const auto &body0 = ctx->core_context->bodies[pair.obj1];
          const auto &body1 = ctx->core_context->bodies[pair.obj2];
//           const auto &trans0 = ctx->context->transforms->at(body0.transform_index);
//           const auto &trans1 = ctx->context->transforms->at(body1.transform_index);
          const auto &trans0 = ctx->core_context->new_transforms[body0.transform_index];
          const auto &trans1 = ctx->core_context->new_transforms[body1.transform_index];
          const auto &shape0 = ctx->core_context->shapes[body0.shape_index]; // plane
          const auto &shape1 = ctx->core_context->shapes[body1.shape_index]; // box
          collision::return_data rd;
          const bool ret = collision::test_objects_mpr(ctx->core_context->points.data(), shape0, trans0, shape1, trans1, rd);
          if (!ret) continue;
          
//           std::cout << "pair.obj1 " << pair.obj1 << "\n"; // plane
//           std::cout << "pair.obj2 " << pair.obj2 << "\n"; // box
//           PRINT_VEC4("rd.point_a", rd.point_a);
//           PRINT_VEC4("rd.point_b", rd.point_b);
//           PRINT_VEC4("rd.normal ", rd.normal);
//           PRINT_VAR("rd.distance ", rd.distance);
//           
//           // push_velocity по идее должен быть равен (dist * normal) / time_step
//           const vec4 push_vel = (glm::abs(rd.distance) * rd.normal) / MCS_TO_SEC(ONE_SECOND/20);
//           PRINT_VEC4("push_vel", push_vel);
          
//           ASSERT(false);
          
          // проверяем подходят ли объекты под условия описанные в хедере
//           const bool check_ret = check_proxy(pair.obj1, pair.obj2, ctx);
          
          add_manifold(ctx, i, rd);
        }
      }
      
      void compute_manifolds_parallel::add_manifold(context* ctx, const uint32_t &pair_index, const collision::return_data &rd) {
        const auto &pair = ctx->broadphase_context->pairs[pair_index];
        const auto &body0 = ctx->core_context->bodies[pair.obj1];
        const auto &body1 = ctx->core_context->bodies[pair.obj2];
        const auto &trans0 = ctx->core_context->new_transforms[body0.transform_index];
        const auto &trans1 = ctx->core_context->new_transforms[body1.transform_index];
        const auto &shape0 = ctx->core_context->shapes[body0.shape_index];
        const auto &shape1 = ctx->core_context->shapes[body1.shape_index];
        
        const auto points = ctx->core_context->points.data();
        const auto default_contact_breaking_treshold = ctx->default_contact_breaking_treshold;
        auto m = ctx->add_manifold(pair.obj1, pair.obj2, [points, &shape0, &shape1, &default_contact_breaking_treshold] () -> core::scalar {
          return std::min(shape0.get_contact_breaking_threshold(points, default_contact_breaking_treshold), shape1.get_contact_breaking_threshold(points, default_contact_breaking_treshold));
        }, std::min(body0.contact_processing_threshold, body1.contact_processing_threshold));
        const bool new_col = m->points_count == 0;
        const vec4 local_point_a = m->body0 == UINT32_MAX || m->body0 == pair.obj1 ? trans0.inv_transform(rd.point_a) : trans0.inv_transform(rd.point_b);
        const vec4 local_point_b = m->body0 == UINT32_MAX || m->body0 == pair.obj1 ? trans1.inv_transform(rd.point_b) : trans1.inv_transform(rd.point_a);
        
//         PRINT_VEC4("local_point_a", local_point_a)
//         PRINT_VEC4("local_point_b", local_point_b)
//         PRINT_VAR("contact_breaking_threshold", m->contact_breaking_threshold)
//         PRINT_VAR("default_contact_breaking_treshold", default_contact_breaking_treshold)
        
        manifold_point p = manifold_point(local_point_a, local_point_b, rd.normal, rd.distance);
        const uint32_t nearest = m->get_nearest_point(p);
        
        // как его заполнить данными?
//         p.world_position_a = m->body0 == pair.obj1 ? rd.point_a : rd.point_b;
//         p.world_position_b = m->body0 == pair.obj1 ? rd.point_b : rd.point_a;
        p.world_position_a = rd.point_b + rd.normal * rd.distance;
        p.world_position_b = rd.point_b;
        
        ASSERT(ctx->data_callback.combined_friction_callback);
        ASSERT(ctx->data_callback.combined_restitution_callback);
        ASSERT(ctx->data_callback.combined_rolling_friction_callback);
        ASSERT(ctx->data_callback.combined_spinning_friction_callback);
        
        p.combined_friction = ctx->data_callback.combined_friction_callback(body0, body1);
        p.combined_restitution = ctx->data_callback.combined_restitution_callback(body0, body1);
        p.combined_rolling_friction = ctx->data_callback.combined_rolling_friction_callback(body0, body1);
        p.combined_spinning_friction = ctx->data_callback.combined_spinning_friction_callback(body0, body1);
        
        if (body0.body_flags.contact_stiffness_damping() || body1.body_flags.contact_stiffness_damping()) {
          ASSERT(ctx->data_callback.combined_stiffness_callback);
          ASSERT(ctx->data_callback.combined_contact_damping_callback);
          p.combined_contact_damping1 = ctx->data_callback.combined_contact_damping_callback(body0, body1);
          p.combined_contact_stiffness1 = ctx->data_callback.combined_stiffness_callback(body0, body1);
          p.contact_point_flags.contact_stiffness_damping(true);
        }
        
        if (body0.body_flags.friction_anchor() || body1.body_flags.friction_anchor()) {
          p.contact_point_flags.friction_anchor(true);
        }
        
        core::plane_space(rd.normal, p.lateral_friction_dir1 , p.lateral_friction_dir2);
        
        if (nearest != UINT32_MAX) {
//           PRINT("replaced")
          m->replace_contact_point(p, nearest);
        } else {
//           PRINT("added")
          m->add_manifold_point(p);
        }
        
        // тут должен быть еще колбек для материала по поверхностям
        
        if (new_col && ctx->manifold_callbacks.started) {
          ctx->manifold_callbacks.started(m);
        }
        
        m->refresh(m->body0 == pair.obj1 ? trans0 : trans1, m->body0 == pair.obj1 ? trans1 : trans0);
        if (m->points_count == 0) {
          ASSERT(false);
          ctx->remove_manifold(m);
        }
      }
      
      bool compute_manifolds_parallel::check_proxy(const uint32_t &obj1_index, const uint32_t &obj2_index, context* ctx) {
        const auto &body0 = ctx->core_context->bodies[obj1_index];
        const auto &body1 = ctx->core_context->bodies[obj2_index];
        
        if (body0.is_dynamic()) return false;
        if (body1.is_dynamic()) return false;
        
        // нужно еще какие нибудь дополнительные флаги добавить
        // у тела должен быть дополнительный прокси индекс
        // этот прокси должен не индекс объекта в пару складывать, а свой
        // метод с дополнительным прокси плох тем что мы не сможем 
        // взбираться на лесницу упираясь в стенку
        // можно еще по другому поступить: проверить есть ли хотя бы
        // один объект который похож на ступеньку, обработать только его
        // в этом кадре, это правда может привести к дурацким последствиям
        // к каким? начнет прыгать на месте в некоторых случаях
        // но скорее всего это единственный вариант сделать адекватную ступеньку
        
//         for (uint32_t i = 0; i < ctx->context->pairs_count; ++i) {
//           if (ctx->context->trigger_pairs[i].obj1 == ) {
//             
//           }
//         }
        
        return false;
      }
      
      compute_trigger_pairs_parallel::compute_trigger_pairs_parallel(dt::thread_pool* pool) : pool(pool) {}
      void compute_trigger_pairs_parallel::begin(context* ctx) { (void)ctx; }
      void compute_trigger_pairs_parallel::process(context* ctx, const size_t &delta_time) {
        (void)delta_time;
        ctx->core_context->pairs_count = 0;
        if (ctx->core_context->trigger_pairs.size() < ctx->broadphase_context->pairs_count) {
          ctx->core_context->trigger_pairs.resize(ctx->broadphase_context->pairs_count);
        }
        
        utils::submit_works(pool, ctx->broadphase_context->pairs_count, compute, ctx, std::ref(ctx->core_context->pairs_count));
      }
      
      void compute_trigger_pairs_parallel::clear() {}
      void compute_trigger_pairs_parallel::compute(const size_t &start, const size_t &count, context* ctx, std::atomic<uint32_t> &counter) {
        for (size_t i = start; i < start+count; ++i) {
          const auto &pair = ctx->broadphase_context->pairs[i];
          if (pair.pair_flags.not_unique()) continue;
          if (!pair.pair_flags.trigger_only()) continue;
          
          const auto &b0 = ctx->core_context->bodies[pair.obj1];
          const auto &b1 = ctx->core_context->bodies[pair.obj2];
          const auto &trans0 = ctx->core_context->new_transforms[b0.transform_index];
          const auto &trans1 = ctx->core_context->new_transforms[b1.transform_index];
          const auto &shape0 = ctx->core_context->shapes[b0.shape_index];
          const auto &shape1 = ctx->core_context->shapes[b1.shape_index];
          
          vec4 mtv;
          scalar dist;
          // предполагается что SAT проверка быстрее чем mpr
          const bool ret = collision::test_objects_SAT(ctx->core_context->points.data(), shape0, trans0, shape1, trans1, mtv, dist);
          if (!ret) continue;
          
          const uint32_t index = counter.fetch_add(1);
          ctx->core_context->trigger_pairs[index].obj1 = pair.obj1;
          ctx->core_context->trigger_pairs[index].obj2 = pair.obj2;
          ctx->core_context->trigger_pairs[index].dist = dist;
          mtv.storeu(ctx->core_context->trigger_pairs[index].mtv);
        }
      }
      
      struct light_spin_lock {
        size_t* memory;
        std::atomic<size_t*> &blocked_memory;
        
        light_spin_lock(std::atomic<size_t*> &mem) : memory(nullptr), blocked_memory(mem) {
          do {
            std::this_thread::sleep_for(std::chrono::microseconds(1));
            memory = blocked_memory.exchange(nullptr);
          } while (memory == nullptr);
        }
        
        ~light_spin_lock() {
          blocked_memory = memory;
        }
      };
      
      struct pair_checker {
        std::atomic<size_t*> memory;
        size_t count;
        
        pair_checker(size_t* mem, size_t count) : memory(mem), count(count) {
          memset(memory, 0, sizeof(size_t)*count);
//           for (size_t i = 0; i < count; ++i) {
//             memory[i] = 0;
//           }
        }
        
        bool has_pair(const uint32_t &id1, const uint32_t &id2) {
          const uint32_t mem_index1 = id1 / SIZE_WIDTH;
          const size_t num_mask1 = 1 << (id1 % SIZE_WIDTH);
          
          const uint32_t mem_index2 = id2 / SIZE_WIDTH;
          const size_t num_mask2 = 1 << (id2 % SIZE_WIDTH);
          
          {
            light_spin_lock s(memory);
            size_t* mem = s.memory;
            
            if (((mem[mem_index1] & num_mask1) == num_mask1) || 
                ((mem[mem_index2] & num_mask2) == num_mask2)) return true;
            mem[mem_index1] |= num_mask1;
            mem[mem_index2] |= num_mask2;
          }
          
          return false;
        }
      };
      
      struct manifold_comparator {
        bool operator() (const persistent_manifold* m1, const persistent_manifold* m2) const {
          return m1->batch_id < m2->batch_id;
        }
      };
      
      compute_batches_parallel::compute_batches_parallel(dt::thread_pool* pool) : pool(pool) {}
      void compute_batches_parallel::begin(context* ctx) { ctx->batches_count.clear(); }
      void compute_batches_parallel::process(context* ctx, const size_t &delta_time) {
        // стандартное раскидывание батчей
        // нужно ли сортировать? сортировка к сожалению в одном потоке
        
        const size_t mem_count = ctx->manifolds.size() / SIZE_WIDTH;
        size_t memory[mem_count];
        std::atomic<size_t> counter(ctx->manifolds.size());
        
        uint32_t batch_id = 0;
        while (counter > 0) {
          std::atomic<uint32_t> batch_counter(0);
          pair_checker checker(memory, mem_count);
          utils::submit_works(pool, ctx->manifolds.size(), compute, batch_id, ctx, &checker, std::ref(counter), std::ref(batch_counter));
          ctx->batches_count.push_back(batch_counter);
          ++batch_id;
        }
        
        std::sort(ctx->manifolds_array.begin(), ctx->manifolds_array.end(), manifold_comparator());
      }
      
      void compute_batches_parallel::clear() {}
      
      void compute_batches_parallel::compute(const size_t &start, const size_t &count, const uint32_t &batch_id, context* ctx, pair_checker* checker, std::atomic<size_t> &counter, std::atomic<uint32_t> &batch_counter) {
        for (size_t i = start; i < start+count; ++i) {
          auto m = ctx->manifolds_array[i];
          if (m->batch_id != UINT32_MAX) continue;
          if (checker->has_pair(m->body0, m->body1)) continue;
          
          m->batch_id = batch_id;
          counter.fetch_sub(1);
          batch_counter.fetch_add(1);
        }
      }
    }
  }
}
