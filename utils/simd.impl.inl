//#include "simd.h"

namespace simd {
  #define EPSILON 0.000001f

  float mix(const float &a, const float &b, const float &f) {
    return a * (1 - f) + b * f;
  }

  quat mix(const quat &x, const quat &y, const float &a) {
    const float cosTheta = dot(x, y);

    // если cosTheta близок к 1, то мы можем получить деление на нуль
    if (cosTheta >= 1.0f - EPSILON) {
      // линейная интерполяция
      return lerp(x, y, a);
    }

    const float angle = std::acos(cosTheta);
    return (std::sin((1.0f - a) * angle) * x + std::sin(a * angle) * y) / std::sin(angle);
  }

  quat slerp(const quat &x, const quat &y, const float &a) {
    quat z = y;
    float cosTheta = dot(x, y);

    // если cosTheta < 0, мы будем интерполировать через всю сферу
    if (cosTheta < 0.0f) {
      z = -y;
      cosTheta = -cosTheta;
    }

    // если cosTheta близок к 1, то мы можем получить деление на нуль
    if (cosTheta >= 1.0f - EPSILON) {
      // линейная интерполяция
      return lerp(x, y, a);
    }

    const float angle = std::acos(cosTheta);
    return (std::sin((1.0f - a) * angle) * x + std::sin(a * angle) * y) / std::sin(angle);
  }

  quat rotate(const quat &q, const float &angle, const vec4 &axis) {
    vec4 tmp = axis;

    const float len = length(tmp);
    if (std::abs(len - 1.0f) > EPSILON) {
      tmp *= (1.0f / len);
    }

    const float angleRad = angle * 0.5f;
    const float sin = std::sin(angleRad);
    tmp *= sin;

    const float cos = std::cos(angleRad);

    return q * quat(cos, tmp.x, tmp.y, tmp.z);
  }

  const quat rollConst = quat(1.0f, 1.0f, -1.0f, -1.0f);
  float roll(const quat &q) {
    __m128 tmp = simd_swizzle(q, 3, 2, 0, 0);
    tmp = q * quat(tmp);
    __m128 tmp2 = simd_swizzle(tmp, 1, 0, 0, 0);
    tmp = _mm_add_ps(tmp, tmp2);
    const float t1 = 2.0f * _mm_cvtss_f32(tmp);

    const float t2 = dot(q, q*rollConst);

    return std::atan2(t1, t2);
  }

  const quat pitchConst = quat(1.0f, -1.0f, -1.0f, 1.0f);
  float pitch(const quat &q) {
    __m128 tmp = simd_swizzle(q, 3, 2, 0, 0);
    tmp = q * quat(tmp);
    __m128 tmp2 = simd_swizzle(tmp, 1, 0, 0, 0);
    tmp = _mm_add_ps(tmp, tmp2);
    const float t1 = 2.0f * _mm_cvtss_f32(tmp);

    const float t2 = dot(q, q*pitchConst);

    return std::atan2(t1, t2);
  }

  inline float clamp(float n, float lower, float upper) {
    return std::max(lower, std::min(n, upper));
  }

  float yaw(const quat &q) {
    __m128 tmp = simd_swizzle(q, 3, 2, 0, 0);
    tmp = q * quat(tmp);
    __m128 tmp2 = simd_swizzle(tmp, 1, 0, 0, 0);
    tmp = _mm_sub_ps(tmp, tmp2);
    const float t1 = -2.0f * _mm_cvtss_f32(tmp);

    return std::asin(clamp(t1, -1.0f, 1.0f));
  }

  vec4 axis(const quat &q) {
    const float tmp1 = 1.0f - q.w * q.w;

    if (tmp1 <= 0.0f) {
      return vec4(0.0f, 0.0f, 1.0f, 0.0f);
    }

    const float tmp2 = 1.0f / std::sqrt(tmp1);
    const vec4 tmp3 = (q*tmp2).native;
    return vec4(tmp3.x, tmp3.y, tmp3.z, 0.0f);
  }

  quat angleAxis(const float &angle, const vec4 &axis) {
    const float angleRad = angle * 0.5f;
    const float sin = std::sin(angleRad);
    const vec4 tmp = axis * sin;

    const float cos = std::cos(angleRad);

    return quat(cos, tmp.x, tmp.y, tmp.z);
  }

  mat4 cast(const quat &q) {
    float arr[4];
    q.store(arr);

    const float qxx = arr[1] * arr[1]; // q.x * q.x;
    const float qyy = arr[2] * arr[2]; // q.y * q.y;
    const float qzz = arr[3] * arr[3]; // q.z * q.z;
    const float qxz = arr[1] * arr[3]; // q.x * q.z;
    const float qxy = arr[1] * arr[2]; // q.x * q.y;
    const float qyz = arr[2] * arr[3]; // q.y * q.z;
    const float qwx = arr[0] * arr[1]; // q.w * q.x;
    const float qwy = arr[0] * arr[2]; // q.w * q.y;
    const float qwz = arr[0] * arr[3]; // q.w * q.z;

    const float m00 = 1.0f - 2.0f * (qyy + qzz);
    const float m01 = 2.0f        * (qxy + qwz);
    const float m02 = 2.0f        * (qxz - qwy);

    const float m10 = 2.0f        * (qxy - qwz);
    const float m11 = 1.0f - 2.0f * (qxx + qzz);
    const float m12 = 2.0f        * (qyz + qwx);

    const float m20 = 2.0f        * (qxz + qwy);
    const float m21 = 2.0f        * (qyz - qwx);
    const float m22 = 1.0f - 2.0f * (qxx + qyy);

    return mat4(
      m00,   m01,  m02, 0.0f,
      m10,   m11,  m12, 0.0f,
      m20,   m21,  m22, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f
    );
  }

