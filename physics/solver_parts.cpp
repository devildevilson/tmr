#include "solver_parts.h"

#include "solver_context.h"
#include "narrowphase_context.h"
#include "solver_constraint.h"
#include "physics_context.h"
#include "works_utils.h"
#include "joint_interface.h"
#include "solver_helper.h"
#include "Utility.h"

namespace devils_engine {
  namespace physics {
    namespace solver {
      scalar restitution_curve(scalar rel_vel, scalar restitution, scalar velocity_threshold) {
        if (glm::abs(rel_vel) < velocity_threshold) return 0.0f;
        return restitution * -rel_vel;
      }
      
      vec4 apply_anisotropic_friction(const core::rigid_body* body, const core::transform* trans, const vec4 &friction_dir, const uint32_t &mode) {
        vec4 f_dir = friction_dir;
        // анизатропное трение я еще не сделал
        if (body != nullptr && body->has_anisotropic_friction(mode)) {
          vec4 loc_lateral = f_dir * trans->rot;
          const vec4 fric = body->anisotropic_friction;
          loc_lateral *= fric;
          f_dir = trans->rot * loc_lateral;
        }
        
        return f_dir;
      }
      
      void contact_constraint_init(context* ctx, const narrowphase::manifold_point &cp, const vec4 &rel_pos1, const vec4 &rel_pos2, const size_t &delta_time, scalar &relaxation, constraint &c) {
        auto &solver_body1 = ctx->solver_bodies[c.solver_body_a];
        auto &solver_body2 = ctx->solver_bodies[c.solver_body_b];
        const auto body0 = solver_body1.body_index == UINT32_MAX ? nullptr : &ctx->context->bodies[c.solver_body_a];
        const auto body1 = solver_body2.body_index == UINT32_MAX ? nullptr : &ctx->context->bodies[c.solver_body_b];
        
        const scalar time = MCS_TO_SEC(delta_time);
        const scalar inv_time = 1.0f / time;
        scalar cfm = ctx->solver_info.global_cfm;
        scalar erp = ctx->solver_info.erp2;
        relaxation = ctx->solver_info.sor;
        
        if (cp.contact_point_flags.has_contact_cfm() || cp.contact_point_flags.has_contact_erp()) {
          if (cp.contact_point_flags.has_contact_cfm()) cfm = cp.contact_cfm;
          if (cp.contact_point_flags.has_contact_erp()) erp = cp.contact_erp;
        } else if (cp.contact_point_flags.contact_stiffness_damping()) {
          scalar denom = time * cp.combined_contact_stiffness1 + cp.combined_contact_damping1;
          if (denom < EPSILON) denom = EPSILON;
          cfm = 1.0f / denom;
          erp = (time * cp.combined_contact_stiffness1) / denom;
        }
        
        cfm *= inv_time;
        
        const vec4 torque_axis0 = simd::cross(rel_pos1, cp.world_normal_b);
        const vec4 torque_axis1 = simd::cross(rel_pos2, cp.world_normal_b);
        c.angular_component_a = body0 != nullptr ? body0->inv_inertia_tensor *  torque_axis0 * body0->angular_factor : vec4(0,0,0,0);
        c.angular_component_b = body1 != nullptr ? body1->inv_inertia_tensor * -torque_axis0 * body1->angular_factor : vec4(0,0,0,0);
        
        {
          vec4 vec;
          scalar denom0 = 0.0f;
          scalar denom1 = 0.0f;
          
          vec = simd::cross(c.angular_component_a, rel_pos1);
          denom0 = body0 != nullptr ? body0->inverse_mass + simd::dot(cp.world_normal_b, vec) : 0.0f;
          vec = simd::cross(c.angular_component_b, rel_pos2);
          denom1 = body1 != nullptr ? body1->inverse_mass + simd::dot(cp.world_normal_b, vec) : 0.0f;
          c.jac_diag_ab_inv = relaxation / (denom0 + denom1 + cfm);
        }
        
        c.contact_normal1 = body0 != nullptr ? cp.world_normal_b : vec4(0,0,0,0);
        c.relpos1_cross_normal = body0 != nullptr ? torque_axis0 : vec4(0,0,0,0);
        
        c.contact_normal2 = body1 != nullptr ? -cp.world_normal_b : vec4(0,0,0,0);
        c.relpos2_cross_normal = body1 != nullptr ? -torque_axis1 : vec4(0,0,0,0);
        
        scalar restitution = 0.0f;
        scalar penetration = cp.distance1 + ctx->solver_info.linear_slop;
        
        {
          vec4 vel1 = body0 != nullptr ? body0->get_velocity_in_local_point(rel_pos1) : vec4(0,0,0,0);
          vec4 vel2 = body1 != nullptr ? body1->get_velocity_in_local_point(rel_pos2) : vec4(0,0,0,0);
          
          vec4 vel = vel1 - vel2;
          scalar rel_vel = simd::dot(cp.world_normal_b, vel);
          
          c.friction = cp.combined_friction;
          
          restitution = restitution_curve(rel_vel, cp.combined_restitution, ctx->solver_info.restitution_velocity_threshold);
          if (restitution <= 0.0f) restitution = 0.0f;
        }
        
        // warm starting
        if (ctx->solver_info.solver_mode.use_warmstarting()) {
          c.applied_impulse = cp.applied_impulse * ctx->solver_info.warmstarting_factor;
          solver_body1.apply_impulse( c.contact_normal1 * solver_body1.inv_mass, c.angular_component_a,  c.applied_impulse);
          solver_body2.apply_impulse(-c.contact_normal2 * solver_body2.inv_mass,-c.angular_component_b, -c.applied_impulse);
        } else {
          c.applied_impulse = 0.0f;
        }
        
        c.applied_push_impulse = 0.0f;
        
        {
          vec4 externalForceImpulseA = solver_body1.external_force_impulse;
          vec4 externalTorqueImpulseA = solver_body1.external_torque_impulse;
          vec4 externalForceImpulseB = solver_body2.external_force_impulse;
          vec4 externalTorqueImpulseB = solver_body2.external_torque_impulse;
          
#ifdef _DEBUG
          if (body0 == nullptr) ASSERT(simd::length2(externalForceImpulseA)  == 0.0f);
          if (body0 == nullptr) ASSERT(simd::length2(externalTorqueImpulseA) == 0.0f);
          if (body1 == nullptr) ASSERT(simd::length2(externalForceImpulseB)  == 0.0f);
          if (body1 == nullptr) ASSERT(simd::length2(externalTorqueImpulseB) == 0.0f);
#endif

          scalar vel1Dotn = simd::dot(c.contact_normal1, solver_body1.linear_velocity + externalForceImpulseA) + simd::dot(c.relpos1_cross_normal, solver_body1.angular_velocity + externalTorqueImpulseA);
          scalar vel2Dotn = simd::dot(c.contact_normal2, solver_body2.linear_velocity + externalForceImpulseB) + simd::dot(c.relpos2_cross_normal, solver_body2.angular_velocity + externalTorqueImpulseB);
          scalar rel_vel = vel1Dotn + vel2Dotn;

          scalar positionalError = 0.f;
          scalar velocityError = restitution - rel_vel;  // * damping;

          if (penetration > 0) {
            positionalError = 0;
            velocityError -= penetration * inv_time;
          } else {
            positionalError = -penetration * erp * inv_time;
          }

          scalar penetrationImpulse = positionalError * c.jac_diag_ab_inv;
          scalar velocityImpulse = velocityError * c.jac_diag_ab_inv;
          
//           PRINT_VAR("penetration",penetration)

          if (!ctx->solver_info.solver_mode.split_impulse() || (penetration > ctx->solver_info.split_impulse_penetration_threshold)) {
            //combine position and velocity into rhs
            c.rhs = penetrationImpulse + velocityImpulse;  //-solverConstraint.m_contactNormal1.dot(bodyA->m_externalForce*bodyA->m_invMass-bodyB->m_externalForce/bodyB->m_invMass)*solverConstraint.m_jacDiagABInv;
            c.rhs_penetration = 0.f;
          } else {
            //split position and velocity into rhs and m_rhsPenetration
            c.rhs = velocityImpulse;
            c.rhs_penetration = penetrationImpulse;
          }
          
          c.cfm = cfm * c.jac_diag_ab_inv;
          c.lower_limit = 0;
          c.upper_limit = 1e10f;
        }
      }
      
