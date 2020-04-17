#ifndef SOLVER_HELPER_H
#define SOLVER_HELPER_H

#include "solver_constraint.h"
#include <atomic>

namespace devils_engine {
  namespace physics {
    namespace solver {
      core::scalar resolve_single_constraint_row(solver::body &body_a, solver::body &body_b, const solver::constraint &c);
      core::scalar resolve_single_constraint_row_lower_limit(solver::body &body_a, solver::body &body_b, const solver::constraint &c);
      core::scalar resolve_split_penetration_impulse(solver::body &body_a, solver::body &body_b, const solver::constraint &c);
      void atomic_max(std::atomic<uint32_t> &data, const core::scalar &value);
    }
  }
}

#endif