  quat cast(const mat4 &mat) {
    float arr[16];
    mat[0].store(&arr[0*4]);
    mat[1].store(&arr[1*4]);
    mat[2].store(&arr[2*4]);
    mat[3].store(&arr[3*4]);

    // взятие одного числа из __m128 компилируется в очень плохой код

//     const float fourXSquaredMinus1 = mat[0][0] - mat[1][1] - mat[2][2];
//     const float fourYSquaredMinus1 = mat[1][1] - mat[0][0] - mat[2][2];
//     const float fourZSquaredMinus1 = mat[2][2] - mat[0][0] - mat[1][1];
//     const float fourWSquaredMinus1 = mat[0][0] + mat[1][1] + mat[2][2];
    const float fourXSquaredMinus1 = arr[0*4+0] - arr[1*4+1] - arr[2*4+2];
    const float fourYSquaredMinus1 = arr[1*4+1] - arr[0*4+0] - arr[2*4+2];
    const float fourZSquaredMinus1 = arr[2*4+2] - arr[0*4+0] - arr[1*4+1];
    const float fourWSquaredMinus1 = arr[0*4+0] + arr[1*4+1] + arr[2*4+2];

    int biggestIndex = 0;
    float fourBiggestSquaredMinus1 = fourWSquaredMinus1;
    if (fourXSquaredMinus1 > fourBiggestSquaredMinus1) {
      fourBiggestSquaredMinus1 = fourXSquaredMinus1;
      biggestIndex = 1;
    }

    if (fourYSquaredMinus1 > fourBiggestSquaredMinus1) {
      fourBiggestSquaredMinus1 = fourYSquaredMinus1;
      biggestIndex = 2;
    }

    if (fourZSquaredMinus1 > fourBiggestSquaredMinus1) {
      fourBiggestSquaredMinus1 = fourZSquaredMinus1;
      biggestIndex = 3;
    }

    const float biggestVal = std::sqrt(fourBiggestSquaredMinus1 + 1.0f) * 0.5f;
    const float mult = 0.25f / biggestVal;

    float w = 0.0f, x = 0.0f, y = 0.0f, z = 0.0f;
//     switch(biggestIndex) {
//     case 0:
//       w = biggestVal;
//       x = (mat[1][2] - mat[2][1]) * mult;
//       y = (mat[2][0] - mat[0][2]) * mult;
//       z = (mat[0][1] - mat[1][0]) * mult;
//       break;
//     case 1:
//       w = (mat[1][2] - mat[2][1]) * mult;
//       x = biggestVal;
//       y = (mat[0][1] + mat[1][0]) * mult;
//       z = (mat[2][0] + mat[0][2]) * mult;
//       break;
//     case 2:
//       w = (mat[2][0] - mat[0][2]) * mult;
//       x = (mat[0][1] + mat[1][0]) * mult;
//       y = biggestVal;
//       z = (mat[1][2] + mat[2][1]) * mult;
//       break;
//     case 3:
//       w = (mat[0][1] - mat[1][0]) * mult;
//       x = (mat[2][0] + mat[0][2]) * mult;
//       y = (mat[1][2] + mat[2][1]) * mult;
//       z = biggestVal;
//       break;
//
//     default:					// Silence a -Wswitch-default warning in GCC. Should never actually get here. Assert is just for sanity.
//       assert(false);
//       break;
//     }
    switch(biggestIndex) {
    case 0:
      w = biggestVal;
      x = (arr[1*4+2] - arr[2*4+1]) * mult;
      y = (arr[2*4+0] - arr[0*4+2]) * mult;
      z = (arr[0*4+1] - arr[1*4+0]) * mult;
      break;
    case 1:
      w = (arr[1*4+2] - arr[2*4+1]) * mult;
      x = biggestVal;
      y = (arr[0*4+1] + arr[1*4+0]) * mult;
      z = (arr[2*4+0] + arr[0*4+2]) * mult;
      break;
    case 2:
      w = (arr[2*4+0] - arr[0*4+2]) * mult;
      x = (arr[0*4+1] + arr[1*4+0]) * mult;
      y = biggestVal;
      z = (arr[1*4+2] + arr[2*4+1]) * mult;
      break;
    case 3:
      w = (arr[0*4+1] - arr[1*4+0]) * mult;
      x = (arr[2*4+0] + arr[0*4+2]) * mult;
      y = (arr[1*4+2] + arr[2*4+1]) * mult;
      z = biggestVal;
      break;

    default:					// Silence a -Wswitch-default warning in GCC. Should never actually get here. Assert is just for sanity.
      assert(false);
      break;
    }

    return quat(w, x, y, z);
  }

  // чекать http://gruntthepeon.free.fr/ssemath/

  const ivec4 min_norm_pos = ivec4(0x00800000);
  const ivec4 mant_mask = ivec4(0x7f800000);
  const ivec4 inv_mant_mask = ivec4(~0x7f800000);
  const ivec4 sign_mask = ivec4((int)0x80000000);
  const ivec4 inv_sign_mask = ivec4(~0x80000000);
  const ivec4 const_1 = ivec4(1);
  const ivec4 const_inv1 = ivec4(~1);
  const ivec4 const_2 = ivec4(2);
  const ivec4 const_4 = ivec4(4);
  const ivec4 const_0x7f = ivec4(0x7f);

  const vec4 const_0p5f = vec4(0.5f);
  const vec4 const_1p0f = vec4(1.0f);
  const vec4 cephes_SQRTHF = vec4(0.707106781186547524);
  const vec4 cephes_log_p0 = vec4(7.0376836292E-2);
  const vec4 cephes_log_p1 = vec4(- 1.1514610310E-1);
  const vec4 cephes_log_p2 = vec4(1.1676998740E-1);
  const vec4 cephes_log_p3 = vec4(- 1.2420140846E-1);
  const vec4 cephes_log_p4 = vec4(+ 1.4249322787E-1);
  const vec4 cephes_log_p5 = vec4(- 1.6668057665E-1);
  const vec4 cephes_log_p6 = vec4(+ 2.0000714765E-1);
  const vec4 cephes_log_p7 = vec4(- 2.4999993993E-1);
  const vec4 cephes_log_p8 = vec4(+ 3.3333331174E-1);
  const vec4 cephes_log_q1 = vec4(-2.12194440e-4);
  const vec4 cephes_log_q2 = vec4(0.693359375);