      void init_torsional_friction_constraint(
        context* ctx, 
        const narrowphase::manifold_point &cp, 
        const vec4 &normal_axis, 
        const vec4 &rel_pos1, 
        const vec4 &rel_pos2, 
        const scalar &combined_torsional_friction,
        const scalar &relaxation,
        const scalar &desired_velocity,
        const scalar &cfm_slip,
        constraint &c
      ) {
        vec4 normalAxis(0, 0, 0, 0);
        
        auto &solver_body1 = ctx->solver_bodies[c.solver_body_a];
        auto &solver_body2 = ctx->solver_bodies[c.solver_body_b];
        const auto body0 = solver_body1.body_index == UINT32_MAX ? nullptr : &ctx->context->bodies[solver_body1.body_index];
        const auto body1 = solver_body2.body_index == UINT32_MAX ? nullptr : &ctx->context->bodies[solver_body2.body_index];

        c.contact_normal1 = normalAxis;
        c.contact_normal2 = -normalAxis;

        c.friction = combined_torsional_friction;
        c.original_contact_point = nullptr;

        c.applied_impulse = 0.f;
        c.applied_push_impulse = 0.f;

        {
          const vec4 ftorqueAxis1 = -normal_axis;
          c.relpos1_cross_normal = ftorqueAxis1;
          c.angular_component_a = solver_body1.body_index != UINT32_MAX ? body0->inv_inertia_tensor * ftorqueAxis1 * body0->angular_factor : vec4(0, 0, 0, 0);
        }
        {
          const vec4 ftorqueAxis1 = normal_axis;
          c.relpos2_cross_normal = ftorqueAxis1;
          c.angular_component_b = solver_body2.body_index != UINT32_MAX ? body1->inv_inertia_tensor * ftorqueAxis1 * body1->angular_factor : vec4(0, 0, 0, 0);
        }

        {
          vec4 iMJaA = solver_body1.body_index != UINT32_MAX ? body0->inv_inertia_tensor * c.relpos1_cross_normal : vec4(0, 0, 0, 0);
          vec4 iMJaB = solver_body2.body_index != UINT32_MAX ? body1->inv_inertia_tensor * c.relpos2_cross_normal : vec4(0, 0, 0, 0);
          scalar sum = 0;
          sum += simd::dot(iMJaA, c.relpos1_cross_normal);
          sum += simd::dot(iMJaB, c.relpos2_cross_normal);
          c.jac_diag_ab_inv = scalar(1.) / sum;
        }

        {
          scalar rel_vel;
          scalar vel1Dotn = simd::dot(c.contact_normal1,      solver_body1.body_index != UINT32_MAX ? solver_body1.linear_velocity + solver_body1.external_force_impulse : vec4(0, 0, 0, 0)) + 
                            simd::dot(c.relpos1_cross_normal, solver_body1.body_index != UINT32_MAX ? solver_body1.angular_velocity : vec4(0, 0, 0, 0));
                            
          scalar vel2Dotn = simd::dot(c.contact_normal2,      solver_body2.body_index != UINT32_MAX ? solver_body2.linear_velocity + solver_body2.external_force_impulse : vec4(0, 0, 0, 0)) + 
                            simd::dot(c.relpos2_cross_normal, solver_body2.body_index != UINT32_MAX ? solver_body2.angular_velocity : vec4(0, 0, 0, 0));

          rel_vel = vel1Dotn + vel2Dotn;

          // btScalar positionalError = 0.f;

          scalar velocityError = desired_velocity - rel_vel;
          scalar velocityImpulse = velocityError * c.jac_diag_ab_inv;
          c.rhs = velocityImpulse;
          c.cfm = cfm_slip;
          c.lower_limit = -c.friction;
          c.upper_limit =  c.friction;
        }
        
        (void)cp;
        (void)rel_pos1;
        (void)rel_pos2;
        (void)relaxation;
      }
      
      void init_friction_constraint(
        context* ctx, 
        const narrowphase::manifold_point &cp, 
        const vec4 &normal_axis, 
        const vec4 &rel_pos1, 
        const vec4 &rel_pos2, 
        const scalar &relaxation,
        const scalar &desired_velocity,
        const scalar &cfm_slip,
        const size_t &delta_time,
        constraint &c
      ) {
        const auto& solverBodyA = ctx->solver_bodies[c.solver_body_a];
        const auto& solverBodyB = ctx->solver_bodies[c.solver_body_b];
        const auto body0 = c.solver_body_a != UINT32_MAX ? &ctx->context->bodies[c.solver_body_a] : nullptr;
        const auto body1 = c.solver_body_b != UINT32_MAX ? &ctx->context->bodies[c.solver_body_b] : nullptr;

        c.friction = cp.combined_friction;
        c.original_contact_point = nullptr;

        c.applied_impulse = 0.f;
        c.applied_push_impulse = 0.f;
        
//         std::cout << "\n";

        if (body0 != nullptr) {
          c.contact_normal1 = normal_axis;
          vec4 ftorqueAxis1 = simd::cross(rel_pos1, c.contact_normal1);
          c.relpos1_cross_normal = ftorqueAxis1;
          c.angular_component_a = body0->inv_inertia_tensor * ftorqueAxis1 * body0->angular_factor;
        } else {
          c.contact_normal1 = vec4(0, 0, 0, 0);
          c.relpos1_cross_normal = vec4(0, 0, 0, 0);
          c.angular_component_a = vec4(0, 0, 0, 0);
        }

        if (body1 != nullptr) {
          c.contact_normal2 = -normal_axis;
          vec4 ftorqueAxis1 = simd::cross(rel_pos2, c.contact_normal2);
          c.relpos2_cross_normal = ftorqueAxis1;
          c.angular_component_b = body1->inv_inertia_tensor * ftorqueAxis1 * body1->angular_factor;
        } else {
          c.contact_normal2 = vec4(0, 0, 0, 0);
          c.relpos2_cross_normal = vec4(0, 0, 0, 0);
          c.angular_component_b = vec4(0, 0, 0, 0);
        }

        {
          vec4 vec;
          scalar denom0 = 0.f;
          scalar denom1 = 0.f;
          
          if (body0 != nullptr) {
            vec = simd::cross(c.angular_component_a, rel_pos1);
            denom0 = body0->inverse_mass + simd::dot(normal_axis, vec);
          }
          
          if (body1 != nullptr) {
            vec = simd::cross(-c.angular_component_b, rel_pos2);
            denom1 = body1->inverse_mass + simd::dot(normal_axis, vec);
          }
          
          ASSERT(body0 != nullptr || body1 != nullptr);
          
          scalar denom = relaxation / (denom0 + denom1);
          c.jac_diag_ab_inv = denom;
          
//           PRINT_VAR("friction denom0",denom0);
//           PRINT_VAR("friction denom1",denom1);
//           PRINT_VAR("friction denom",denom);
        }

        {
          scalar rel_vel;
          scalar vel1Dotn = simd::dot(c.contact_normal1,      body0 != nullptr ? solverBodyA.linear_velocity + solverBodyA.external_force_impulse : vec4(0, 0, 0, 0)) + 
                            simd::dot(c.relpos1_cross_normal, body0 != nullptr ? solverBodyA.angular_velocity : vec4(0, 0, 0, 0));
          scalar vel2Dotn = simd::dot(c.contact_normal2,      body1 != nullptr ? solverBodyB.linear_velocity + solverBodyB.external_force_impulse : vec4(0, 0, 0, 0)) + 
                            simd::dot(c.relpos2_cross_normal, body1 != nullptr ? solverBodyB.angular_velocity : vec4(0, 0, 0, 0));

          rel_vel = vel1Dotn + vel2Dotn;

          // scalar positionalError = 0.f;

          scalar velocityError = desired_velocity - rel_vel;
          scalar velocityImpulse = velocityError * c.jac_diag_ab_inv;

          scalar penetrationImpulse = scalar(0);

          if (cp.contact_point_flags.friction_anchor()) {
            scalar distance = simd::dot(cp.world_position_a - cp.world_position_b, normal_axis);
            scalar positionalError = -distance * ctx->solver_info.friction_erp / MCS_TO_SEC(delta_time);
            penetrationImpulse = positionalError * c.jac_diag_ab_inv;
          }

          c.rhs = penetrationImpulse + velocityImpulse;
          c.rhs_penetration = 0.f;
          c.cfm = cfm_slip;
          c.lower_limit = -c.friction;
          c.upper_limit =  c.friction;
          
//           PRINT_VAR("friction penetrationImpulse",penetrationImpulse);
//           PRINT_VAR("friction rel_vel",rel_vel);
//           PRINT_VAR("friction c.jac_diag_ab_inv",c.jac_diag_ab_inv);
//           PRINT_VAR("friction desired_velocity",desired_velocity);
//           PRINT_VAR("friction velocityImpulse",velocityImpulse);
//           PRINT_VAR("friction c.rhs",c.rhs);
//           std::cout << "\n";
        }
      }
      
