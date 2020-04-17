#ifndef NARROWPHASE_PARTS_H
#define NARROWPHASE_PARTS_H

#include "simulation_part.h"
#include "collision_utils.h"
// #include "narrowphase_context.h"
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <vector>
#include <atomic>

namespace dt {
  class thread_pool;
}

// в игре потребуется сделать ступеньки
// я планировал сделать так же как было раньше 
// то есть предварительный обход объектов с нормалью вверх
// но я боюсь что это может сломать физику
// лучше бы теми же констреинтами сделать необходимые действия
// можно все констейнты направить в иную сторону по условию
// условие? объект должен обладать нормалью, угол между ней и гравитацией < 45 градусов
// объект должен быть статическим? (статика + кинематика?), проекция на нормаль должна 
// различаться меньше чем stair_height, объект взаимодействия тоже только статика и кинематика?
// в этом случае констраинт должен быть направлен по нормали, выталкивание? проекция
// нужно изменить манифолд
// возможно нужно сделать специальный второй прокси, от которого все что требуется это проверить 
// есть ли объекты выше и чуть впереди ступеньки
// значит триггер пары нужно посчитать до манифолдов
// 

namespace devils_engine {
  namespace physics {
    namespace core {
      struct context;
    }
    
    namespace narrowphase {
      struct context;
      
      // мне нужны уникальные пары, пары легко проверить используя unordered_set
      class unique_pairs_parallel : public core::simulation_part<context> {
      public:
        unique_pairs_parallel(dt::thread_pool* pool);
        void begin(context* ctx) override;
        void process(context* ctx, const size_t &delta_time) override;
        void clear() override;
        bool has_pair(const uint32_t &idx1, const uint32_t &idx2);
      private:
        dt::thread_pool* pool;
        std::mutex mutex;
        std::unordered_set<size_t> unique;
      };
      
      class update_manifolds_parallel : public core::simulation_part<context> {
      public:
        update_manifolds_parallel(dt::thread_pool* pool);
        void begin(context* ctx) override;
        void process(context* ctx, const size_t &delta_time) override;
        void clear() override;
        static void update(const size_t &start, const size_t &count, context* ctx);
      private:
        dt::thread_pool* pool;
      };
      
      // в чем проблема с манифолдами? суть в том что у объектов может быть много уникальных точек пересечения
      // (больше 4 даже), минковский алгоритм находит в данный момент только одну точку за раз
      // + должен быть способ подтверждать точку пересечения, 
      // задача: найти точки уникальные, подтвердить что это уникальные точки пересечения, вычислить какие то дополнительные данные
      // возможно добавить еще шаг, для вычисления триггерных точек (точнее понятное дело что триггер нужно вычислять иначе)
      class compute_manifolds_parallel : public core::simulation_part<context> {
      public:
        compute_manifolds_parallel(dt::thread_pool* pool);
        void begin(context* ctx) override;
        void process(context* ctx, const size_t &delta_time) override;
        void clear() override;
        static void find_stair(const size_t &start, const size_t &count, context* ctx, void* ptr);
        static void compute(const size_t &start, const size_t &count, context* ctx, void* ptr);
        static void add_manifold(context* ctx, const uint32_t &pair_index, const collision::return_data &rd);
        static bool check_proxy(const uint32_t &obj1_index, const uint32_t &obj2_index, context* ctx);
      private:
        dt::thread_pool* pool;
      };
      
      // нужно посчитать соприкосновение триггерных пар
      // для них по идее достаточно SAT
      class compute_trigger_pairs_parallel : public core::simulation_part<context> {
      public:
        compute_trigger_pairs_parallel(dt::thread_pool* pool);
        void begin(context* ctx) override;
        void process(context* ctx, const size_t &delta_time) override;
        void clear() override;
        static void compute(const size_t &start, const size_t &count, context* ctx, std::atomic<uint32_t> &counter);
      private:
        dt::thread_pool* pool;
      };
      
      struct pair_checker;
      class compute_batches_parallel : public core::simulation_part<context> {
      public:
        compute_batches_parallel(dt::thread_pool* pool);
        void begin(context* ctx) override;
        void process(context* ctx, const size_t &delta_time) override;
        void clear() override;
        static void compute(const size_t &start, const size_t &count, const uint32_t &batch_id, context* ctx, pair_checker* checker, std::atomic<size_t> &counter, std::atomic<uint32_t> &batch_counter);
      private:
        dt::thread_pool* pool;
      };
    }
  }
}

#endif