  vec4 log(const vec4 &vec) {
    const vec4 one = const_1p0f;

    const vec4 invalid_mask = vec <= vec4(0.0f); //_mm_cmple_ps(x, _mm_setzero_ps());

    vec4 x = max(vec, cast(min_norm_pos)); // _mm_max_ps(x, *(v4sf*)_ps_min_norm_pos);  /* cut off denormalized stuff */

    ivec4 emm0 = cast(x) >> 23; //_mm_srli_epi32(_mm_castps_si128(x), 23);

    /* keep only the fractional part */
    x &= cast(inv_mant_mask); //_mm_and_ps(x, *(v4sf*)_ps_inv_mant_mask);
    x |= const_0p5f; //_mm_or_ps(x, *(v4sf*)_ps_0p5);

    emm0 -= const_0x7f;
    vec4 e = emm0; // _mm_cvtepi32_ps(emm0);

    e += one; // _mm_add_ps(e, one);

    /* part2:
      if( x < SQRTHF ) {
        e -= 1;
        x = x + x - 1.0;
      } else { x = x - 1.0; }
    */

    const vec4 mask = x < cephes_SQRTHF; //_mm_cmplt_ps(x, *(v4sf*)_ps_cephes_SQRTHF);
    vec4 tmp = x & mask; //_mm_and_ps(x, mask);
    x -= one; //_mm_sub_ps(x, one);
    e -= one & mask; // _mm_sub_ps(e, _mm_and_ps(one, mask));
    x += tmp; // _mm_add_ps(x, tmp);


    vec4 z = x*x; //_mm_mul_ps(x,x);

    vec4 y = cephes_log_p0; //*(v4sf*)_ps_cephes_log_p0;
    y *= x; //_mm_mul_ps(y, x);
    y += cephes_log_p1; //_mm_add_ps(y, *(v4sf*)_ps_cephes_log_p1);
    y *= x; //_mm_mul_ps(y, x);
    y += cephes_log_p2; // _mm_add_ps(y, *(v4sf*)_ps_cephes_log_p2);
    y *= x; //_mm_mul_ps(y, x);
    y += cephes_log_p3; //_mm_add_ps(y, *(v4sf*)_ps_cephes_log_p3);
    y *= x; //_mm_mul_ps(y, x);
    y += cephes_log_p4; //_mm_add_ps(y, *(v4sf*)_ps_cephes_log_p4);
    y *= x; //_mm_mul_ps(y, x);
    y += cephes_log_p5; //_mm_add_ps(y, *(v4sf*)_ps_cephes_log_p5);
    y *= x; //_mm_mul_ps(y, x);
    y += cephes_log_p6; // _mm_add_ps(y, *(v4sf*)_ps_cephes_log_p6);
    y *= x; //_mm_mul_ps(y, x);
    y += cephes_log_p7; // _mm_add_ps(y, *(v4sf*)_ps_cephes_log_p7);
    y *= x; //_mm_mul_ps(y, x);
    y += cephes_log_p8; //_mm_add_ps(y, *(v4sf*)_ps_cephes_log_p8);
    y *= x; //_mm_mul_ps(y, x);

    y += z; //_mm_mul_ps(y, z);


    tmp = e * cephes_log_q1; //_mm_mul_ps(e, *(v4sf*)_ps_cephes_log_q1);
    y += tmp; //_mm_add_ps(y, tmp);


    tmp = z * const_0p5f; // _mm_mul_ps(z, *(v4sf*)_ps_0p5);
    y -= tmp; // _mm_sub_ps(y, tmp);

    tmp = e * cephes_log_q2; //_mm_mul_ps(e, *(v4sf*)_ps_cephes_log_q2);
    x += y; //_mm_add_ps(x, y);
    x += tmp; //_mm_add_ps(x, tmp);
    x |= invalid_mask; //_mm_or_ps(x, invalid_mask); // negative arg will be NAN
    return x;
  }

  const vec4 exp_hi = vec4( 88.3762626647949f);
  const vec4 exp_lo = vec4(-88.3762626647949f);

  const vec4 cephes_LOG2EF = vec4( 1.44269504088896341);
  const vec4 cephes_exp_C1 = vec4( 0.693359375);
  const vec4 cephes_exp_C2 = vec4(-2.12194440e-4);

  const vec4 cephes_exp_p0 = vec4(1.9875691500E-4);
  const vec4 cephes_exp_p1 = vec4(1.3981999507E-3);
  const vec4 cephes_exp_p2 = vec4(8.3334519073E-3);
  const vec4 cephes_exp_p3 = vec4(4.1665795894E-2);
  const vec4 cephes_exp_p4 = vec4(1.6666665459E-1);
  const vec4 cephes_exp_p5 = vec4(5.0000001201E-1);

