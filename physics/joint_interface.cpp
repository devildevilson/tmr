#include "joint_interface.h"

namespace devils_engine {
  namespace physics {
    namespace core {
      joint_interface::joint_interface(const enum joint_type &type, const uint32_t &body0, const uint32_t &body1) : 
        breaking_impulse_threshold(scalar(INFINITY)), 
        applied_impulse(0.0f), 
        body0(body0), 
        body1(body1), 
        override_num_solver_iterations(UINT32_MAX), 
        jtype(type), 
        is_enabled(true), 
        feedback_is_needed(false), 
        joint_feedback(nullptr) 
      {}
      
      bool joint_interface::enabled() const { return is_enabled; }
      void joint_interface::set_enabled(const bool value) { is_enabled = value; }
      
      void joint_interface::set_feedback(struct feedback* joint_feedback) { this->joint_feedback = joint_feedback; }
      const struct joint_interface::feedback* joint_interface::feedback() const { return joint_feedback; }
      struct joint_interface::feedback* joint_interface::feedback() { return joint_feedback; }
      
      bool joint_interface::needs_feedback() const { return feedback_is_needed; }
      void joint_interface::enable_feedback(const bool value) { feedback_is_needed = value; }
      joint_type joint_interface::type() const { return jtype; }
      scalar joint_interface::get_motor_factor(scalar pos, scalar low_lim, scalar upp_lim, scalar vel, scalar time_fact) {
        if (low_lim > upp_lim) {
          return scalar(1.0f);
        } else if (low_lim == upp_lim) {
          return scalar(0.0f);
        }
        
        scalar lim_fact = scalar(1.0f);
        scalar delta_max = vel / time_fact;
        
        if (delta_max < scalar(0.0f)) {
          
          if ((pos >= low_lim) && (pos < (low_lim - delta_max))) {
            lim_fact = (low_lim - pos) / delta_max;
          } else if (pos < low_lim) {
            lim_fact = scalar(0.0f);
          } else {
            lim_fact = scalar(1.0f);
          }
          
        } else if (delta_max > scalar(0.0f)) {
          
          if ((pos <= upp_lim) && (pos > (upp_lim - delta_max))) {
            lim_fact = (upp_lim - pos) / delta_max;
          } else if (pos > upp_lim) {
            lim_fact = scalar(0.0f);
          } else {
            lim_fact = scalar(1.0f);
          }
          
        } else {
          lim_fact = scalar(0.0f);
        }
        return lim_fact;
      }
    }
  }
}
