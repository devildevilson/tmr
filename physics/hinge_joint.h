#ifndef HINGE_JOINT_H
#define HINGE_JOINT_H

#include "joint_interface.h"
#include "jacobian_entry.h"
#include "transform.h"

#define DE_USE_CENTER_LIMIT 1

namespace devils_engine {
  namespace physics {
    namespace core {
      struct context;
    }
    
    namespace utils {
      struct angular_limit {
        core::scalar center;
        core::scalar half_range;
        core::scalar softness;
        core::scalar bias_factor;
        core::scalar relaxation_factor;
        core::scalar correction;
        core::scalar sign;

        bool solve_limit;
        
        angular_limit() :
          center(0.0f),
          half_range(-1.0f),
          softness(0.9f),
          bias_factor(0.3f),
          relaxation_factor(1.0f),
          correction(0.0f),
          sign(0.0f),
          solve_limit(false)
        {}
        
        // Sets all limit's parameters.
        // When low > high limit becomes inactive.
        // When high - low > 2PI limit is ineffective too becouse no angle can exceed the limit
        void set(const core::scalar &low, const core::scalar &high, const core::scalar &softness = 0.9f, const core::scalar &bias_factor = 0.3f, const core::scalar &relaxation_factor = 1.0f);
        
        // Checks conastaint angle against limit. If limit is active and the angle violates the limit
        // correction is calculated.
        void test(const core::scalar &angle);
        
        // Checks given angle against limit. If limit is active and angle doesn't fit it, the angle
        // returned is modified so it equals to the limit closest to given angle.
        void fit(core::scalar& angle) const;
        
        core::scalar get_error() const;
        core::scalar get_low() const;
        core::scalar get_high() const;
      };
    }
    
    namespace constraints {
      class hinge : public core::joint_interface {
      public:
        struct flags {
          enum {
            CFM_STOP = (1 << 0),
            ERP_STOP = (1 << 1),
            CFM_NORM = (1 << 2),
            ERP_NORM = (1 << 3)
          };
          
          uint32_t container;
          
          flags();
          flags(const uint32_t &flags);
          
          bool cfm_stop() const;
          bool erp_stop() const;
          bool cfm_norm() const;
          bool erp_norm() const;
        };
        
        core::jacobian_entry jac[3];     //3 orthogonal linear constraints
        core::jacobian_entry jac_ang[3];  //2 orthogonal angular constraints+ 1 for limit/motor

        core::transform rb_a_frame;  // constraint axii. Assumes z is hinge axis.
        core::transform rb_b_frame;

        core::context* ctx;
        core::scalar motor_target_velocity;
        core::scalar max_motor_impulse;

#ifdef DE_USE_CENTER_LIMIT
        utils::angular_limit limit;
#else
        core::scalar m_lowerLimit;
        core::scalar m_upperLimit;
        core::scalar m_limitSign;
        core::scalar m_correction;

        core::scalar m_limitSoftness;
        core::scalar m_biasFactor;
        core::scalar m_relaxationFactor;

        bool m_solveLimit;
#endif

        core::scalar k_hinge;

        core::scalar acc_limit_impulse;
        core::scalar hinge_angle;
        core::scalar reference_sign;

        bool angular_only;
        bool enable_angular_motor;
        bool use_solve_constraint_obsolete;
        bool use_offset_for_constraint_frame;
        bool use_reference_frame_a;

        core::scalar acc_motor_impulse;

        struct flags hinge_flags;
        core::scalar normal_cfm;
        core::scalar normal_erp;
        core::scalar stop_cfm;
        core::scalar stop_erp;
        
        hinge(core::context* ctx, const uint32_t &body0, const uint32_t &body1, const core::vec4& pivot_a, const core::vec4& pivot_b, const core::vec4& axis_a, const core::vec4& axis_b, bool use_reference_frame_a = false);
        hinge(core::context* ctx, const uint32_t &body0, const core::vec4& pivot_a, const core::vec4& axis_a, bool use_reference_frame_a = false);
        hinge(core::context* ctx, const uint32_t &body0, const uint32_t &body1, const core::transform& rb_a_frame, const core::transform& rb_b_frame, bool use_reference_frame_a = false);
        hinge(core::context* ctx, const uint32_t &body0, const core::transform& rb_a_frame, bool use_reference_frame_a = false);
        void build_jacobian() override;
        void get_parameters1(parameters1 &param1) override;
        void get_parameters2(parameters2 &param2) override;
        void update_rhs(const size_t &delta_time);
        void set_frames(const core::transform& frame_a, const core::transform& frame_b);
        core::scalar get_angle();
        core::scalar get_angle(const core::transform& transA, const core::transform& transB);
        void test_limit(const core::transform& transA, const core::transform& transB);
        void set_param(const uint32_t &num, const core::scalar &value, const uint32_t &axis = UINT32_MAX) override;
        core::scalar get_param(const uint32_t &num, const uint32_t &axis = UINT32_MAX) const override;
        
        void parameters2_using_frame_offset(parameters2 &param2, const core::transform& frame_a, const core::transform& frame_b, const core::vec4 &ang_vel_a, const core::vec4 &ang_vel_b);
        void parameters2(parameters2 &param2, const core::transform& frame_a, const core::transform& frame_b, const core::vec4 &ang_vel_a, const core::vec4 &ang_vel_b);
      };
    }
  }
}

#endif