  vec4 exp(const vec4 &vec) {
    vec4 tmp, fx;
    const vec4 one = const_1p0f;

    vec4 x = max(min(vec, exp_hi), exp_lo);

    /* express exp(x) as exp(g + n*log(2)) */
    fx = x * cephes_LOG2EF + const_0p5f;

    /* how to perform a floorf with SSE: just below */
    ivec4 emm0 = truncate(fx); //_mm_cvttps_epi32(fx);
    tmp = emm0; //_mm_cvtepi32_ps(emm0);

    /* if greater, substract 1 */
    vec4 mask = tmp > fx; //_mm_cmpgt_ps(tmp, fx);
    mask += one; // _mm_and_ps(mask, one);
    fx = tmp - mask; //_mm_sub_ps(tmp, mask);

    tmp = fx * cephes_exp_C1; //_mm_mul_ps(fx, *(v4sf*)_ps_cephes_exp_C1);
    vec4 z = fx * cephes_exp_C2; // _mm_mul_ps(fx, *(v4sf*)_ps_cephes_exp_C2);
    x -= tmp; //_mm_sub_ps(x, tmp);
    x -= z; // _mm_sub_ps(x, z);

    z = x*x; //_mm_mul_ps(x,x);

    vec4 y = cephes_exp_p0; //*(v4sf*)_ps_cephes_exp_p0;
    y *= x; //_mm_mul_ps(y, x);
    y += cephes_exp_p1; //_mm_add_ps(y, *(v4sf*)_ps_cephes_exp_p1);
    y *= x; // _mm_mul_ps(y, x);
    y += cephes_exp_p2; // _mm_add_ps(y, *(v4sf*)_ps_cephes_exp_p2);
    y *= x; // _mm_mul_ps(y, x);
    y += cephes_exp_p3; // _mm_add_ps(y, *(v4sf*)_ps_cephes_exp_p3);
    y *= x; // _mm_mul_ps(y, x);
    y += cephes_exp_p4; // _mm_add_ps(y, *(v4sf*)_ps_cephes_exp_p4);
    y *= x; // _mm_mul_ps(y, x);
    y += cephes_exp_p5; // _mm_add_ps(y, *(v4sf*)_ps_cephes_exp_p5);
    y *= z; // _mm_mul_ps(y, z);
    y += x; // _mm_add_ps(y, x);
    y += one; // _mm_add_ps(y, one);

    /* build 2^n */

    emm0 = truncate(fx); // _mm_cvttps_epi32(fx);
    emm0 += const_0x7f; // _mm_add_epi32(emm0, *(v4si*)_pi32_0x7f);
    emm0 <<= 23; // _mm_slli_epi32(emm0, 23);
    vec4 pow2n = cast(emm0); // _mm_castsi128_ps(emm0);

    y *= pow2n; // _mm_mul_ps(y, pow2n);
    return y;
  }

  const vec4 minus_cephes_DP1 = vec4(-0.78515625);
  const vec4 minus_cephes_DP2 = vec4(-2.4187564849853515625e-4);
  const vec4 minus_cephes_DP3 = vec4(-3.77489497744594108e-8);
  const vec4 sincof_p0 = vec4(-1.9515295891E-4);
  const vec4 sincof_p1 = vec4( 8.3321608736E-3);
  const vec4 sincof_p2 = vec4(-1.6666654611E-1);
  const vec4 coscof_p0 = vec4( 2.443315711809948E-005);
  const vec4 coscof_p1 = vec4(-1.388731625493765E-003);
  const vec4 coscof_p2 = vec4( 4.166664568298827E-002);
  const vec4 cephes_FOPI = vec4(1.27323954473516); // 4 / M_PI

  vec4 sin(const vec4 &vec) {
    vec4 xmm1, xmm2, xmm3, sign_bit, y;

    ivec4 emm0, emm2;

    sign_bit = vec;
    /* take the absolute value */
    vec4 x = vec & cast(inv_sign_mask); // _mm_and_ps(x, *(v4sf*)_ps_inv_sign_mask);
    /* extract the sign bit (upper one) */
    sign_bit &= cast(sign_mask); // _mm_and_ps(sign_bit, *(v4sf*)_ps_sign_mask);

    /* scale by 4/Pi */
    y = x * cephes_FOPI; // _mm_mul_ps(x, *(v4sf*)_ps_cephes_FOPI);

    /* store the integer part of y in mm0 */
    emm2 = truncate(y); //_mm_cvttps_epi32(y);

    /* j=(j+1) & (~1) (see the cephes sources) */
    emm2 += const_1; //_mm_add_epi32(emm2, *(v4si*)_pi32_1);
    emm2 &= const_inv1; //_mm_and_si128(emm2, *(v4si*)_pi32_inv1);
    y = emm2; // _mm_cvtepi32_ps(emm2);

    /* get the swap sign flag */
    emm0 = emm2 & const_4; //_mm_and_si128(emm2, *(v4si*)_pi32_4);
    emm0 <<= 29; //_mm_slli_epi32(emm0, 29);

    /* get the polynom selection mask
      there is one polynom for 0 <= x <= Pi/4
      and another one for Pi/4<x<=Pi/2

      Both branches will be computed.
    */
    emm2 &= const_2; //_mm_and_si128(emm2, *(v4si*)_pi32_2);
    emm2 = emm2 == ivec4(0); //_mm_cmpeq_epi32(emm2, _mm_setzero_si128());

    vec4 swap_sign_bit = cast(emm0); //_mm_castsi128_ps(emm0);
    vec4 poly_mask = cast(emm2); //_mm_castsi128_ps(emm2);
    sign_bit ^= swap_sign_bit; //_mm_xor_ps(sign_bit, swap_sign_bit);

    /* The magic pass: "Extended precision modular arithmetic"
      x = ((x - y * DP1) - y * DP2) - y * DP3; */
    //x = ((x - y * minus_cephes_DP1) - y * minus_cephes_DP2) - y * minus_cephes_DP3;
    x = ((x + y * minus_cephes_DP1) + y * minus_cephes_DP2) + y * minus_cephes_DP3;

    // что за прикол? в коментарии указан минус, а в коде значения складываются?
    // ну лучше наверное сложить

    //xmm1 = minus_cephes_DP1; //*(v4sf*)_ps_minus_cephes_DP1;
    //xmm2 = minus_cephes_DP2; //*(v4sf*)_ps_minus_cephes_DP2;
    //xmm3 = minus_cephes_DP3; //*(v4sf*)_ps_minus_cephes_DP3;
    //xmm1 = _mm_mul_ps(y, xmm1);
    //xmm2 = _mm_mul_ps(y, xmm2);
    //xmm3 = _mm_mul_ps(y, xmm3);
    //x = _mm_add_ps(x, xmm1);
    //x = _mm_add_ps(x, xmm2);
    //x = _mm_add_ps(x, xmm3);

    /* Evaluate the first polynom  (0 <= x <= Pi/4) */
    y = coscof_p0; // *(v4sf*)_ps_coscof_p0;
    vec4 z = x*x; // _mm_mul_ps(x,x);

    y *= z; // _mm_mul_ps(y, z);
    y += coscof_p1; //_mm_add_ps(y, *(v4sf*)_ps_coscof_p1);
    y *= z; //_mm_mul_ps(y, z);
    y += coscof_p2; //_mm_add_ps(y, *(v4sf*)_ps_coscof_p2);
    y *= z; //_mm_mul_ps(y, z);
    y *= z; //_mm_mul_ps(y, z);
    vec4 tmp = z * const_0p5f; //_mm_mul_ps(z, *(v4sf*)_ps_0p5);
    y -= tmp; //_mm_sub_ps(y, tmp);
    // что тут надо сделать?
    //y += const_1; //_mm_add_ps(y, *(v4sf*)_ps_1);
    y += cast(const_1);

    /* Evaluate the second polynom  (Pi/4 <= x <= 0) */

    vec4 y2 = sincof_p0; // *(v4sf*)_ps_sincof_p0;
    y2 *= z; // _mm_mul_ps(y2, z);
    y2 += sincof_p1; // _mm_add_ps(y2, *(v4sf*)_ps_sincof_p1);
    y2 *= z; // _mm_mul_ps(y2, z);
    y2 += sincof_p2; // _mm_add_ps(y2, *(v4sf*)_ps_sincof_p2);
    y2 *= z; // _mm_mul_ps(y2, z);
    y2 *= x; // _mm_mul_ps(y2, x);
    y2 += x; // _mm_add_ps(y2, x);

    /* select the correct result from the two polynoms */
    xmm3 = poly_mask;
    y2 = xmm3 & y2; // _mm_and_ps(xmm3, y2); //, xmm3);
    y = andnot(xmm3, y); // _mm_andnot_ps(xmm3, y);
    y += y2; // _mm_add_ps(y,y2);

    /* update the sign */
    y ^= sign_bit; // _mm_xor_ps(y, sign_bit);
    return y;
  }

