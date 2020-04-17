#include "rigid_body.h"

#include <cstring>
#include <stdexcept>

#include "physics_context.h"

namespace devils_engine {
  namespace physics {
    namespace core {
      rigid_body::flags::flags() : container(0) {}
      rigid_body::flags::flags(const uint32_t &flags) : container(flags) {}
      bool rigid_body::flags::static_object() const { return (container & STATIC_OBJECT) == STATIC_OBJECT; }
      bool rigid_body::flags::kinematic_object() const { return (container & KINEMATIC_OBJECT) == KINEMATIC_OBJECT; }
      bool rigid_body::flags::no_contact() const { return (container & NO_CONTACT_RESPONCE) == NO_CONTACT_RESPONCE; }
      bool rigid_body::flags::contact_stiffness_damping() const { return (container & HAS_CONTACT_STIFFNESS_DAMPING) == HAS_CONTACT_STIFFNESS_DAMPING; }
      bool rigid_body::flags::friction_anchor() const { return (container & HAS_FRICTION_ANCHOR) == HAS_FRICTION_ANCHOR; }
      bool rigid_body::flags::custom_material_callback() const { return (container & CUSTOM_MATERIAL_CALLBACK) == CUSTOM_MATERIAL_CALLBACK; }
      bool rigid_body::flags::character_object() const { return (container & CHARACTER_OBJECT) == CHARACTER_OBJECT; }
      bool rigid_body::flags::collision_sound_trigger() const { return (container & HAS_COLLISION_SOUND_TRIGGER) == HAS_COLLISION_SOUND_TRIGGER; }
      bool rigid_body::flags::collision_object() const { return (container & COLLISION_OBJECT) == COLLISION_OBJECT; }
      bool rigid_body::flags::rigid_body() const { return (container & RIGID_BODY) == RIGID_BODY; }
      bool rigid_body::flags::ghost_object() const { return (container & GHOST_OBJECT) == GHOST_OBJECT; }
      bool rigid_body::flags::soft_body() const { return (container & SOFT_BODY) == SOFT_BODY; }
      bool rigid_body::flags::fluid() const { return (container & FLUID) == FLUID; }
      bool rigid_body::flags::user_type() const { return (container & USER_TYPE) == USER_TYPE; }
      bool rigid_body::flags::gyroscopic_force_explicit() const { return (container & GYROSCOPIC_FORCE_EXPLICIT) == GYROSCOPIC_FORCE_EXPLICIT; }
      bool rigid_body::flags::gyroscopic_force_implicit_world() const { return (container & GYROSCOPIC_FORCE_IMPLICIT_WORLD) == GYROSCOPIC_FORCE_IMPLICIT_WORLD; }
      bool rigid_body::flags::gyroscopic_force_implicit_body() const { return (container & GYROSCOPIC_FORCE_IMPLICIT_BODY) == GYROSCOPIC_FORCE_IMPLICIT_BODY; }
      rigid_body::flags & rigid_body::flags::operator|=(const uint32_t &new_flags) { container |= new_flags; return *this; }
      rigid_body::flags & rigid_body::flags::operator&=(const uint32_t &new_flags) { container &= new_flags; return *this; }
      
      rigid_body::rigid_body() : 
        linear_factor(1.0f, 1.0f, 1.0f, 0.0f), 
        angular_factor(1.0f, 1.0f, 1.0f, 0.0f),
        anisotropic_friction(1.0f, 1.0f, 1.0f, 0.0f),
        inverse_mass(0.0f), 
        gravity_factor(1.0f), 
        linear_damping(0.0f), 
        angular_damping(0.0f), 
        contact_damping(0.1f),
        contact_stiffness(DE_LARGE_FLOAT),
        friction(0.0f),
        rolling_friction(0.0f),
        spinning_friction(0.0f),
        restitution(0.0f),
        linear_sleeping_threshold(0.0f),
        angular_sleeping_threshold(0.0f),
        additional_damping_factor(glm::uintBitsToFloat(UINT32_MAX)),
        additional_linear_damping_threshold_sqr(0.0f),
        additional_angular_damping_threshold_sqr(0.0f),
        additional_angular_damping_factor(0.0f),
        contact_processing_threshold(DE_LARGE_FLOAT),
//         body_flags(flags::RIGID_BODY)
        shape_index(UINT32_MAX),
        proxy_index(UINT32_MAX),
        transform_index(UINT32_MAX),
        anisotropic_friction_type(0)
      {}
      
