#include "core_parts.h"
#include "physics_context.h"
#include "works_utils.h"
#include "Utility.h"

namespace devils_engine {
  namespace physics {
    namespace core {
      copy_transforms::copy_transforms() {}
      void copy_transforms::begin(context* ctx) { (void)ctx; }
      void copy_transforms::process(context* ctx, const size_t &delta_time) {
        (void)delta_time;
        ASSERT(ctx->transforms->size() == ctx->new_transforms.size() && ctx->new_transforms.size() == ctx->old_transforms.size());
        memcpy(ctx->old_transforms.data(), ctx->new_transforms.data(), ctx->new_transforms.size()*sizeof(ctx->new_transforms[0]));
      }
      
      void copy_transforms::clear() {}
      
      apply_gravity::apply_gravity(dt::thread_pool* pool) : pool(pool) {}
      void apply_gravity::begin(context* ctx) { (void)ctx; }
      void apply_gravity::process(context* ctx, const size_t &delta_time) {
        (void)delta_time;
        utils::submit_works(pool, ctx->bodies.size(), apply_rigid_body_gravity, ctx);
      }
      
      void apply_gravity::clear() {}
      void apply_gravity::apply_rigid_body_gravity(const size_t &start, const size_t &count, context* ctx) {
        for (size_t i = start; i < start+count; ++i) {
          rigid_body &b = ctx->bodies[i];
          if (!b.valid()) continue;
          if (b.body_flags.static_object()) continue;
          
          b.apply_gravity(ctx->gravity_data->gravity * b.gravity_factor);
        }
      }

      predict_motion::predict_motion(dt::thread_pool* pool) : pool(pool) {}
      void predict_motion::begin(context* ctx) { (void)ctx; }
      void predict_motion::process(context* ctx, const size_t &delta_time) {
        utils::submit_works(pool, ctx->bodies.size(), predict, ctx, delta_time);
      }
      
      void predict_motion::clear() {}
      void predict_motion::predict(const size_t &start, const size_t &count, context* ctx, const size_t &delta_time) {
        for (size_t i = start; i < start+count; ++i) {
          rigid_body &b = ctx->bodies[i];
          if (!b.valid()) continue;
          if (b.body_flags.static_object()) continue;
          
          b.apply_damping(delta_time);
          ctx->new_transforms[b.transform_index] = b.predict(ctx->transforms->at(b.transform_index), delta_time);
          
          // примерно тоже самое и в integrate_transform?
          // в чем суть? после прохождения всех симуляций 
          // у нас записывается изменение скоростей, которые
          // мы должны применить к объекту, по идее тогда объект 
          // займет свое место, с другой стороны, мы применяем 
          // скорости два раза, причем оба раза походу учитывается 
          // гравитация
        }
      }
      
      integrate_transform::integrate_transform(dt::thread_pool* pool) : pool(pool) {}
      void integrate_transform::begin(context* ctx) { (void)ctx; }
      void integrate_transform::process(context* ctx, const size_t &delta_time) {
        utils::submit_works(pool, ctx->bodies.size(), integrate, ctx, delta_time);
      }
      
      void integrate_transform::clear() {}
      void integrate_transform::integrate(const size_t &start, const size_t &count, context* ctx, const size_t &delta_time) {
        for (size_t i = start; i < start+count; ++i) {
          rigid_body &b = ctx->bodies[i];
          if (!b.valid()) continue;
          if (b.body_flags.static_object()) continue;
          
//           PRINT_VEC4("integrate pos",ctx->new_transforms[b.transform_index].pos)
//           PRINT_VEC4("b vel",b.linear_velocity)
          ctx->new_transforms[b.transform_index] = b.predict(ctx->new_transforms[b.transform_index], delta_time);
//           PRINT_VEC4("integrate pos",ctx->new_transforms[b.transform_index].pos)
          
        }
      }
      
      interpolate_transform::interpolate_transform(dt::thread_pool* pool) : pool(pool) {}
      void interpolate_transform::begin(context* ctx) { (void)ctx; }
      void interpolate_transform::process(context* ctx, const core::scalar &alpha) {
        utils::submit_works(pool, ctx->bodies.size(), interpolate, ctx, alpha);
        ctx->clear();
      }
      
      void interpolate_transform::clear() {}
      void interpolate_transform::interpolate(const size_t &start, const size_t &count, context* ctx, const core::scalar &alpha) {
        for (size_t i = start; i < start+count; ++i) {
          rigid_body &b = ctx->bodies[i];
          if (!b.valid()) continue;
          if (b.body_flags.static_object()) continue;
          
          const transform &old = ctx->old_transforms[b.transform_index];
          const transform &new_t = ctx->new_transforms[b.transform_index];
          transform &current = ctx->transforms->at(b.transform_index);
          current.pos   = simd::mix(old.pos, new_t.pos, alpha);
          current.rot   = simd::slerp(old.rot, new_t.rot, alpha);
          current.scale = simd::mix(old.scale, new_t.scale, alpha);
          
          // где то ранее неправильно вычисляется ротация
//           PRINT_VEC4("old.rot", old.rot)
//           PRINT_VEC4("new_t.rot", new_t.rot)
//           PRINT_VAR("alpha", alpha)
//           PRINT_VEC4("current.rot", current.rot)
          
          b.clear_forces();
        }
      }
      
      clear_forces::clear_forces(dt::thread_pool* pool) : pool(pool) {}
      void clear_forces::begin(context* ctx) { (void)ctx; }
      void clear_forces::process(context* ctx, const size_t &delta_time) {
        (void)delta_time;
        utils::submit_works(pool, ctx->bodies.size(), clear_forces_func, ctx);
      }
      
      void clear_forces::clear() {}
      void clear_forces::clear_forces_func(const size_t &start, const size_t &count, context* ctx) {
        for (size_t i = start; i < start+count; ++i) {
          rigid_body &b = ctx->bodies[i];
          if (!b.valid()) continue;
          if (b.body_flags.static_object()) continue;
          
          b.clear_forces();
        }
      }
    }
  }
}
