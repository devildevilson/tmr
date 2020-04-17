#include "solver_helper.h"

namespace devils_engine {
  namespace physics {
    namespace solver {
      core::scalar resolve_single_constraint_row(solver::body &body_a, solver::body &body_b, const solver::constraint &c) {
        core::scalar delta_impulse = c.rhs - core::scalar(c.applied_impulse) * c.cfm;
//         PRINT_VAR("row c.rhs", c.rhs)
//         PRINT_VAR("row c.applied_impulse", c.applied_impulse)
//         PRINT_VAR("row c.cfm", c.cfm)
//         PRINT_VAR("row delta_impulse", delta_impulse)
        const core::scalar deltaVel1Dotn = simd::dot(c.contact_normal1, body_a.get_delta_linear_velocity()) + simd::dot(c.relpos1_cross_normal, body_a.get_delta_angular_velocity());
        const core::scalar deltaVel2Dotn = simd::dot(c.contact_normal2, body_b.get_delta_linear_velocity()) + simd::dot(c.relpos2_cross_normal, body_b.get_delta_angular_velocity());

        // const btScalar delta_rel_vel = deltaVel1Dotn - deltaVel2Dotn;
        delta_impulse -= deltaVel1Dotn * c.jac_diag_ab_inv;
        delta_impulse -= deltaVel2Dotn * c.jac_diag_ab_inv;
//         PRINT_VAR("row delta_impulse", delta_impulse)

        const core::scalar sum = core::scalar(c.applied_impulse) + delta_impulse;
        if (sum < c.lower_limit) {
          delta_impulse = c.lower_limit - c.applied_impulse;
          c.applied_impulse = c.lower_limit;
        } else if (sum > c.upper_limit) {
          delta_impulse = c.upper_limit - c.applied_impulse;
          c.applied_impulse = c.upper_limit;
        } else {
          c.applied_impulse = sum;
        }

//         PRINT_VAR("row delta_impulse", delta_impulse)
        body_a.apply_impulse(c.contact_normal1 * body_a.inv_mass, c.angular_component_a, delta_impulse);
        body_b.apply_impulse(c.contact_normal2 * body_b.inv_mass, c.angular_component_b, delta_impulse);

        return delta_impulse * (1.0f / c.jac_diag_ab_inv);
      }
      
      core::scalar resolve_single_constraint_row_lower_limit(solver::body &body_a, solver::body &body_b, const solver::constraint &c) {
        core::scalar delta_impulse = c.rhs - core::scalar(c.applied_impulse) * c.cfm;
//         PRINT_VAR("lower delta_impulse", delta_impulse)
        const core::scalar deltaVel1Dotn = simd::dot(c.contact_normal1, body_a.get_delta_linear_velocity()) + simd::dot(c.relpos1_cross_normal, body_a.get_delta_angular_velocity());
        const core::scalar deltaVel2Dotn = simd::dot(c.contact_normal2, body_b.get_delta_linear_velocity()) + simd::dot(c.relpos2_cross_normal, body_b.get_delta_angular_velocity());

        delta_impulse -= deltaVel1Dotn * c.jac_diag_ab_inv;
        delta_impulse -= deltaVel2Dotn * c.jac_diag_ab_inv;
//         PRINT_VAR("lower delta_impulse", delta_impulse)
        const core::scalar sum = core::scalar(c.applied_impulse) + delta_impulse;
        if (sum < c.lower_limit) {
          delta_impulse = c.lower_limit - c.applied_impulse;
          c.applied_impulse = c.lower_limit;
        } else {
          c.applied_impulse = sum;
        }
        
//         PRINT_VAR("lower delta_impulse", delta_impulse)
        body_a.apply_impulse(c.contact_normal1 * body_a.inv_mass, c.angular_component_a, delta_impulse);
        body_b.apply_impulse(c.contact_normal2 * body_b.inv_mass, c.angular_component_b, delta_impulse);

        return delta_impulse * (1.0f / c.jac_diag_ab_inv);
      }
      
      core::scalar resolve_split_penetration_impulse(solver::body &body_a, solver::body &body_b, const solver::constraint &c) {
        core::scalar delta_impulse = 0.0f;
        if (c.rhs_penetration != 0.0f) {
          // тут еще подсчитывалось количество gNumSplitImpulseRecoveries
          delta_impulse = c.rhs_penetration - c.applied_push_impulse * c.cfm;
          const core::scalar delta_vel1_dot_n = simd::dot(c.contact_normal1, body_a.get_push_velocity()) + simd::dot(c.relpos1_cross_normal, body_a.get_turn_velocity());
          const core::scalar delta_vel2_dot_n = simd::dot(c.contact_normal2, body_b.get_push_velocity()) + simd::dot(c.relpos2_cross_normal, body_b.get_turn_velocity());
          
          delta_impulse -= delta_vel1_dot_n * c.jac_diag_ab_inv;
          delta_impulse -= delta_vel2_dot_n * c.jac_diag_ab_inv;
          const core::scalar sum = c.applied_push_impulse + delta_impulse;
          if (sum < c.lower_limit) {
            delta_impulse = c.lower_limit - c.applied_push_impulse;
            c.applied_push_impulse = c.lower_limit;
          } else c.applied_push_impulse = sum;
          
//           PRINT_VAR("delta_impulse",delta_impulse)
          
          body_a.apply_push_impulse( c.contact_normal1 * body_a.inv_mass, c.angular_component_a,  delta_impulse);
          body_b.apply_push_impulse( c.contact_normal2 * body_b.inv_mass, c.angular_component_b,  delta_impulse);
        }
        
        return delta_impulse * (1.0f / c.jac_diag_ab_inv);
      }
      
      void atomic_max(std::atomic<uint32_t> &data, const core::scalar &value) {
        uint32_t old_val = data;
        const uint32_t val = glm::floatBitsToUint(value);
        while (glm::uintBitsToFloat(old_val) < value && !data.compare_exchange_weak(old_val, val)) {}
      }
    }
  }
}