      rigid_body::rigid_body(const create_info &info, context* ctx) :
//         trans(info.start_world_transform),
        linear_factor(1.0f, 1.0f, 1.0f, 0.0f), 
        angular_factor(1.0f, 1.0f, 1.0f, 0.0f), 
        anisotropic_friction(1.0f, 1.0f, 1.0f, 0.0f),
        inverse_mass(info.mass == 0.0f ? 0.0f : 1.0f / info.mass), 
        gravity_factor(info.gravity_factor), 
        linear_damping(info.linear_damping), 
        angular_damping(info.angular_damping), 
        contact_damping(0.1f),
        contact_stiffness(DE_LARGE_FLOAT),
        friction(info.friction),
        rolling_friction(info.rolling_friction),
        spinning_friction(info.spinning_friction),
        restitution(info.restitution),
        linear_sleeping_threshold(info.linear_sleeping_threshold),
        angular_sleeping_threshold(info.angular_sleeping_threshold),
        additional_damping_factor(info.additional_damping_factor),
        additional_linear_damping_threshold_sqr(info.additional_linear_damping_threshold_sqr),
        additional_angular_damping_threshold_sqr(info.additional_angular_damping_threshold_sqr),
        additional_angular_damping_factor(info.additional_angular_damping_factor),
        contact_processing_threshold(DE_LARGE_FLOAT),
//         body_flags(flags::RIGID_BODY),
        shape_index(info.shape_index),
        proxy_index(UINT32_MAX),
        transform_index(info.transform_index),
        anisotropic_friction_type(0)
      {
        set_mass_properties(info.mass, info.local_inertia);
        update_inertia_tensor(ctx);
      }
      
      rigid_body::rigid_body(const uint32_t &transform_index, const uint32_t &shape_index, const scalar &mass, const vec4 &inertia, context* ctx) : 
//         trans(start_world_transform),
        linear_factor(1.0f, 1.0f, 1.0f, 0.0f), 
        angular_factor(1.0f, 1.0f, 1.0f, 0.0f), 
        anisotropic_friction(1.0f, 1.0f, 1.0f, 0.0f),
        inverse_mass(mass == 0.0f ? 0.0f : 1.0f / mass), 
        gravity_factor(1.0f), 
        linear_damping(0.0f), 
        angular_damping(0.0f), 
        contact_damping(0.1f),
        contact_stiffness(DE_LARGE_FLOAT),
        friction(0.0f),
        rolling_friction(0.0f),
        spinning_friction(0.0f),
        restitution(0.0f),
        linear_sleeping_threshold(0.0f),
        angular_sleeping_threshold(0.0f),
        additional_damping_factor(glm::uintBitsToFloat(UINT32_MAX)),
        additional_linear_damping_threshold_sqr(0.0f),
        additional_angular_damping_threshold_sqr(0.0f),
        additional_angular_damping_factor(0.0f),
        contact_processing_threshold(DE_LARGE_FLOAT),
//         body_flags(flags::RIGID_BODY),
        shape_index(shape_index),
        proxy_index(UINT32_MAX),
        transform_index(transform_index),
        anisotropic_friction_type(0)
      {
        set_mass_properties(mass, inertia);
        update_inertia_tensor(ctx);
      }
      
      bool rigid_body::valid() const {
        return transform_index != UINT32_MAX;
      }
      
      bool rigid_body::is_dynamic() const {
        ASSERT(!(body_flags.static_object() && body_flags.kinematic_object()));
        return !body_flags.static_object() && !body_flags.kinematic_object();
      }
      
      bool rigid_body::is_static() const {
        ASSERT(!(body_flags.static_object() && body_flags.kinematic_object()));
        return body_flags.static_object() && !body_flags.kinematic_object();
      }
      
      bool rigid_body::is_kinematic() const {
        ASSERT(!(body_flags.static_object() && body_flags.kinematic_object()));
        return !body_flags.static_object() && body_flags.kinematic_object();
      }
      
      void rigid_body::proceed(const transform &trans, context* ctx) {
        set_center_of_mass(trans, ctx);
      }
      
