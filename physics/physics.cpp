#include "physics.h"

#include "shared_time_constant.h"

namespace devils_engine {
  namespace systems {
//     physics::physics(const size_t &container_size) : container(container_size), accumulator(0) {}
//     physics::~physics() {
//       for (auto part : parts) {
//         container.destroy(part);
//       }
//     }
//     
//     void physics::update(const size_t &time, const size_t &simulation_interval, const uint32_t &max_steps, devils_engine::physics::core_context* ctx) {
//       for (auto part : parts) {
//         part->begin(ctx);
//       }
//       
//       for (auto part : post_parts) {
//         part->begin(ctx);
//       }
//       
//       accumulator += std::min(time, size_t(ACCUMULATOR_MAX_CONSTANT));
//       if (accumulator > ACCUMULATOR_MAX_CONSTANT) {
//         accumulator = accumulator % ACCUMULATOR_MAX_CONSTANT;
//         // по идее это лаги 
//       }
//       
//       size_t steps = max_steps;
//       while (accumulator >= simulation_interval && steps > 0) {
//         for (auto part : parts) {
//           part->process(ctx);
//         }
//         
//         --steps;
//         accumulator -= simulation_interval;
//       }
//       
//       const float alpha = float(accumulator) / float(simulation_interval);
//       interpolation->process(ctx, alpha);
//       
//       for (auto part : post_parts) {
//         part->process(ctx);
//       }
//     }
//     
//     void physics::clear() {
//       for (auto part : parts) {
//         part->clear();
//       }
//       
//       for (auto part : post_parts) {
//         part->clear();
//       }
//     }
  }
}