      void set_friction_constraint_impulse(context* ctx, const narrowphase::manifold_point &cp, constraint &c) {
        auto& solverBodyA = ctx->solver_bodies[c.solver_body_a];
        auto& solverBodyB = ctx->solver_bodies[c.solver_body_b];
        const auto body0 = c.solver_body_a == UINT32_MAX ? &ctx->context->bodies[c.solver_body_a] : nullptr;
        const auto body1 = c.solver_body_b == UINT32_MAX ? &ctx->context->bodies[c.solver_body_b] : nullptr;

        {
          auto& frictionConstraint1 = ctx->solver_friction_constraints[c.friction_index];
          if (ctx->solver_info.solver_mode.use_warmstarting()) {
            frictionConstraint1.applied_impulse = cp.applied_impulse_lateral1 * ctx->solver_info.warmstarting_factor;
            if (body0) solverBodyA.apply_impulse( frictionConstraint1.contact_normal1 * body0->inverse_mass,  frictionConstraint1.angular_component_a,  frictionConstraint1.applied_impulse);
            if (body1) solverBodyB.apply_impulse(-frictionConstraint1.contact_normal2 * body1->inverse_mass, -frictionConstraint1.angular_component_b, -frictionConstraint1.applied_impulse);
          } else {
            frictionConstraint1.applied_impulse = 0.f;
          }
        }

        if (ctx->solver_info.solver_mode.use_2_friction_directions()) {
          auto& frictionConstraint2 = ctx->solver_friction_constraints[c.friction_index + 1];
          if (ctx->solver_info.solver_mode.use_warmstarting()) {
            frictionConstraint2.applied_impulse = cp.applied_impulse_lateral2 * ctx->solver_info.warmstarting_factor;
            if (body0) solverBodyA.apply_impulse( frictionConstraint2.contact_normal1 * body0->inverse_mass,  frictionConstraint2.angular_component_a,  frictionConstraint2.applied_impulse);
            if (body1) solverBodyB.apply_impulse(-frictionConstraint2.contact_normal2 * body1->inverse_mass, -frictionConstraint2.angular_component_b, -frictionConstraint2.applied_impulse);
          } else {
            frictionConstraint2.applied_impulse = 0.f;
          }
        }
      }
      