      transform rigid_body::predict(const transform &old_transform, const size_t &time) {
        transform t;
        transform::integrate(old_transform, linear_velocity, angular_velocity, MCS_TO_SEC(time), t);
        return t;
      }
      
      void rigid_body::save_kinematic_state(const size_t &time) {
        if (time == 0) return;
        // мы должны использовать мотион стейт
//         if (motion != nullptr) {}
        
//         transform::calculate_velocity();
        
      }
      
      void rigid_body::apply_gravity(const vec4 &gravity) {
        // проверять флаги?
        apply_central_force(gravity);
      }
      
      void rigid_body::apply_damping(const size_t &time) {
        const scalar time_step = MCS_TO_SEC(time);
        
  //       linear_velocity *= glm::clamp(1.0f - linear_damping * time_step, 0.0f, 1.0f);
  //       angular_velocity *= glm::clamp(1.0f - angular_damping* time_step, 0.0f, 1.0f);
        linear_velocity *= glm::pow(1.0f - linear_damping, time_step);
        angular_velocity *= glm::pow(1.0f - angular_damping, time_step);
        
        if (glm::floatBitsToUint(additional_damping_factor) == UINT32_MAX) return;
        
        if (simd::length2(linear_velocity) < additional_linear_damping_threshold_sqr && simd::length2(angular_velocity) < additional_angular_damping_threshold_sqr) {
          linear_velocity *= additional_damping_factor;
          angular_velocity *= additional_damping_factor;
        }
        
        const scalar speed = simd::length(linear_velocity);
        if (speed < linear_damping) {
          const scalar damp_vel = 0.005f;
          if (speed > damp_vel) {
            const vec4 dir = linear_velocity / speed;
            linear_velocity -= dir * damp_vel;
          } else {
            linear_velocity = vec4(0.0f, 0.0f, 0.0f, 0.0f);
          }
        }
        
        const scalar ang_speed = simd::length(angular_velocity);
        if (ang_speed < angular_damping) {
          const scalar damp_vel = 0.005f;
          if (ang_speed > damp_vel) {
            const vec4 dir = angular_velocity / ang_speed;
            angular_velocity -= dir * damp_vel;
          } else {
            angular_velocity = vec4(0.0f, 0.0f, 0.0f, 0.0f);
          }
        }
      }
      
      void rigid_body::set_mass_properties(const scalar &mass, const vec4 &inertia) {
        if (std::abs(mass) < EPSILON) {
          body_flags |= flags::STATIC_OBJECT;
          inverse_mass = 0.0f;
        } else {
          body_flags &= ~flags::STATIC_OBJECT;
          inverse_mass = 1.0f / mass;
        }
        
        // в булете выставляется гравитация своя для каждого объекта
        // зачем? гравитация это ускорение на массу, а масса у нас содержится только инвертированная
        // проще мне кажется просто массу вернуть
        
        float arr[4];
        inertia.storeu(arr);
        inv_local_inertia = vec4(
          arr[0] != 0.0f ? 1.0f / arr[0] : 0.0f,
          arr[1] != 0.0f ? 1.0f / arr[1] : 0.0f,
          arr[2] != 0.0f ? 1.0f / arr[2] : 0.0f,
          0.0f
        );
        
        inv_mass = linear_factor * inverse_mass;
      }
      
      void rigid_body::set_linear_factor(const vec4 &factor) {
        linear_factor = factor;
        inv_mass = linear_factor * inverse_mass;
      }
      
      void rigid_body::integrate_velocities(const size_t &time) {
        // если статика или кинематика, то ничего не делаем
        
        const scalar step = MCS_TO_SEC(time);
        linear_velocity += force * inverse_mass * step;
        angular_velocity += inv_inertia_tensor * torque * step;
        
  #define MAX_ANGVEL PI_H
        
        const scalar angvel = simd::length(angular_velocity);
        if (angvel * step / MAX_ANGVEL) {
          angular_velocity *= MAX_ANGVEL / step / angvel;
        }
      }
      
      void rigid_body::set_center_of_mass(const transform &t, context* ctx) {
        // если кинематический объект то старую трансформу кладем в трансформу интерполяции
        // если нет, то туда кладем новую трансформу
        // обновляем интерполяционную линейную скорость и скорость вращения
        // нужно ли мне это? наверное, нужно потом добавить
        
//         trans = t;
        ctx->new_transforms[transform_index] = t;
        update_inertia_tensor(ctx);
//         throw std::runtime_error("not working");
      }
      