  vec4 cos(const vec4 &vec) {
    vec4 xmm1, /*xmm2 = vec4(),*/ xmm3, y;

    ivec4 emm0, emm2;

    /* take the absolute value */
    vec4 x = vec & cast(inv_sign_mask); // _mm_and_ps(x, *(v4sf*)_ps_inv_sign_mask);

    /* scale by 4/Pi */
    y = x * cephes_FOPI; // _mm_mul_ps(x, *(v4sf*)_ps_cephes_FOPI);

    /* store the integer part of y in mm0 */
    emm2 = truncate(y); // _mm_cvttps_epi32(y);

    /* j=(j+1) & (~1) (see the cephes sources) */
    emm2 += const_1; // _mm_add_epi32(emm2, *(v4si*)_pi32_1);
    emm2 &= const_inv1; //_mm_and_si128(emm2, *(v4si*)_pi32_inv1);
    y = emm2; // _mm_cvtepi32_ps(emm2);

    emm2 -= const_2; // _mm_sub_epi32(emm2, *(v4si*)_pi32_2);

    /* get the swap sign flag */
    emm0 = andnot(emm2, const_4); // _mm_andnot_si128(emm2, *(v4si*)_pi32_4);
    emm0 <<= 29; // _mm_slli_epi32(emm0, 29);

    /* get the polynom selection mask */
    emm2 &= const_2; // _mm_and_si128(emm2, *(v4si*)_pi32_2);
    emm2 = emm2 == ivec4(); //_mm_cmpeq_epi32(emm2, _mm_setzero_si128());

    vec4 sign_bit = cast(emm0); // _mm_castsi128_ps(emm0);
    vec4 poly_mask = cast(emm2); // _mm_castsi128_ps(emm2);

    /* The magic pass: "Extended precision modular arithmetic"
      x = ((x - y * DP1) - y * DP2) - y * DP3; */

    // потому что у нас -DP используется
    x = ((x + y * minus_cephes_DP1) + y * minus_cephes_DP2) + y * minus_cephes_DP3;

    //xmm1 = *(v4sf*)_ps_minus_cephes_DP1;
    //xmm2 = *(v4sf*)_ps_minus_cephes_DP2;
    //xmm3 = *(v4sf*)_ps_minus_cephes_DP3;
    //xmm1 = _mm_mul_ps(y, xmm1);
    //xmm2 = _mm_mul_ps(y, xmm2);
    //xmm3 = _mm_mul_ps(y, xmm3);
    //x = _mm_add_ps(x, xmm1);
    //x = _mm_add_ps(x, xmm2);
    //x = _mm_add_ps(x, xmm3);

    /* Evaluate the first polynom  (0 <= x <= Pi/4) */
    y = coscof_p0; // *(v4sf*)_ps_coscof_p0;
    vec4 z = x*x;  //_mm_mul_ps(x,x);

    y *= z; //_mm_mul_ps(y, z);
    y += coscof_p1; // _mm_add_ps(y, *(v4sf*)_ps_coscof_p1);
    y *= z; //_mm_mul_ps(y, z);
    y += coscof_p2; // _mm_add_ps(y, *(v4sf*)_ps_coscof_p2);
    y *= z; // _mm_mul_ps(y, z);
    y *= z; // _mm_mul_ps(y, z);
    vec4 tmp = z * const_0p5f; // _mm_mul_ps(z, *(v4sf*)_ps_0p5);
    y -= tmp; // _mm_sub_ps(y, tmp);
    y += const_1; //_mm_add_ps(y, *(v4sf*)_ps_1);
    y += cast(const_1);

    /* Evaluate the second polynom  (Pi/4 <= x <= 0) */

    vec4 y2 = sincof_p0; // *(v4sf*)_ps_sincof_p0;
    y2 *= z; // _mm_mul_ps(y2, z);
    y2 += sincof_p1; // _mm_add_ps(y2, *(v4sf*)_ps_sincof_p1);
    y2 *= z; // _mm_mul_ps(y2, z);
    y2 += sincof_p2; // _mm_add_ps(y2, *(v4sf*)_ps_sincof_p2);
    y2 *= z; // _mm_mul_ps(y2, z);
    y2 *= x; // _mm_mul_ps(y2, x);
    y2 += x; // _mm_add_ps(y2, x);

    /* select the correct result from the two polynoms */
    xmm3 = poly_mask;
    y2 = xmm3 & y2; // _mm_and_ps(xmm3, y2); //, xmm3);
    y = andnot(xmm3, y); // _mm_andnot_ps(xmm3, y);
    y += y2; // _mm_add_ps(y,y2);

    /* update the sign */
    y ^= sign_bit; // _mm_xor_ps(y, sign_bit);

    return y;
  }

