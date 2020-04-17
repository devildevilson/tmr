#ifndef SOLVER_CONTEXT_H
#define SOLVER_CONTEXT_H

#include <vector>
#include "solver_constraint.h"
#include "core_context.h"

// что здесь? нужно написать сначало пайплайн для солвера

namespace devils_engine {
  namespace physics {
    namespace core {
      struct context;
      struct rigid_body;
    }
    
    namespace narrowphase {
      struct context;
    }
    
    namespace solver {
      struct info {
        struct flags {
          enum {
            RANDOMIZE_ORDER                               = (1 << 0),
            FRICTION_SEPARATE                             = (1 << 1),
            USE_WARMSTARTING                              = (1 << 2),
            USE_2_FRICTION_DIRECTIONS                     = (1 << 3),
            ENABLE_FRICTION_DIRECTION_CACHING             = (1 << 4),
            DISABLE_VELOCITY_DEPENDENT_FRICTION_DIRECTION = (1 << 5),
            CACHE_FRIENDLY                                = (1 << 6),
            INTERLEAVE_CONTACT_AND_FRICTION_CONSTRAINTS   = (1 << 7),
            ALLOW_ZERO_LENGTH_FRICTION_DIRECTIONS         = (1 << 8),
            DISABLE_IMPLICIT_CONE_FRICTION                = (1 << 9),
            JOINT_FEEDBACK_IN_WORLD_SPACE                 = (1 << 10),
            JOINT_FEEDBACK_IN_JOINT_FRAME                 = (1 << 11),
            SPLIT_IMPULSE                                 = (1 << 12)
          };
          
          uint32_t container;
          
          flags();
          flags(const uint32_t &flags);
          
          bool randomize_order() const;
          bool friction_separate() const;
          bool use_warmstarting() const;
          bool use_2_friction_directions() const;
          bool enable_friction_direction_caching() const;
          bool disable_velosity_dependent_friction_direction() const;
          bool cache_friendly() const;
          bool interleave_contact_and_friction_constraints() const;
          bool allow_zero_length_friction_directions() const;
          bool disable_implicit_cone_friction() const;
          bool joint_feedback_in_world_space() const;
          bool joint_feedback_in_joint_frame() const;
          bool split_impulse() const;
        };
        
        scalar tau;
        scalar damping;  // global non-contact constraint damping, can be locally overridden by constraints during 'getInfo2'.
        scalar friction;
        //scalar timeStep;
        scalar restitution;
        uint32_t num_iterations;
        scalar max_error_reduction;
        scalar sor;           // successive over-relaxation term
        scalar erp;           // error reduction for non-contact constraints
        scalar erp2;          // error reduction for contact constraints
        scalar global_cfm;    // constraint force mixing for contacts and non-contacts
        scalar friction_erp;  // error reduction for friction constraints
        scalar friction_cfm;  // constraint force mixing for friction constraints

        scalar split_impulse_penetration_threshold;
        scalar split_impulse_turn_erp;
        scalar linear_slop;
        scalar warmstarting_factor;

        flags solver_mode;
        int resting_contact_restitution_threshold;
        uint32_t minimum_solver_batch_size;
        scalar max_gyroscopic_force;
        scalar single_axis_rolling_friction_threshold;
        scalar least_squares_residual_threshold;
        scalar restitution_velocity_threshold;
        int report_solver_analytics;
        
        inline info() :
          tau(0.6f),
          damping(1.0f),
          friction(0.3f),
          restitution(0.0f),
          num_iterations(10),
          max_error_reduction(20.0f),
          sor(1.0f),
          erp(0.2f),
          erp2(0.2f),
          global_cfm(0.0f),
          friction_erp(0.2f),
          friction_cfm(0.0f),
          split_impulse_penetration_threshold(-0.04f),
          split_impulse_turn_erp(0.1f),
          linear_slop(0.0f),
          warmstarting_factor(0.85f),
          solver_mode(flags::USE_WARMSTARTING | flags::SPLIT_IMPULSE),
          resting_contact_restitution_threshold(2),
          minimum_solver_batch_size(128),
          max_gyroscopic_force(100.0f),
          single_axis_rolling_friction_threshold(1e30f),
          least_squares_residual_threshold(0.0f),
          restitution_velocity_threshold(0.2f),
          report_solver_analytics(0)
        {}
      };
      
      struct context : public core::context_interface {
        info solver_info;
        narrowphase::context* narrowphase;
        core::context* context;
        std::vector<body> solver_bodies;
        
        uint32_t solver_constraints_size;
        uint32_t solver_friction_constraints_size;
        uint32_t solver_rolling_friction_constraints_size;
        uint32_t solver_non_contact_rows_count;
        std::vector<constraint> solver_constraints;
        std::vector<constraint> solver_friction_constraints;
        std::vector<constraint> solver_rolling_friction_constraints;
        std::vector<constraint> solver_non_contact_constraints;
        
//         uint32_t get_solver_body(const core::rigid_body* body);
      };
    }
  }
}

#endif