      void rigid_body::apply_central_force(const vec4 &force) {
        this->force += force * linear_factor;
      }
      
      void rigid_body::apply_torque(const vec4 &torque) {
        this->torque += torque * angular_factor;
      }
      
      void rigid_body::apply_force(const vec4 &force, const vec4 &rel_pos) {
        apply_central_force(force);
        apply_torque(simd::cross(rel_pos, force * linear_factor));
      }
      
      void rigid_body::apply_central_impulse(const vec4 &impulse) {
        linear_velocity += impulse * linear_factor * inverse_mass;
      }
      
      void rigid_body::apply_torque_impulse(const vec4 &torque) {
        angular_velocity += inv_inertia_tensor * torque * angular_factor;
      }
      
      void rigid_body::apply_impulse(const vec4 &impulse, const vec4 &rel_pos) {
        if (inverse_mass != 0.0f) {
          apply_central_impulse(impulse);
          if (simd::dot(angular_factor, angular_factor) > EPSILON) {
            apply_torque_impulse(simd::cross(rel_pos, impulse * linear_factor));
          }
        }
      }
      
      void rigid_body::clear_forces() {
        force = vec4(0.0f, 0.0f, 0.0f, 0.0f);
        torque = vec4(0.0f, 0.0f, 0.0f, 0.0f);
      }
      
      void rigid_body::update_inertia_tensor(context* ctx) {
        const mat4 basis = mat4(ctx->new_transforms[transform_index].rot);
        inv_inertia_tensor = simd::scale(basis, inv_local_inertia) * simd::transpose(basis);
      }
      
      vec4 rigid_body::get_velocity_in_local_point(const vec4 &rel_pos) const {
        return linear_velocity + simd::cross(angular_velocity, rel_pos);
      }
      
      vec4 rigid_body::get_local_inertia() const {
        float arr[4];
        inv_local_inertia.storeu(arr);
        return vec4(
          arr[0] != 0.0f ? 1.0f / arr[0] : 0.0f,
          arr[1] != 0.0f ? 1.0f / arr[1] : 0.0f,
          arr[2] != 0.0f ? 1.0f / arr[2] : 0.0f,
          0.0f
        );
      }
      
      scalar rigid_body::get_mass() const {
        return inverse_mass == 0.0f ? 0.0f : 1.0f / inverse_mass;
      }
      
      void rigid_body::translate(const vec4 &v, context* ctx) {
        ctx->new_transforms[transform_index].pos += v;
//         trans.pos += v;
      }
      // getAabb
      
      scalar rigid_body::compute_impulse_denominator(const vec4 &pos, const vec4 &normal, context* ctx) const {
//         const vec4 r0 = pos - trans.pos;
        const vec4 r0 = pos - ctx->new_transforms[transform_index].pos;
        const vec4 c0 = simd::cross(r0, normal);
        const vec4 vec = simd::cross(c0 * inv_inertia_tensor, r0);
        return inverse_mass + simd::dot(normal, vec);
      }
      
      scalar rigid_body::compute_angular_impulse_denominator(const vec4 &axis) const {
        const vec4 vec = axis * inv_inertia_tensor;
        return simd::dot(axis, vec);
      }
      
      inline vec4 evalEulerEqn(const vec4& w1, const vec4& w0, const vec4& T, const scalar dt, const mat4& I) {
        const vec4 w2 = I * w1 + simd::cross(w1, I * w1) * dt - (T * dt + I * w0);
        return w2;
      }
      
      inline mat4 evalEulerEqnDeriv(const vec4& w1, const vec4& w0, const scalar dt, const mat4& I) {
        mat4 w1x, Iw1x;
        const vec4 Iwi = (I * w1);
        simd::skew_symmetric_matrix(w1, w1x[0], w1x[1], w1x[2]);
        simd::skew_symmetric_matrix(Iwi, Iw1x[0], Iw1x[1], Iw1x[2]);

        const mat4 dfw1 = I + (w1x * I - Iw1x) * dt;
        return dfw1;
      }
      