  void sincos(const vec4 &input, vec4 &s, vec4 &c) {
    vec4 xmm1, xmm2, xmm3 = vec4(), sign_bit_sin, y;
    ivec4 emm0, emm2, emm4;

    sign_bit_sin = input;

    /* take the absolute value */
    vec4 x = input & cast(inv_sign_mask); // _mm_and_ps(x, *(v4sf*)_ps_inv_sign_mask);

    /* extract the sign bit (upper one) */
    sign_bit_sin &= cast(sign_mask); // _mm_and_ps(sign_bit_sin, *(v4sf*)_ps_sign_mask);

    /* scale by 4/Pi */
    y = x * cephes_FOPI; // _mm_mul_ps(x, *(v4sf*)_ps_cephes_FOPI);

    /* store the integer part of y in emm2 */
    //emm2 = truncate(y); // _mm_cvttps_epi32(y);

    /* j=(j+1) & (~1) (see the cephes sources) */
    emm2 = (truncate(y) + const_1) & const_inv1;

    //emm2 += const_1; // _mm_add_epi32(emm2, *(v4si*)_pi32_1);
    //emm2 &= const_inv1; // _mm_and_si128(emm2, *(v4si*)_pi32_inv1);
    y = vec4(emm2); // _mm_cvtepi32_ps(emm2);

    emm4 = emm2;

    /* get the swap sign flag for the sine */
    //emm0 = (emm2 & const_4) << 29;
    vec4 swap_sign_bit_sin = cast((emm2 & const_4) << 29);

    //emm0 = emm2 & const_4; // _mm_and_si128(emm2, *(v4si*)_pi32_4);
    //emm0 <<= 29; //_mm_slli_epi32(emm0, 29);
    //vec4 swap_sign_bit_sin = cast(emm0); // _mm_castsi128_ps(emm0);

    /* get the polynom selection mask for the sine*/
    vec4 poly_mask = cast((emm2 & const_2) == ivec4());

    //emm2 &= const_2; // _mm_and_si128(emm2, *(v4si*)_pi32_2);
    //emm2 = emm2 == ivec4(); // _mm_cmpeq_epi32(emm2, _mm_setzero_si128());
    //vec4 poly_mask = cast(emm2); // _mm_castsi128_ps(emm2);

    /* The magic pass: "Extended precision modular arithmetic"
      x = ((x - y * DP1) - y * DP2) - y * DP3; */
    x = ((x + y * minus_cephes_DP1) + y * minus_cephes_DP2) + y * minus_cephes_DP3;

    //xmm1 = *(v4sf*)_ps_minus_cephes_DP1;
    //xmm2 = *(v4sf*)_ps_minus_cephes_DP2;
    //xmm3 = *(v4sf*)_ps_minus_cephes_DP3;
    //xmm1 = _mm_mul_ps(y, xmm1);
    //xmm2 = _mm_mul_ps(y, xmm2);
    //xmm3 = _mm_mul_ps(y, xmm3);
    //x = _mm_add_ps(x, xmm1);
    //x = _mm_add_ps(x, xmm2);
    //x = _mm_add_ps(x, xmm3);

    vec4 sign_bit_cos = cast(andnot(emm4 - const_2, const_4) <<= 29);

    //emm4 -= const_2; // _mm_sub_epi32(emm4, *(v4si*)_pi32_2);
    //emm4 = andnot(emm4, const_4); // _mm_andnot_si128(emm4, *(v4si*)_pi32_4);
    //emm4 <<= 29; //_mm_slli_epi32(emm4, 29);
    //vec4 sign_bit_cos = cast(emm4); // _mm_castsi128_ps(emm4);

    sign_bit_sin ^= swap_sign_bit_sin; // _mm_xor_ps(sign_bit_sin, swap_sign_bit_sin);


    /* Evaluate the first polynom  (0 <= x <= Pi/4) */
    vec4 z = x*x; // _mm_mul_ps(x,x);
    vec4 tmp = z * const_0p5f;

    y = ((coscof_p0 * z + coscof_p1) * z + coscof_p2) * z * z - tmp + cast(const_1);

    //y = coscof_p0; // *(v4sf*)_ps_coscof_p0;

    //y *= z; // _mm_mul_ps(y, z);
    //y += coscof_p1; // _mm_add_ps(y, *(v4sf*)_ps_coscof_p1);
    //y *= z; // _mm_mul_ps(y, z);
    //y += coscof_p2; // _mm_add_ps(y, *(v4sf*)_ps_coscof_p2);
    //y *= z; // _mm_mul_ps(y, z);
    //y *= z; // _mm_mul_ps(y, z);
    //vec4 tmp = z * const_0p5f; // _mm_mul_ps(z, *(v4sf*)_ps_0p5);
    //y -= tmp; // _mm_sub_ps(y, tmp);
    //y += const_1; // _mm_add_ps(y, *(v4sf*)_ps_1);
    //y += cast(const_1);

    /* Evaluate the second polynom  (Pi/4 <= x <= 0) */

    vec4 y2 = ((sincof_p0 * z + sincof_p1) * z + sincof_p2) * z * x + x;

    //vec4 y2 = sincof_p0; // *(v4sf*)_ps_sincof_p0;
    //y2 *= z; //_mm_mul_ps(y2, z);
    //y2 += sincof_p1; // _mm_add_ps(y2, *(v4sf*)_ps_sincof_p1);
    //y2 *= z; // _mm_mul_ps(y2, z);
    //y2 += sincof_p2; // _mm_add_ps(y2, *(v4sf*)_ps_sincof_p2);
    //y2 *= z; // _mm_mul_ps(y2, z);
    //y2 *= x; // _mm_mul_ps(y2, x);
    //y2 += x; // _mm_add_ps(y2, x);

    /* select the correct result from the two polynoms */
    xmm3 = poly_mask;
    vec4 ysin2 = xmm3 & y2; // _mm_and_ps(xmm3, y2);
    vec4 ysin1 = andnot(xmm3, y); // _mm_andnot_ps(xmm3, y);
    y2 -= ysin2; // _mm_sub_ps(y2,ysin2);
    y -= ysin1; // _mm_sub_ps(y, ysin1);

    xmm1 = ysin1 + ysin2; // _mm_add_ps(ysin1,ysin2);
    xmm2 = y + y2; //_mm_add_ps(y,y2);

    /* update the sign */
    s = xmm1 ^ sign_bit_sin; // _mm_xor_ps(xmm1, sign_bit_sin);
    c = xmm2 ^ sign_bit_cos; // _mm_xor_ps(xmm2, sign_bit_cos);
  }

#define makeShuffleMask(x,y,z,w) (x | (y<<2) | (z<<4) | (w<<6))

