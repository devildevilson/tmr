#include "hinge_joint.h"

#include "physics_context.h"

namespace devils_engine {
  namespace physics {
    core::scalar normalize_angle(const core::scalar &angle) {
      const core::scalar a = glm::mod(angle, core::scalar(PI_2));
      if (a < -PI) return a + PI_2;
      if (a >  PI) return a - PI_2;
      return a;
    }
    
    core::scalar normalize_angle_positive(const core::scalar &angle) {
      return glm::mod(glm::mod(angle, core::scalar(2.0f * PI)) + core::scalar(2.0f * PI), core::scalar(2.0f * PI));
    }
    
    core::scalar shortest_angular_distance(const core::scalar &acc_ang, const core::scalar &cur_ang) {
      const core::scalar res = normalize_angle(normalize_angle_positive(normalize_angle_positive(cur_ang) - normalize_angle_positive(acc_ang)));
      return res;
    }
    
    core::scalar shortest_angle_update(const core::scalar &acc_ang, const core::scalar &cur_ang) {
      const core::scalar tol = 0.3f;
      const core::scalar res = shortest_angular_distance(acc_ang, cur_ang);
      if (glm::abs(res) > tol) return cur_ang;
      else return acc_ang + res;
      return cur_ang;
    }
    
    namespace utils {
      void angular_limit::set(const core::scalar &low, const core::scalar &high, const core::scalar &softness, const core::scalar &bias_factor, const core::scalar &relaxation_factor) {
        half_range = (high - low) / 2.0f;
        center = normalize_angle(low + half_range);
        this->softness = softness;
        this->bias_factor = bias_factor;
        this->relaxation_factor = relaxation_factor;
      }
      
      void angular_limit::test(const core::scalar &angle) {
        correction = 0.0f;
        sign = 0.0f;
        solve_limit = false;
        
        if (half_range >= 0.0f) {
          core::scalar deviation = normalize_angle(angle - center);
          if (deviation < -half_range) {
            solve_limit = true;
            correction = -(deviation + half_range);
            sign = +1.0f;
          } else if (deviation > half_range) {
            solve_limit = true;
            correction = half_range - deviation;
            sign = -1.0f;
          }
        }
      }
      
      void angular_limit::fit(core::scalar& angle) const {
        if (half_range <= 0.0f) return;
        
        core::scalar relative_angle = normalize_angle(angle - center);
        if (glm::abs(relative_angle - half_range) < EPSILON) return;
        
        if (relative_angle > 0.0f) angle = get_high();
        else angle = get_low();
      }
      
      core::scalar angular_limit::get_error() const {
        return correction * sign;
      }
      
      core::scalar angular_limit::get_low() const {
        return normalize_angle(center - half_range);
      }
      
      core::scalar angular_limit::get_high() const {
        return normalize_angle(center + half_range);
      }
    }
    
    namespace constraints {
      using core::mat4;
      using core::vec4;
      using core::scalar;
      using core::quat;
      
      quat shortest_arc(const vec4 &v0, const vec4 &v1) {
        const vec4 c = simd::cross(v0, v1);
        const scalar d = simd::dot(v0, v1);
        
        if (d < -1.0f + EPSILON) {
          vec4 n, u;
          core::plane_space(v0, n, u);
          float arr[4];
          n.storeu(arr);
          return quat(0.0f, arr[0], arr[1], arr[2]);
        }
        
        const scalar s = glm::sqrt((1.0f + d) * 2.0f);
        const scalar rs = 1.0f / s;
        float arr[4];
        c.storeu(arr);
        return quat(s * 0.5f, arr[0] * rs, arr[1] * rs, arr[2] * rs);
      }
      
      hinge::flags::flags() : container(0) {}
      hinge::flags::flags(const uint32_t &flags) : container(flags) {}
      
      bool hinge::flags::cfm_stop() const { return (container & CFM_STOP) == CFM_STOP; }
      bool hinge::flags::erp_stop() const { return (container & ERP_STOP) == ERP_STOP; }
      bool hinge::flags::cfm_norm() const { return (container & CFM_NORM) == CFM_NORM; }
      bool hinge::flags::erp_norm() const { return (container & ERP_NORM) == ERP_NORM; }
      
      hinge::hinge(core::context* ctx, const uint32_t &body0, const uint32_t &body1, const core::vec4& pivot_a, const core::vec4& pivot_b, const core::vec4& axis_a, const core::vec4& axis_b, bool use_reference_frame_a) : 
        core::joint_interface(core::joint_type::hinge, body0, body1),
        ctx(ctx),
        use_solve_constraint_obsolete(false),
        use_offset_for_constraint_frame(true),
        use_reference_frame_a(use_reference_frame_a),
        hinge_flags(0),
        normal_cfm(0),
        normal_erp(0),
        stop_cfm(0),
        stop_erp(0)
      {
        rb_a_frame.pos = pivot_a;
        const auto &b0 = ctx->bodies[body0];
        const auto &t0 = ctx->transforms->at(b0.transform_index);
        const mat4 trans_basis = simd::transpose(t0.get_basis());
        
        vec4 rbAxisA1 = trans_basis[0];

        vec4 rbAxisA2;
        scalar projection = simd::dot(axis_a, rbAxisA1);
        if (projection >= 1.0f - EPSILON) {
          rbAxisA1 = -trans_basis[2];
          rbAxisA2 =  trans_basis[1];
        } else if (projection <= -1.0f + EPSILON) {
          rbAxisA1 = trans_basis[2];
          rbAxisA2 = trans_basis[1];
        } else {
          rbAxisA2 = simd::cross(axis_a, rbAxisA1);
          rbAxisA1 = simd::cross(rbAxisA2, axis_a);
        }
        
        const mat4 basis_a = simd::transpose(mat4(rbAxisA1, rbAxisA2, axis_a, vec4(0.0f, 0.0f, 0.0f, 1.0f)));
        rb_a_frame.rot = quat(basis_a);

        quat rotationArc = shortest_arc(axis_a, axis_b);
        vec4 rbAxisB1 = rotationArc * rbAxisA1; //  quatRotate(rotationArc, rbAxisA1)
        vec4 rbAxisB2 = simd::cross(axis_b ,rbAxisB1);

        rb_b_frame.pos = pivot_b;
        const mat4 basis_b = simd::transpose(mat4(rbAxisB1, rbAxisB2, axis_b, vec4(0.0f, 0.0f, 0.0f, 1.0f)));
        rb_b_frame.rot = quat(basis_b);

//         #ifndef _BT_USE_CENTER_LIMIT_
//         //start with free
//         m_lowerLimit = btScalar(1.0f);
//         m_upperLimit = btScalar(-1.0f);
//         m_biasFactor = 0.3f;
//         m_relaxationFactor = 1.0f;
//         m_limitSoftness = 0.9f;
//         m_solveLimit = false;
//         #endif
        reference_sign = use_reference_frame_a ? scalar(-1.f) : scalar(1.f);
      }
      
