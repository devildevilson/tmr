#include "transform.h"
// #include "Utility.h"

namespace devils_engine {
  namespace physics {
    namespace core {
      transform::transform() : pos(0.0f, 0.0f, 0.0f, 1.0f), rot(1.0f, 0.0f, 0.0f, 0.0f), scale(1.0f, 1.0f, 1.0f, 1.0f) {}
      transform::transform(const transform &t) : pos(t.pos), rot(t.rot), scale(t.scale) { ASSERT(scale.w == 1.0f); }
      transform::transform(transform&& t) : pos(t.pos), rot(t.rot), scale(t.scale) { ASSERT(scale.w == 1.0f); }
      transform::transform(const vec4 &pos, const quat &rot, const vec4 &scale) : pos(pos), rot(rot), scale(scale) { ASSERT(scale.w == 1.0f); }
      transform & transform::operator=(const transform &t) { pos = t.pos; rot = t.rot; scale = t.scale; ASSERT(scale.w == 1.0f); return *this; }
      transform & transform::operator=(transform&& t) { pos = t.pos; rot = t.rot; scale = t.scale; ASSERT(scale.w == 1.0f); return *this; }
      mat4 transform::get_basis() const { return mat4(rot); }
      void transform::indentity() { pos = vec4(0.0f, 0.0f, 0.0f, 1.0f); rot = quat(1.0f, 0.0f, 0.0f, 0.0f); scale = vec4(1.0f, 1.0f, 1.0f, 1.0f); }
      transform & transform::operator*=(const transform &t) { pos += rot * t.pos; rot *= t.rot; scale *= t.scale; ASSERT(scale.w == 1.0f); return *this; }
      
      vec4 transform::transform_vector(const vec4 &vec) const {
        const vec4 trans = vec + pos*vec4(1,1,1,0);
        const vec4 rotated = rot * trans;
        const vec4 scaled = scale * rotated;
        
//         PRINT_VEC4("pos",pos);
//         PRINT_VEC4("translated",trans);
//         PRINT_VEC4("rotated   ",rotated);
//         PRINT_VEC4("scaled    ",scaled);
        
        return scaled;
//         return scale * (pos + (rot * (vec-pos)));
      }
      
      vec4 transform::inv_transform(const vec4 &vec) const {
//         const vec4 tmp = vec - pos;
//         const mat4 basis = mat4(rot);
//         const mat4 scale_m = simd::scale(mat4(), 1.0f / scale);
//         return scale_m * (simd::transpose(basis) * tmp);
        return ((1.0f / scale) * vec) * rot - pos*simd::vec4(1,1,1,0);
      }
      
      void transform::integrate(const transform &current, const simd::vec4 &linvel, const simd::vec4 &angvel, const scalar &time_step, transform &predicted) {
        predicted.pos = current.pos + linvel * time_step;
        
        vec4 axis;
        scalar fAngle2 = simd::length2(angvel);
        scalar fAngle = 0;
        if (fAngle2 > EPSILON) {
          fAngle = std::sqrt(fAngle2);
        }

        //limit the angular motion
        if (fAngle * time_step > ANGULAR_MOTION_THRESHOLD) {
          fAngle = ANGULAR_MOTION_THRESHOLD / time_step;
        }

        if (fAngle < scalar(0.001)) {
          // use Taylor's expansions of sync function
          axis = angvel * (scalar(0.5) * time_step - (time_step * time_step * time_step) * (scalar(0.020833333333)) * fAngle * fAngle);
        } else {
          // sync(fAngle) = sin(c*fAngle)/t
          axis = angvel * (std::sin(scalar(0.5) * fAngle * time_step) / fAngle);
        }
        scalar arr[4];
        axis.storeu(arr);
        // кватернион задается как w,x,y,z (!)
        quat dorn(std::cos(fAngle * time_step * scalar(0.5)), arr[0], arr[1], arr[2]);
        quat orn0 = current.rot;

        quat predictedOrn = dorn * orn0;
        predictedOrn = simd::normalize(predictedOrn);
        
        // по идее length2 всегда больше 
        if (simd::length2(predictedOrn) > EPSILON) predicted.rot = predictedOrn;
        else predicted.rot = current.rot;
      }
      
      void transform::calculate_diff_axis_angle(const transform& transform0, const transform& transform1, vec4& axis, scalar& angle) {
        quat dorn = transform1.rot * simd::inverse(transform0.rot);
        //dmat.getRotation(dorn);

        // floating point inaccuracy can lead to w component > 1..., which breaks
        dorn = simd::normalize(dorn);

        angle = simd::angle(dorn);
        float arr[4];
        dorn.storeu(arr);
        axis = vec4(arr[0], arr[1], arr[2], 0.0f);
        //check for axis length
        scalar len = simd::length2(axis);
        if (len < EPSILON * EPSILON) axis = vec4(1.0f, 0.0f, 0.0f, 0.0f);
        else axis /= std::sqrt(len);
      }
      
      void transform::calculate_velocity(const transform& transform0, const transform& transform1, const scalar &time_step, vec4& linvel, vec4& angvel) {
        linvel = (transform1.pos - transform0.pos) / time_step;
        vec4 axis;
        scalar angle;
        calculate_diff_axis_angle(transform0, transform1, axis, angle);
        angvel = axis * angle / time_step;
      }
      
      transform inverse(const transform &t) {
        //const auto rot = quat(simd::transpose(mat4(t.rot)));
        const auto rot = simd::inverse(t.rot);
        return transform(rot * -t.pos, rot, t.scale); // ???
      }
      
      transform operator*(const transform &t1, const transform &t2) { return transform(t1.pos + t1.rot * t2.pos, t1.rot * t2.rot, t1.scale * t2.scale); }
    }
  }
}