  // только для трансформ матриц
  mat4 inverse_transform(const mat4 &mat) {
    #define SMALL_NUMBER (1.e-8f)

    mat4 r;

    // transpose 3x3, we know m03 = m13 = m23 = 0
    __m128 t0 = movelh(mat[0], mat[1]);     // 00, 01, 10, 11
    __m128 t1 = movehl(mat[1], mat[0]);     // 02, 03, 12, 13
    r[0] = _mm_shuffle_ps(t0, mat[2], makeShuffleMask(0,2,0,3)); // 00, 10, 20, 23(=0)
    r[1] = _mm_shuffle_ps(t0, mat[2], makeShuffleMask(1,3,1,3)); // 01, 11, 21, 23(=0)
    r[2] = _mm_shuffle_ps(t1, mat[2], makeShuffleMask(0,2,2,3)); // 02, 12, 22, 23(=0)

    // (SizeSqr(mVec[0]), SizeSqr(mVec[1]), SizeSqr(mVec[2]), 0)
    vec4 sizeSqr;
    sizeSqr =  r[0] * r[0];
    sizeSqr += r[1] * r[1];
    sizeSqr += r[2] * r[2];

    // optional test to avoid divide by 0
    const vec4 one(1.f);
    // for each component, if (sizeSqr < SMALL_NUMBER) sizeSqr = 1;
    vec4 rSizeSqr = blendv(one / sizeSqr, one, sizeSqr < vec4(SMALL_NUMBER));

    r[0] = r[0] * rSizeSqr;
    r[1] = r[1] * rSizeSqr;
    r[2] = r[2] * rSizeSqr;

    // last line
    r[3]  = r[0] * simd_swizzle1(mat[3], 0);
    r[3] += r[1] * simd_swizzle1(mat[3], 1);
    r[3] += r[2] * simd_swizzle1(mat[3], 2);
    r[3]  = vec4(0.f, 0.f, 0.f, 1.f) - r[3];

    return r;
  }

  // для всех
  mat4 inverse(const mat4 &mat) {
    __m128 minor0, minor1, minor2, minor3;
    __m128 row0, row1, row2, row3;
    __m128 det, tmp1;

    tmp1 = _mm_shuffle_ps(mat[0], mat[1], makeShuffleMask(0, 1, 0, 1)); //tmp1 = _mm_loadh_pi(_mm_loadl_pi(tmp1, (__m64*)(src)), (__m64*)(src+ 4));
    row1 = _mm_shuffle_ps(mat[2], mat[3], makeShuffleMask(0, 1, 0, 1)); //row1 = _mm_loadh_pi(_mm_loadl_pi(row1, (__m64*)(src+8)), (__m64*)(src+12));

    row0 = _mm_shuffle_ps(tmp1, row1, 0x88);
    row1 = _mm_shuffle_ps(row1, tmp1, 0xDD);

    tmp1 = _mm_shuffle_ps(mat[0], mat[1], makeShuffleMask(2, 3, 2, 3)); //tmp1 = _mm_loadh_pi(_mm_loadl_pi(tmp1, (__m64*)(src+ 2)), (__m64*)(src+ 6));
    row3 = _mm_shuffle_ps(mat[2], mat[3], makeShuffleMask(2, 3, 2, 3)); //row3 = _mm_loadh_pi(_mm_loadl_pi(row3, (__m64*)(src+10)), (__m64*)(src+14));

    row2 = _mm_shuffle_ps(tmp1, row3, 0x88);
    row3 = _mm_shuffle_ps(row3, tmp1, 0xDD);

    tmp1 = _mm_mul_ps(row2, row3);
    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);

