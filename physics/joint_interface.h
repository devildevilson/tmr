#ifndef CONSTRAINT_INTERFACE_H
#define CONSTRAINT_INTERFACE_H

#include "physics_core.h"

#ifndef DE_MATRIX_REPRESENTATION_VEC4
#define DE_MATRIX_REPRESENTATION_VEC4
#endif
// #define DE_MATRIX_REPRESENTATION_SCALAR

namespace devils_engine {
  namespace physics {
    namespace core {
      enum class joint_type {
        point2point = 3,
        hinge,
        conetwist,
        D6,
        slider,
        contact,
        D6_spring,
        gear,
        fixed,
        D6_spring_2,
        max_type
      };
      
      class joint_interface {
      public:
        enum joint_param {
          ERP_PARAMETER = 1,
          STOP_ERP_PARAMETER,
          CFM_PARAMETER,
          STOP_CFM_PARAMETER
        };
        
        struct feedback {
          vec4 applied_force_body_a;
          vec4 applied_torque_body_a;
          vec4 applied_force_body_b;
          vec4 applied_torque_body_b;
        };
        
        struct parameters1 {
          uint32_t num_constraint_rows, nub;
        };
        
        struct parameters2 {
          // integrator parameters: frames per second (1/stepsize), default error
          // reduction parameter (0..1).
          scalar fps, erp;

          // for the first and second body, pointers to two (linear and angular)
          // n*3 jacobian sub matrices, stored by rows. these matrices will have
          // been initialized to 0 on entry. if the second body is zero then the
          // J2xx pointers may be 0.
#ifdef DE_MATRIX_REPRESENTATION_VEC4
          vec4 *J1_linear_axis, *J1_angular_axis, *J2_linear_axis, *J2_angular_axis;
#else
          scalar *J1_linear_axis, *J1_angular_axis, *J2_linear_axis, *J2_angular_axis;
#endif

          // elements to jump from one row to the next in J's
          uint32_t rowskip;

          // right hand sides of the equation J*v = c + cfm * lambda. cfm is the
          // "constraint force mixing" vector. c is set to zero on entry, cfm is
          // set to a constant value (typically very small or zero) value on entry.
          scalar *constraint_error, *cfm;

          // lo and hi limits for variables (set to -/+ infinity on entry).
          scalar *lower_limit, *upper_limit;

          // number of solver iterations
          uint32_t num_iterations;

          //damping of the velocity
          scalar damping;
        };
        
        scalar breaking_impulse_threshold;
        scalar applied_impulse;
        uint32_t body0;
        uint32_t body1;
        uint32_t override_num_solver_iterations;
        
        joint_interface(const joint_type &type, const uint32_t &body0, const uint32_t &body1);
        virtual ~joint_interface() {}
        virtual void build_jacobian() = 0; // это неиспользуемый метод
        virtual void get_parameters1(parameters1 &param1) = 0;
        virtual void get_parameters2(parameters2 &param2) = 0;
        
        bool enabled() const;
        void set_enabled(const bool value);
        
        void set_feedback(struct feedback* joint_feedback);
        const struct feedback* feedback() const;
        struct feedback* feedback();
        
        bool needs_feedback() const;
        void enable_feedback(const bool value);
        joint_type type() const;
        
        ///override the default global value of a parameter (such as ERP or CFM), optionally provide the axis (0..5).
        ///If no axis is provided, it uses the default axis for this constraint.
        virtual void set_param(const uint32_t &num, const scalar &value, const uint32_t &axis = UINT32_MAX) = 0;

        ///return the local value of parameter
        virtual scalar get_param(const uint32_t &num, const uint32_t &axis = UINT32_MAX) const = 0;
        
        scalar get_motor_factor(scalar pos, scalar low_lim, scalar upp_lim, scalar vel, scalar time_fact);
      protected:
        enum joint_type jtype;
        bool is_enabled;
        bool feedback_is_needed;
        struct feedback* joint_feedback;
      };
    }
  }
}

#endif
