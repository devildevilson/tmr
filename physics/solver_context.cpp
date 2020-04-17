#include "solver_context.h"

namespace devils_engine {
  namespace physics {
    namespace solver {
      info::flags::flags() : container(0) {}
      info::flags::flags(const uint32_t &flags) : container(flags) {}
      
      bool info::flags::randomize_order() const { return (container & RANDOMIZE_ORDER) == RANDOMIZE_ORDER; }
      bool info::flags::friction_separate() const { return (container & FRICTION_SEPARATE) == FRICTION_SEPARATE; }
      bool info::flags::use_warmstarting() const { return (container & USE_WARMSTARTING) == USE_WARMSTARTING; }
      bool info::flags::use_2_friction_directions() const { return (container & USE_2_FRICTION_DIRECTIONS) == USE_2_FRICTION_DIRECTIONS; }
      bool info::flags::enable_friction_direction_caching() const { return (container & ENABLE_FRICTION_DIRECTION_CACHING) == ENABLE_FRICTION_DIRECTION_CACHING; }
      bool info::flags::disable_velosity_dependent_friction_direction() const { return (container & DISABLE_VELOCITY_DEPENDENT_FRICTION_DIRECTION) == DISABLE_VELOCITY_DEPENDENT_FRICTION_DIRECTION; }
      bool info::flags::cache_friendly() const { return (container & CACHE_FRIENDLY) == CACHE_FRIENDLY; }
      bool info::flags::interleave_contact_and_friction_constraints() const { return (container & INTERLEAVE_CONTACT_AND_FRICTION_CONSTRAINTS) == INTERLEAVE_CONTACT_AND_FRICTION_CONSTRAINTS; }
      bool info::flags::allow_zero_length_friction_directions() const { return (container & ALLOW_ZERO_LENGTH_FRICTION_DIRECTIONS) == ALLOW_ZERO_LENGTH_FRICTION_DIRECTIONS; }
      bool info::flags::disable_implicit_cone_friction() const { return (container & DISABLE_IMPLICIT_CONE_FRICTION) == DISABLE_IMPLICIT_CONE_FRICTION; }
      bool info::flags::joint_feedback_in_world_space() const { return (container & JOINT_FEEDBACK_IN_WORLD_SPACE) == JOINT_FEEDBACK_IN_WORLD_SPACE; }
      bool info::flags::joint_feedback_in_joint_frame() const { return (container & JOINT_FEEDBACK_IN_JOINT_FRAME) == JOINT_FEEDBACK_IN_JOINT_FRAME; }
      bool info::flags::split_impulse() const { return (container & SPLIT_IMPULSE) == SPLIT_IMPULSE; }
    }
  }
}