      hinge::hinge(core::context* ctx, const uint32_t &body0, const core::vec4& pivot_a, const core::vec4& axis_a, bool use_reference_frame_a) :
        core::joint_interface(core::joint_type::hinge, body0, UINT32_MAX),
        ctx(ctx),
        angular_only(false),
        enable_angular_motor(false),
        use_solve_constraint_obsolete(false),
        use_offset_for_constraint_frame(true),
        use_reference_frame_a(use_reference_frame_a),
        hinge_flags(0),
        normal_cfm(0),
        normal_erp(0),
        stop_cfm(0),
        stop_erp(0)
      {
        // since no frame is given, assume this to be zero angle and just pick rb transform axis
        // fixed axis in worldspace
        vec4 rbAxisA1, rbAxisA2;
        core::plane_space(axis_a, rbAxisA1, rbAxisA2);

        rb_a_frame.pos = pivot_a;
        const mat4 basis_a = simd::transpose(mat4(rbAxisA1, rbAxisA2, axis_a, vec4(0.0f, 0.0f, 0.0f, 1.0f)));
        rb_a_frame.rot = quat(basis_a);
        
        const auto &b0 = ctx->bodies[body0];
        const auto &t0 = ctx->transforms->at(b0.transform_index);

        vec4 axisInB = t0.pos * axis_a;

        quat rotationArc = shortest_arc(axis_a, axisInB);
        vec4 rbAxisB1 = rotationArc * rbAxisA1; // quatRotate(rotationArc, rbAxisA1);
        vec4 rbAxisB2 = simd::cross(axisInB, rbAxisB1);

        rb_b_frame.pos = t0.transform_vector(pivot_a);
        const mat4 basis_b = simd::transpose(mat4(rbAxisB1, rbAxisB2, axisInB, vec4(0.0f, 0.0f, 0.0f, 1.0f)));
        rb_b_frame.rot = quat(basis_b);

//         #ifndef _BT_USE_CENTER_LIMIT_
//         //start with free
//         m_lowerLimit = btScalar(1.0f);
//         m_upperLimit = btScalar(-1.0f);
//         m_biasFactor = 0.3f;
//         m_relaxationFactor = 1.0f;
//         m_limitSoftness = 0.9f;
//         m_solveLimit = false;
//         #endif
        reference_sign = use_reference_frame_a ? scalar(-1.f) : scalar(1.f);
      }
      
      hinge::hinge(core::context* ctx, const uint32_t &body0, const uint32_t &body1, const core::transform& rb_a_frame, const core::transform& rb_b_frame, bool use_reference_frame_a) :
        core::joint_interface(core::joint_type::hinge, body0, body1),
        rb_a_frame(rb_a_frame),
        rb_b_frame(rb_b_frame),
        ctx(ctx),
        angular_only(false),
        enable_angular_motor(false),
        use_solve_constraint_obsolete(false),
        use_offset_for_constraint_frame(true),
        use_reference_frame_a(use_reference_frame_a),
        hinge_flags(0),
        normal_cfm(0),
        normal_erp(0),
        stop_cfm(0),
        stop_erp(0)
      {
//         #ifndef _BT_USE_CENTER_LIMIT_
//         //start with free
//         m_lowerLimit = btScalar(1.0f);
//         m_upperLimit = btScalar(-1.0f);
//         m_biasFactor = 0.3f;
//         m_relaxationFactor = 1.0f;
//         m_limitSoftness = 0.9f;
//         m_solveLimit = false;
//         #endif
        reference_sign = use_reference_frame_a ? scalar(-1.f) : scalar(1.f);
      }
      
      hinge::hinge(core::context* ctx, const uint32_t &body0, const core::transform& rb_a_frame, bool use_reference_frame_a) :
        core::joint_interface(core::joint_type::hinge, body0, UINT32_MAX),
        rb_a_frame(rb_a_frame),
        ctx(ctx),
        angular_only(false),
        enable_angular_motor(false),
        use_solve_constraint_obsolete(false),
        use_offset_for_constraint_frame(true),
        use_reference_frame_a(use_reference_frame_a),
        hinge_flags(0),
        normal_cfm(0),
        normal_erp(0),
        stop_cfm(0),
        stop_erp(0)
      {
        const auto &b0 = ctx->bodies[body0];
        const auto &t0 = ctx->transforms->at(b0.transform_index);
        
        rb_b_frame.pos = t0.transform_vector(rb_a_frame.pos);
        
//         #ifndef _BT_USE_CENTER_LIMIT_
//         //start with free
//         m_lowerLimit = btScalar(1.0f);
//         m_upperLimit = btScalar(-1.0f);
//         m_biasFactor = 0.3f;
//         m_relaxationFactor = 1.0f;
//         m_limitSoftness = 0.9f;
//         m_solveLimit = false;
//         #endif
        reference_sign = use_reference_frame_a ? scalar(-1.f) : scalar(1.f);        
      }
      
