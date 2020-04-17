#ifndef JACOBIAN_ENTRY_H
#define JACOBIAN_ENTRY_H

#include "physics_core.h"

namespace devils_engine {
  namespace physics {
    namespace core {
      struct jacobian_entry {
        vec4 linear_joint_axis;
        vec4 aJ;
        vec4 bJ;
        vec4 MinvJt0;
        vec4 MinvJt1;
        scalar adiag;
        
        jacobian_entry() : adiag(0.0f) {}
        jacobian_entry(
          const mat4& world2a,
          const mat4& world2b,
          const vec4& rel_pos1, 
          const vec4& rel_pos2,
          const vec4& joint_axis,
          const vec4& inertia_inv_a,
          const scalar &mass_inv_a,
          const vec4& inertia_inv_b,
          const scalar &mass_inv_b
        ) : 
          linear_joint_axis(joint_axis),
          aJ(world2a * simd::cross(rel_pos1,  linear_joint_axis)),
          bJ(world2b * simd::cross(rel_pos2, -linear_joint_axis)),
          MinvJt0(inertia_inv_a * aJ),
          MinvJt1(inertia_inv_b * bJ),
          adiag(mass_inv_a + simd::dot(MinvJt0, aJ) + mass_inv_b + simd::dot(MinvJt1, bJ))
        {
          ASSERT(adiag > 0.0f);
        }
        
        jacobian_entry(
          const vec4& joint_axis,
          const mat4& world2a,
          const mat4& world2b,
          const vec4& inertia_inv_a,
          const vec4& inertia_inv_b
        ) :
          linear_joint_axis(0,0,0,0),
          aJ(world2a *  joint_axis),
          bJ(world2b * -joint_axis),
          MinvJt0(inertia_inv_a * aJ),
          MinvJt1(inertia_inv_b * bJ),
          adiag(simd::dot(MinvJt0, aJ) + simd::dot(MinvJt1, bJ))
        {
          ASSERT(adiag > 0.0f);
        }
        
        jacobian_entry(
          const vec4& axis_in_a,
          const vec4& axis_in_b,
          const vec4& inertia_inv_a,
          const vec4& inertia_inv_b
        ) : 
          linear_joint_axis(0,0,0,0),
          aJ( axis_in_a),
          bJ(-axis_in_b),
          MinvJt0(inertia_inv_a * aJ),
          MinvJt1(inertia_inv_b * bJ),
          adiag(simd::dot(MinvJt0, aJ) + simd::dot(MinvJt1, bJ))
        {
          ASSERT(adiag > 0.0f);
        }
        
        jacobian_entry(
          const mat4& world2a,
          const vec4& rel_pos1, 
          const vec4& rel_pos2,
          const vec4& joint_axis,
          const vec4& inertia_inv_a,
          const scalar &mass_inv_a
        ) :
          linear_joint_axis(joint_axis),
          aJ(world2a * simd::cross(rel_pos1,  linear_joint_axis)),
          bJ(world2a * simd::cross(rel_pos2, -linear_joint_axis)),
          MinvJt0(inertia_inv_a * aJ),
          MinvJt1(0,0,0,0),
          adiag(mass_inv_a + simd::dot(MinvJt0, aJ))
        {
          ASSERT(adiag > 0.0f);
        }
        
        scalar get_non_diagonal(const jacobian_entry &jacB, const scalar &mass_inv_a) const {
          const jacobian_entry& jacA = *this;
          const scalar lin = mass_inv_a * simd::dot(jacA.linear_joint_axis, jacB.linear_joint_axis);
          const scalar ang = simd::dot(jacA.MinvJt0, jacB.aJ);
          return lin + ang;
        }
        
        scalar get_non_diagonal(const jacobian_entry &jacB, const scalar &mass_inv_a, const scalar &mass_inv_b) const {
          const jacobian_entry& jacA = *this;
          const vec4 lin = jacA.linear_joint_axis * jacB.linear_joint_axis;
          const vec4 ang0 = jacA.MinvJt0 * jacB.aJ;
          const vec4 ang1 = jacA.MinvJt1 * jacB.bJ;
          const vec4 lin0 = mass_inv_a * lin;
          const vec4 lin1 = mass_inv_b * lin;
          const vec4 sum = ang0 + ang1 + lin0 + lin1;
          float arr[4];
          sum.storeu(arr);
          return arr[0] + arr[1] + arr[2];
        }
        
        scalar get_relative_velocity(const vec4& linvelA, const vec4& angvelA, const vec4& linvelB, const vec4& angvelB) const {
          vec4 linrel = linvelA - linvelB;
          vec4 angvela = angvelA * aJ;
          vec4 angvelb = angvelB * bJ;
          linrel *= linear_joint_axis;
          angvela += angvelb;
          angvela += linrel;
          float arr[4];
          angvela.storeu(arr);
          const scalar rel_vel2 = arr[0] + arr[1] + arr[2];
          return rel_vel2 + EPSILON;
        }
      };
    }
  }
}

#endif