    minor0 = _mm_mul_ps(row1, tmp1);
    minor1 = _mm_mul_ps(row0, tmp1);

    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);

    minor0 = _mm_sub_ps(_mm_mul_ps(row1, tmp1), minor0);
    minor1 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor1);
    minor1 = _mm_shuffle_ps(minor1, minor1, 0x4E);

    tmp1 = _mm_mul_ps(row1, row2);
    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);

    minor0 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor0);
    minor3 = _mm_mul_ps(row0, tmp1);

    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);

    minor0 = _mm_sub_ps(minor0, _mm_mul_ps(row3, tmp1));
    minor3 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor3);
    minor3 = _mm_shuffle_ps(minor3, minor3, 0x4E);

    tmp1 = _mm_mul_ps(_mm_shuffle_ps(row1, row1, 0x4E), row3);
    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
    row2 = _mm_shuffle_ps(row2, row2, 0x4E);

    minor0 = _mm_add_ps(_mm_mul_ps(row2, tmp1), minor0);
    minor2 = _mm_mul_ps(row0, tmp1);

    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);

    minor0 = _mm_sub_ps(minor0, _mm_mul_ps(row2, tmp1));
    minor2 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor2);
    minor2 = _mm_shuffle_ps(minor2, minor2, 0x4E);

    tmp1 = _mm_mul_ps(row0, row1);
    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);

    minor2 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor2);
    minor3 = _mm_sub_ps(_mm_mul_ps(row2, tmp1), minor3);

    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);

    minor2 = _mm_sub_ps(_mm_mul_ps(row3, tmp1), minor2);
    minor3 = _mm_sub_ps(minor3, _mm_mul_ps(row2, tmp1));

    tmp1 = _mm_mul_ps(row0, row3);
    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);

    minor1 = _mm_sub_ps(minor1, _mm_mul_ps(row2, tmp1));
    minor2 = _mm_add_ps(_mm_mul_ps(row1, tmp1), minor2);

    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);

    minor1 = _mm_add_ps(_mm_mul_ps(row2, tmp1), minor1);
    minor2 = _mm_sub_ps(minor2, _mm_mul_ps(row1, tmp1));

    tmp1 = _mm_mul_ps(row0, row2);
    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);

    minor1 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor1);
    minor3 = _mm_sub_ps(minor3, _mm_mul_ps(row1, tmp1));

    tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);

    minor1 = _mm_sub_ps(minor1, _mm_mul_ps(row3, tmp1));
    minor3 = _mm_add_ps(_mm_mul_ps(row1, tmp1), minor3);

    det = _mm_mul_ps(row0, minor0);
    det = _mm_add_ps(_mm_shuffle_ps(det, det, 0x4E), det);
    det = _mm_add_ss(_mm_shuffle_ps(det, det, 0xB1), det);

    tmp1 = _mm_rcp_ss(det);

    det = _mm_sub_ss(_mm_add_ss(tmp1, tmp1), _mm_mul_ss(det, _mm_mul_ss(tmp1, tmp1)));
    det = _mm_shuffle_ps(det, det, 0x00);

    minor0 = _mm_mul_ps(det, minor0);
    minor1 = _mm_mul_ps(det, minor1);
    minor2 = _mm_mul_ps(det, minor2);
    minor3 = _mm_mul_ps(det, minor3);

    return mat4(
      minor0,
      minor1,
      minor2,
      minor3
    );
  }

  mat4 rotate(const mat4 &mat, const float &angle, const vec4 &axis) {
    const float c = ::cos(angle);
    const float s = ::sin(angle);

    const vec4 a(normalize(axis));
    const vec4 temp(a * (1.0f - c));
    //const float temp = angle * (1.0f - c);

    float arr[4];
    axis.store(arr);
    mat4 rotate;
    rotate[0][0] = c + temp[0] * arr[0];
    rotate[0][1] = temp[0] * arr[1] + s * arr[2];
    rotate[0][2] = temp[0] * arr[2] - s * arr[1];

    rotate[1][0] = temp[1] * arr[0] - s * arr[2];
    rotate[1][1] = c + temp[1] * axis[1];
    rotate[1][2] = temp[1] * arr[2] + s * arr[0];

    rotate[2][0] = temp[2] * arr[0] + s * arr[1];
    rotate[2][1] = temp[2] * arr[1] - s * arr[0];
    rotate[2][2] = c + temp[2] * axis[2];

    mat4 result;
    result[0] = mat[0] * rotate[0][0] + mat[1] * rotate[0][1] + mat[2] * rotate[0][2];
    result[1] = mat[0] * rotate[1][0] + mat[1] * rotate[1][1] + mat[2] * rotate[1][2];
    result[2] = mat[0] * rotate[2][0] + mat[1] * rotate[2][1] + mat[2] * rotate[2][2];
    result[3] = mat[3];
    return result;
  }

  mat4 lookAt(const vec4 &eye, const vec4 &center, const vec4 &up) {
    const vec4 f(normalize(center - eye));
    const vec4 s(normalize(cross(f, up)));
    const vec4 u(cross(s, f));

    mat4 result(1.0f);
    result[0][0] = s.x;
    result[1][0] = s.y;
    result[2][0] = s.z;
    result[0][1] = u.x;
    result[1][1] = u.y;
    result[2][1] = u.z;
    result[0][2] =-f.x;
    result[1][2] =-f.y;
    result[2][2] =-f.z;
    result[3][0] =-dot(s, eye);
    result[3][1] =-dot(vec4(u.x, u.y, u.z, 0.0f), eye);
    result[3][2] = dot(f, eye);
    return result;
  }
}
