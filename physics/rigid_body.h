#ifndef RIGID_BODY_H
#define RIGID_BODY_H

// в булете трансформа представляет собой вектор + матрица поворота
// можно ли успешно добавить скейл?
#include "physics_core.h"
#include "transform.h"

namespace devils_engine {
  namespace physics {
    namespace collision {
      struct shape;
    }
    
    namespace core {
//       class motion_state;
//       struct collision_shape;
//       struct broadphase_proxy;
      struct context;
      
      struct rigid_body {
        enum anisotropic_friction {
          anisotropic_friction_disabled = 0,
          anisotropic_kinetic_friction  = (1 << 0),
          anisotropic_rolling_friction  = (1 << 1)
        };
        
        struct flags {
          enum {
            STATIC_OBJECT                 = (1 << 0),
            KINEMATIC_OBJECT              = (1 << 1),
            NO_CONTACT_RESPONCE           = (1 << 2),
            HAS_CONTACT_STIFFNESS_DAMPING = (1 << 3),
            HAS_FRICTION_ANCHOR           = (1 << 4),
            CUSTOM_MATERIAL_CALLBACK      = (1 << 5),
            CHARACTER_OBJECT              = (1 << 6),
            HAS_COLLISION_SOUND_TRIGGER   = (1 << 7),
            COLLISION_OBJECT              = (1 << 8),
            RIGID_BODY                    = (1 << 9),
            GHOST_OBJECT                  = (1 << 10),
            SOFT_BODY                     = (1 << 11),
            FLUID                         = (1 << 12),
            USER_TYPE                     = (1 << 13),
            GYROSCOPIC_FORCE_EXPLICIT     = (1 << 14),
            GYROSCOPIC_FORCE_IMPLICIT_WORLD = (1 << 15),
            GYROSCOPIC_FORCE_IMPLICIT_BODY = (1 << 16)
          };
          
          uint32_t container;
          
          flags();
          flags(const uint32_t &flags);
          bool static_object() const;
          bool kinematic_object() const;
          bool no_contact() const;
          bool contact_stiffness_damping() const;
          bool friction_anchor() const;           // наверное не потребуется
          bool custom_material_callback() const;
          bool character_object() const;
          bool collision_sound_trigger() const;
          bool collision_object() const;
          bool rigid_body() const;
          bool ghost_object() const;
          bool soft_body() const;
          bool fluid() const;
          bool user_type() const;
          bool gyroscopic_force_explicit() const;
          bool gyroscopic_force_implicit_world() const;
          bool gyroscopic_force_implicit_body() const;
          
          rigid_body::flags & operator|=(const uint32_t &new_flags);
          rigid_body::flags & operator&=(const uint32_t &new_flags);
        };
        
        // в булете есть еще нессколько полей в btCollisionObject
        // дополнительная трансформа, линейная скорость, вращательная скорость
        
//         transform trans;
        vec4 linear_velocity;
        vec4 angular_velocity;
        
        vec4 linear_factor;
        vec4 angular_factor;
        vec4 delta_angular_velocity;
        vec4 inv_mass;
        vec4 push_velocity;
        vec4 turn_velocity;
        vec4 anisotropic_friction;

        vec4 force;
        vec4 torque;
        
        vec4 inv_local_inertia;
        mat4 inv_inertia_tensor;
        
        scalar inverse_mass;
        scalar gravity_factor;
        scalar linear_damping;
        scalar angular_damping;
        scalar contact_damping;
        scalar contact_stiffness;
        scalar friction;
        scalar rolling_friction;
        scalar spinning_friction;
        scalar restitution;
        scalar linear_sleeping_threshold;
        scalar angular_sleeping_threshold;
        scalar additional_damping_factor;
        scalar additional_linear_damping_threshold_sqr;
        scalar additional_angular_damping_threshold_sqr;
        scalar additional_angular_damping_factor;
        scalar contact_processing_threshold;
        scalar stair_height;
        
        flags body_flags;
//         motion_state* motion; // пока не понял для чего это
//         collision_shape* shape;
//         broadphase_proxy* proxy;
        uint32_t shape_index;
        uint32_t proxy_index;
//         uint32_t stair_proxy_index;
        uint32_t transform_index;
        
        uint32_t anisotropic_friction_type;
        
        struct create_info {
          // пока что без motion_state
//           motion_state* motion_state;
//           transform start_world_transform;

//           collision_shape* shape;
          vec4 local_inertia;
          
          scalar mass;
          scalar linear_damping;
          scalar angular_damping;

          // трение
          scalar friction;
          // трение вращения
          scalar rolling_friction;
          scalar spinning_friction;  // torsional friction around contact normal

          // упругость
          scalar restitution;

          scalar linear_sleeping_threshold;
          scalar angular_sleeping_threshold;

          // дополнительное коэффициент изменения движения (помогает с джиттерингом, повышает стабильность)
          // если не нужно, то выставить в 0xffffffff
          scalar additional_damping_factor;
          scalar additional_linear_damping_threshold_sqr;
          scalar additional_angular_damping_threshold_sqr;
          scalar additional_angular_damping_factor;
          
          scalar gravity_factor;
          scalar stair_height;
          
          uint32_t transform_index;
          uint32_t shape_index;
          
          uint32_t collision_group;
          uint32_t collision_filter;
          uint32_t collision_trigger;
        };
        
        rigid_body();
        rigid_body(const create_info &info, context* ctx);
        rigid_body(const uint32_t &transform_index, const uint32_t &shape_index, const scalar &mass, const vec4 &inertia, context* ctx);
        
        bool valid() const;
        bool is_dynamic() const;
        bool is_static() const;
        bool is_kinematic() const;
        void proceed(const transform &trans, context* ctx);
        transform predict(const transform &old_transform, const size_t &time);
        void save_kinematic_state(const size_t &time); // ???
        void apply_gravity(const vec4 &gravity);
        void apply_damping(const size_t &time);
        void set_mass_properties(const scalar &mass, const vec4 &inertia);
        void set_linear_factor(const vec4 &factor);
        void integrate_velocities(const size_t &time);
        void set_center_of_mass(const transform &t, context* ctx);
        void apply_central_force(const vec4 &force);
        void apply_torque(const vec4 &torque);
        void apply_force(const vec4 &force, const vec4 &rel_pos);
        void apply_central_impulse(const vec4 &impulse);
        void apply_torque_impulse(const vec4 &torque);
        void apply_impulse(const vec4 &impulse, const vec4 &rel_pos);
        void clear_forces();
        void update_inertia_tensor(context* ctx);
        vec4 get_velocity_in_local_point(const vec4 &rel_pos) const;
        vec4 get_local_inertia() const;
        scalar get_mass() const;
        void translate(const vec4 &v, context* ctx);
        scalar compute_impulse_denominator(const vec4 &pos, const vec4 &normal, context* ctx) const;
        scalar compute_angular_impulse_denominator(const vec4 &axis) const;
        vec4 compute_gyroscopic_impulse_implicit_world(const transform &trans, const size_t &time) const;
        vec4 compute_gyroscopic_impulse_implicit_body(const transform &trans, const size_t &time) const;
        vec4 compute_gyroscopic_force_explicit(const transform &trans, const scalar &max_force) const;
        bool has_anisotropic_friction(const uint32_t &mode) const;
        void set_contact_damping_and_stiffness(const scalar &damping, const scalar &stiffness);
      };
      
      rigid_body::flags operator|(const rigid_body::flags &flags, const uint32_t &new_flags);
      rigid_body::flags operator&(const rigid_body::flags &flags, const uint32_t &new_flags);
    }
  }
}

#endif
