#ifndef SOLVER_PARTS_H
#define SOLVER_PARTS_H

#include "physics_core.h"
#include "simulation_part.h"
#include <cstdint>
#include <atomic>

namespace dt {
  class thread_pool;
}

namespace devils_engine {
  namespace physics {
    namespace solver {
      struct context;
      
      class convert_constraints_parallel : public core::simulation_part<context> {
      public:
        convert_constraints_parallel(dt::thread_pool* pool);
        ~convert_constraints_parallel();
        void begin(context* ctx) override;
        void process(context* ctx, const size_t &delta_time) override;
        void clear() override;
        static void init_body(context* ctx, const uint32_t &index, const size_t &delta_time);
        static void convert(const size_t &start, const size_t &count, context* ctx, const size_t &delta_time, std::atomic<uint32_t> &constraints_size, std::atomic<uint32_t> &frictions_size, std::atomic<uint32_t> &rolling_frictions_size);
        static void convert_bodies(const size_t &start, const size_t &count, context* ctx, const size_t &delta_time);
        static void convert_joints1(const size_t &start, const size_t &count, context* ctx, char* memory, std::atomic<uint32_t> &rows);
        static void convert_joints2(const size_t &start, const size_t &count, context* ctx, char* memory, const size_t &delta_time, const uint32_t &rows_count, std::atomic<uint32_t> &current_row);
      private:
        size_t info1_memory_size;
        char* info1_memory;
        dt::thread_pool* pool;
      };
      
      class solve_constraints_parallel : public core::simulation_part<context> {
      public:
        solve_constraints_parallel(dt::thread_pool* pool);
        void begin(context* ctx) override;
        void process(context* ctx, const size_t &delta_time) override;
        void clear() override;
        core::scalar solve_single_iteration(context* ctx, const uint32_t &current_iteration);
        void solve_split_impulse_penetrations(context* ctx);
        static void solve_non_contact_constraints(const size_t &start, const size_t &count, context* ctx, const uint32_t &current_iteration, std::atomic<uint32_t> &least_squares_residual);
        static void solve_contact_constraints_interleave(const size_t &start, const size_t &count, context* ctx, std::atomic<uint32_t> &least_squares_residual);
        static void solve_contact_constraints(const size_t &start, const size_t &count, context* ctx, std::atomic<uint32_t> &least_squares_residual);
        static void solve_friction_constraints(const size_t &start, const size_t &count, context* ctx, std::atomic<uint32_t> &least_squares_residual);
        static void solve_rolling_friction_constraints(const size_t &start, const size_t &count, context* ctx, std::atomic<uint32_t> &least_squares_residual);
      private:
        dt::thread_pool* pool;
      };
      
      class solver_finish_parallel : public core::simulation_part<context> {
      public:
        solver_finish_parallel(dt::thread_pool* pool);
        void begin(context* ctx) override;
        void process(context* ctx, const size_t &delta_time) override;
        void clear() override;
        static void write_back_contacts(const size_t &start, const size_t &count, context* ctx);
        static void write_back_joints(const size_t &start, const size_t &count, context* ctx, const size_t &delta_time);
        static void write_back_bodies(const size_t &start, const size_t &count, context* ctx, const size_t &delta_time);
      private:
        dt::thread_pool* pool;
      };
    }
  }
}

#endif
