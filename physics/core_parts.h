#ifndef CORE_PARTS_H
#define CORE_PARTS_H

#include "simulation_part.h"
#include "physics_core.h"

namespace dt {
  class thread_pool;
}

namespace devils_engine {
  namespace physics {
    namespace core {
      struct context;
      
      class copy_transforms : public core::simulation_part<context> {
      public:
        copy_transforms();
        void begin(context* ctx) override;
        void process(context* ctx, const size_t &delta_time) override;
        void clear() override;
      };
      
      class apply_gravity : public core::simulation_part<context> {
      public:
        apply_gravity(dt::thread_pool* pool);
        void begin(context* ctx) override;
        void process(context* ctx, const size_t &delta_time) override;
        void clear() override;
        static void apply_rigid_body_gravity(const size_t &start, const size_t &count, context* ctx);
      private:
        dt::thread_pool* pool;
      };
      
      class predict_motion : public core::simulation_part<context> {
      public:
        predict_motion(dt::thread_pool* pool);
        void begin(context* ctx) override;
        void process(context* ctx, const size_t &delta_time) override;
        void clear() override;
        static void predict(const size_t &start, const size_t &count, context* ctx, const size_t &delta_time);
      private:
        dt::thread_pool* pool;
      };
      
      // броадфаза, нарровфаза, солвер
      
      class integrate_transform : public core::simulation_part<context> {
      public:
        integrate_transform(dt::thread_pool* pool);
        void begin(context* ctx) override;
        void process(context* ctx, const size_t &delta_time) override;
        void clear() override;
        static void integrate(const size_t &start, const size_t &count, context* ctx, const size_t &delta_time);
      private:
        dt::thread_pool* pool;
      };
      
      // нужно еще интерполировать 
      // как для интерполяции использовать 2 массива трансформ вместо 3
      // не знаю, по мне так никак
      class interpolate_transform : public core::interpolate_part<context> {
      public:
        interpolate_transform(dt::thread_pool* pool);
        void begin(context* ctx) override;
        void process(context* ctx, const core::scalar &alpha) override;
        void clear() override;
        static void interpolate(const size_t &start, const size_t &count, context* ctx, const core::scalar &alpha);
      private:
        dt::thread_pool* pool;
      };
      
      class clear_forces : public core::simulation_part<context> {
      public:
        clear_forces(dt::thread_pool* pool);
        void begin(context* ctx) override;
        void process(context* ctx, const size_t &delta_time) override;
        void clear() override;
        static void clear_forces_func(const size_t &start, const size_t &count, context* ctx);
      private:
        dt::thread_pool* pool;
      };
    }
  }
}

#endif
