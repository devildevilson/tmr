#ifndef SOLVER_CONSTRAINT_H
#define SOLVER_CONSTRAINT_H

#include "physics_core.h"
#include "transform.h"
#include "Utility.h"
#include <mutex>

namespace devils_engine {
  namespace physics {
    namespace solver {
      using core::vec4;
      using core::scalar;
      
      struct constraint {
        vec4 relpos1_cross_normal;
        vec4 contact_normal1;

        vec4 relpos2_cross_normal;
        vec4 contact_normal2;  //usually m_contactNormal2 == -m_contactNormal1, but not always

        vec4 angular_component_a;
        vec4 angular_component_b;

        mutable scalar applied_push_impulse; // в булете это вектор
        mutable scalar applied_impulse;      // в булете это вектор

        scalar friction;
        scalar jac_diag_ab_inv;
        scalar rhs;
        scalar cfm;

        scalar lower_limit;
        scalar upper_limit;
        scalar rhs_penetration;
        
        uint32_t override_num_solver_iterations;
        uint32_t friction_index;
        uint32_t solver_body_a;
        uint32_t solver_body_b;
        
        union {
          void* original_contact_point;
          scalar unused;
          uint32_t num_rows_for_non_contact_constraint;
        };
        
        inline constraint() : 
          applied_push_impulse(0.0f),
          applied_impulse(0.0f),
          friction(0.0f),
          jac_diag_ab_inv(0.0f),
          rhs(0.0f),
          cfm(0.0f),
          lower_limit(1.0f),
          upper_limit(0.0f),
          rhs_penetration(0.0f),
          override_num_solver_iterations(0),
          friction_index(UINT32_MAX),
          solver_body_a(UINT32_MAX),
          solver_body_b(UINT32_MAX),
          original_contact_point(nullptr)
        {}
      };
      
      // есть вызов apply_impulse в параллельном коде
      // проще на мой взгляд здесь мьютекс воткнуть
      // все опрерации здесь (a+b)+c=a+(b+c) (забыл как это свойство называется)
      // одна опреация процессора атомарна? врядли
      struct body {
        core::transform transform;
        vec4 delta_linear_velocity;
        vec4 delta_angular_velocity;
        vec4 angular_factor;
        vec4 linear_factor;
        vec4 inv_mass;
        vec4 push_velocity;
        vec4 turn_velocity;
        vec4 linear_velocity;
        vec4 angular_velocity;
        vec4 external_force_impulse;
        vec4 external_torque_impulse;
        
        uint32_t body_index;
        std::mutex mutex;
        
        // не будет ли из-за мьютекса проседать параллелизм?
        // альтернатива - использовать батчи, которые еще нужно посчитать
        // + большое количество вычислений которое могло быть распределено
        // будет ограничено рамками батча
        // с другой стороны подход с батчами легко переписать под гпу
        // (хотя мьютекс можно смоделировать и в гпу)
        
        body() : body_index(UINT32_MAX) {}
        
        vec4 velocity_in_local_point(const vec4 &rel_pos) const {
          if (body_index == UINT32_MAX) return vec4(0.0f, 0.0f, 0.0f, 0.0f);
          return linear_factor + external_force_impulse + simd::cross(angular_velocity + external_torque_impulse, rel_pos);
        }
        
        vec4 get_angular_velocity() const {
          if (body_index == UINT32_MAX) return vec4(0.0f, 0.0f, 0.0f, 0.0f);
          return angular_velocity + delta_angular_velocity;
        }
        
        vec4 get_delta_linear_velocity() {
          if (body_index == UINT32_MAX) return vec4(0,0,0,0);
          std::unique_lock<std::mutex> lock(mutex);
          return delta_linear_velocity;
        }
        
        vec4 get_delta_angular_velocity() {
          if (body_index == UINT32_MAX) return vec4(0,0,0,0);
          std::unique_lock<std::mutex> lock(mutex);
          return delta_angular_velocity;
        }
        
        vec4 get_push_velocity() {
          if (body_index == UINT32_MAX) return vec4(0,0,0,0);
          std::unique_lock<std::mutex> lock(mutex);
          return push_velocity;
        }
        
        vec4 get_turn_velocity() {
          if (body_index == UINT32_MAX) return vec4(0,0,0,0);
          std::unique_lock<std::mutex> lock(mutex);
          return turn_velocity;
        }
        
        void apply_impulse(const vec4 &linear_component, const vec4 &angular_component, const scalar impulse_magnitude) {
          if (body_index == UINT32_MAX) return;
          const vec4 tmp_lin = linear_component * impulse_magnitude * linear_factor;
          const vec4 tmp_ang = angular_component * (impulse_magnitude * angular_factor);
          
          std::unique_lock<std::mutex> lock(mutex);
          delta_linear_velocity += tmp_lin;
          delta_angular_velocity += tmp_ang;
        }
        
        void apply_push_impulse(const vec4 &linear_component, const vec4 &angular_component, const scalar impulse_magnitude) {
          if (body_index == UINT32_MAX) return;
          const vec4 tmp_lin = linear_component * impulse_magnitude * linear_factor;
          const vec4 tmp_ang = angular_component * (impulse_magnitude * angular_factor);
          std::unique_lock<std::mutex> lock(mutex);
          push_velocity += tmp_lin;
          turn_velocity += tmp_ang;
//           PRINT_VAR ("impulse_magnitude",impulse_magnitude)
//           PRINT_VEC4("linear_component ",linear_component)
//           PRINT_VEC4("tmp_lin          ",tmp_lin)
//           PRINT_VEC4("push_velocity    ",push_velocity)
        }
        
        void writeback_velocity_and_transform(const scalar &delta_time, const scalar &split_impulse_turn_erp) {
          if (body_index == UINT32_MAX) return;
          
          linear_velocity += delta_linear_velocity;
          angular_velocity += delta_angular_velocity;

          //correct the position/orientation based on push/turn recovery
          if (simd::length2(push_velocity) > EPSILON || simd::length2(turn_velocity) > EPSILON) {
            core::transform new_trans;
            core::transform::integrate(transform, push_velocity, turn_velocity * split_impulse_turn_erp, delta_time, new_trans);
            transform = new_trans;
          }
        }
        
        void writeback_velocity() {
          if (body_index == UINT32_MAX) return;
          linear_velocity += delta_linear_velocity;
          angular_velocity += delta_angular_velocity;
        }
      };
    }
  }
}

#endif