      void convert_joint(context* ctx, core::joint_interface* constraint, struct constraint* constraint_row, const core::joint_interface::parameters1 &param1, const size_t &delta_time) {
        const auto solver_body_a = &ctx->solver_bodies[constraint->body0];
        const auto solver_body_b = constraint->body1 == UINT32_MAX ? nullptr : &ctx->solver_bodies[constraint->body1];
        const auto body_a = &ctx->context->bodies[constraint->body0];
        const auto body_b = constraint->body1 == UINT32_MAX ? nullptr : &ctx->context->bodies[constraint->body1];
        
        uint32_t override_num_iterations = constraint->override_num_solver_iterations > 0 ? constraint->override_num_solver_iterations : ctx->solver_info.num_iterations;
        override_num_iterations = std::min(override_num_iterations, ctx->solver_info.num_iterations);
        
        for (uint32_t i = 0; i < param1.num_constraint_rows; ++i) {
          memset(&constraint_row[i], 0, sizeof(struct constraint));
//           constraint_row[i] = struct constraint;
          constraint_row[i].lower_limit = -INFINITY;
          constraint_row[i].upper_limit =  INFINITY;
          constraint_row[i].applied_impulse = 0.0f;
          constraint_row[i].applied_push_impulse = 0.0f;
          constraint_row[i].solver_body_a = constraint->body0;
          constraint_row[i].solver_body_b = constraint->body1;
          constraint_row[i].override_num_solver_iterations = override_num_iterations;
        }
        
        // лучше чтобы J1_linear_axis передавался как угодно но не через scalar*
        // возможно стоит переделать на указатели vec4
        core::joint_interface::parameters2 param2;
        param2.fps = 1.0f / MCS_TO_SEC(delta_time);
        param2.erp = ctx->solver_info.erp;
#ifdef DE_MATRIX_REPRESENTATION_VEC4
        param2.J1_linear_axis  = &constraint_row->contact_normal1;
        param2.J1_angular_axis = &constraint_row->relpos1_cross_normal;
        param2.J2_linear_axis  = &constraint_row->contact_normal2;
        param2.J2_angular_axis = &constraint_row->relpos2_cross_normal;
        param2.rowskip = sizeof(struct constraint) / sizeof(vec4);
        ASSERT(param2.rowskip * sizeof(vec4) == sizeof(struct constraint));
#else
        param2.J1_linear_axis  = &constraint_row->contact_normal1.arr[0];
        param2.J1_angular_axis = &constraint_row->relpos1_cross_normal.arr[0];
        param2.J2_linear_axis  = &constraint_row->contact_normal2.arr[0];
        param2.J2_angular_axis = &constraint_row->relpos2_cross_normal.arr[0];
        param2.rowskip = sizeof(struct constraint) / sizeof(scalar);
        ASSERT(param2.rowskip * sizeof(scalar) == sizeof(struct constraint));
#endif
        param2.constraint_error = &constraint_row->rhs;
        param2.damping = ctx->solver_info.damping;
        constraint_row->cfm = ctx->solver_info.global_cfm;
        param2.cfm = &constraint_row->cfm;
        param2.lower_limit = &constraint_row->lower_limit;
        param2.upper_limit = &constraint_row->upper_limit;
        param2.num_iterations = ctx->solver_info.num_iterations;
        constraint->get_parameters2(param2); // этот метод крайне не эффективен
        
        for (uint32_t i = 0; i < param1.num_constraint_rows; ++i) {
          auto &c = constraint_row[i];
          c.upper_limit = std::min(c.upper_limit,  constraint->breaking_impulse_threshold);
          c.lower_limit = std::max(c.lower_limit, -constraint->breaking_impulse_threshold);
          c.original_contact_point = constraint;
          
          {
            const vec4& ftorqueAxis1 = c.relpos1_cross_normal;
            c.angular_component_a = body_a->inv_inertia_tensor * ftorqueAxis1 * body_a->angular_factor;
          }
          {
            const vec4& ftorqueAxis2 = c.relpos2_cross_normal;
            c.angular_component_b = body_b == nullptr ? vec4 (0,0,0,0) : body_b->inv_inertia_tensor * ftorqueAxis2 * body_b->angular_factor;
          }

          {
            vec4 iMJlA = c.contact_normal1 * body_a->inverse_mass;
            vec4 iMJaA = body_a->inv_inertia_tensor * c.relpos1_cross_normal;
            vec4 iMJlB = c.contact_normal2 * (body_b == nullptr ? 0.0f : body_b->inverse_mass);  //sign of normal?
            vec4 iMJaB = (body_b == nullptr ? core::mat4() : body_b->inv_inertia_tensor) * c.relpos2_cross_normal;

            scalar sum = simd::dot(iMJlA, c.contact_normal1);
            sum += simd::dot(iMJaA, c.relpos1_cross_normal);
            sum += simd::dot(iMJlB, c.contact_normal2);
            sum += simd::dot(iMJaB, c.relpos2_cross_normal);
            scalar fsum = glm::abs(sum);
            ASSERT(fsum > EPSILON);
            scalar sorRelaxation = 1.f;  //todo: get from globalInfo?
            c.jac_diag_ab_inv = fsum > EPSILON ? sorRelaxation / sum : 0.f;
          }

          {
            scalar rel_vel;
            vec4 externalForceImpulseA = body_a != nullptr ? solver_body_a->external_force_impulse : vec4(0, 0, 0, 0);
            vec4 externalTorqueImpulseA = body_a != nullptr ? solver_body_a->external_torque_impulse : vec4(0, 0, 0, 0);

            vec4 externalForceImpulseB = body_b != nullptr ? solver_body_b->external_force_impulse : vec4(0, 0, 0, 0);
            vec4 externalTorqueImpulseB = body_b != nullptr ? solver_body_b->external_torque_impulse : vec4(0, 0, 0, 0);

            scalar vel1Dotn = simd::dot(c.contact_normal1, (body_a != nullptr ? body_a->linear_velocity : vec4(0,0,0,0)) + externalForceImpulseA) + 
                              simd::dot(c.relpos1_cross_normal, (body_a != nullptr ? body_a->linear_velocity : vec4(0,0,0,0)) + externalTorqueImpulseA);

            scalar vel2Dotn = simd::dot(c.contact_normal2, (body_b != nullptr ? body_b->linear_velocity : vec4(0,0,0,0)) + externalForceImpulseB) + 
                              simd::dot(c.relpos2_cross_normal, (body_b != nullptr ? body_b->linear_velocity : vec4(0,0,0,0)) + externalTorqueImpulseB);

            rel_vel = vel1Dotn + vel2Dotn;
            scalar restitution = 0.f;
            scalar positionalError = c.rhs;  //already filled in by getConstraintInfo2
            scalar velocityError = restitution - rel_vel * param2.damping;
            scalar penetrationImpulse = positionalError * c.jac_diag_ab_inv;
            scalar velocityImpulse = velocityError * c.jac_diag_ab_inv;
            c.rhs = penetrationImpulse + velocityImpulse;
            c.applied_impulse = 0.f;
          }
        }
      }
      
      convert_constraints_parallel::convert_constraints_parallel(dt::thread_pool* pool) : info1_memory_size(0), info1_memory(nullptr), pool(pool) {}
      convert_constraints_parallel::~convert_constraints_parallel() { delete [] info1_memory; }
      void convert_constraints_parallel::begin(context* ctx) { (void)ctx; }
      void convert_constraints_parallel::process(context* ctx, const size_t &delta_time) {
        const size_t solver_contact_constraint_size = ctx->narrowphase->manifolds_array.size() * MANIFOLD_CACHE_SIZE;
        if (ctx->solver_constraints.size() < solver_contact_constraint_size) {
          ctx->solver_constraints.resize(solver_contact_constraint_size);
        }
        
        const size_t solver_friction_constraint_size = ctx->solver_info.solver_mode.use_2_friction_directions() ? solver_contact_constraint_size * 2 : solver_contact_constraint_size;
        if (ctx->solver_friction_constraints.size() < solver_friction_constraint_size) {
          ctx->solver_friction_constraints.resize(solver_friction_constraint_size);
        }
        
        const size_t solver_rolling_constraint_size = solver_contact_constraint_size * 3;
        if (ctx->solver_rolling_friction_constraints.size() < solver_rolling_constraint_size) {
          ctx->solver_rolling_friction_constraints.resize(solver_rolling_constraint_size);
        }
        
        if (ctx->solver_bodies.size() != ctx->context->bodies.size()) {
          std::vector<solver::body> array(ctx->context->bodies.size());
          std::swap(ctx->solver_bodies, array);
          //ctx->solver_bodies.resize(ctx->context->bodies.size());
        }
        
        if (info1_memory_size < ctx->context->joints.size()) {
          if (info1_memory != nullptr) delete [] info1_memory;
          info1_memory_size = sizeof(core::joint_interface::parameters1)*ctx->context->joints.size();
          info1_memory = new char[info1_memory_size];
        }
        
        memset(info1_memory, 0, info1_memory_size);
        
        std::atomic<uint32_t> constraints_size(0);
        std::atomic<uint32_t> frictions_size(0);
        std::atomic<uint32_t> rolling_frictions_size(0);
        std::atomic<uint32_t> row_counter(0);
        
        utils::submit_works(pool, ctx->context->bodies.size(), convert_bodies, ctx, delta_time);
        utils::submit_works(pool, ctx->narrowphase->manifolds_array.size(), convert, ctx, delta_time, std::ref(constraints_size), std::ref(frictions_size), std::ref(rolling_frictions_size));
        utils::submit_works(pool, ctx->context->joints.size(), convert_joints1, ctx, info1_memory, std::ref(row_counter));
        const uint32_t row_count = row_counter;
        if (ctx->solver_non_contact_constraints.size() < row_count) {
          ctx->solver_non_contact_constraints.resize(row_count);
        }
        row_counter = 0;
        
        utils::submit_works(pool, ctx->context->joints.size(), convert_joints2, ctx, info1_memory, delta_time, row_count, std::ref(row_counter));
        
        ctx->solver_constraints_size = constraints_size;
        ctx->solver_friction_constraints_size = frictions_size;
        ctx->solver_rolling_friction_constraints_size = rolling_frictions_size;
        ctx->solver_non_contact_rows_count = row_count;
      }
      
      void convert_constraints_parallel::clear() {}
      
