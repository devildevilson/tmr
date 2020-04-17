#ifndef CONSTRAINT_INTERFACE_H
#define CONSTRAINT_INTERFACE_H

#include "physics_core.h"

namespace devils_engine {
  namespace physics {
    namespace core {
      class joint_interface {
      public:
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
          scalar *J1_linear_axis, *J1_angular_axis, *J2_linear_axis, *J2_angular_axis;

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
        
        virtual ~joint_interface() {}
        virtual void build_jacobian() = 0;
        
        
      protected:
        scalar breaking_impulse_threshold;
        scalar applied_impulse;
        uint32_t body0;
        uint32_t body1;
      };
    }
  }
}

#endif
