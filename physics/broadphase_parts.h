#ifndef BROADPHASE_PARTS_H
#define BROADPHASE_PARTS_H

#include "simulation_part.h"
#include <atomic>
#include <mutex>

namespace dt {
  class thread_pool;
}

namespace devils_engine {
  namespace physics {
    namespace broadphase {
      struct octree_context;
      
      // вот эти 5 вещей по идее и составляют броадфазу
      class compute_aabb_parallel : public core::simulation_part<octree_context> {
      public:
        compute_aabb_parallel(dt::thread_pool* pool);
        void begin(octree_context* ctx) override;
        void process(octree_context* ctx, const size_t &delta_time) override;
        void clear() override;
      private:
        dt::thread_pool* pool;
      };
      
      class update_octree_parallel : public core::simulation_part<octree_context> {
      public:
        update_octree_parallel(dt::thread_pool* pool);
        void begin(octree_context* ctx) override;
        void process(octree_context* ctx, const size_t &delta_time) override;
        void clear() override;
      private:
        dt::thread_pool* pool;
      };
      
      class compute_pairs_parallel : public core::simulation_part<octree_context> {
      public:
        compute_pairs_parallel(dt::thread_pool* pool);
        void begin(octree_context* ctx) override;
        void process(octree_context* ctx, const size_t &delta_time) override;
        void clear() override;
        static void calc_pairs(const size_t &start, const size_t &count, octree_context* ctx, std::atomic<uint32_t> &pair_counter);
        static void calc_pairs_rec(octree_context* ctx, const size_t &proxy_index, const uint32_t &node_index, std::atomic<uint32_t> &pair_counter);
        static void check_and_add(const uint32_t &proxy1_index, const uint32_t &proxy2_index, octree_context* ctx, std::atomic<uint32_t> &pair_counter);
      private:
        dt::thread_pool* pool;
      };
      
      class cast_rays_parallel : public core::simulation_part<octree_context> {
      public:
        cast_rays_parallel(dt::thread_pool* pool);
        void begin(octree_context* ctx) override;
        void process(octree_context* ctx, const size_t &delta_time) override;
        void clear() override;
        static void calc_rays(const size_t &start, const size_t &count, octree_context* ctx, dt::thread_pool* pool);
        static void calc_rays_rec(octree_context* ctx, const uint32_t &ray_index, const uint32_t &node_index, std::mutex &mutex);
        static void check_and_add(octree_context* ctx, const uint32_t &ray_index, const uint32_t &proxy_index, std::mutex &mutex);
      private:
        dt::thread_pool* pool;
      };
      
      class check_frustums_parallel : public core::simulation_part<octree_context> {
      public:
        check_frustums_parallel(dt::thread_pool* pool);
        void begin(octree_context* ctx) override;
        void process(octree_context* ctx, const size_t &delta_time) override;
        void clear() override;
        static void calc_frustums(const size_t &start, const size_t &count, octree_context* ctx, dt::thread_pool* pool, std::atomic<uint32_t> &pair_counter);
        static void calc_frustums_rec(octree_context* ctx, const size_t &frustum_index, const uint32_t &node_index, std::atomic<uint32_t> &pair_counter);
        static void check_and_add(octree_context* ctx, const uint32_t &frustum_index, const uint32_t &proxy_index, std::atomic<uint32_t> &pair_counter);
      private:
        dt::thread_pool* pool;
      };
    }
  }
}

#endif