      void convert_constraints_parallel::init_body(context* ctx, const uint32_t &index, const size_t &delta_time) {
//         if (ctx->solver_bodies[index].body_index != UINT32_MAX) return;
        
        // проблема в том что может потребоваться пустой боди сделать
        // с другой стороны это вообще нафиг не нужно
        // (нужно только для кеша), но у нас есть несколько переменных 
        // которые похоже что будут использоваться 
        // тут проблема в том что нам не нужно столько тел
        
        auto &b = ctx->solver_bodies[index];
        b.delta_linear_velocity = vec4(0.0f, 0.0f, 0.0f, 0.0f);
        b.delta_angular_velocity = vec4(0.0f, 0.0f, 0.0f, 0.0f);
        b.push_velocity = vec4(0.0f, 0.0f, 0.0f, 0.0f);
        b.turn_velocity = vec4(0.0f, 0.0f, 0.0f, 0.0f);
        
        const auto &rb = ctx->context->bodies[index];
        if (rb.valid() && (rb.body_flags.kinematic_object() || rb.inverse_mass != 0.0f)) {
          b.body_index = index;
//           b.transform = ctx->context->transforms->at(rb.transform_index);
          b.transform = ctx->context->new_transforms[rb.transform_index];
          b.inv_mass = vec4(rb.inverse_mass, rb.inverse_mass, rb.inverse_mass, 0.0f) * rb.linear_factor;
          b.angular_factor = rb.angular_factor;
          b.linear_factor = rb.linear_factor;
          b.linear_velocity = rb.linear_velocity;
          b.angular_velocity = rb.angular_velocity;
          b.external_force_impulse = rb.force * rb.inverse_mass * MCS_TO_SEC(delta_time);
          b.external_torque_impulse = rb.torque * rb.inv_inertia_tensor * MCS_TO_SEC(delta_time);
          return;
        }
      
        b.body_index = UINT32_MAX;
        b.transform.indentity();
        b.inv_mass = vec4(0, 0, 0, 0.0f);
        b.angular_factor = vec4(1, 1, 1, 0);
        b.linear_factor = vec4(1, 1, 1, 0);
        b.linear_velocity = vec4(0, 0, 0, 0.0f);
        b.angular_velocity = vec4(0, 0, 0, 0.0f);
        b.external_force_impulse = vec4(0, 0, 0, 0.0f);
        b.external_torque_impulse = vec4(0, 0, 0, 0.0f);
      }
      