      // что это? надеюсь мне это не пригодится
      vec4 rigid_body::compute_gyroscopic_impulse_implicit_world(const transform &trans, const size_t &time) const {
        // use full newton-euler equations.  common practice to drop the wxIw term. want it for better tumbling behavior.
        // calculate using implicit euler step so it's stable.

        const vec4 inertiaLocal = get_local_inertia();
        const vec4 w0 = angular_velocity;

        mat4 I;

        const mat4 basis = trans.get_basis();
        I = simd::scale(basis, inertiaLocal) * simd::transpose(basis);

        // use newtons method to find implicit solution for new angular velocity (w')
        // f(w') = -(T*step + Iw) + Iw' + w' + w'xIw'*step = 0
        // df/dw' = I + 1xIw'*step + w'xI*step

        vec4 w1 = w0;

        // one step of newton's method
        {
          const vec4 fw = evalEulerEqn(w1, w0, vec4(0, 0, 0, 0), MCS_TO_SEC(time), I);
          const mat4 dfw = evalEulerEqnDeriv(w1, w0, MCS_TO_SEC(time), I);

          vec4 dw;
          dw = simd::solve33(dfw, fw);
          //const btMatrix3x3 dfw_inv = dfw.inverse();
          //dw = dfw_inv*fw;

          w1 -= dw;
        }

        vec4 gf = (w1 - w0);
        return gf;
      }
      
      vec4 rigid_body::compute_gyroscopic_impulse_implicit_body(const transform &trans, const size_t &time) const {
        const vec4 idl = get_local_inertia();
        const vec4 omega1 = angular_velocity;
        const quat q = trans.rot;

        // Convert to body coordinates
        //vec4 omegab = quatRotate(simd::inverse(q), omega1);
        vec4 omegab = simd::inverse(q) * omega1;
        const mat4 Ib = simd::scale(mat4(), idl);
//         Ib.setValue(idl.x(), 0, 0,
//               0, idl.y(), 0,
//               0, 0, idl.z());

        const vec4 ibo = Ib * omegab;

        // Residual vector
        const vec4 f = MCS_TO_SEC(time) * simd::cross(omegab, ibo);

        mat4 skew0;
        simd::skew_symmetric_matrix(omegab, skew0[0], skew0[1], skew0[2]);
        vec4 om = Ib * omegab;
        mat4 skew1;
        simd::skew_symmetric_matrix(om, skew1[0], skew1[1], skew1[2]);

        // Jacobian
        mat4 J = Ib + (skew0 * Ib - skew1) * MCS_TO_SEC(time);

        // btMatrix3x3 Jinv = J.inverse();
        // btVector3 omega_div = Jinv*f;
        const vec4 omega_div = simd::solve33(J, f);

        // Single Newton-Raphson update
        omegab = omegab - omega_div;  //Solve33(J, f);
        // Back to world coordinates
        //const vec4 omega2 = quatRotate(q, omegab);
        const vec4 omega2 = q * omegab;
        const vec4 gf = omega2 - omega1;
        return gf;
      }
      
      vec4 rigid_body::compute_gyroscopic_force_explicit(const transform &trans, const scalar &max_force) const {
        const vec4 inertia_local = get_local_inertia();
        const mat4 m = trans.get_basis();
        const mat4 inertia_tensor = simd::scale(m, inertia_local) * simd::transpose(m);
        const vec4 tmp = inertia_tensor * angular_velocity;
        vec4 gf = simd::cross(angular_velocity, tmp);
        const scalar l2 = simd::length2(gf);
        if (l2 > max_force * max_force) {
          gf *= 1.0f / std::sqrt(l2) * max_force;
        }
        
        return gf;
      }
      
      bool rigid_body::has_anisotropic_friction(const uint32_t &mode) const {
        return (anisotropic_friction_type & mode) == mode;
      }
      
      void rigid_body::set_contact_damping_and_stiffness(const scalar &damping, const scalar &stiffness) {
        contact_damping = damping;
        contact_stiffness = stiffness;
        body_flags |= flags::HAS_CONTACT_STIFFNESS_DAMPING;
        if (contact_stiffness < EPSILON) contact_stiffness = EPSILON;
      }
      
      rigid_body::flags operator|(const rigid_body::flags &flags, const uint32_t &new_flags) {
        return rigid_body::flags(flags.container | new_flags);
      }
      
      rigid_body::flags operator&(const rigid_body::flags &flags, const uint32_t &new_flags) {
        return rigid_body::flags(flags.container & new_flags);
      }
    }
  }
}