      void hinge::build_jacobian() {
        // неиспользуется
      }
      
      void hinge::get_parameters1(parameters1 &param1) {
        param1.num_constraint_rows = 5;
        param1.nub = 1;
        
        const auto b0 = &ctx->bodies[body0];
        const auto &t0 = ctx->transforms->at(b0->transform_index);
        if (body1 == UINT32_MAX) test_limit(t0, core::transform());
        else {
          const auto b1 = &ctx->bodies[body1];
          const auto &t1 = ctx->transforms->at(b1->transform_index);
          test_limit(t0, t1);
        }
        
        if (limit.solve_limit || enable_angular_motor) {
          ++param1.num_constraint_rows;
          --param1.nub;
        }
      }
      
      void hinge::get_parameters2(struct parameters2 &param2) {
        if (use_offset_for_constraint_frame) {
          const auto b0 = &ctx->bodies[body0];
          const auto &t0 = ctx->transforms->at(b0->transform_index);
          if (body1 == UINT32_MAX) parameters2_using_frame_offset(param2, t0, core::transform(), b0->angular_velocity, vec4());
          else {
            const auto b1 = &ctx->bodies[body1];
            const auto &t1 = ctx->transforms->at(b1->transform_index);
            parameters2_using_frame_offset(param2, t0, t1, b0->angular_velocity, b1->angular_velocity);
          }
        } else {
          const auto b0 = &ctx->bodies[body0];
          const auto &t0 = ctx->transforms->at(b0->transform_index);
          if (body1 == UINT32_MAX) parameters2(param2, t0, core::transform(), b0->angular_velocity, vec4());
          else {
            const auto b1 = &ctx->bodies[body1];
            const auto &t1 = ctx->transforms->at(b1->transform_index);
            parameters2(param2, t0, t1, b0->angular_velocity, b1->angular_velocity);
          }
        }
      }
      
      void hinge::update_rhs(const size_t &delta_time) {
        (void)delta_time;
      }
      
      void hinge::set_frames(const core::transform& frame_a, const core::transform& frame_b) {
        rb_a_frame = frame_a;
        rb_b_frame = frame_b;
        build_jacobian();
      }
      
      core::scalar hinge::get_angle() {
        const auto b0 = &ctx->bodies[body0];
        const auto &t0 = ctx->transforms->at(b0->transform_index);
        if (body1 == UINT32_MAX) return get_angle(t0, core::transform());
        
        const auto b1 = &ctx->bodies[body1];
        const auto &t1 = ctx->transforms->at(b1->transform_index);
        return get_angle(t0, t1);
      }
      
      core::scalar hinge::get_angle(const core::transform& transA, const core::transform& transB) {
        const mat4 trans_basis_a = simd::transpose(rb_a_frame.get_basis());
//         const mat4 trans_basis_b = simd::transpose(rb_b_frame.get_basis());
        const vec4 refAxis0 = transA.get_basis() * trans_basis_a[0]; //m_rbAFrame.getBasis().getColumn(0);
        const vec4 refAxis1 = transA.get_basis() * trans_basis_a[1]; //m_rbAFrame.getBasis().getColumn(1);
        const vec4 swingAxis = transB.get_basis() * rb_b_frame.get_basis().column(1); // trans_basis_b[1]; //m_rbBFrame.getBasis().getColumn(1);
        // btScalar angle = btAtan2Fast(swingAxis.dot(refAxis0), swingAxis.dot(refAxis1));
        scalar angle = atan2(simd::dot(swingAxis, refAxis0), simd::dot(swingAxis, refAxis1));
        return reference_sign * angle;
      }
      
      void hinge::test_limit(const core::transform& transA, const core::transform& transB) {
        hinge_angle = get_angle(transA, transB);
        limit.test(hinge_angle);
      }
      
      void hinge::set_param(const uint32_t &num, const core::scalar &value, const uint32_t &axis) {
        if ((axis == UINT32_MAX) || (axis == 5)) {
          switch (num) {
            case joint_interface::STOP_ERP_PARAMETER:
              stop_erp = value;
              hinge_flags = flags(hinge_flags.container | flags::ERP_STOP);
              break;
            case joint_interface::STOP_CFM_PARAMETER:
              stop_cfm = value;
              hinge_flags = flags(hinge_flags.container | flags::CFM_STOP);
              break;
            case joint_interface::CFM_PARAMETER:
              normal_cfm = value;
              hinge_flags = flags(hinge_flags.container | flags::CFM_NORM);
              break;
            case joint_interface::ERP_PARAMETER:
              normal_erp = value;
              hinge_flags = flags(hinge_flags.container | flags::ERP_NORM);
              break;
            default:
              ASSERT(0);
          }
        } else {
          ASSERT(0);
        }
      }
      