      void convert_constraints_parallel::convert(const size_t &start, const size_t &count, context* ctx, const size_t &delta_time, std::atomic<uint32_t> &constraints_size, std::atomic<uint32_t> &frictions_size, std::atomic<uint32_t> &rolling_frictions_size) {
        for (size_t i = start; i < start+count; ++i) {
          // тут мы создаем констраинт каждый раз заново
          // констраинты по каждой точке я так понял + силы трения (трение возможно в обе стороны + трение вращения)
          // нужно посчитать заранее сколько памяти выделять
          // или мож использовать мемори пул, хотя мне и так и сяк нужно в вектор складывать
          
          auto m = ctx->narrowphase->manifolds_array[i];
          
          const auto &body0 = ctx->context->bodies[m->body0];
          const auto &body1 = ctx->context->bodies[m->body1];
//           const auto &trans0 = ctx->context->transforms->at(body0.transform_index);
//           const auto &trans1 = ctx->context->transforms->at(body1.transform_index);
          const auto &trans0 = ctx->context->new_transforms[body0.transform_index];
          const auto &trans1 = ctx->context->new_transforms[body1.transform_index];
          
          //const uint32_t solver_body_index = ctx->get_solver_body(&body0);
//           init_body(ctx, m->body0, delta_time); // вот это дело нужно обойти отдельно
//           init_body(ctx, m->body1, delta_time);
          
          for (uint32_t i = 0; i < m->points_count; ++i) {
            auto &cp = m->point_cache[i];
            if (cp.distance1 > m->contact_processing_threshold) continue;
            
            const uint32_t constraint_indices_addition = 1;
            const uint32_t constraint_index = constraints_size.fetch_add(constraint_indices_addition);
            ASSERT(constraint_index < ctx->solver_constraints.size());
            constraint &c = ctx->solver_constraints[constraint_index];
            c = constraint();
            c.solver_body_a = m->body0;
            c.solver_body_b = m->body1;
            c.original_contact_point = &cp;
            
            ASSERT(m->body0 != UINT32_MAX || m->body1 != UINT32_MAX);
            
            const vec4 &pos1 = cp.world_position_a;
            const vec4 &pos2 = cp.world_position_b;
            const vec4 &rel_pos1 = pos1 - trans0.pos;
            const vec4 &rel_pos2 = pos2 - trans1.pos;
            
            auto &solver_body1 = ctx->solver_bodies[m->body0];
            auto &solver_body2 = ctx->solver_bodies[m->body1];
            
            // тут нужно хотя бы вычислить несколько вещей заранее
            vec4 vel1 = solver_body1.velocity_in_local_point(rel_pos1);
            vec4 vel2 = solver_body2.velocity_in_local_point(rel_pos2);
            
            // в булете создаются солвер боди
            // в которых видимо и происходит все вычисления
            // 
            
            vec4 vel = vel1 - vel2;
            scalar rel_vel = simd::dot(cp.world_normal_b, vel);
            
            scalar relaxation;
            contact_constraint_init(ctx, cp, rel_pos1, rel_pos2, delta_time, relaxation, c);
            
            // rolling friction
            //c.friction_index = frictions_size.fetch_add(1);
            uint32_t friction_index_to_constraint = UINT32_MAX;
            if (cp.combined_rolling_friction > 0.0f) {
              vec4 axis1, axis2;
              core::plane_space(cp.world_normal_b, axis1, axis2);
              axis1 = simd::normalize(axis1);
              axis2 = simd::normalize(axis2);
              
              axis1 = apply_anisotropic_friction(&body0, &trans0, axis1, core::rigid_body::anisotropic_rolling_friction);
              axis1 = apply_anisotropic_friction(&body1, &trans1, axis1, core::rigid_body::anisotropic_rolling_friction);
              axis2 = apply_anisotropic_friction(&body0, &trans0, axis2, core::rigid_body::anisotropic_rolling_friction);
              axis2 = apply_anisotropic_friction(&body1, &trans1, axis2, core::rigid_body::anisotropic_rolling_friction);
              
              const bool axis1_exist = simd::length2(axis1) > EPSILON;
              const bool axis2_exist = simd::length2(axis2) > EPSILON;
              const uint32_t rolling_constraint_addition = 1 + uint32_t(axis1_exist) + uint32_t(axis2_exist);
              const uint32_t rolling_index = rolling_frictions_size.fetch_add(rolling_constraint_addition);
              auto &fc =   ctx->solver_rolling_friction_constraints[rolling_index+0];
              fc = constraint();
              fc.solver_body_a = m->body0;
              fc.solver_body_b = m->body1;
              fc.friction_index = constraint_index;
              init_torsional_friction_constraint(ctx, cp, cp.world_normal_b, rel_pos1, rel_pos2, cp.combined_spinning_friction, relaxation, 0.0f, 0.0f, fc);
              
              if (axis1_exist) {
                auto &fc = ctx->solver_rolling_friction_constraints[rolling_index+1];
                fc = constraint();
                fc.solver_body_a = m->body0;
                fc.solver_body_b = m->body1;
                fc.friction_index = constraint_index;
                init_torsional_friction_constraint(ctx, cp, axis1, rel_pos1, rel_pos2, cp.combined_rolling_friction, relaxation, 0.0f, 0.0f, fc);
              }
              
              if (axis2_exist) {
                auto &fc = ctx->solver_rolling_friction_constraints[rolling_index+1+uint32_t(axis1_exist)];
                fc = constraint();
                fc.solver_body_a = m->body0;
                fc.solver_body_b = m->body1;
                fc.friction_index = constraint_index;
                init_torsional_friction_constraint(ctx, cp, axis2, rel_pos1, rel_pos2, cp.combined_rolling_friction, relaxation, 0.0f, 0.0f, fc);
              }
            }
            
            // friction
            const uint32_t friction_constraint_addition = 1 + uint32_t(ctx->solver_info.solver_mode.use_2_friction_directions());
            const uint32_t friction_index = frictions_size.fetch_add(friction_constraint_addition);
            if (!ctx->solver_info.solver_mode.enable_friction_direction_caching() || !cp.contact_point_flags.lateral_friction_initialized()) {
              cp.lateral_friction_dir1 = vel - cp.world_normal_b * rel_vel;
              scalar lat_rel_vel = simd::length2(cp.lateral_friction_dir1);
              
              if (ctx->solver_info.solver_mode.disable_velosity_dependent_friction_direction() && lat_rel_vel > EPSILON) {
                cp.lateral_friction_dir1 *= 1.0f / std::sqrt(lat_rel_vel);
                cp.lateral_friction_dir1 = apply_anisotropic_friction(&body0, &trans0, cp.lateral_friction_dir1, core::rigid_body::anisotropic_rolling_friction);
                cp.lateral_friction_dir1 = apply_anisotropic_friction(&body1, &trans1, cp.lateral_friction_dir1, core::rigid_body::anisotropic_rolling_friction);
                
                auto &fc = ctx->solver_friction_constraints[friction_index+0];
                fc = constraint();
                fc.solver_body_a = m->body0;
                fc.solver_body_b = m->body1;
                fc.friction_index = constraint_index;
                friction_index_to_constraint = friction_index;
                init_friction_constraint(ctx, cp, cp.lateral_friction_dir1, rel_pos1, rel_pos2, relaxation, 0.0f, 0.0f, delta_time, fc);
                
                if (ctx->solver_info.solver_mode.use_2_friction_directions()) {
                  cp.lateral_friction_dir2 = simd::cross(cp.lateral_friction_dir1, cp.world_normal_b);
                  cp.lateral_friction_dir2 = simd::normalize(cp.lateral_friction_dir2);
                  cp.lateral_friction_dir2 = apply_anisotropic_friction(&body0, &trans0, cp.lateral_friction_dir2, core::rigid_body::anisotropic_rolling_friction);
                  cp.lateral_friction_dir2 = apply_anisotropic_friction(&body1, &trans1, cp.lateral_friction_dir2, core::rigid_body::anisotropic_rolling_friction);
                  
                  auto &fc = ctx->solver_friction_constraints[friction_index+1];
                  fc = constraint();
                  fc.solver_body_a = m->body0;
                  fc.solver_body_b = m->body1;
                  fc.friction_index = constraint_index;
                  init_friction_constraint(ctx, cp, cp.lateral_friction_dir2, rel_pos1, rel_pos2, relaxation, 0.0f, 0.0f, delta_time, fc);
                }
              } else {
                core::plane_space(cp.world_normal_b, cp.lateral_friction_dir1, cp.lateral_friction_dir2);
              
                cp.lateral_friction_dir1 = apply_anisotropic_friction(&body0, &trans0, cp.lateral_friction_dir1, core::rigid_body::anisotropic_rolling_friction);
                cp.lateral_friction_dir1 = apply_anisotropic_friction(&body1, &trans1, cp.lateral_friction_dir1, core::rigid_body::anisotropic_rolling_friction);
                
                auto &fc = ctx->solver_friction_constraints[friction_index+0];
                fc = constraint();
                fc.solver_body_a = m->body0;
                fc.solver_body_b = m->body1;
                fc.friction_index = constraint_index;
                friction_index_to_constraint = friction_index;
                init_friction_constraint(ctx, cp, cp.lateral_friction_dir1, rel_pos1, rel_pos2, relaxation, 0.0f, 0.0f, delta_time, fc);
                
                if (ctx->solver_info.solver_mode.use_2_friction_directions()) {
                  cp.lateral_friction_dir2 = apply_anisotropic_friction(&body0, &trans0, cp.lateral_friction_dir2, core::rigid_body::anisotropic_rolling_friction);
                  cp.lateral_friction_dir2 = apply_anisotropic_friction(&body1, &trans1, cp.lateral_friction_dir2, core::rigid_body::anisotropic_rolling_friction);
                  
                  auto &fc = ctx->solver_friction_constraints[friction_index+1];
                  fc = constraint();
                  fc.solver_body_a = m->body0;
                  fc.solver_body_b = m->body1;
                  fc.friction_index = constraint_index;
                  init_friction_constraint(ctx, cp, cp.lateral_friction_dir2, rel_pos1, rel_pos2, relaxation, 0.0f, 0.0f, delta_time, fc);
                }
                
                if (ctx->solver_info.solver_mode.use_2_friction_directions() && ctx->solver_info.solver_mode.disable_velosity_dependent_friction_direction()) {
                  cp.contact_point_flags = narrowphase::manifold_point::flags(cp.contact_point_flags.container | narrowphase::manifold_point::flags::LATERAL_FRICTION_INITIALIZED);
                }
              }
            } else {
              auto &fc = ctx->solver_friction_constraints[friction_index+0];
              fc = constraint();
              fc.solver_body_a = m->body0;
              fc.solver_body_b = m->body1;
              fc.friction_index = constraint_index;
              friction_index_to_constraint = friction_index;
              init_friction_constraint(ctx, cp, cp.lateral_friction_dir1, rel_pos1, rel_pos2, relaxation, 0.0f, 0.0f, delta_time, fc);
              
              if (ctx->solver_info.solver_mode.use_2_friction_directions()) {
                auto &fc = ctx->solver_friction_constraints[friction_index+1];
                fc = constraint();
                fc.solver_body_a = m->body0;
                fc.solver_body_b = m->body1;
                fc.friction_index = constraint_index;
                init_friction_constraint(ctx, cp, cp.lateral_friction_dir2, rel_pos1, rel_pos2, relaxation, 0.0f, 0.0f, delta_time, fc);
              }
            }
            
            c.friction_index = friction_index_to_constraint;
            set_friction_constraint_impulse(ctx, cp, c);
          }
        }
      }
      
      void convert_constraints_parallel::convert_bodies(const size_t &start, const size_t &count, context* ctx, const size_t &delta_time) {
        for (size_t i = start; i < start+count; ++i) {
          init_body(ctx, i, delta_time);
          
          const auto &body = ctx->context->bodies[i];
//           const auto &trans = ctx->context->transforms->at(body.transform_index);
          const auto &trans = ctx->context->new_transforms[body.transform_index];
          if (body.valid() && body.inverse_mass != 0.0f) {
            auto &solver_body = ctx->solver_bodies[i];
            vec4 gyro_force = vec4(0,0,0,0);
            if (body.body_flags.gyroscopic_force_explicit()) {
              gyro_force = body.compute_gyroscopic_force_explicit(trans, ctx->solver_info.max_gyroscopic_force);
              solver_body.external_torque_impulse -= gyro_force * body.inv_inertia_tensor * MCS_TO_SEC(delta_time);
            }
            
            if (body.body_flags.gyroscopic_force_implicit_world()) {
              gyro_force = body.compute_gyroscopic_impulse_implicit_world(trans, delta_time);
              solver_body.external_torque_impulse += gyro_force;  
            }
            
            if (body.body_flags.gyroscopic_force_implicit_body()) {
              gyro_force = body.compute_gyroscopic_impulse_implicit_body(trans, delta_time);
              solver_body.external_torque_impulse += gyro_force;  
            }
          }
        }
      }
      
