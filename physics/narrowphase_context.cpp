#include "narrowphase_context.h"
#include "collision_shape.h"

namespace devils_engine {
  namespace physics {
    namespace narrowphase {
      core::scalar calculate_combined_friction(const core::rigid_body &body0, const core::rigid_body &body1) {
        core::scalar friction = body0.friction * body1.friction;
        const core::scalar max_friction = 10.0f;
        friction = std::max(friction, -max_friction);
        friction = std::min(friction,  max_friction);
        return friction;
      }
      
      core::scalar calculate_combined_restitution(const core::rigid_body &body0, const core::rigid_body &body1) {
        return body0.restitution * body1.restitution;
      }
      
      core::scalar calculate_combined_rolling_friction(const core::rigid_body &body0, const core::rigid_body &body1) {
        core::scalar friction = body1.friction * body0.rolling_friction + body0.friction * body1.rolling_friction;
        const core::scalar max_friction = 10.0f;
        friction = std::max(friction, -max_friction);
        friction = std::min(friction,  max_friction);
        return friction;
      }
      
      core::scalar calculate_combined_spinning_friction(const core::rigid_body &body0, const core::rigid_body &body1) {
        core::scalar friction = body1.friction * body0.spinning_friction + body0.friction * body1.spinning_friction;
        const core::scalar max_friction = 10.0f;
        friction = std::max(friction, -max_friction);
        friction = std::min(friction,  max_friction);
        return friction;
      }
      
      core::scalar calculate_combined_contact_damping(const core::rigid_body &body0, const core::rigid_body &body1) {
        return body0.contact_damping + body1.contact_damping;
      }
      
      core::scalar calculate_combined_stiffness(const core::rigid_body &body0, const core::rigid_body &body1) {
        core::scalar tmp1 = 1.0f / body0.contact_stiffness;
        core::scalar tmp2 = 1.0f / body1.contact_stiffness;
        core::scalar combined_stiffness = 1.0f / (tmp1 + tmp2);
        return combined_stiffness;
      }
      
      context::context(const create_info &info) : 
        core_context(info.core_context), 
        broadphase_context(info.broadphase_context),
        data_callback{
          calculate_combined_friction,
          calculate_combined_restitution,
          calculate_combined_rolling_friction,
          calculate_combined_spinning_friction,
          calculate_combined_contact_damping,
          calculate_combined_stiffness
        },
        //default_contact_breaking_treshold(CONVEX_SHAPE_COLLISION_MARGIN)
        default_contact_breaking_treshold(0.02f)
      {
        persistent_manifold::callbacks = &manifold_callbacks;
      }
      
      context::~context() {
        for (auto manifold : manifolds_array) {
          manifolds_pool.destroy(manifold);
        }
      }
      
      persistent_manifold* context::add_manifold(const uint32_t &idx0, const uint32_t &idx1, const compute_contact_breaking_threshold_f &contact_breaking_threshold_f, const core::scalar &contact_processing_threshold) {
        const uint32_t first = std::min(idx0, idx1);
        const uint32_t second = std::max(idx0, idx1);
        const size_t key = (size_t(second) << 32) | size_t(first);
        
        std::unique_lock<std::mutex> lock(mutex);
        auto itr = manifolds.find(key);
        if (itr != manifolds.end()) return itr->second;
        
        // contact_breaking_threshold - очень тяжело вычисляется
        // нужно вычислять только по необходимости
        // еще вариант использовать только коэффициент по умолчанию
        auto ptr = manifolds_pool.create(idx0, idx1, contact_breaking_threshold_f(), contact_processing_threshold);
        manifolds.insert(std::make_pair(key, ptr));
        manifolds_array.push_back(ptr);
        return ptr;
      }
      
      void context::remove_manifold(persistent_manifold* m) {
        const uint32_t first = std::min(m->body0, m->body1);
        const uint32_t second = std::max(m->body0, m->body1);
        const size_t key = (size_t(second) << 32) | size_t(first);
        
        std::unique_lock<std::mutex> lock(mutex);
        manifolds.erase(key);
        for (size_t i = 0; i < manifolds_array.size(); ++i) {
          if (manifolds_array[i] == m) {
            std::swap(manifolds_array[i], manifolds_array.back());
            manifolds_array.pop_back();
            break;
          }
        }
        
        manifolds_pool.destroy(m);
      }
    }
  }
}
