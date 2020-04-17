#ifndef PHYSICS_NEW_H
#define PHYSICS_NEW_H

#include <vector>
#include "typeless_container.h"
#include "simulation_part.h"
#include "core_context.h"
#include "shared_time_constant.h"

namespace devils_engine {
//   namespace physics {
//     template <typename CTX>
//     class interpolate_part {
//     public:
//       virtual ~interpolate_part() {}
//       virtual void process(CTX* ctx, const float &alpha) = 0;
//     };
//   }
  
  namespace systems {
    template <typename CTX>
    class physics {
    public:
      physics(const size_t &container_size) : container(container_size), accumulator(0), interpolation(nullptr) {}
      ~physics() {
        for (auto part : parts) {
          container.destroy(part);
        }
        
        for (auto part : post_parts) {
          container.destroy(part);
        }
        
        if (interpolation != nullptr) container.destroy(interpolation);
        
        for (auto context : context_array) {
          container.destroy(context);
        }
      }
      
      template <typename T, typename... Args>
      T* add_part(Args&&... args) {
        T* ptr = container.create<T>(std::forward<Args>(args)...);
        parts.push_back(ptr);
        return ptr;
      }
      
      template <typename T, typename... Args>
      T* add_post_part(Args&&... args) {
        T* ptr = container.create<T>(std::forward<Args>(args)...);
        post_parts.push_back(ptr);
        return ptr;
      }
      
      template <typename T, typename... Args>
      T* add_interpolation(Args&&... args) {
        if (interpolation != nullptr) container.destroy(interpolation);
        auto ptr = container.create<T>(std::forward<Args>(args)...);
        interpolation = ptr;
        return ptr;
      }
      
      template <typename T, typename... Args>
      T* add_context(Args&&... args) {
        T* ptr = container.create<T>(std::forward<Args>(args)...);
        context_array.push_back(ptr);
        return ptr;
      }
      
      void begin(CTX* ctx) {
        for (auto part : parts) {
          part->begin(ctx);
        }
        
        for (auto part : post_parts) {
          part->begin(ctx);
        }
      }
      
      void update(const size_t &time, const size_t &simulation_interval, const uint32_t &max_steps, CTX* ctx) {
        begin(ctx);
        
        accumulator += std::min(time, size_t(ACCUMULATOR_MAX_CONSTANT));
        if (accumulator > ACCUMULATOR_MAX_CONSTANT) {
          accumulator = accumulator % ACCUMULATOR_MAX_CONSTANT;
          // по идее это лаги 
        }
        
        size_t steps = max_steps;
        while (accumulator >= simulation_interval && steps > 0) {
          for (auto part : parts) {
            part->process(ctx, simulation_interval);
          }
          
          --steps;
          accumulator -= simulation_interval;
        }
        
        const float alpha = float(accumulator) / float(simulation_interval);
        interpolation->process(ctx, alpha);
        
        for (auto part : post_parts) {
          part->process(ctx, simulation_interval);
        }
      }
      
      void clear() {
        for (auto part : parts) {
          part->clear();
        }
        
        for (auto part : post_parts) {
          part->clear();
        }
      }
    private:
      utils::typeless_container container;
      size_t accumulator;
      devils_engine::physics::core::interpolate_part<CTX>* interpolation;
      std::vector<devils_engine::physics::core::simulation_part<CTX>*> parts;
      std::vector<devils_engine::physics::core::simulation_part<CTX>*> post_parts;
      std::vector<devils_engine::physics::core::context_interface*> context_array;
    };
  }
}

#endif