      void convert_constraints_parallel::convert_joints1(const size_t &start, const size_t &count, context* ctx, char* memory, std::atomic<uint32_t> &rows) {
        auto info_memory = reinterpret_cast<core::joint_interface::parameters1*>(memory);
        for (size_t i = 0; i < start+count; ++i) {
          auto j = ctx->context->joints[i];
          core::joint_interface::parameters1 &param1 = info_memory[i];
          if (j->enabled()) j->get_parameters1(param1);
          
          rows.fetch_add(param1.num_constraint_rows);
        }
      }
      
      void convert_constraints_parallel::convert_joints2(const size_t &start, const size_t &count, context* ctx, char* memory, const size_t &delta_time, const uint32_t &rows_count, std::atomic<uint32_t> &current_row) {
        auto info_memory = reinterpret_cast<core::joint_interface::parameters1*>(memory);
        for (size_t i = 0; i < start+count; ++i) {
          core::joint_interface::parameters1 &param1 = info_memory[i];
          core::joint_interface* joint = ctx->context->joints[i];
          const uint32_t current_row_index = current_row.fetch_add(param1.num_constraint_rows);
          if (param1.num_constraint_rows > 0) {
            ASSERT(current_row_index < rows_count);
            constraint* constraint_row = &ctx->solver_non_contact_constraints[current_row_index];
            convert_joint(ctx, joint, constraint_row, param1, delta_time);
          }
        }
      }
      
      solve_constraints_parallel::solve_constraints_parallel(dt::thread_pool* pool) : pool(pool) {}
      void solve_constraints_parallel::begin(context* ctx) { (void)ctx; }
      
      void solve_constraints_parallel::process(context* ctx, const size_t &delta_time) {
        (void)delta_time;
        // тут уже нужно использовать батчи
        // батчи видимо должны скорее относится к конкретному констраинту
        // тут количество std bind просто зашкаливает
        // нужно переписать utils::submit_works
        // стало не сказать чтобы сильно лучше
        // 
        
        // this is a special step to resolve penetrations (just for contacts)
        solve_split_impulse_penetrations(ctx);
        
        const uint32_t iterations = ctx->solver_info.num_iterations;
        for (uint32_t iteration = 0; iteration < iterations; ++iteration) {
          const scalar res = solve_single_iteration(ctx, iteration);
          if (res <= ctx->solver_info.least_squares_residual_threshold) {
            // аналитика
            break;
          }
        }
      }
      
      void solve_constraints_parallel::clear() {}
      
      scalar solve_constraints_parallel::solve_single_iteration(context* ctx, const uint32_t &current_iteration) {
        std::atomic<uint32_t> least_squares_residual(0);
        // случайный порядок вычислений
        
        utils::submit_works(pool, ctx->solver_non_contact_rows_count, solve_non_contact_constraints, ctx, current_iteration, std::ref(least_squares_residual));
        
        // здесь еще вычисляются по старому методу некоторые joint'ы
        
        if (ctx->solver_info.solver_mode.interleave_contact_and_friction_constraints()) {
          utils::submit_works(pool, ctx->solver_constraints_size, solve_contact_constraints_interleave, ctx, std::ref(least_squares_residual));
        } else {
          utils::submit_works(pool, ctx->solver_constraints_size, solve_contact_constraints, ctx, std::ref(least_squares_residual));
          utils::submit_works(pool, ctx->solver_friction_constraints_size, solve_friction_constraints, ctx, std::ref(least_squares_residual));
        }
        
        utils::submit_works(pool, ctx->solver_rolling_friction_constraints_size, solve_rolling_friction_constraints, ctx, std::ref(least_squares_residual));
        
        return glm::uintBitsToFloat(least_squares_residual);
      }
      
      void solve_constraints_parallel::solve_split_impulse_penetrations(context* ctx) {
        if (!ctx->solver_info.solver_mode.split_impulse()) return;
        
//         PRINT_VAR("ctx->solver_constraints_size", ctx->solver_constraints_size)
//         ASSERT(ctx->solver_constraints_size < 2);
        
        for (uint32_t itr = 0; itr < ctx->solver_info.num_iterations; ++itr) {
//             scalar leastSquaresResidual = 0.0f;
          utils::submit_works(pool, ctx->solver_constraints_size, [] (const size_t &start, const size_t &count, context* ctx) {
            for (size_t i = start; i < start+count; ++i) {
              const constraint &c = ctx->solver_constraints[i];
              body &body_a = ctx->solver_bodies[c.solver_body_a];
              body &body_b = ctx->solver_bodies[c.solver_body_b];
              const scalar residual = resolve_split_penetration_impulse(body_a, body_b, c);
              if (residual * residual < ctx->solver_info.least_squares_residual_threshold) {
                break;
              }
            }
          }, ctx);
        }
      }
      
      void solve_constraints_parallel::solve_non_contact_constraints(const size_t &start, const size_t &count, context* ctx, const uint32_t &current_iteration, std::atomic<uint32_t> &least_squares_residual) {
        ASSERT(start+count < ctx->solver_non_contact_constraints.size());
        thread_local static body default_body;
        for (size_t i = start; i < start+count; ++i) {
          const constraint &c = ctx->solver_non_contact_constraints[i];
          body &body_a = ctx->solver_bodies[c.solver_body_a];
          body &body_b = c.solver_body_b == UINT32_MAX ? default_body : ctx->solver_bodies[c.solver_body_b];
          
          if (c.override_num_solver_iterations <= current_iteration) continue;
          
          // резолвим
          const scalar residual = resolve_single_constraint_row(body_a, body_b, c);
          // тут нужно еще посчитать максимальный residual
          atomic_max(least_squares_residual, residual * residual);
        }
      }
      
      void solve_constraints_parallel::solve_contact_constraints_interleave(const size_t &start, const size_t &count, context* ctx, std::atomic<uint32_t> &least_squares_residual) {
        const uint32_t mul = ctx->solver_info.solver_mode.use_2_friction_directions() ? 2 : 1;
        ASSERT(start+count < ctx->solver_constraints.size());
        for (size_t i = start; i < start+count; ++i) {
          scalar total_impulse = 0.0f;
          {
            const constraint &c = ctx->solver_constraints[i];
            body &body_a = ctx->solver_bodies[c.solver_body_a];
            body &body_b = ctx->solver_bodies[c.solver_body_b];
            
            const scalar residual = resolve_single_constraint_row_lower_limit(body_a, body_b, c);
            atomic_max(least_squares_residual, residual * residual);
            total_impulse = c.applied_impulse;
          }
          
          {
            constraint &c = ctx->solver_friction_constraints[mul * i];
            body &body_a = ctx->solver_bodies[c.solver_body_a];
            body &body_b = ctx->solver_bodies[c.solver_body_b];
            
            if (total_impulse > 0.0f) {
              c.lower_limit = -(c.friction * total_impulse);
              c.upper_limit =  (c.friction * total_impulse);
              
              const scalar residual = resolve_single_constraint_row(body_a, body_b, c);
              atomic_max(least_squares_residual, residual * residual);
            }
          }
          
          if (ctx->solver_info.solver_mode.use_2_friction_directions()) {
            constraint &c = ctx->solver_friction_constraints[mul * i + 1];
            body &body_a = ctx->solver_bodies[c.solver_body_a];
            body &body_b = ctx->solver_bodies[c.solver_body_b];
            
            if (total_impulse > 0.0f) {
              c.lower_limit = -(c.friction * total_impulse);
              c.upper_limit =  (c.friction * total_impulse);
              
              const scalar residual = resolve_single_constraint_row(body_a, body_b, c);
              atomic_max(least_squares_residual, residual * residual);
            }
          }
        }
      }
      
