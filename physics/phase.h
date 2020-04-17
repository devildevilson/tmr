#ifndef PHASE_H
#define PHASE_H

#include <vector>
#include "typeless_container.h"
#include "simulation_part.h"

namespace devils_engine {
  namespace physics {
    namespace core {
      template <typename PHASE_CTX, typename CTX>
      class phase : public core::simulation_part<PHASE_CTX> {
      public:
        phase(CTX* phase_context, const size_t &container_size) : phase_context(phase_context), container(container_size) {}
        ~phase() {
          for (auto part : parts) {
            container.destroy(part);
          }
        }
        
        template <typename T, typename... Args>
        T* add_part(Args&&... args) {
          T* ptr = container.create<T>(std::forward<Args>(args)...);
          parts.push_back(ptr);
          return ptr;
        }
        
        void begin(PHASE_CTX* ctx) override {
          (void)ctx;
          for (auto part : parts) {
            part->begin(phase_context);
          }
        }
        
        void process(PHASE_CTX* ctx, const size_t &delta_time) override {
          (void)ctx;
          for (auto part : parts) {
            part->process(phase_context, delta_time);
          }
        }
        
        void clear() override {
          for (auto part : parts) {
            part->clear();
          }
        }
      protected:
        CTX* phase_context;
        utils::typeless_container container;
        std::vector<simulation_part<CTX>*> parts;
      };
    }
  }
}

#endif