      core::scalar hinge::get_param(const uint32_t &num, const uint32_t &axis) const {
        core::scalar retVal = 0.0f;
        if ((axis == UINT32_MAX) || (axis == 5)) {
          switch (num) {
            case joint_interface::STOP_ERP_PARAMETER:
              ASSERT(hinge_flags.erp_stop());
              retVal = stop_erp;
              break;
            case joint_interface::STOP_CFM_PARAMETER:
              ASSERT(hinge_flags.cfm_stop());
              retVal = stop_cfm;
              break;
            case joint_interface::CFM_PARAMETER:
              ASSERT(hinge_flags.cfm_norm());
              retVal = normal_cfm;
              break;
            case joint_interface::ERP_PARAMETER:
              ASSERT(hinge_flags.erp_norm());
              retVal = normal_erp;
              break;
            default:
              ASSERT(0);
          }
        } else {
          ASSERT(0);
        }
        
        return retVal;
      }
      
      const vec4 zero_last = vec4(1.0f, 1.0f, 1.0f, 0.0f);
      void hinge::parameters2_using_frame_offset(struct parameters2 &param2, const core::transform& frame_a, const core::transform& frame_b, const core::vec4 &ang_vel_a, const core::vec4 &ang_vel_b) {
        ASSERT(!use_solve_constraint_obsolete);
        int i, s = param2.rowskip;
#ifdef DE_MATRIX_REPRESENTATION_VEC4
        (void)i;
        const uint32_t scalar_skip = s * (sizeof(vec4) / sizeof(scalar));
#else  
        const uint32_t scalar_skip = s;
#endif
        // transforms in world space
        core::transform trA = frame_a * rb_a_frame;
        core::transform trB = frame_b * rb_b_frame;
        // pivot point
        // btVector3 pivotAInW = trA.getOrigin();
        // btVector3 pivotBInW = trB.getOrigin();
        
#if 1
        // difference between frames in WCS
        vec4 ofs = trB.pos - trA.pos;
        // now get weight factors depending on masses
        scalar miA = (*ctx).bodies[body0].inverse_mass;// getRigidBodyA().getInvMass();
        scalar miB = body1 == UINT32_MAX ? 0.0f : (*ctx).bodies[body1].inverse_mass; //getRigidBodyB().getInvMass();
        bool hasStaticBody = (miA < EPSILON) || (miB < EPSILON);
        scalar miS = miA + miB;
        scalar factA, factB;
        if (miS > scalar(0.f)) {
          factA = miB / miS;
        } else {
          factA = scalar(0.5f);
        }
        factB = scalar(1.0f) - factA;
        // get the desired direction of hinge axis
        // as weighted sum of Z-orthos of frameA and frameB in WCS
        vec4 ax1A = trA.get_basis().column(2);
        vec4 ax1B = trB.get_basis().column(2);
        vec4 ax1 = ax1A * factA + ax1B * factB;
        if (simd::length2(ax1) < EPSILON) {
          factA = 0.f;
          factB = 1.f;
          ax1 = ax1A * factA + ax1B * factB;
        }
        ax1 = simd::normalize(ax1);
        // fill first 3 rows
        // we want: velA + wA x relA == velB + wB x relB
        core::transform bodyA_trans = frame_a;
        core::transform bodyB_trans = frame_b;
        int s0 = 0;
        int s1 = s;
        int s2 = s * 2;
        int scalar_s0 = 0;
        int scalar_s1 = scalar_skip;
        int scalar_s2 = scalar_skip * 2;
        int nrow = 2;  // last filled row
        vec4 tmpA, tmpB, relA, relB, p, q;
        // get vector from bodyB to frameB in WCS
        relB = trB.pos - bodyB_trans.pos;
        // get its projection to hinge axis
        vec4 projB = ax1 * simd::dot(relB, ax1);
        // get vector directed from bodyB to hinge axis (and orthogonal to it)
        vec4 orthoB = relB - projB;
        // same for bodyA
        relA = trA.pos - bodyA_trans.pos;
        vec4 projA = ax1 * simd::dot(relA, ax1);
        vec4 orthoA = relA - projA;
        vec4 totalDist = projA - projB;
        // get offset vectors relA and relB
        relA = orthoA + totalDist * factA;
        relB = orthoB - totalDist * factB;
        // now choose average ortho to hinge axis
        p = orthoB * factA + orthoA * factB;
        scalar len2 = simd::length2(p);
        if (len2 > EPSILON) {
          p /= glm::sqrt(len2);
        } else {
          p = trA.get_basis().column(1);
        }
        // make one more ortho
        q = simd::cross(ax1, p);
        // fill three rows
        tmpA = simd::cross(relA, p);
        tmpB = simd::cross(relB, p);
#ifdef DE_MATRIX_REPRESENTATION_VEC4
        param2.J1_angular_axis[s0] =  tmpA * zero_last;
        param2.J1_angular_axis[s0] = -tmpB * zero_last;
#else
        for (i = 0; i < 3; i++) param2.J1_angular_axis[s0 + i] =  tmpA[i];
        for (i = 0; i < 3; i++) param2.J2_angular_axis[s0 + i] = -tmpB[i];
#endif        
        tmpA = simd::cross(relA, q);
        tmpB = simd::cross(relB, q);
        if (hasStaticBody && this->limit.solve_limit) {  
          // to make constraint between static and dynamic objects more rigid
          // remove wA (or wB) from equation if angular limit is hit
          tmpB *= factB;
          tmpA *= factA;
        }
#ifdef DE_MATRIX_REPRESENTATION_VEC4
        param2.J1_angular_axis[s1] =  tmpA * zero_last;
        param2.J1_angular_axis[s1] = -tmpB * zero_last;
#else
        for (i = 0; i < 3; i++) param2.J1_angular_axis[s1 + i] =  tmpA[i];
        for (i = 0; i < 3; i++) param2.J2_angular_axis[s1 + i] = -tmpB[i];
#endif        
        tmpA = simd::cross(relA, ax1);
        tmpB = simd::cross(relB, ax1);
        if (hasStaticBody) {  
          // to make constraint between static and dynamic objects more rigid
          // remove wA (or wB) from equation
          tmpB *= factB;
          tmpA *= factA;
        }
#ifdef DE_MATRIX_REPRESENTATION_VEC4
        param2.J1_angular_axis[s2] =  tmpA * zero_last;
        param2.J1_angular_axis[s2] = -tmpB * zero_last;
#else
        for (i = 0; i < 3; i++) param2.J1_angular_axis[s2 + i] =  tmpA[i];
        for (i = 0; i < 3; i++) param2.J2_angular_axis[s2 + i] = -tmpB[i];
#endif 

        scalar normalErp = (hinge_flags.erp_norm()) ? this->normal_erp : param2.erp;
        scalar k = param2.fps * normalErp;

        if (!angular_only) {
#ifdef DE_MATRIX_REPRESENTATION_VEC4
          param2.J1_linear_axis[s0] =   p * zero_last;
          param2.J1_linear_axis[s1] =   q * zero_last;
          param2.J1_linear_axis[s2] = ax1 * zero_last;
          
          param2.J2_linear_axis[s0] =   -p * zero_last;
          param2.J2_linear_axis[s1] =   -q * zero_last;
          param2.J2_linear_axis[s2] = -ax1 * zero_last;
#else
          for (i = 0; i < 3; i++) param2.J1_linear_axis[s0 + i] = p[i];
          for (i = 0; i < 3; i++) param2.J1_linear_axis[s1 + i] = q[i];
          for (i = 0; i < 3; i++) param2.J1_linear_axis[s2 + i] = ax1[i];

          for (i = 0; i < 3; i++) param2.J2_linear_axis[s0 + i] = -p[i];
          for (i = 0; i < 3; i++) param2.J2_linear_axis[s1 + i] = -q[i];
          for (i = 0; i < 3; i++) param2.J2_linear_axis[s2 + i] = -ax1[i];
#endif

          // compute three elements of right hand side

          scalar rhs = k * simd::dot(p, ofs);
          param2.constraint_error[scalar_s0] = rhs;
          rhs = k * simd::dot(q, ofs);
          param2.constraint_error[scalar_s1] = rhs;
          rhs = k * simd::dot(ax1, ofs);
          param2.constraint_error[scalar_s2] = rhs;
        }
        
        // the hinge axis should be the only unconstrained
        // rotational axis, the angular velocity of the two bodies perpendicular to
        // the hinge axis should be equal. thus the constraint equations are
        //    p*w1 - p*w2 = 0
        //    q*w1 - q*w2 = 0
        // where p and q are unit vectors normal to the hinge axis, and w1 and w2
        // are the angular velocity vectors of the two bodies.
        
        int s3 = 3 * s;
        int s4 = 4 * s;
        int scalar_s3 = 3 * scalar_skip;
        int scalar_s4 = 4 * scalar_skip;
#ifdef DE_MATRIX_REPRESENTATION_VEC4
        param2.J1_angular_axis[s3] =  p * zero_last;
        param2.J1_angular_axis[s4] =  q * zero_last;
        param2.J2_angular_axis[s3] = -p * zero_last;
        param2.J2_angular_axis[s4] = -q * zero_last;
#else
        param2.J1_angular_axis[s3 + 0] = p[0];
        param2.J1_angular_axis[s3 + 1] = p[1];
        param2.J1_angular_axis[s3 + 2] = p[2];
        param2.J1_angular_axis[s4 + 0] = q[0];
        param2.J1_angular_axis[s4 + 1] = q[1];
        param2.J1_angular_axis[s4 + 2] = q[2];

        param2.J2_angular_axis[s3 + 0] = -p[0];
        param2.J2_angular_axis[s3 + 1] = -p[1];
        param2.J2_angular_axis[s3 + 2] = -p[2];
        param2.J2_angular_axis[s4 + 0] = -q[0];
        param2.J2_angular_axis[s4 + 1] = -q[1];
        param2.J2_angular_axis[s4 + 2] = -q[2];
#endif
        
        // compute the right hand side of the constraint equation. set relative
        // body velocities along p and q to bring the hinge back into alignment.
        // if ax1A,ax1B are the unit length hinge axes as computed from bodyA and
        // bodyB, we need to rotate both bodies along the axis u = (ax1 x ax2).
        // if "theta" is the angle between ax1 and ax2, we need an angular velocity
        // along u to cover angle erp*theta in one step :
        //   |angular_velocity| = angle/time = erp*theta / stepsize
        //                      = (erp*fps) * theta
        //    angular_velocity  = |angular_velocity| * (ax1 x ax2) / |ax1 x ax2|
        //                      = (erp*fps) * theta * (ax1 x ax2) / sin(theta)
        // ...as ax1 and ax2 are unit length. if theta is smallish,
        // theta ~= sin(theta), so
        //    angular_velocity  = (erp*fps) * (ax1 x ax2)
        // ax1 x ax2 is in the plane space of ax1, so we project the angular
        // velocity to p and q to find the right hand side.
        k = param2.fps * normalErp;  //??

        vec4 u = simd::cross(ax1A, ax1B);
        param2.constraint_error[scalar_s3] = k * simd::dot(u, p);
        param2.constraint_error[scalar_s4] = k * simd::dot(u, q);
#endif
        
        // check angular limits
        nrow = 4;  // last filled row
        int srow;
        int scalar_srow;
        scalar limit_err = scalar(0.0);
        int limit = 0;
        if (this->limit.solve_limit) {
//         #ifdef _BT_USE_CENTER_LIMIT_
          limit_err = this->limit.correction * reference_sign;
//         #else
//           limit_err = m_correction * m_referenceSign;
//         #endif
          limit = (limit_err > scalar(0.0)) ? 1 : 2;
        }
        // if the hinge has joint limits or motor, add in the extra row
        bool powered = enable_angular_motor;
        if (limit || powered)
        {
          nrow++;
          srow = nrow * param2.rowskip;
          scalar_srow = nrow * scalar_skip;
#ifdef DE_MATRIX_REPRESENTATION_VEC4
          param2.J1_angular_axis[srow] =  ax1 * zero_last;
          param2.J1_angular_axis[srow] = -ax1 * zero_last;
#else
          param2.J1_angular_axis[srow + 0] = ax1[0];
          param2.J1_angular_axis[srow + 1] = ax1[1];
          param2.J1_angular_axis[srow + 2] = ax1[2];

          param2.J2_angular_axis[srow + 0] = -ax1[0];
          param2.J2_angular_axis[srow + 1] = -ax1[1];
          param2.J2_angular_axis[srow + 2] = -ax1[2];
#endif

          scalar lostop = this->limit.get_low(); //getLowerLimit();
          scalar histop = this->limit.get_high();
          if (limit && (glm::abs(lostop - histop) < EPSILON)) {  
            // the joint motor is ineffective
            powered = false;
          }
          
          param2.constraint_error[scalar_srow] = scalar(0.0f);
          scalar currERP = (hinge_flags.erp_stop()) ? this->stop_erp : normalErp;
          if (powered) {
            if (hinge_flags.cfm_norm()) {
              param2.cfm[scalar_srow] = normal_cfm;
            }
            scalar mot_fact = get_motor_factor(hinge_angle, lostop, histop, motor_target_velocity, param2.fps * currERP);
            param2.constraint_error[scalar_srow] += mot_fact * motor_target_velocity * reference_sign;
            param2.lower_limit[scalar_srow] = -max_motor_impulse;
            param2.upper_limit[scalar_srow] =  max_motor_impulse;
          }
          
          if (limit) {
            k = param2.fps * currERP;
            param2.constraint_error[scalar_srow] += k * limit_err;
            if (hinge_flags.cfm_stop()) {
              param2.cfm[scalar_srow] = this->stop_cfm;
            }
            
            if (lostop == histop) { // limited low and high simultaneously
              param2.lower_limit[scalar_srow] = -INFINITY;
              param2.upper_limit[scalar_srow] =  INFINITY;
            } else if (limit == 1) {  // low limit
              param2.lower_limit[scalar_srow] = 0;
              param2.upper_limit[scalar_srow] = INFINITY;
            } else {  // high limit
              param2.lower_limit[scalar_srow] = -INFINITY;
              param2.upper_limit[scalar_srow] =  0;
            }
            // bounce (we'll use slider parameter abs(1.0 - m_dampingLimAng) for that)
//         #ifdef _BT_USE_CENTER_LIMIT_
            scalar bounce = this->limit.relaxation_factor;// getRelaxationFactor();
//         #else
//             btScalar bounce = m_relaxationFactor;
//         #endif
            if (bounce > scalar(0.0)) {
              scalar vel = simd::dot(ang_vel_a, ax1);
              vel -= simd::dot(ang_vel_b, ax1);
              // only apply bounce if the velocity is incoming, and if the
              // resulting c[] exceeds what we already have.
              if (limit == 1) {  // low limit
                if (vel < 0) {
                  scalar newc = -bounce * vel;
                  if (newc > param2.constraint_error[scalar_srow]) {
                    param2.constraint_error[scalar_srow] = newc;
                  }
                }
              } else {  // high limit - all those computations are reversed
                if (vel > 0) {
                  scalar newc = -bounce * vel;
                  if (newc < param2.constraint_error[scalar_srow]) {
                    param2.constraint_error[scalar_srow] = newc;
                  }
                }
              }
            }
//         #ifdef _BT_USE_CENTER_LIMIT_
            param2.constraint_error[scalar_srow] *= this->limit.bias_factor;
//         #else
//             info->m_constraintError[srow] *= m_biasFactor;
//         #endif
          }  // if(limit)
        }      // if angular limit or powered
      }
      