      void solve_constraints_parallel::solve_contact_constraints(const size_t &start, const size_t &count, context* ctx, std::atomic<uint32_t> &least_squares_residual) {
        ASSERT(start+count <= ctx->solver_constraints.size());
        for (size_t i = start; i < start+count; ++i) {
          const constraint &c = ctx->solver_constraints[i];
          body &body_a = ctx->solver_bodies[c.solver_body_a];
          body &body_b = ctx->solver_bodies[c.solver_body_b];
          
//           PRINT_VAR("contact c.rhs",c.rhs)
          
          const scalar residual = resolve_single_constraint_row_lower_limit(body_a, body_b, c);
          atomic_max(least_squares_residual, residual * residual);
        }
      }
      
      void solve_constraints_parallel::solve_friction_constraints(const size_t &start, const size_t &count, context* ctx, std::atomic<uint32_t> &least_squares_residual) {
        ASSERT(start+count <= ctx->solver_friction_constraints.size());
        for (size_t i = start; i < start+count; ++i) {
          constraint &c = ctx->solver_friction_constraints[i];
          body &body_a = ctx->solver_bodies[c.solver_body_a];
          body &body_b = ctx->solver_bodies[c.solver_body_b];
          ASSERT(c.friction_index != UINT32_MAX);
          const scalar total_impulse = ctx->solver_constraints[c.friction_index].applied_impulse;
          
//           PRINT_VAR("friction c.rhs",c.rhs)
          
          if (total_impulse > 0.0f) {
            c.lower_limit = -(c.friction * total_impulse);
            c.upper_limit =  (c.friction * total_impulse);
            
            const scalar residual = resolve_single_constraint_row(body_a, body_b, c);
            atomic_max(least_squares_residual, residual * residual);
          }
        }
      }
      
      void solve_constraints_parallel::solve_rolling_friction_constraints(const size_t &start, const size_t &count, context* ctx, std::atomic<uint32_t> &least_squares_residual) {
        ASSERT(start+count <= ctx->solver_rolling_friction_constraints.size());
        for (size_t i = start; i < start+count; ++i) {
          constraint &c = ctx->solver_rolling_friction_constraints[i];
          body &body_a = ctx->solver_bodies[c.solver_body_a];
          body &body_b = ctx->solver_bodies[c.solver_body_b];
          ASSERT(c.friction_index != UINT32_MAX);
          const scalar total_impulse = ctx->solver_constraints[c.friction_index].applied_impulse;
          
//           PRINT_VAR("rolling c.rhs",c.rhs)
          
          if (total_impulse > 0.0f) {
            scalar magnitude = c.friction * total_impulse;
            magnitude = std::min(magnitude, c.friction);
            
            c.lower_limit = -magnitude;
            c.upper_limit =  magnitude;
            
            const scalar residual = resolve_single_constraint_row(body_a, body_b, c);
            atomic_max(least_squares_residual, residual * residual);
          }
        }
      }
      
      solver_finish_parallel::solver_finish_parallel(dt::thread_pool* pool) : pool(pool) {}
      void solver_finish_parallel::begin(context* ctx) { (void)ctx; }
      void solver_finish_parallel::process(context* ctx, const size_t &delta_time) {
        if (ctx->solver_info.solver_mode.use_warmstarting()) {
          utils::submit_works(pool, ctx->solver_constraints_size, write_back_contacts, ctx);
        }
        
        utils::submit_works(pool, ctx->solver_non_contact_constraints.size(), write_back_joints, ctx, delta_time);
        utils::submit_works(pool, ctx->solver_bodies.size(), write_back_bodies, ctx, delta_time);
      }
      
      void solver_finish_parallel::clear() {}
      
      void solver_finish_parallel::write_back_contacts(const size_t &start, const size_t &count, context* ctx) {
        for (size_t i = start; i < start+count; ++i) {
          const constraint &c = ctx->solver_constraints[i];
          auto pt = reinterpret_cast<narrowphase::manifold_point*>(c.original_contact_point);
          ASSERT(pt != nullptr);
          pt->applied_impulse = c.applied_impulse;
          ASSERT(c.friction_index != UINT32_MAX);
          pt->applied_impulse_lateral1 = ctx->solver_friction_constraints[c.friction_index].applied_impulse;
          if (ctx->solver_info.solver_mode.use_2_friction_directions()) {
            pt->applied_impulse_lateral2 = ctx->solver_friction_constraints[c.friction_index+1].applied_impulse;
          }
        }
      }
      
      void solver_finish_parallel::write_back_joints(const size_t &start, const size_t &count, context* ctx, const size_t &delta_time) {
        for (size_t i = start; i < start+count; ++i) {
          const constraint &c = ctx->solver_non_contact_constraints[i];
          auto constr = reinterpret_cast<core::joint_interface*>(c.original_contact_point);
          auto fb = constr->feedback();
          if (fb != nullptr) {
            const core::rigid_body* rb_a = &ctx->context->bodies[constr->body0];
            const core::rigid_body* rb_b = constr->body1 == UINT32_MAX ? nullptr : &ctx->context->bodies[constr->body1];
            fb->applied_force_body_a += c.contact_normal1 * c.applied_impulse * rb_a->linear_factor / MCS_TO_SEC(delta_time);
            fb->applied_force_body_b += c.contact_normal2 * c.applied_impulse * (rb_b ? rb_b->linear_factor : vec4(0,0,0,0)) / MCS_TO_SEC(delta_time);
            fb->applied_torque_body_a += c.relpos1_cross_normal * rb_a->angular_factor * c.applied_impulse / MCS_TO_SEC(delta_time);
            fb->applied_torque_body_b += c.relpos2_cross_normal * (rb_b ? rb_b->angular_factor : vec4(0,0,0,0)) * c.applied_impulse / MCS_TO_SEC(delta_time); /*RGM ???? */
          }
          
          constr->applied_impulse = c.applied_impulse;
          if (glm::abs(c.applied_impulse) >= constr->breaking_impulse_threshold) {
            constr->set_enabled(false);
          }
        }
      }
      
      void solver_finish_parallel::write_back_bodies(const size_t &start, const size_t &count, context* ctx, const size_t &delta_time) {
        for (size_t i = start; i < start+count; ++i) {
          body &b = ctx->solver_bodies[i];
          if (b.body_index == UINT32_MAX) continue;
          
          core::rigid_body* rb = &ctx->context->bodies[b.body_index];
//           PRINT_VEC4("rb vel",rb->linear_velocity)
//           PRINT_VEC4("b vel",b.linear_velocity)
//           PRINT_VEC4("b delta vel",b.delta_linear_velocity)
          
          if (ctx->solver_info.solver_mode.split_impulse()) b.writeback_velocity_and_transform(MCS_TO_SEC(delta_time), ctx->solver_info.split_impulse_turn_erp);
          else b.writeback_velocity();
          
//           PRINT_VEC4("rb vel",rb->linear_velocity)
//           PRINT_VEC4("b vel",b.linear_velocity)
          
          rb->linear_velocity = b.linear_velocity + b.external_force_impulse;
          rb->angular_velocity = b.angular_velocity + b.external_torque_impulse;
          
          PRINT_VEC4("rb vel",rb->linear_velocity)
          PRINT_VEC4("b ext force",b.external_force_impulse)
          PRINT_VEC4("b lin  vel ",b.linear_velocity)
          PRINT_VEC4("b push vel ",b.push_velocity)
          
          if (ctx->solver_info.solver_mode.split_impulse()) {
            //ctx->context->transforms->at(rb->transform_index) = b.transform;
//             PRINT_VEC4("pos",ctx->context->new_transforms[rb->transform_index].pos);
            ctx->context->new_transforms[rb->transform_index] = b.transform;
//             PRINT_VEC4("pos",ctx->context->new_transforms[rb->transform_index].pos);
          }
        }
      }
    }
  }
}