      void hinge::parameters2(struct parameters2 &param2, const core::transform& frame_a, const core::transform& frame_b, const core::vec4 &ang_vel_a, const core::vec4 &ang_vel_b) {
        ASSERT(!use_solve_constraint_obsolete);
        int i, skip = param2.rowskip;
#ifdef DE_MATRIX_REPRESENTATION_VEC4
        const uint32_t scalar_skip = skip * (sizeof(vec4) / sizeof(scalar));
#else  
        const uint32_t scalar_skip = skip;
#endif
        // transforms in world space
        core::transform trA = frame_a * rb_a_frame;
        core::transform trB = frame_b * rb_b_frame;
        // pivot point
        vec4 pivotAInW = trA.pos;
        vec4 pivotBInW = trB.pos;
        // linear (all fixed)

        if (!angular_only) {
#ifdef DE_MATRIX_REPRESENTATION_VEC4
          //ASSERT(skip == 1);
          param2.J1_linear_axis[0] = vec4(1.0f, 0.0f, 0.0f, 0.0f);
          param2.J1_linear_axis[skip] = vec4(0.0f, 1.0f, 0.0f, 0.0f);
          param2.J1_linear_axis[2*skip] = vec4(0.0f, 0.0f, 1.0f, 0.0f);
          
          param2.J2_linear_axis[0] = vec4(-1.0f, 0.0f, 0.0f, 0.0f);
          param2.J2_linear_axis[skip] = vec4(0.0f, -1.0f, 0.0f, 0.0f);
          param2.J2_linear_axis[2*skip] = vec4(0.0f, 0.0f, -1.0f, 0.0f);
#else
          param2.J1_linear_axis[0] = 1;
          param2.J1_linear_axis[skip + 1] = 1;
          param2.J1_linear_axis[2 * skip + 2] = 1;

          param2.J2_linear_axis[0] = -1;
          param2.J2_linear_axis[skip + 1] = -1;
          param2.J2_linear_axis[2 * skip + 2] = -1;
#endif
        }

        vec4 a1 = pivotAInW - frame_a.pos;
        {
          vec4 angular0, angular1, angular2, a1neg = -a1;
          simd::skew_symmetric_matrix(a1neg, angular0, angular1, angular2);
          
#ifdef DE_MATRIX_REPRESENTATION_VEC4
          param2.J1_angular_axis[0] = angular0 * zero_last;
          param2.J1_angular_axis[skip] = angular1 * zero_last;
          param2.J1_angular_axis[2*skip] = angular2 * zero_last;
#else 
          for (i = 0; i < 3; ++i) param2.J1_angular_axis[i] = angular0[i];
          for (i = 0; i < 3; ++i) param2.J1_angular_axis[i + skip] = angular1[i];
          for (i = 0; i < 3; ++i) param2.J1_angular_axis[i + 2 * skip] = angular2[i];
#endif
          
//           vec4* angular0 = (btVector3*)(info->m_J1angularAxis);
//           vec4* angular1 = (btVector3*)(info->m_J1angularAxis + skip);
//           vec4* angular2 = (btVector3*)(info->m_J1angularAxis + 2 * skip);
//           vec4 a1neg = -a1;
//           a1neg.getSkewSymmetricMatrix(angular0, angular1, angular2);
        }
        
        vec4 a2 = pivotBInW - frame_b.pos;
        {
          vec4 angular0, angular1, angular2;
          simd::skew_symmetric_matrix(a2, angular0, angular1, angular2);
          
#ifdef DE_MATRIX_REPRESENTATION_VEC4
          param2.J2_angular_axis[0] = angular0 * zero_last;
          param2.J2_angular_axis[skip] = angular1 * zero_last;
          param2.J2_angular_axis[2*skip] = angular2 * zero_last;
#else
          for (i = 0; i < 3; ++i) param2.J2_angular_axis[i] = angular0[i];
          for (i = 0; i < 3; ++i) param2.J2_angular_axis[i + skip] = angular1[i];
          for (i = 0; i < 3; ++i) param2.J2_angular_axis[i + 2 * skip] = angular2[i];
#endif
          
//           btVector3* angular0 = (btVector3*)(info->m_J2angularAxis);
//           btVector3* angular1 = (btVector3*)(info->m_J2angularAxis + skip);
//           btVector3* angular2 = (btVector3*)(info->m_J2angularAxis + 2 * skip);
//           a2.getSkewSymmetricMatrix(angular0, angular1, angular2);
        }
        // linear RHS
        scalar normalErp = (hinge_flags.erp_norm()) ? this->normal_erp : param2.erp;

        scalar k = param2.fps * normalErp;
        if (!angular_only) {
          for (i = 0; i < 3; i++) {
            param2.constraint_error[i * scalar_skip] = k * (pivotBInW[i] - pivotAInW[i]);
          }
        }
        // make rotations around X and Y equal
        // the hinge axis should be the only unconstrained
        // rotational axis, the angular velocity of the two bodies perpendicular to
        // the hinge axis should be equal. thus the constraint equations are
        //    p*w1 - p*w2 = 0
        //    q*w1 - q*w2 = 0
        // where p and q are unit vectors normal to the hinge axis, and w1 and w2
        // are the angular velocity vectors of the two bodies.
        // get hinge axis (Z)
        const mat4 trans_basis = simd::transpose(trA.get_basis());
//         vec4 ax1 = trA.getBasis().getColumn(2);
//         // get 2 orthos to hinge axis (X, Y)
//         vec4 p = trA.getBasis().getColumn(0);
//         vec4 q = trA.getBasis().getColumn(1);
        vec4 ax1 = trans_basis[2];
        // get 2 orthos to hinge axis (X, Y)
        vec4 p = trans_basis[0];
        vec4 q = trans_basis[1];
        // set the two hinge angular rows
        int s3 = 3 * param2.rowskip;
        int s4 = 4 * param2.rowskip;
        int scalar_s3 = 3 * scalar_skip;
        int scalar_s4 = 4 * scalar_skip;

#ifdef DE_MATRIX_REPRESENTATION_VEC4
        param2.J1_angular_axis[s3] =  p * zero_last;
        param2.J1_angular_axis[s4] =  q * zero_last;
        param2.J2_angular_axis[s3] = -p * zero_last;
        param2.J2_angular_axis[s4] = -q * zero_last;
        
#else
        param2.J1_angular_axis[s3 + 0] = p[0];
        param2.J1_angular_axis[s3 + 1] = p[1];
        param2.J1_angular_axis[s3 + 2] = p[2];
        param2.J1_angular_axis[s4 + 0] = q[0];
        param2.J1_angular_axis[s4 + 1] = q[1];
        param2.J1_angular_axis[s4 + 2] = q[2];

        param2.J2_angular_axis[s3 + 0] = -p[0];
        param2.J2_angular_axis[s3 + 1] = -p[1];
        param2.J2_angular_axis[s3 + 2] = -p[2];
        param2.J2_angular_axis[s4 + 0] = -q[0];
        param2.J2_angular_axis[s4 + 1] = -q[1];
        param2.J2_angular_axis[s4 + 2] = -q[2];
#endif
        
        // compute the right hand side of the constraint equation. set relative
        // body velocities along p and q to bring the hinge back into alignment.
        // if ax1,ax2 are the unit length hinge axes as computed from body1 and
        // body2, we need to rotate both bodies along the axis u = (ax1 x ax2).
        // if `theta' is the angle between ax1 and ax2, we need an angular velocity
        // along u to cover angle erp*theta in one step :
        //   |angular_velocity| = angle/time = erp*theta / stepsize
        //                      = (erp*fps) * theta
        //    angular_velocity  = |angular_velocity| * (ax1 x ax2) / |ax1 x ax2|
        //                      = (erp*fps) * theta * (ax1 x ax2) / sin(theta)
        // ...as ax1 and ax2 are unit length. if theta is smallish,
        // theta ~= sin(theta), so
        //    angular_velocity  = (erp*fps) * (ax1 x ax2)
        // ax1 x ax2 is in the plane space of ax1, so we project the angular
        // velocity to p and q to find the right hand side.
        
        vec4 ax2 = trB.get_basis().column(2);
        vec4 u = simd::cross(ax1, ax2);
        param2.constraint_error[scalar_s3] = k * simd::dot(u, p);
        param2.constraint_error[scalar_s4] = k * simd::dot(u, q);
        // check angular limits
        int nrow = 4;  // last filled row
        int srow;
        int scalar_srow;
        scalar limit_err = scalar(0.0);
        int limit = 0;
        if (this->limit.solve_limit) {
//         #ifdef _BT_USE_CENTER_LIMIT_
          limit_err = this->limit.correction * reference_sign;
//         #else
//           limit_err = m_correction * m_referenceSign;
//         #endif
          limit = (limit_err > scalar(0.0)) ? 1 : 2;
        }
        
        // if the hinge has joint limits or motor, add in the extra row
        bool powered = enable_angular_motor;
        if (limit || powered) {
          nrow++;
          srow = nrow * param2.rowskip;
          scalar_srow = nrow * scalar_skip;
#ifdef DE_MATRIX_REPRESENTATION_VEC4
          param2.J1_angular_axis[srow] =  ax1 * zero_last;
          param2.J2_angular_axis[srow] = -ax1 * zero_last;
#else
          param2.J1_angular_axis[srow + 0] = ax1[0];
          param2.J1_angular_axis[srow + 1] = ax1[1];
          param2.J1_angular_axis[srow + 2] = ax1[2];

          param2.J2_angular_axis[srow + 0] = -ax1[0];
          param2.J2_angular_axis[srow + 1] = -ax1[1];
          param2.J2_angular_axis[srow + 2] = -ax1[2];
#endif

          scalar lostop = this->limit.get_low();// getLowerLimit();
          scalar histop = this->limit.get_high();// getUpperLimit();
          if (limit && (glm::abs(lostop - histop) < EPSILON)) {  // the joint motor is ineffective
            powered = false;
          }
          
          param2.constraint_error[scalar_srow] = scalar(0.0f);
          scalar currERP = (hinge_flags.erp_stop()) ? this->stop_erp : normalErp;
          if (powered) {
            if (hinge_flags.cfm_norm()) {
              param2.cfm[scalar_srow] = normal_cfm;
            }
            
            scalar mot_fact = get_motor_factor(hinge_angle, lostop, histop, motor_target_velocity, param2.fps * currERP);
            param2.constraint_error[scalar_srow] += mot_fact * motor_target_velocity * reference_sign;
            param2.lower_limit[scalar_srow] = -max_motor_impulse;
            param2.upper_limit[scalar_srow] =  max_motor_impulse;
          }
          
          if (limit) {
            k = param2.fps * currERP;
            param2.constraint_error[scalar_srow] += k * limit_err;
            if (hinge_flags.cfm_stop()) {
              param2.cfm[scalar_srow] = this->stop_cfm;
            }
            
            if (glm::abs(lostop - histop) < EPSILON) { // limited low and high simultaneously
              param2.lower_limit[scalar_srow] = -INFINITY;
              param2.upper_limit[scalar_srow] =  INFINITY;
            } else if (limit == 1) {  // low limit
              param2.lower_limit[scalar_srow] = 0;
              param2.upper_limit[scalar_srow] = INFINITY;
            } else {  // high limit
              param2.lower_limit[scalar_srow] = -INFINITY;
              param2.upper_limit[scalar_srow] =  0;
            }
            
            // bounce (we'll use slider parameter abs(1.0 - m_dampingLimAng) for that)
//         #ifdef _BT_USE_CENTER_LIMIT_
            scalar bounce = this->limit.relaxation_factor;
//         #else
//             btScalar bounce = m_relaxationFactor;
//         #endif
            if (bounce > scalar(0.0)) {
              scalar vel = simd::dot(ang_vel_a, ax1);
              vel -= simd::dot(ang_vel_b, ax1);
              // only apply bounce if the velocity is incoming, and if the
              // resulting c[] exceeds what we already have.
              if (limit == 1) {  // low limit
                if (vel < 0) {
                  scalar newc = -bounce * vel;
                  if (newc > param2.constraint_error[scalar_srow]) {
                    param2.constraint_error[scalar_srow] = newc;
                  }
                }
              } else {  // high limit - all those computations are reversed
                if (vel > 0) {
                  scalar newc = -bounce * vel;
                  if (newc < param2.constraint_error[scalar_srow]) {
                    param2.constraint_error[scalar_srow] = newc;
                  }
                }
              }
            }
//         #ifdef _BT_USE_CENTER_LIMIT_
            param2.constraint_error[scalar_srow] *= this->limit.bias_factor;
//         #else
//             param2.constraint_error[srow] *= m_biasFactor;
//         #endif
          }  // if(limit)
        }      // if angular limit or powered
      }
    }
  }
}
