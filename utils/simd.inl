//#include "simd.h"

#include <cmath>
#include <algorithm>

#ifdef _DEBUG
  #include <cassert>
  #define ASSERT(expr) assert(expr)
#else
  #define ASSERT(expr)
#endif

namespace simd {

#ifdef __SSE__
  inline vec4::vec4() : native(_mm_setzero_ps()) {
    native = _mm_setzero_ps();
  }

  inline constexpr vec4::vec4(const vec4 &vec) : native(vec.native) {
//     this->native = vec.native;
  }

  inline vec4::vec4(const float &x, const float &y, const float &z, const float &w) : native(_mm_setr_ps(x, y, z, w)) {
//     native = _mm_setr_ps(x, y, z, w);
  }

  inline vec4::vec4(const float &f1) : native(_mm_set1_ps(f1)) {
//     native = _mm_set1_ps(f1);
  }

  inline vec4::vec4(const float* ptr) : native(_mm_load_ps(ptr)) {
//     native = _mm_load_ps(ptr);
  }

//#ifdef __SSE2__
//  inline vec4::vec4(const ivec4 &vec) {
//
//  }
//#endif

  inline constexpr vec4::vec4(__m128 native) : native(native) {
//     this->native = native;
  }

//   inline vec4::~vec4() {}

  constexpr uint32_t vec4::length() const {
    return 4;
  }

  inline int vec4::movemask() {
    return _mm_movemask_ps(native);
  }

  inline float vec4::hsum() const {
#ifndef __SSE3__
                                                                              // native = [ D C | B A ]
    __m128 shuf   = _mm_shuffle_ps(native, native, _MM_SHUFFLE(2, 3, 0, 1)); // [ C D | A B ]
    __m128 sums   = _mm_add_ps(native, shuf);                                // sums = [ D+C C+D | B+A A+B ]
    shuf          = _mm_movehl_ps(shuf, sums);                               // [   C   D | D+C C+D ] // let the compiler avoid a mov by reusing shuf
    sums          = _mm_add_ss(sums, shuf);
    return _mm_cvtss_f32(sums);
#else
    __m128 shuf = _mm_movehdup_ps(native);        // broadcast elements 3,1 to 2,0
    __m128 sums = _mm_add_ps(native, shuf);
    shuf        = _mm_movehl_ps(shuf, sums); // high half -> low half
    sums        = _mm_add_ss(sums, shuf);
    return        _mm_cvtss_f32(sums);
#endif
  }

  inline vec4 vec4::hsum_vec() const {
#ifndef __SSE3__
    // native = [ D C | B A ]
    __m128 shuf   = _mm_shuffle_ps(native, native, _MM_SHUFFLE(2, 3, 0, 1)); // [ C D | A B ]
    __m128 sums   = _mm_add_ps(native, shuf);                                // sums = [ D+C C+D | B+A A+B ]
    shuf          = _mm_movehl_ps(shuf, sums);
    return _mm_add_ss(sums, shuf);
#else
    __m128 shuf = _mm_movehdup_ps(native);        // broadcast elements 3,1 to 2,0
    __m128 sums = _mm_add_ps(native, shuf);
    shuf        = _mm_movehl_ps(shuf, sums); // high half -> low half
    return   vec4(_mm_cvtss_f32(_mm_add_ss(sums, shuf)));
#endif
  }

  inline vec4 vec4::aproximate_inverse() {
    return _mm_rcp_ps(native);
  }

  inline vec4 vec4::aproximate_inverse_sqrt() {
    return _mm_rsqrt_ps(native);
  }

  inline vec4 & vec4::abs() {
    native = _mm_max_ps(native, _mm_sub_ps(_mm_setzero_ps(), native));
    return *this;
  }

  inline void vec4::load(const float* p) {
    native = _mm_load_ps(p);
  }

  inline void vec4::store(float* p) const {
    _mm_store_ps(p, native);
  }

  inline vec4 & vec4::operator-() {
    native = _mm_sub_ps(_mm_setzero_ps(), native);
    return *this;
  }

  inline vec4 & vec4::operator-= (const vec4 &rhs) {
    this->native = _mm_sub_ps(this->native, rhs.native);
    return *this;
  }

  inline vec4 & vec4::operator+= (const vec4 &rhs) {
    this->native = _mm_add_ps(this->native, rhs.native);
    return *this;
  }

  inline vec4 & vec4::operator*= (const vec4 &rhs) {
    this->native = _mm_mul_ps(this->native, rhs.native);
    return *this;
  }

  inline vec4 & vec4::operator/= (const vec4 &rhs) {
    this->native = _mm_div_ps(this->native, rhs.native);
    return *this;
  }

  inline vec4 & vec4::operator-=(const float &value) {
    this->native = _mm_sub_ps(this->native, _mm_set1_ps(value));
    return *this;
  }

  inline vec4 & vec4::operator+=(const float &value) {
    this->native = _mm_add_ps(this->native, _mm_set1_ps(value));
    return *this;
  }

  inline vec4 & vec4::operator*=(const float &value) {
    this->native = _mm_mul_ps(this->native, _mm_set1_ps(value));
    return *this;
  }

  inline vec4 & vec4::operator/=(const float &value) {
    this->native = _mm_div_ps(this->native, _mm_set1_ps(value));
    return *this;
  }

  inline vec4 & vec4::operator--() {
    native = _mm_sub_ps(native, _mm_set1_ps(1.0f));
    return *this;
  }

  inline vec4 & vec4::operator++() {
    native = _mm_add_ps(native, _mm_set1_ps(1.0f));
    return *this;
  }

  inline vec4 vec4::operator--(int) {
    const vec4 tmp(*this);
    operator--();
    return tmp;
  }

  inline vec4 vec4::operator++(int) {
    const vec4 tmp(*this);
    operator++();
    return tmp;
  }

  inline vec4 & vec4::operator~() {
    native = _mm_xor_ps(native, _mm_set1_ps(0xffffffff));
    return *this;
  }

  inline vec4 & vec4::operator&=(const vec4 &vec) {
    native = _mm_and_ps(native, vec);
    return *this;
  }

  inline vec4 & vec4::operator|=(const vec4 &vec) {
    native = _mm_or_ps(native, vec);
    return *this;
  }

  inline vec4 & vec4::operator^=(const vec4 &vec) {
    native = _mm_xor_ps(native, vec);
    return *this;
  }

  inline vec4::operator __m128() const {
    return native;
  }

  inline vec4 & vec4::operator=(const vec4 &vec) {
    this->native = vec.native;
    return *this;
  }

//#ifdef __SSE2__
//  inline vec4 & vec4::operator=(const ivec4 &vec);
//#endif

  inline vec4 & vec4::operator=(__m128 x) {
    native = x;
    return *this;
  }

  inline float & vec4::operator[](const uint32_t &index) {
    return arr[index];
  }

  inline const float & vec4::operator[](const uint32_t &index) const {
    return arr[index];
  }

  inline vec4 operator-(const vec4 &vec1, const vec4 &vec2) {
    return _mm_sub_ps(vec1, vec2);
  }

  inline vec4 operator+(const vec4 &vec1, const vec4 &vec2) {
    return _mm_add_ps(vec1, vec2);
  }

  inline vec4 operator*(const vec4 &vec1, const vec4 &vec2) {
    return _mm_mul_ps(vec1, vec2);
  }

  inline vec4 operator/(const vec4 &vec1, const vec4 &vec2) {
    return _mm_div_ps(vec1, vec2);
  }

  inline vec4 operator-(const vec4 &vec1, const float &value) {
    return _mm_sub_ps(vec1, _mm_set1_ps(value));
  }

  inline vec4 operator+(const vec4 &vec1, const float &value) {
    return _mm_add_ps(vec1, _mm_set1_ps(value));
  }

  inline vec4 operator*(const vec4 &vec1, const float &value) {
    return _mm_mul_ps(vec1, _mm_set1_ps(value));
  }

  inline vec4 operator/(const vec4 &vec1, const float &value) {
    return _mm_div_ps(vec1, _mm_set1_ps(value));
  }

  inline vec4 operator-(const float &value, const vec4 &vec1) {
    return _mm_sub_ps(_mm_set1_ps(value), vec1);
  }

  inline vec4 operator+(const float &value, const vec4 &vec1) {
    return _mm_add_ps(_mm_set1_ps(value), vec1);
  }

  inline vec4 operator*(const float &value, const vec4 &vec1) {
    return _mm_mul_ps(_mm_set1_ps(value), vec1);
  }

  inline vec4 operator/(const float &value, const vec4 &vec1) {
    return _mm_div_ps(_mm_set1_ps(value), vec1);
  }

  inline vec4 operator>(const vec4 &vec1, const vec4 &vec2) {
    return _mm_cmpgt_ps(vec1, vec2);
  }

  inline vec4 operator>=(const vec4 &vec1, const vec4 &vec2) {
    return _mm_cmpge_ps(vec1, vec2);
  }

  inline vec4 operator<(const vec4 &vec1, const vec4 &vec2) {
    return _mm_cmplt_ps(vec1, vec2);
  }

  inline vec4 operator<=(const vec4 &vec1, const vec4 &vec2) {
    return _mm_cmple_ps(vec1, vec2);
  }

  inline vec4 operator==(const vec4 &vec1, const vec4 &vec2) {
    return _mm_cmpeq_ps(vec1, vec2);
  }

  inline vec4 operator!=(const vec4 &vec1, const vec4 &vec2) {
    return _mm_cmpneq_ps(vec1, vec2);
  }

  inline vec4 operator&(const vec4 &vec1, const vec4 &vec2) {
    return _mm_and_ps(vec1, vec2);
  }

  inline vec4 operator|(const vec4 &vec1, const vec4 &vec2) {
    return _mm_or_ps(vec1, vec2);
  }

  inline vec4 operator^(const vec4 &vec1, const vec4 &vec2) {
    return _mm_xor_ps(vec1, vec2);
  }

  inline vec4 andnot(const vec4 &vec1, const vec4 &vec2) {
    return _mm_andnot_ps(vec1, vec2);
  }

  inline vec4 max(const vec4 &vec1, const vec4 &vec2) {
    return _mm_max_ps(vec1, vec2);
  }

  inline vec4 min(const vec4 &vec1, const vec4 &vec2) {
    return _mm_min_ps(vec1, vec2);
  }

  inline vec4 abs(const vec4 &vec) {
    return _mm_max_ps(vec, _mm_sub_ps(_mm_setzero_ps(), vec));
  }

  inline vec4 sqrt(const vec4 &vec) {
    return _mm_sqrt_ps(vec);
  }

  const vec4 crossConst = vec4(1.0f, 1.0f, 1.0f, 0.0f);
  // 12 инструкций
  // у второго способа что-то не то со знаком у Y Z, и значит инструкций уже не 12
  inline vec4 cross(const vec4 &a, const vec4 &b) {
    return _mm_sub_ps(
      _mm_mul_ps(_mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 0, 2, 1)), _mm_shuffle_ps(b, b, _MM_SHUFFLE(3, 1, 0, 2))),
      _mm_mul_ps(_mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 1, 0, 2)), _mm_shuffle_ps(b, b, _MM_SHUFFLE(3, 0, 2, 1)))
    );

//     __m128 a_yzx = _mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 0, 2, 1));
//     __m128 b_yzx = _mm_shuffle_ps(b, b, _MM_SHUFFLE(3, 0, 2, 1));
//     __m128 result = _mm_sub_ps(_mm_mul_ps(b, a_yzx), _mm_mul_ps(a, b_yzx));
//     return _mm_shuffle_ps(result, result, _MM_SHUFFLE(3, 0, 2, 1));
  }

  inline float hsum(const vec4 &vec) {
#ifndef __SSE3__
                                                                         // native = [ D C | B A ]
      __m128 shuf   = _mm_shuffle_ps(vec, vec, _MM_SHUFFLE(2, 3, 0, 1)); // [ C D | A B ]
      __m128 sums   = _mm_add_ps(vec, shuf);                             // sums = [ D+C C+D | B+A A+B ]
      shuf          = _mm_movehl_ps(shuf, sums);                         // [   C   D | D+C C+D ] // let the compiler avoid a mov by reusing shuf
      sums          = _mm_add_ss(sums, shuf);
      return _mm_cvtss_f32(sums);
#else
      __m128 shuf = _mm_movehdup_ps(vec);        // broadcast elements 3,1 to 2,0
      __m128 sums = _mm_add_ps(vec, shuf);
      shuf        = _mm_movehl_ps(shuf, sums); // high half -> low half
      sums        = _mm_add_ss(sums, shuf);
      return        _mm_cvtss_f32(sums);
#endif
    }

    inline vec4 hsum_vec(const vec4 &vec) {
#ifndef __SSE3__
      // native = [ D C | B A ]
      __m128 shuf   = _mm_shuffle_ps(vec, vec, _MM_SHUFFLE(2, 3, 0, 1)); // [ C D | A B ]
      __m128 sums   = _mm_add_ps(vec, shuf);                                // sums = [ D+C C+D | B+A A+B ]
      shuf          = _mm_movehl_ps(shuf, sums);
      return _mm_add_ss(sums, shuf);
#else
      __m128 shuf = _mm_movehdup_ps(vec);        // broadcast elements 3,1 to 2,0
      __m128 sums = _mm_add_ps(vec, shuf);
      shuf        = _mm_movehl_ps(shuf, sums); // high half -> low half
      return   vec4(_mm_cvtss_f32(_mm_add_ss(sums, shuf)));
#endif
    }

  inline float dot(const vec4 &vec1, const vec4 &vec2) {
#ifdef __SSE4_1__
    return _mm_cvtss_f32(_mm_dp_ps(vec1, vec2, 0xff));
#else
    return hsum(_mm_mul_ps(vec1.native, vec2.native));
#endif
  }

  // 33 инструкции (ох уж этот _MM_TRANSPOSE4_PS)
  inline vec4 dot4(vec4 vec11, vec4 vec12, vec4 vec13, vec4 vec14,
                   vec4 vec21, vec4 vec22, vec4 vec23, vec4 vec24) {
    _MM_TRANSPOSE4_PS(vec11, vec12, vec13, vec14);
    _MM_TRANSPOSE4_PS(vec21, vec22, vec23, vec24);

    vec21 = _mm_mul_ps(vec11, vec21);
    vec22 = _mm_mul_ps(vec12, vec22);
    vec23 = _mm_mul_ps(vec13, vec23);
    vec24 = _mm_mul_ps(vec14, vec24);

    vec21 = _mm_add_ps(vec21, vec22);
    vec21 = _mm_add_ps(vec21, vec23);
    vec21 = _mm_add_ps(vec21, vec24);

    return vec21;
  }

  inline vec4 dot4(const vec4 &xxxx1, const vec4 &yyyy1, const vec4 &zzzz1,
                   const vec4 &xxxx2, const vec4 &yyyy2, const vec4 &zzzz2) {
    const vec4 tmp1 = xxxx1 * xxxx2;
    const vec4 tmp2 = yyyy1 * yyyy2;
    const vec4 tmp3 = zzzz1 * zzzz2;

    return tmp1 + tmp2 + tmp3;
  }

  // 8 инструкций
  inline vec4 dot4_no_transpose(const vec4 &mat11, const vec4 &mat12, const vec4 &mat13, const vec4 &mat14,
                                const vec4 &mat21, const vec4 &mat22, const vec4 &mat23, const vec4 &mat24) {
    __m128 vec[4];
    vec[0] = _mm_mul_ps(mat11, mat21);
    vec[1] = _mm_mul_ps(mat12, mat22);
    vec[2] = _mm_mul_ps(mat13, mat23);
    vec[3] = _mm_mul_ps(mat14, mat24);

    vec[0] = _mm_add_ps(vec[0], vec[1]);
    vec[0] = _mm_add_ps(vec[0], vec[2]);
    vec[0] = _mm_add_ps(vec[0], vec[3]);

    return vec[0];

//     mat21 = _mm_mul_ps(mat11, mat21);
//     mat22 = _mm_mul_ps(mat12, mat22);
//     mat23 = _mm_mul_ps(mat13, mat23);
//     mat24 = _mm_mul_ps(mat14, mat24);
//
//     mat21 = _mm_add_ps(mat21, mat22);
//     mat21 = _mm_add_ps(mat21, mat23);
//     mat21 = _mm_add_ps(mat21, mat24);
//
//     return mat21;
  }

  // 8 инструкций
  inline float length(const vec4 &vec) {
    //return _mm_cvtss_f32(_mm_sqrt_ss(hsum_vec(_mm_mul_ps(vec, vec))));
    return std::sqrt(hsum(_mm_mul_ps(vec, vec)));
  }

  // 9 инструкций
  inline vec4 length_vec(const vec4 &vec) {
    return _mm_sqrt_ps(hsum_vec(_mm_mul_ps(vec, vec)));
  }

  // 7 инструкций
  inline float length2(const vec4 &vec) {
    return hsum(_mm_mul_ps(vec, vec));
  }

  // 8 инструкций
  inline vec4 length2_vec(const vec4 &vec) {
    return hsum_vec(_mm_mul_ps(vec, vec));
  }

  // 9 инструкций
  inline vec4 length4(const vec4 &vec1, const vec4 &vec2, const vec4 &vec3, const vec4 &vec4) {
    __m128 vecOut[] = {vec1, vec2, vec3, vec4};
    _MM_TRANSPOSE4_PS(vecOut[0], vecOut[1], vecOut[2], vecOut[3]);

    vecOut[0] = _mm_mul_ps(vec1, vec1);
    vecOut[1] = _mm_mul_ps(vec2, vec2);
    vecOut[2] = _mm_mul_ps(vec3, vec3);
    vecOut[3] = _mm_mul_ps(vec4, vec4);

    vecOut[0] = _mm_add_ps(vecOut[0], vecOut[1]);
    vecOut[0] = _mm_add_ps(vecOut[0], vecOut[2]);
    vecOut[0] = _mm_add_ps(vecOut[0], vecOut[3]);

    return _mm_sqrt_ps(vecOut[0]);
  }

  inline vec4 length4_no_transpose(const vec4 &vec1, const vec4 &vec2, const vec4 &vec3, const vec4 &vec4) {
    __m128 vecOut[4];

    vecOut[0] = _mm_mul_ps(vec1, vec1);
    vecOut[1] = _mm_mul_ps(vec2, vec2);
    vecOut[2] = _mm_mul_ps(vec3, vec3);
    vecOut[3] = _mm_mul_ps(vec4, vec4);

    vecOut[0] = _mm_add_ps(vecOut[0], vecOut[1]);
    vecOut[0] = _mm_add_ps(vecOut[0], vecOut[2]);
    vecOut[0] = _mm_add_ps(vecOut[0], vecOut[3]);

    return _mm_sqrt_ps(vecOut[0]);
  }

  inline vec4 length4(const vec4 &xxxx, const vec4 &yyyy, const vec4 &zzzz) {
    return sqrt(dot4(xxxx, yyyy, zzzz, xxxx, yyyy, zzzz));
  }

  inline vec4 length24(const vec4 &vec1, const vec4 &vec2, const vec4 &vec3, const vec4 &vec4) {
    return dot4(vec1, vec2, vec3, vec4, vec1, vec2, vec3, vec4);
  }

  inline vec4 length24_no_transpose(const vec4 &vec1, const vec4 &vec2, const vec4 &vec3, const vec4 &vec4) {
    return dot4_no_transpose(vec1, vec2, vec3, vec4, vec1, vec2, vec3, vec4);
  }

  inline vec4 length24(const vec4 &xxxx, const vec4 &yyyy, const vec4 &zzzz) {
    return dot4(xxxx, yyyy, zzzz, xxxx, yyyy, zzzz);
  }

  inline vec4 normalize(const vec4 &vec) {
    return _mm_div_ps(vec, length_vec(vec));
  }

  inline void normalize4(vec4 &xxxx, vec4 &yyyy, vec4 &zzzz) {
    const vec4 l = length4(xxxx, yyyy, zzzz);
    xxxx /= l;
    yyyy /= l;
    zzzz /= l;
  }

  inline norm_output normalize4(const vec4 &xxxx, const vec4 &yyyy, const vec4 &zzzz) {
    const vec4 l = length4(xxxx, yyyy, zzzz);
    return {xxxx / l, yyyy / l, zzzz / l};
  }

  inline float distance(const vec4 &a, const vec4 &b) {
    return length(b - a);
  }

  inline vec4 distance4(const vec4 &xxxx1, const vec4 &yyyy1, const vec4 &zzzz1,
                        const vec4 &xxxx2, const vec4 &yyyy2, const vec4 &zzzz2) {
    return length4(xxxx2 - xxxx1, yyyy2 - yyyy1, zzzz2 - zzzz1);
  }

  inline float distance2(const vec4 &a, const vec4 &b) {
    return length2(b - a);
  }

  inline vec4 distance24(const vec4 &xxxx1, const vec4 &yyyy1, const vec4 &zzzz1,
                         const vec4 &xxxx2, const vec4 &yyyy2, const vec4 &zzzz2) {
    return length24(xxxx2 - xxxx1, yyyy2 - yyyy1, zzzz2 - zzzz1);
  }

  // normalize4 по всей видимости будет медленее чем 4 normalize подряд

  inline int movemask(const vec4 &vec) {
    return _mm_movemask_ps(vec);
  }

  // как назвать это дело?
  // 0xffffffff если оба не НаН
  inline vec4 notnan(const vec4 &a, const vec4 &b) {
    return _mm_cmpord_ps(a, b);
  }

  // 0xffffffff если хотя бы один НаН
  inline vec4 nan(const vec4 &a, const vec4 &b) {
    return _mm_cmpunord_ps(a, b);
  }

  inline vec4 aproximate_inverse(const vec4 &vec) {
    return _mm_rcp_ps(vec);
  }

  inline vec4 aproximate_inverse_sqrt(const vec4 &vec) {
    return _mm_rsqrt_ps(vec);
  }

  inline vec4 movehl(const vec4 &a, const vec4 &b) {
    return _mm_movehl_ps(a, b);
  }

  inline vec4 movelh(const vec4 &a, const vec4 &b) {
    return _mm_movelh_ps(a, b);
  }

//   #define shuffle(a, b, x, y, z, w) vec4(_mm_shuffle_ps(a, b, _MM_SHUFFLE(x, y, z, w)))
//   #define swizzle(a, x, y, z, w)    vec4(_mm_shuffle_ps(a, a, _MM_SHUFFLE(x, y, z, w)))
//   #define swizzle1(a, x)            vec4(_mm_shuffle_ps(a, a, _MM_SHUFFLE(x, x, x, x)))

  inline vec4 blendv(const vec4 &a, const vec4 &b, const vec4 &i) {
    return _mm_blendv_ps(a, b, i);
  }

  inline quat::quat() {
    native = _mm_setzero_ps();
  }

  inline quat::quat(const quat &q) : native(q.native) {
//     native = q.native;
  }

#ifdef __SSE3__
  inline quat::quat(const mat4 &mat) : native(cast(mat)) {
    //native = cast(mat);
  }
#endif

  inline quat::quat(const float &w, const float &x, const float &y, const float &z) {
    native = _mm_setr_ps(w, x, y, z);
  }

  inline quat::quat(const vec4 &a, const vec4 &b) {
    const float d = dot(a, b);
    const vec4 const2(0.0f, 0.0f, 0.0f, d + 1.0f);

    //const vec4 local = cross(a, b);

    const vec4 local = swizzle<2, 1, 0, 3>(cross(a, b) + const2);
    //quat q(d + 1.0f, local.x, local.y, local.z);
    *this = normalize(quat(local.native));
  }

  inline quat::quat(const float* ptr) {
    native = _mm_load_ps(ptr);
  }

  inline quat::quat(__m128 native) : native(native) {
//     this->native = native;
  }

//   inline quat::~quat() {}

//   inline constexpr uint32_t quat::length() const { return 4; }

  inline void quat::load(const float* p) {
    native = _mm_load_ps(p);
  }

  inline void quat::store(float* p) const {
    _mm_store_ps(p, native);
  }

//   static const vec4 conjConst = vec4(1.0f, -1.0f, -1.0f, -1.0f);

//   inline quat & quat::operator-() {
//     native = _mm_sub_ps(_mm_setzero_ps(), native);
//     return *this;
//   }

  inline quat & quat::operator-=(const quat &rhs) {
    native = _mm_sub_ps(native, rhs.native);
    return *this;
  }

  inline quat & quat::operator+=(const quat &rhs) {
    native = _mm_add_ps(native, rhs.native);
    return *this;
  }

  const vec4 quatMulConst = vec4(-1.0f, 1.0f, 1.0f, 1.0f);

  inline quat & quat::operator*=(const quat &rhs) {
//     w = p.w * q.w - p.x * q.x - p.y * q.y - p.z * q.z;
//     x = p.w * q.x + p.x * q.w + p.y * q.z - p.z * q.y;
//     y = p.w * q.y + p.y * q.w + p.z * q.x - p.x * q.z;
//     z = p.w * q.z + p.z * q.w + p.x * q.y - p.y * q.x;

    __m128 t1 = _mm_mul_ps(
      simd_swizzle1(native, 0), rhs
    );

    __m128 t2 = _mm_mul_ps(
      simd_swizzle(native, 1, 1, 2, 3), simd_swizzle(rhs, 1, 0, 0, 0)
    );

    __m128 t3 = _mm_mul_ps(
      simd_swizzle(native, 2, 2, 3, 1), simd_swizzle(rhs, 2, 3, 1, 2)
    );

    __m128 t4 = _mm_mul_ps(
      simd_swizzle(native, 3, 3, 1, 2), simd_swizzle(rhs, 3, 2, 3, 1)
    );

    t2 = _mm_mul_ps(t2, quatMulConst);
    t3 = _mm_mul_ps(t3, quatMulConst);

//     native = t1 + t2 + t3 - t4;
    native = _mm_sub_ps(_mm_add_ps(_mm_add_ps(t1, t2), t3), t4);

    return *this;
  }

  inline quat & quat::operator*=(const float &value) {
    native = _mm_mul_ps(native, _mm_set1_ps(value));
    return *this;
  }

  inline quat & quat::operator/=(const float &value) {
    native = _mm_div_ps(native, _mm_set1_ps(value));
    return *this;
  }

  inline quat::operator __m128() const {
    return native;
  }

#ifdef __SSE3__
  inline quat::operator mat4() const {
    return cast(*this);
  }
#endif

  inline quat & quat::operator=(const quat &q) {
    native = q.native;
    return *this;
  }
#ifdef __SSE3__
  inline quat & quat::operator=(const mat4 &mat) {
    native = cast(mat);
    return *this;
  }
#endif

  inline quat & quat::operator=(__m128 x) {
    native = x;
    return *this;
  }

  inline float & quat::operator[](const uint32_t &index) {
    return arr[index];
  }

  inline const float & quat::operator[](const uint32_t &index) const {
    return arr[index];
  }

  inline quat operator-(const quat &q) {
    return quat(_mm_sub_ps(_mm_setzero_ps(), q.native));
  }

  inline quat operator-(const quat &a, const quat &b) {
    return quat(_mm_sub_ps(a, b));
  }

  inline quat operator+(const quat &a, const quat &b) {
    return quat(_mm_add_ps(a, b));
  }

  inline quat operator*(const quat &a, const quat &b) {
//     w = p.w * q.w - p.x * q.x - p.y * q.y - p.z * q.z;
//     x = p.w * q.x + p.x * q.w + p.y * q.z - p.z * q.y;
//     y = p.w * q.y + p.y * q.w + p.z * q.x - p.x * q.z;
//     z = p.w * q.z + p.z * q.w + p.x * q.y - p.y * q.x;

//     __m128 t1 = _mm_mul_ps(
//       simd_swizzle1(a, 0), b
//     );

    // не правильный swizzle, теперь поправил

    __m128 t1 = _mm_mul_ps(
      swizzle1<0>(a), b
    );

    __m128 t2 = _mm_mul_ps(
      simd_swizzle(a, 3, 2, 1, 1), simd_swizzle(b, 0, 0, 0, 1)
    );

    __m128 t3 = _mm_mul_ps(
      simd_swizzle(a, 1, 3, 2, 2), simd_swizzle(b, 2, 1, 3, 2)
    );

    __m128 t4 = _mm_mul_ps(
      simd_swizzle(a, 2, 1, 3, 3), simd_swizzle(b, 1, 3, 2, 3)
    );

//     __m128 t1 = _mm_mul_ps(
//       simd_swizzle1(a, 3), b
//     );
//
//     __m128 t2 = _mm_mul_ps(
//       simd_swizzle(a, 2, 2, 1, 0), simd_swizzle(b, 2, 3, 3, 3)
//     );
//
//     __m128 t3 = _mm_mul_ps(
//       simd_swizzle(a, 1, 1, 0, 2), simd_swizzle(b, 1, 0, 2, 1)
//     );
//
//     __m128 t4 = _mm_mul_ps(
//       simd_swizzle(a, 0, 0, 2, 1), simd_swizzle(b, 0, 1, 0, 2)
//     );

    t2 = _mm_mul_ps(t2, quatMulConst);
    t3 = _mm_mul_ps(t3, quatMulConst);
    //t4 = _mm_sub_ps(_mm_setzero_ps(), t4);

//     native = t1 + t2 + t3 - t4;
    return quat(_mm_sub_ps(_mm_add_ps(_mm_add_ps(t1, t2), t3), t4));
  }

  inline quat operator*(const quat &a, const float &value) {
    return quat(_mm_mul_ps(a, _mm_set1_ps(value)));
  }

  inline quat operator/(const quat &a, const float &value) {
    return quat(_mm_div_ps(a, _mm_set1_ps(value)));
  }

  inline quat operator*(const float &value, const quat &a) {
    return quat(_mm_mul_ps(_mm_set1_ps(value), a));
  }

  inline quat operator/(const float &value, const quat &a) {
    return quat(_mm_div_ps(_mm_set1_ps(value), a));
  }

  inline vec4 operator*(const quat &q, const vec4 &v) {
    const vec4 quatVec = swizzle<0, 3, 2, 1>(vec4(q.native));
    const vec4 uv = cross(quatVec, v);
    const vec4 uuv = cross(quatVec, uv);
    const float w = _mm_cvtss_f32(q);
    const vec4 tmp = v + ((uv * w) + uuv) * 2.0f;

    return tmp;
  }

  inline vec4 operator*(const vec4 &v, const quat &q) {
    return inverse(q) * v;
  }

  const vec4 conjugateConst = vec4(1.0f, -1.0f, -1.0f, -1.0f);

  inline quat conjugate(const quat &q) {
    return quat(_mm_mul_ps(q, conjugateConst));
  }

  inline quat inverse(const quat &q) {
    return conjugate(q) / dot(q, q);
  }

  inline float dot(const quat &a, const quat &b) {
// #ifdef __SSE4_1__
//     return _mm_cvtss_f32(_mm_dp_ps(a, b, 0xff));
// #else
//     return hsum(_mm_mul_ps(a.native, b.native));
// #endif

    return dot(vec4(a.native), vec4(b.native));
  }

  inline float length(const quat &q) {
    //return _mm_cvtss_f32(_mm_sqrt_ps(vec4(dot(q, q))));
    return std::sqrt(dot(q, q));
  }

  inline quat normalize(const quat &q) {
    const float l = length(q);
    if (l <= 0.0f) return quat(1, 0, 0, 0);

    //return q / length(q);
    return q / l;
  }

//   #define EPSILON 0.000001f
//
//   float mix(const float &a, const float &b, const float &f) {
//     return a * (1 - f) + b * f;
//   }
//
//   quat mix(const quat &x, const quat &y, const float &a) {
//     const float cosTheta = dot(x, y);
//
//     // если cosTheta близок к 1, то мы можем получить деление на нуль
//     if (cosTheta >= 1.0f - EPSILON) {
//       // линейная интерполяция
//       return lerp(x, y, a);
//     }
//
//     const float angle = std::acos(cosTheta);
//     return (std::sin((1.0f - a) * angle) * x + std::sin(a * angle) * y) / std::sin(angle);
//   }

  inline quat lerp(const quat &x, const quat &y, const float &a) {
    assert(a >= 0.0f);
    assert(a <= 1.0f);

    return x * (1.0f - a) + (y * a);
  }

//   quat slerp(const quat &x, const quat &y, const float &a) {
//     quat z = y;
//     float cosTheta = dot(x, y);
//
//     // если cosTheta < 0, мы будем интерполировать через всю сферу
//     if (cosTheta < 0.0f) {
//       z = -y;
//       cosTheta = -cosTheta;
//     }
//
//     // если cosTheta близок к 1, то мы можем получить деление на нуль
//     if (cosTheta >= 1.0f - EPSILON) {
//       // линейная интерполяция
//       return lerp(x, y, a);
//     }
//
//     const float angle = std::acos(cosTheta);
//     return (std::sin((1.0f - a) * angle) * x + std::sin(a * angle) * y) / std::sin(angle);
//   }
//
//   quat rotate(const quat &q, const float &angle, const vec4 &axis) {
//     vec4 tmp = axis;
//
//     const float len = length(tmp);
//     if (std::abs(len - 1.0f) > EPSILON) {
//       tmp *= (1.0f / len);
//     }
//
//     const float angleRad = angle * 0.5f;
//     const float sin = std::sin(angleRad);
//     tmp *= sin;
//
//     const float cos = std::cos(angleRad);
//
//     return q * quat(cos, tmp.x, tmp.y, tmp.z);
//   }

  inline vec4 eulerAngles(const quat &q) {
    return vec4(pitch(q), yaw(q), roll(q), 0.0f);
  }

//   const quat rollConst = quat(1.0f, 1.0f, -1.0f, -1.0f);
//   float roll(const quat &q) {
//     __m128 tmp = simd_swizzle(q, 3, 2, 0, 0);
//     tmp = q * quat(tmp);
//     __m128 tmp2 = simd_swizzle(tmp, 1, 0, 0, 0);
//     tmp = _mm_add_ps(tmp, tmp2);
//     const float t1 = 2.0f * _mm_cvtss_f32(tmp);
//
//     const float t2 = dot(q, q*rollConst);
//
//     return std::atan2(t1, t2);
//   }
//
//   const quat pitchConst = quat(1.0f, -1.0f, -1.0f, 1.0f);
//   float pitch(const quat &q) {
//     __m128 tmp = simd_swizzle(q, 3, 2, 0, 0);
//     tmp = q * quat(tmp);
//     __m128 tmp2 = simd_swizzle(tmp, 1, 0, 0, 0);
//     tmp = _mm_add_ps(tmp, tmp2);
//     const float t1 = 2.0f * _mm_cvtss_f32(tmp);
//
//     const float t2 = dot(q, q*pitchConst);
//
//     return std::atan2(t1, t2);
//   }

//   inline float clamp(float n, float lower, float upper) {
//     return std::max(lower, std::min(n, upper));
//   }

//   float yaw(const quat &q) {
//     __m128 tmp = simd_swizzle(q, 3, 2, 0, 0);
//     tmp = q * quat(tmp);
//     __m128 tmp2 = simd_swizzle(tmp, 1, 0, 0, 0);
//     tmp = _mm_sub_ps(tmp, tmp2);
//     const float t1 = -2.0f * _mm_cvtss_f32(tmp);
//
//     return std::asin(clamp(t1, -1.0f, 1.0f));
//   }

  inline float angle(const quat &q) {
    return std::acos(q.w) * 2.0f;
  }

//   vec4 axis(const quat &q) {
//     const float tmp1 = 1.0f - q.w * q.w;
//
//     if (tmp1 <= 0.0f) {
//       return vec4(0.0f, 0.0f, 1.0f, 0.0f);
//     }
//
//     const float tmp2 = 1.0f / std::sqrt(tmp1);
//     const vec4 tmp3 = (q*tmp2).native;
//     return vec4(tmp3.x, tmp3.y, tmp3.z, 0.0f);
//   }
//
//   quat angleAxis(const float &angle, const vec4 &axis) {
//     const float angleRad = angle * 0.5f;
//     const float sin = std::sin(angleRad);
//     const vec4 tmp = axis * sin;
//
//     const float cos = std::cos(angleRad);
//
//     return quat(cos, tmp.x, tmp.y, tmp.z);
//   }
//
//   mat4 cast(const quat &q) {
//     float arr[4];
//     q.store(arr);
//
//     const float qxx = arr[1] * arr[1]; // q.x * q.x;
//     const float qyy = arr[2] * arr[2]; // q.y * q.y;
//     const float qzz = arr[3] * arr[3]; // q.z * q.z;
//     const float qxz = arr[1] * arr[3]; // q.x * q.z;
//     const float qxy = arr[1] * arr[2]; // q.x * q.y;
//     const float qyz = arr[2] * arr[3]; // q.y * q.z;
//     const float qwx = arr[0] * arr[1]; // q.w * q.x;
//     const float qwy = arr[0] * arr[2]; // q.w * q.y;
//     const float qwz = arr[0] * arr[3]; // q.w * q.z;
//
//     const float m00 = 1.0f - 2.0f * (qyy + qzz);
//     const float m01 = 2.0f        * (qxy + qwz);
//     const float m02 = 2.0f        * (qxz - qwy);
//
//     const float m10 = 2.0f        * (qxy - qwz);
//     const float m11 = 1.0f - 2.0f * (qxx + qzz);
//     const float m12 = 2.0f        * (qyz + qwx);
//
//     const float m20 = 2.0f        * (qxz + qwy);
//     const float m21 = 2.0f        * (qyz - qwx);
//     const float m22 = 1.0f - 2.0f * (qxx + qyy);
//
//     return mat4(
//       m00,   m01,  m02, 0.0f,
//       m10,   m11,  m12, 0.0f,
//       m20,   m21,  m22, 0.0f,
//       0.0f, 0.0f, 0.0f, 1.0f
//     );
//   }
//
//   quat cast(const mat4 &mat) {
//     float arr[16];
//     mat[0].store(&arr[0*4]);
//     mat[1].store(&arr[1*4]);
//     mat[2].store(&arr[2*4]);
//     mat[3].store(&arr[3*4]);
//
//     // взятие одного числа из __m128 компилируется в очень плохой код
//
// //     const float fourXSquaredMinus1 = mat[0][0] - mat[1][1] - mat[2][2];
// //     const float fourYSquaredMinus1 = mat[1][1] - mat[0][0] - mat[2][2];
// //     const float fourZSquaredMinus1 = mat[2][2] - mat[0][0] - mat[1][1];
// //     const float fourWSquaredMinus1 = mat[0][0] + mat[1][1] + mat[2][2];
//     const float fourXSquaredMinus1 = arr[0*4+0] - arr[1*4+1] - arr[2*4+2];
//     const float fourYSquaredMinus1 = arr[1*4+1] - arr[0*4+0] - arr[2*4+2];
//     const float fourZSquaredMinus1 = arr[2*4+2] - arr[0*4+0] - arr[1*4+1];
//     const float fourWSquaredMinus1 = arr[0*4+0] + arr[1*4+1] + arr[2*4+2];
//
//     int biggestIndex = 0;
//     float fourBiggestSquaredMinus1 = fourWSquaredMinus1;
//     if (fourXSquaredMinus1 > fourBiggestSquaredMinus1) {
//       fourBiggestSquaredMinus1 = fourXSquaredMinus1;
//       biggestIndex = 1;
//     }
//
//     if (fourYSquaredMinus1 > fourBiggestSquaredMinus1) {
//       fourBiggestSquaredMinus1 = fourYSquaredMinus1;
//       biggestIndex = 2;
//     }
//
//     if (fourZSquaredMinus1 > fourBiggestSquaredMinus1) {
//       fourBiggestSquaredMinus1 = fourZSquaredMinus1;
//       biggestIndex = 3;
//     }
//
//     const float biggestVal = std::sqrt(fourBiggestSquaredMinus1 + 1.0f) * 0.5f;
//     const float mult = 0.25f / biggestVal;
//
//     float w = 0.0f, x = 0.0f, y = 0.0f, z = 0.0f;
// //     switch(biggestIndex) {
// //     case 0:
// //       w = biggestVal;
// //       x = (mat[1][2] - mat[2][1]) * mult;
// //       y = (mat[2][0] - mat[0][2]) * mult;
// //       z = (mat[0][1] - mat[1][0]) * mult;
// //       break;
// //     case 1:
// //       w = (mat[1][2] - mat[2][1]) * mult;
// //       x = biggestVal;
// //       y = (mat[0][1] + mat[1][0]) * mult;
// //       z = (mat[2][0] + mat[0][2]) * mult;
// //       break;
// //     case 2:
// //       w = (mat[2][0] - mat[0][2]) * mult;
// //       x = (mat[0][1] + mat[1][0]) * mult;
// //       y = biggestVal;
// //       z = (mat[1][2] + mat[2][1]) * mult;
// //       break;
// //     case 3:
// //       w = (mat[0][1] - mat[1][0]) * mult;
// //       x = (mat[2][0] + mat[0][2]) * mult;
// //       y = (mat[1][2] + mat[2][1]) * mult;
// //       z = biggestVal;
// //       break;
// //
// //     default:					// Silence a -Wswitch-default warning in GCC. Should never actually get here. Assert is just for sanity.
// //       assert(false);
// //       break;
// //     }
//     switch(biggestIndex) {
//     case 0:
//       w = biggestVal;
//       x = (arr[1*4+2] - arr[2*4+1]) * mult;
//       y = (arr[2*4+0] - arr[0*4+2]) * mult;
//       z = (arr[0*4+1] - arr[1*4+0]) * mult;
//       break;
//     case 1:
//       w = (arr[1*4+2] - arr[2*4+1]) * mult;
//       x = biggestVal;
//       y = (arr[0*4+1] + arr[1*4+0]) * mult;
//       z = (arr[2*4+0] + arr[0*4+2]) * mult;
//       break;
//     case 2:
//       w = (arr[2*4+0] - arr[0*4+2]) * mult;
//       x = (arr[0*4+1] + arr[1*4+0]) * mult;
//       y = biggestVal;
//       z = (arr[1*4+2] + arr[2*4+1]) * mult;
//       break;
//     case 3:
//       w = (arr[0*4+1] - arr[1*4+0]) * mult;
//       x = (arr[2*4+0] + arr[0*4+2]) * mult;
//       y = (arr[1*4+2] + arr[2*4+1]) * mult;
//       z = biggestVal;
//       break;
//
//     default:					// Silence a -Wswitch-default warning in GCC. Should never actually get here. Assert is just for sanity.
//       assert(false);
//       break;
//     }
//
//     return quat(w, x, y, z);
//   }

  // судя по предыдущим записям здесь сгерерируется 12 инструкций
  inline bool collision_aabb(const vec4 &pos1, const vec4 &extents1, const vec4 &pos2, const vec4 &extents2) {
    return ((abs(pos1 - pos2) < extents1 + extents2).movemask() & 0x111) == 0x111;
  }
#endif

#ifdef __SSE2__
  inline ivec4::ivec4() {
    native = _mm_setzero_si128();
  }

  inline ivec4::ivec4(const int32_t &x, const int32_t &y, const int32_t &z, const int32_t &w) {
    native = _mm_setr_epi32(x, y, z, w);
  }

  inline ivec4::ivec4(const int32_t &x) {
    native = _mm_set1_epi32(x);
  }

  inline ivec4::ivec4(const ivec4 &vec) {
    native = vec.native;
  }

  inline ivec4::ivec4(const vec4 &vec) {
    native = _mm_cvtps_epi32(vec);
  }

  inline ivec4::ivec4(__m128i native) {
    this->native = native;
  }

  inline ivec4::~ivec4() {}

//   inline constexpr uint32_t ivec4::length() const {
//     return 4;
//   }

#ifdef __SSSE3__
  inline ivec4 & ivec4::hadd(const ivec4 &vec) {
    native = _mm_hadd_epi32(native, vec);
    return *this;
  }

  inline ivec4 & ivec4::hsub(const ivec4 &vec) {
    native = _mm_hsub_epi32(native, vec);
    return *this;
  }

  inline ivec4 & ivec4::abs() {
    native = _mm_abs_epi32(native);
    return *this;
  }
#endif

  inline ivec4 & ivec4::operator-() {
    native = _mm_sub_epi32(_mm_setzero_si128(), native);
    return *this;
  }

  inline ivec4 & ivec4::operator-=(const ivec4 &rhs) {
    native = _mm_sub_epi32(native, rhs.native);
    return *this;
  }

  inline ivec4 & ivec4::operator+=(const ivec4 &rhs) {
    native = _mm_add_epi32(native, rhs.native);
    return *this;
  }

  inline ivec4 & ivec4::operator*=(const ivec4 &rhs) {
#ifdef __SSE4_1__
    native = _mm_mullo_epi32(native, rhs);
    return *this;
#else
    __m128i tmp1 = _mm_mul_epu32(native, rhs); /* mul 2,0*/
    __m128i tmp2 = _mm_mul_epu32( _mm_srli_si128(native, 4), _mm_srli_si128(rhs, 4)); /* mul 3,1 */
    return _mm_unpacklo_epi32(_mm_shuffle_epi32(tmp1, _MM_SHUFFLE (0,0,2,0)), _mm_shuffle_epi32(tmp2, _MM_SHUFFLE (0,0,2,0))); /* shuffle results to [63..0] and pack */
#endif
  }

  // деления целых чисел нет :(
//     inline ivec4 & operator/= (const ivec4 &rhs) {
//       this->native = _mm_div_ps(this->native, rhs.native);
//       return *this;
//     }

  inline ivec4 & ivec4::operator-=(const int32_t &value) {
    this->native = _mm_sub_epi32(native, _mm_set1_epi32(value));
    return *this;
  }

  inline ivec4 & ivec4::operator+=(const int32_t &value) {
    this->native = _mm_add_epi32(native, _mm_set1_epi32(value));
    return *this;
  }

  inline ivec4 & ivec4::operator*=(const int32_t &value) {
#ifdef __SSE4_1__
    native = _mm_mul_epi32(native, _mm_set1_epi32(value));
    return *this;
#else
    __m128i val = _mm_set1_epi32(value);
    __m128i tmp1 = _mm_mul_epu32(native, val); /* mul 2,0*/
    __m128i tmp2 = _mm_mul_epu32( _mm_srli_si128(native, 4), _mm_srli_si128(val, 4)); /* mul 3,1 */
    native = _mm_unpacklo_epi32(_mm_shuffle_epi32(tmp1, _MM_SHUFFLE (0,0,2,0)), _mm_shuffle_epi32(tmp2, _MM_SHUFFLE (0,0,2,0))); /* shuffle results to [63..0] and pack */
    return *this;
#endif
  }

  // деления целых чисел нет :(
//     inline ivec4 & operator/= (const int32_t &value) {
//       this->native = _mm_div_ps(native, _mm_set1_epi32(value));
//       return *this;
//     }

  inline ivec4 & ivec4::operator--() {
    native = _mm_sub_epi32(native, _mm_set1_epi32(1));
    return *this;
  }

  inline ivec4 & ivec4::operator++() {
    native = _mm_add_epi32(native, _mm_set1_epi32(1));
    return *this;
  }

  inline ivec4 ivec4::operator--(int) {
    const ivec4 tmp(*this);
    operator--();
    return tmp;
  }

  inline ivec4 ivec4::operator++(int) {
    const ivec4 tmp(*this);
    operator++();
    return tmp;
  }

  inline ivec4 & ivec4::operator~() {
    native = _mm_xor_si128(native, _mm_set1_epi32(0xffffffff));
    return *this;
  }

  inline ivec4 & ivec4::operator|=(const ivec4 &vec) {
    native = _mm_or_si128(native, vec);
    return *this;
  }

  inline ivec4 & ivec4::operator^=(const ivec4 &vec) {
    native = _mm_xor_si128(native, vec);
    return *this;
  }

  inline ivec4 & ivec4::operator&=(const ivec4 &vec) {
    native = _mm_and_si128(native, vec);
    return *this;
  }

  inline ivec4 & ivec4::operator<<=(const int32_t &shift) {
    native = _mm_slli_epi32(native, shift);
    return *this;
  }

  inline ivec4 & ivec4::operator>>=(const int32_t &shift) {
    native = _mm_srli_epi32(native, shift);
    return *this;
  }

  inline ivec4::operator __m128i() const {
    return native;
  }

  inline ivec4 & ivec4::operator=(const ivec4 &vec) {
    this->native = vec.native;
    return *this;
  }

  inline ivec4 & ivec4::operator=(const vec4 &vec) {
    native = _mm_cvtps_epi32(vec.native);
    return *this;
  }

  inline ivec4 & ivec4::operator=(__m128i x) {
    native = x;
    return *this;
  }

  inline int32_t & ivec4::operator[](const uint32_t &index) {
    return arr[index];
  }

  inline const int32_t & ivec4::operator[](const uint32_t &index) const {
    return arr[index];
  }

  inline ivec4 operator-(const ivec4 &vec1, const ivec4 &vec2) {
    return _mm_sub_epi32(vec1, vec2);
  }

  inline ivec4 operator+(const ivec4 &vec1, const ivec4 &vec2) {
    return _mm_add_epi32(vec1, vec2);
  }

  inline ivec4 operator*(const ivec4 &vec1, const ivec4 &vec2) {
#ifdef __SSE4_1__
    return _mm_mullo_epi32(vec1, vec2);
#else
    __m128i tmp1 = _mm_mul_epu32(vec1, vec2); /* mul 2,0*/
    __m128i tmp2 = _mm_mul_epu32( _mm_srli_si128(vec1, 4), _mm_srli_si128(vec2, 4)); /* mul 3,1 */
    return _mm_unpacklo_epi32(_mm_shuffle_epi32(tmp1, _MM_SHUFFLE (0,0,2,0)), _mm_shuffle_epi32(tmp2, _MM_SHUFFLE (0,0,2,0))); /* shuffle results to [63..0] and pack */
#endif
  }

  // деления целых чисел нет :(
//     inline ivec4 operator/(const ivec4 &vec) const {
//       return _mm_div_epi32(native, vec.native);
//     }

  inline ivec4 operator-(const ivec4 &vec, const int32_t &value) {
    return _mm_sub_epi32(vec, _mm_set1_epi32(value));
  }

  inline ivec4 operator+(const ivec4 &vec, const int32_t &value) {
    return _mm_add_epi32(vec, _mm_set1_epi32(value));
  }

  inline ivec4 operator*(const ivec4 &vec, const int32_t &value) {
#ifdef __SSE4_1__
    return _mm_mullo_epi32(vec, _mm_set1_epi32(value));
#else
    __m128i val = _mm_set1_epi32(value);
    __m128i tmp1 = _mm_mul_epu32(vec, val); /* mul 2,0*/
    __m128i tmp2 = _mm_mul_epu32( _mm_srli_si128(vec, 4), _mm_srli_si128(val, 4)); /* mul 3,1 */
    return _mm_unpacklo_epi32(_mm_shuffle_epi32(tmp1, _MM_SHUFFLE (0,0,2,0)), _mm_shuffle_epi32(tmp2, _MM_SHUFFLE (0,0,2,0))); /* shuffle results to [63..0] and pack */
#endif
  }

  inline ivec4 operator-(const int32_t &value, const ivec4 &vec) {
    return _mm_sub_epi32(_mm_set1_epi32(value), vec);
  }

  inline ivec4 operator+(const int32_t &value, const ivec4 &vec) {
    return _mm_add_epi32(_mm_set1_epi32(value), vec);
  }

  inline ivec4 operator*(const int32_t &value, const ivec4 &vec) {
#ifdef __SSE4_1__
    return _mm_mullo_epi32(_mm_set1_epi32(value), vec);
#else
    __m128i val = _mm_set1_epi32(value);
    __m128i tmp1 = _mm_mul_epu32(val, vec); /* mul 2,0*/
    __m128i tmp2 = _mm_mul_epu32(_mm_srli_si128(val, 4), _mm_srli_si128(vec, 4)); /* mul 3,1 */
    return _mm_unpacklo_epi32(_mm_shuffle_epi32(tmp1, _MM_SHUFFLE (0,0,2,0)), _mm_shuffle_epi32(tmp2, _MM_SHUFFLE (0,0,2,0))); /* shuffle results to [63..0] and pack */
#endif
  }

  inline ivec4 div(const ivec4 &vec, const int32_t &value) {
    return ivec4(vec4(vec) / float(value));
  }

  inline ivec4 div(const ivec4 &vec1, const ivec4 &vec2) {
    return ivec4(vec4(vec1) / vec4(vec2));
  }

  // деления целых чисел нет :(
//     inline ivec4 operator/(const int32_t &value) const {
//       return _mm_div_ps(native, _mm_set1_epi32(value));
//     }

  inline ivec4 operator|(const ivec4 &vec1, const ivec4 &vec2) {
    return _mm_or_si128(vec1, vec2);
  }

  inline ivec4 operator>(const ivec4 &vec1, const ivec4 &vec2) {
    return _mm_cmpgt_epi32(vec1, vec2);
  }

  inline ivec4 operator>=(const ivec4 &vec1, const ivec4 &vec2) {
//       return _mm_cmpge_epi32(native, vec.native);
    return ivec4(vec1 > vec2) | ivec4(vec1 == vec2);
  }

  inline ivec4 operator<(const ivec4 &vec1, const ivec4 &vec2) {
    return _mm_cmplt_epi32(vec1, vec2);
  }

  // этого оператора нет (сработает ли это?)
  inline ivec4 operator<=(const ivec4 &vec1, const ivec4 &vec2) {
    //return _mm_cmple_epi32(native, vec.native);
    return ivec4(vec1 < vec2) | ivec4(vec1 == vec2);
  }

  inline ivec4 operator==(const ivec4 &vec1, const ivec4 &vec2) {
    return _mm_cmpeq_epi32(vec1, vec2);
  }

  inline ivec4 operator!=(const ivec4 &vec1, const ivec4 &vec2) {
    //return _mm_cmpneq_epi32(native, vec.native);
    return ~(vec1 == vec2);
  }

  inline ivec4 operator&(const ivec4 &vec1, const ivec4 &vec2) {
    return _mm_and_si128(vec1, vec2);
  }

  inline ivec4 operator^(const ivec4 &vec1, const ivec4 &vec2) {
    return _mm_xor_si128(vec1, vec2);
  }

  inline ivec4 operator<<(const ivec4 &vec, const int32_t &shift) {
    return _mm_slli_epi32(vec, shift);
  }

  inline ivec4 operator>>(const ivec4 &vec, const int32_t &shift) {
    return _mm_srli_epi32(vec, shift);
  }

  inline ivec4 andnot(const ivec4 &vec1, const ivec4 &vec2) {
    return _mm_andnot_si128(vec1, vec2);
  }

  inline vec4 cast(const ivec4 &vec) {
    return _mm_castsi128_ps(vec);
  }

  inline ivec4 cast(const vec4 &vec) {
    return _mm_castps_si128(vec);
  }

  inline ivec4 truncate(const vec4 &vec) {
    return _mm_cvttps_epi32(vec);
  }

//   // чекать http://gruntthepeon.free.fr/ssemath/
//
//   const ivec4 min_norm_pos = ivec4(0x00800000);
//   const ivec4 mant_mask = ivec4(0x7f800000);
//   const ivec4 inv_mant_mask = ivec4(~0x7f800000);
//   const ivec4 sign_mask = ivec4((int)0x80000000);
//   const ivec4 inv_sign_mask = ivec4(~0x80000000);
//   const ivec4 const_1 = ivec4(1);
//   const ivec4 const_inv1 = ivec4(~1);
//   const ivec4 const_2 = ivec4(2);
//   const ivec4 const_4 = ivec4(4);
//   const ivec4 const_0x7f = ivec4(0x7f);
//
//   const vec4 const_0p5f = vec4(0.5f);
//   const vec4 const_1p0f = vec4(1.0f);
//   const vec4 cephes_SQRTHF = vec4(0.707106781186547524);
//   const vec4 cephes_log_p0 = vec4(7.0376836292E-2);
//   const vec4 cephes_log_p1 = vec4(- 1.1514610310E-1);
//   const vec4 cephes_log_p2 = vec4(1.1676998740E-1);
//   const vec4 cephes_log_p3 = vec4(- 1.2420140846E-1);
//   const vec4 cephes_log_p4 = vec4(+ 1.4249322787E-1);
//   const vec4 cephes_log_p5 = vec4(- 1.6668057665E-1);
//   const vec4 cephes_log_p6 = vec4(+ 2.0000714765E-1);
//   const vec4 cephes_log_p7 = vec4(- 2.4999993993E-1);
//   const vec4 cephes_log_p8 = vec4(+ 3.3333331174E-1);
//   const vec4 cephes_log_q1 = vec4(-2.12194440e-4);
//   const vec4 cephes_log_q2 = vec4(0.693359375);
//
//   vec4 log(const vec4 &vec) {
//     const vec4 one = const_1p0f;
//
//     const vec4 invalid_mask = vec <= vec4(0.0f); //_mm_cmple_ps(x, _mm_setzero_ps());
//
//     vec4 x = max(vec, cast(min_norm_pos)); // _mm_max_ps(x, *(v4sf*)_ps_min_norm_pos);  /* cut off denormalized stuff */
//
//     ivec4 emm0 = cast(x) >> 23; //_mm_srli_epi32(_mm_castps_si128(x), 23);
//
//     /* keep only the fractional part */
//     x &= cast(inv_mant_mask); //_mm_and_ps(x, *(v4sf*)_ps_inv_mant_mask);
//     x |= const_0p5f; //_mm_or_ps(x, *(v4sf*)_ps_0p5);
//
//     emm0 -= const_0x7f;
//     vec4 e = emm0; // _mm_cvtepi32_ps(emm0);
//
//     e += one; // _mm_add_ps(e, one);
//
//     /* part2:
//       if( x < SQRTHF ) {
//         e -= 1;
//         x = x + x - 1.0;
//       } else { x = x - 1.0; }
//     */
//
//     const vec4 mask = x < cephes_SQRTHF; //_mm_cmplt_ps(x, *(v4sf*)_ps_cephes_SQRTHF);
//     vec4 tmp = x & mask; //_mm_and_ps(x, mask);
//     x -= one; //_mm_sub_ps(x, one);
//     e -= one & mask; // _mm_sub_ps(e, _mm_and_ps(one, mask));
//     x += tmp; // _mm_add_ps(x, tmp);
//
//
//     vec4 z = x*x; //_mm_mul_ps(x,x);
//
//     vec4 y = cephes_log_p0; //*(v4sf*)_ps_cephes_log_p0;
//     y *= x; //_mm_mul_ps(y, x);
//     y += cephes_log_p1; //_mm_add_ps(y, *(v4sf*)_ps_cephes_log_p1);
//     y *= x; //_mm_mul_ps(y, x);
//     y += cephes_log_p2; // _mm_add_ps(y, *(v4sf*)_ps_cephes_log_p2);
//     y *= x; //_mm_mul_ps(y, x);
//     y += cephes_log_p3; //_mm_add_ps(y, *(v4sf*)_ps_cephes_log_p3);
//     y *= x; //_mm_mul_ps(y, x);
//     y += cephes_log_p4; //_mm_add_ps(y, *(v4sf*)_ps_cephes_log_p4);
//     y *= x; //_mm_mul_ps(y, x);
//     y += cephes_log_p5; //_mm_add_ps(y, *(v4sf*)_ps_cephes_log_p5);
//     y *= x; //_mm_mul_ps(y, x);
//     y += cephes_log_p6; // _mm_add_ps(y, *(v4sf*)_ps_cephes_log_p6);
//     y *= x; //_mm_mul_ps(y, x);
//     y += cephes_log_p7; // _mm_add_ps(y, *(v4sf*)_ps_cephes_log_p7);
//     y *= x; //_mm_mul_ps(y, x);
//     y += cephes_log_p8; //_mm_add_ps(y, *(v4sf*)_ps_cephes_log_p8);
//     y *= x; //_mm_mul_ps(y, x);
//
//     y += z; //_mm_mul_ps(y, z);
//
//
//     tmp = e * cephes_log_q1; //_mm_mul_ps(e, *(v4sf*)_ps_cephes_log_q1);
//     y += tmp; //_mm_add_ps(y, tmp);
//
//
//     tmp = z * const_0p5f; // _mm_mul_ps(z, *(v4sf*)_ps_0p5);
//     y -= tmp; // _mm_sub_ps(y, tmp);
//
//     tmp = e * cephes_log_q2; //_mm_mul_ps(e, *(v4sf*)_ps_cephes_log_q2);
//     x += y; //_mm_add_ps(x, y);
//     x += tmp; //_mm_add_ps(x, tmp);
//     x |= invalid_mask; //_mm_or_ps(x, invalid_mask); // negative arg will be NAN
//     return x;
//   }
//
//   const vec4 exp_hi = vec4( 88.3762626647949f);
//   const vec4 exp_lo = vec4(-88.3762626647949f);
//
//   const vec4 cephes_LOG2EF = vec4( 1.44269504088896341);
//   const vec4 cephes_exp_C1 = vec4( 0.693359375);
//   const vec4 cephes_exp_C2 = vec4(-2.12194440e-4);
//
//   const vec4 cephes_exp_p0 = vec4(1.9875691500E-4);
//   const vec4 cephes_exp_p1 = vec4(1.3981999507E-3);
//   const vec4 cephes_exp_p2 = vec4(8.3334519073E-3);
//   const vec4 cephes_exp_p3 = vec4(4.1665795894E-2);
//   const vec4 cephes_exp_p4 = vec4(1.6666665459E-1);
//   const vec4 cephes_exp_p5 = vec4(5.0000001201E-1);
//
//   vec4 exp(const vec4 &vec) {
//     vec4 tmp, fx;
//     const vec4 one = const_1p0f;
//
//     vec4 x = max(min(vec, exp_hi), exp_lo);
//
//     /* express exp(x) as exp(g + n*log(2)) */
//     fx = x * cephes_LOG2EF + const_0p5f;
//
//     /* how to perform a floorf with SSE: just below */
//     ivec4 emm0 = truncate(fx); //_mm_cvttps_epi32(fx);
//     tmp = emm0; //_mm_cvtepi32_ps(emm0);
//
//     /* if greater, substract 1 */
//     vec4 mask = tmp > fx; //_mm_cmpgt_ps(tmp, fx);
//     mask += one; // _mm_and_ps(mask, one);
//     fx = tmp - mask; //_mm_sub_ps(tmp, mask);
//
//     tmp = fx * cephes_exp_C1; //_mm_mul_ps(fx, *(v4sf*)_ps_cephes_exp_C1);
//     vec4 z = fx * cephes_exp_C2; // _mm_mul_ps(fx, *(v4sf*)_ps_cephes_exp_C2);
//     x -= tmp; //_mm_sub_ps(x, tmp);
//     x -= z; // _mm_sub_ps(x, z);
//
//     z = x*x; //_mm_mul_ps(x,x);
//
//     vec4 y = cephes_exp_p0; //*(v4sf*)_ps_cephes_exp_p0;
//     y *= x; //_mm_mul_ps(y, x);
//     y += cephes_exp_p1; //_mm_add_ps(y, *(v4sf*)_ps_cephes_exp_p1);
//     y *= x; // _mm_mul_ps(y, x);
//     y += cephes_exp_p2; // _mm_add_ps(y, *(v4sf*)_ps_cephes_exp_p2);
//     y *= x; // _mm_mul_ps(y, x);
//     y += cephes_exp_p3; // _mm_add_ps(y, *(v4sf*)_ps_cephes_exp_p3);
//     y *= x; // _mm_mul_ps(y, x);
//     y += cephes_exp_p4; // _mm_add_ps(y, *(v4sf*)_ps_cephes_exp_p4);
//     y *= x; // _mm_mul_ps(y, x);
//     y += cephes_exp_p5; // _mm_add_ps(y, *(v4sf*)_ps_cephes_exp_p5);
//     y *= z; // _mm_mul_ps(y, z);
//     y += x; // _mm_add_ps(y, x);
//     y += one; // _mm_add_ps(y, one);
//
//     /* build 2^n */
//
//     emm0 = truncate(fx); // _mm_cvttps_epi32(fx);
//     emm0 += const_0x7f; // _mm_add_epi32(emm0, *(v4si*)_pi32_0x7f);
//     emm0 <<= 23; // _mm_slli_epi32(emm0, 23);
//     vec4 pow2n = cast(emm0); // _mm_castsi128_ps(emm0);
//
//     y *= pow2n; // _mm_mul_ps(y, pow2n);
//     return y;
//   }
//
//   const vec4 minus_cephes_DP1 = vec4(-0.78515625);
//   const vec4 minus_cephes_DP2 = vec4(-2.4187564849853515625e-4);
//   const vec4 minus_cephes_DP3 = vec4(-3.77489497744594108e-8);
//   const vec4 sincof_p0 = vec4(-1.9515295891E-4);
//   const vec4 sincof_p1 = vec4( 8.3321608736E-3);
//   const vec4 sincof_p2 = vec4(-1.6666654611E-1);
//   const vec4 coscof_p0 = vec4( 2.443315711809948E-005);
//   const vec4 coscof_p1 = vec4(-1.388731625493765E-003);
//   const vec4 coscof_p2 = vec4( 4.166664568298827E-002);
//   const vec4 cephes_FOPI = vec4(1.27323954473516); // 4 / M_PI
//
//   vec4 sin(const vec4 &vec) {
//     vec4 xmm1, xmm2, xmm3, sign_bit, y;
//
//     ivec4 emm0, emm2;
//
//     sign_bit = vec;
//     /* take the absolute value */
//     vec4 x = vec & cast(inv_sign_mask); // _mm_and_ps(x, *(v4sf*)_ps_inv_sign_mask);
//     /* extract the sign bit (upper one) */
//     sign_bit &= cast(sign_mask); // _mm_and_ps(sign_bit, *(v4sf*)_ps_sign_mask);
//
//     /* scale by 4/Pi */
//     y = x * cephes_FOPI; // _mm_mul_ps(x, *(v4sf*)_ps_cephes_FOPI);
//
//     /* store the integer part of y in mm0 */
//     emm2 = truncate(y); //_mm_cvttps_epi32(y);
//
//     /* j=(j+1) & (~1) (see the cephes sources) */
//     emm2 += const_1; //_mm_add_epi32(emm2, *(v4si*)_pi32_1);
//     emm2 &= const_inv1; //_mm_and_si128(emm2, *(v4si*)_pi32_inv1);
//     y = emm2; // _mm_cvtepi32_ps(emm2);
//
//     /* get the swap sign flag */
//     emm0 = emm2 & const_4; //_mm_and_si128(emm2, *(v4si*)_pi32_4);
//     emm0 <<= 29; //_mm_slli_epi32(emm0, 29);
//
//     /* get the polynom selection mask
//       there is one polynom for 0 <= x <= Pi/4
//       and another one for Pi/4<x<=Pi/2
//
//       Both branches will be computed.
//     */
//     emm2 &= const_2; //_mm_and_si128(emm2, *(v4si*)_pi32_2);
//     emm2 = emm2 == ivec4(0); //_mm_cmpeq_epi32(emm2, _mm_setzero_si128());
//
//     vec4 swap_sign_bit = cast(emm0); //_mm_castsi128_ps(emm0);
//     vec4 poly_mask = cast(emm2); //_mm_castsi128_ps(emm2);
//     sign_bit ^= swap_sign_bit; //_mm_xor_ps(sign_bit, swap_sign_bit);
//
//     /* The magic pass: "Extended precision modular arithmetic"
//       x = ((x - y * DP1) - y * DP2) - y * DP3; */
//     //x = ((x - y * minus_cephes_DP1) - y * minus_cephes_DP2) - y * minus_cephes_DP3;
//     x = ((x + y * minus_cephes_DP1) + y * minus_cephes_DP2) + y * minus_cephes_DP3;
//
//     // что за прикол? в коментарии указан минус, а в коде значения складываются?
//     // ну лучше наверное сложить
//
//     //xmm1 = minus_cephes_DP1; //*(v4sf*)_ps_minus_cephes_DP1;
//     //xmm2 = minus_cephes_DP2; //*(v4sf*)_ps_minus_cephes_DP2;
//     //xmm3 = minus_cephes_DP3; //*(v4sf*)_ps_minus_cephes_DP3;
//     //xmm1 = _mm_mul_ps(y, xmm1);
//     //xmm2 = _mm_mul_ps(y, xmm2);
//     //xmm3 = _mm_mul_ps(y, xmm3);
//     //x = _mm_add_ps(x, xmm1);
//     //x = _mm_add_ps(x, xmm2);
//     //x = _mm_add_ps(x, xmm3);
//
//     /* Evaluate the first polynom  (0 <= x <= Pi/4) */
//     y = coscof_p0; // *(v4sf*)_ps_coscof_p0;
//     vec4 z = x*x; // _mm_mul_ps(x,x);
//
//     y *= z; // _mm_mul_ps(y, z);
//     y += coscof_p1; //_mm_add_ps(y, *(v4sf*)_ps_coscof_p1);
//     y *= z; //_mm_mul_ps(y, z);
//     y += coscof_p2; //_mm_add_ps(y, *(v4sf*)_ps_coscof_p2);
//     y *= z; //_mm_mul_ps(y, z);
//     y *= z; //_mm_mul_ps(y, z);
//     vec4 tmp = z * const_0p5f; //_mm_mul_ps(z, *(v4sf*)_ps_0p5);
//     y -= tmp; //_mm_sub_ps(y, tmp);
//     // что тут надо сделать?
//     //y += const_1; //_mm_add_ps(y, *(v4sf*)_ps_1);
//     y += cast(const_1);
//
//     /* Evaluate the second polynom  (Pi/4 <= x <= 0) */
//
//     vec4 y2 = sincof_p0; // *(v4sf*)_ps_sincof_p0;
//     y2 *= z; // _mm_mul_ps(y2, z);
//     y2 += sincof_p1; // _mm_add_ps(y2, *(v4sf*)_ps_sincof_p1);
//     y2 *= z; // _mm_mul_ps(y2, z);
//     y2 += sincof_p2; // _mm_add_ps(y2, *(v4sf*)_ps_sincof_p2);
//     y2 *= z; // _mm_mul_ps(y2, z);
//     y2 *= x; // _mm_mul_ps(y2, x);
//     y2 += x; // _mm_add_ps(y2, x);
//
//     /* select the correct result from the two polynoms */
//     xmm3 = poly_mask;
//     y2 = xmm3 & y2; // _mm_and_ps(xmm3, y2); //, xmm3);
//     y = andnot(xmm3, y); // _mm_andnot_ps(xmm3, y);
//     y += y2; // _mm_add_ps(y,y2);
//
//     /* update the sign */
//     y ^= sign_bit; // _mm_xor_ps(y, sign_bit);
//     return y;
//   }
//
//   vec4 cos(const vec4 &vec) {
//     vec4 xmm1, /*xmm2 = vec4(),*/ xmm3, y;
//
//     ivec4 emm0, emm2;
//
//     /* take the absolute value */
//     vec4 x = vec & cast(inv_sign_mask); // _mm_and_ps(x, *(v4sf*)_ps_inv_sign_mask);
//
//     /* scale by 4/Pi */
//     y = x * cephes_FOPI; // _mm_mul_ps(x, *(v4sf*)_ps_cephes_FOPI);
//
//     /* store the integer part of y in mm0 */
//     emm2 = truncate(y); // _mm_cvttps_epi32(y);
//
//     /* j=(j+1) & (~1) (see the cephes sources) */
//     emm2 += const_1; // _mm_add_epi32(emm2, *(v4si*)_pi32_1);
//     emm2 &= const_inv1; //_mm_and_si128(emm2, *(v4si*)_pi32_inv1);
//     y = emm2; // _mm_cvtepi32_ps(emm2);
//
//     emm2 -= const_2; // _mm_sub_epi32(emm2, *(v4si*)_pi32_2);
//
//     /* get the swap sign flag */
//     emm0 = andnot(emm2, const_4); // _mm_andnot_si128(emm2, *(v4si*)_pi32_4);
//     emm0 <<= 29; // _mm_slli_epi32(emm0, 29);
//
//     /* get the polynom selection mask */
//     emm2 &= const_2; // _mm_and_si128(emm2, *(v4si*)_pi32_2);
//     emm2 = emm2 == ivec4(); //_mm_cmpeq_epi32(emm2, _mm_setzero_si128());
//
//     vec4 sign_bit = cast(emm0); // _mm_castsi128_ps(emm0);
//     vec4 poly_mask = cast(emm2); // _mm_castsi128_ps(emm2);
//
//     /* The magic pass: "Extended precision modular arithmetic"
//       x = ((x - y * DP1) - y * DP2) - y * DP3; */
//
//     // потому что у нас -DP используется
//     x = ((x + y * minus_cephes_DP1) + y * minus_cephes_DP2) + y * minus_cephes_DP3;
//
//     //xmm1 = *(v4sf*)_ps_minus_cephes_DP1;
//     //xmm2 = *(v4sf*)_ps_minus_cephes_DP2;
//     //xmm3 = *(v4sf*)_ps_minus_cephes_DP3;
//     //xmm1 = _mm_mul_ps(y, xmm1);
//     //xmm2 = _mm_mul_ps(y, xmm2);
//     //xmm3 = _mm_mul_ps(y, xmm3);
//     //x = _mm_add_ps(x, xmm1);
//     //x = _mm_add_ps(x, xmm2);
//     //x = _mm_add_ps(x, xmm3);
//
//     /* Evaluate the first polynom  (0 <= x <= Pi/4) */
//     y = coscof_p0; // *(v4sf*)_ps_coscof_p0;
//     vec4 z = x*x;  //_mm_mul_ps(x,x);
//
//     y *= z; //_mm_mul_ps(y, z);
//     y += coscof_p1; // _mm_add_ps(y, *(v4sf*)_ps_coscof_p1);
//     y *= z; //_mm_mul_ps(y, z);
//     y += coscof_p2; // _mm_add_ps(y, *(v4sf*)_ps_coscof_p2);
//     y *= z; // _mm_mul_ps(y, z);
//     y *= z; // _mm_mul_ps(y, z);
//     vec4 tmp = z * const_0p5f; // _mm_mul_ps(z, *(v4sf*)_ps_0p5);
//     y -= tmp; // _mm_sub_ps(y, tmp);
//     y += const_1; //_mm_add_ps(y, *(v4sf*)_ps_1);
//     y += cast(const_1);
//
//     /* Evaluate the second polynom  (Pi/4 <= x <= 0) */
//
//     vec4 y2 = sincof_p0; // *(v4sf*)_ps_sincof_p0;
//     y2 *= z; // _mm_mul_ps(y2, z);
//     y2 += sincof_p1; // _mm_add_ps(y2, *(v4sf*)_ps_sincof_p1);
//     y2 *= z; // _mm_mul_ps(y2, z);
//     y2 += sincof_p2; // _mm_add_ps(y2, *(v4sf*)_ps_sincof_p2);
//     y2 *= z; // _mm_mul_ps(y2, z);
//     y2 *= x; // _mm_mul_ps(y2, x);
//     y2 += x; // _mm_add_ps(y2, x);
//
//     /* select the correct result from the two polynoms */
//     xmm3 = poly_mask;
//     y2 = xmm3 & y2; // _mm_and_ps(xmm3, y2); //, xmm3);
//     y = andnot(xmm3, y); // _mm_andnot_ps(xmm3, y);
//     y += y2; // _mm_add_ps(y,y2);
//
//     /* update the sign */
//     y ^= sign_bit; // _mm_xor_ps(y, sign_bit);
//
//     return y;
//   }
//
//   void sincos(const vec4 &input, vec4 &s, vec4 &c) {
//     vec4 xmm1, xmm2, xmm3 = vec4(), sign_bit_sin, y;
//     ivec4 emm0, emm2, emm4;
//
//     sign_bit_sin = input;
//
//     /* take the absolute value */
//     vec4 x = input & cast(inv_sign_mask); // _mm_and_ps(x, *(v4sf*)_ps_inv_sign_mask);
//
//     /* extract the sign bit (upper one) */
//     sign_bit_sin &= cast(sign_mask); // _mm_and_ps(sign_bit_sin, *(v4sf*)_ps_sign_mask);
//
//     /* scale by 4/Pi */
//     y = x * cephes_FOPI; // _mm_mul_ps(x, *(v4sf*)_ps_cephes_FOPI);
//
//     /* store the integer part of y in emm2 */
//     //emm2 = truncate(y); // _mm_cvttps_epi32(y);
//
//     /* j=(j+1) & (~1) (see the cephes sources) */
//     emm2 = (truncate(y) + const_1) & const_inv1;
//
//     //emm2 += const_1; // _mm_add_epi32(emm2, *(v4si*)_pi32_1);
//     //emm2 &= const_inv1; // _mm_and_si128(emm2, *(v4si*)_pi32_inv1);
//     y = vec4(emm2); // _mm_cvtepi32_ps(emm2);
//
//     emm4 = emm2;
//
//     /* get the swap sign flag for the sine */
//     //emm0 = (emm2 & const_4) << 29;
//     vec4 swap_sign_bit_sin = cast((emm2 & const_4) << 29);
//
//     //emm0 = emm2 & const_4; // _mm_and_si128(emm2, *(v4si*)_pi32_4);
//     //emm0 <<= 29; //_mm_slli_epi32(emm0, 29);
//     //vec4 swap_sign_bit_sin = cast(emm0); // _mm_castsi128_ps(emm0);
//
//     /* get the polynom selection mask for the sine*/
//     vec4 poly_mask = cast((emm2 & const_2) == ivec4());
//
//     //emm2 &= const_2; // _mm_and_si128(emm2, *(v4si*)_pi32_2);
//     //emm2 = emm2 == ivec4(); // _mm_cmpeq_epi32(emm2, _mm_setzero_si128());
//     //vec4 poly_mask = cast(emm2); // _mm_castsi128_ps(emm2);
//
//     /* The magic pass: "Extended precision modular arithmetic"
//       x = ((x - y * DP1) - y * DP2) - y * DP3; */
//     x = ((x + y * minus_cephes_DP1) + y * minus_cephes_DP2) + y * minus_cephes_DP3;
//
//     //xmm1 = *(v4sf*)_ps_minus_cephes_DP1;
//     //xmm2 = *(v4sf*)_ps_minus_cephes_DP2;
//     //xmm3 = *(v4sf*)_ps_minus_cephes_DP3;
//     //xmm1 = _mm_mul_ps(y, xmm1);
//     //xmm2 = _mm_mul_ps(y, xmm2);
//     //xmm3 = _mm_mul_ps(y, xmm3);
//     //x = _mm_add_ps(x, xmm1);
//     //x = _mm_add_ps(x, xmm2);
//     //x = _mm_add_ps(x, xmm3);
//
//     vec4 sign_bit_cos = cast(andnot(emm4 - const_2, const_4) <<= 29);
//
//     //emm4 -= const_2; // _mm_sub_epi32(emm4, *(v4si*)_pi32_2);
//     //emm4 = andnot(emm4, const_4); // _mm_andnot_si128(emm4, *(v4si*)_pi32_4);
//     //emm4 <<= 29; //_mm_slli_epi32(emm4, 29);
//     //vec4 sign_bit_cos = cast(emm4); // _mm_castsi128_ps(emm4);
//
//     sign_bit_sin ^= swap_sign_bit_sin; // _mm_xor_ps(sign_bit_sin, swap_sign_bit_sin);
//
//
//     /* Evaluate the first polynom  (0 <= x <= Pi/4) */
//     vec4 z = x*x; // _mm_mul_ps(x,x);
//     vec4 tmp = z * const_0p5f;
//
//     y = ((coscof_p0 * z + coscof_p1) * z + coscof_p2) * z * z - tmp + cast(const_1);
//
//     //y = coscof_p0; // *(v4sf*)_ps_coscof_p0;
//
//     //y *= z; // _mm_mul_ps(y, z);
//     //y += coscof_p1; // _mm_add_ps(y, *(v4sf*)_ps_coscof_p1);
//     //y *= z; // _mm_mul_ps(y, z);
//     //y += coscof_p2; // _mm_add_ps(y, *(v4sf*)_ps_coscof_p2);
//     //y *= z; // _mm_mul_ps(y, z);
//     //y *= z; // _mm_mul_ps(y, z);
//     //vec4 tmp = z * const_0p5f; // _mm_mul_ps(z, *(v4sf*)_ps_0p5);
//     //y -= tmp; // _mm_sub_ps(y, tmp);
//     //y += const_1; // _mm_add_ps(y, *(v4sf*)_ps_1);
//     //y += cast(const_1);
//
//     /* Evaluate the second polynom  (Pi/4 <= x <= 0) */
//
//     vec4 y2 = ((sincof_p0 * z + sincof_p1) * z + sincof_p2) * z * x + x;
//
//     //vec4 y2 = sincof_p0; // *(v4sf*)_ps_sincof_p0;
//     //y2 *= z; //_mm_mul_ps(y2, z);
//     //y2 += sincof_p1; // _mm_add_ps(y2, *(v4sf*)_ps_sincof_p1);
//     //y2 *= z; // _mm_mul_ps(y2, z);
//     //y2 += sincof_p2; // _mm_add_ps(y2, *(v4sf*)_ps_sincof_p2);
//     //y2 *= z; // _mm_mul_ps(y2, z);
//     //y2 *= x; // _mm_mul_ps(y2, x);
//     //y2 += x; // _mm_add_ps(y2, x);
//
//     /* select the correct result from the two polynoms */
//     xmm3 = poly_mask;
//     vec4 ysin2 = xmm3 & y2; // _mm_and_ps(xmm3, y2);
//     vec4 ysin1 = andnot(xmm3, y); // _mm_andnot_ps(xmm3, y);
//     y2 -= ysin2; // _mm_sub_ps(y2,ysin2);
//     y -= ysin1; // _mm_sub_ps(y, ysin1);
//
//     xmm1 = ysin1 + ysin2; // _mm_add_ps(ysin1,ysin2);
//     xmm2 = y + y2; //_mm_add_ps(y,y2);
//
//     /* update the sign */
//     s = xmm1 ^ sign_bit_sin; // _mm_xor_ps(xmm1, sign_bit_sin);
//     c = xmm2 ^ sign_bit_cos; // _mm_xor_ps(xmm2, sign_bit_cos);
//   }

  inline vec4::vec4(const ivec4 &vec) {
    native = _mm_cvtepi32_ps(vec);
  }

  inline vec4 & vec4::operator=(const ivec4 &vec) {
    native = _mm_cvtepi32_ps(vec);
    return *this;
  }
#endif

#ifdef __SSE3__
  inline vec4 addsub(const vec4 &a, const vec4 &b) {
    return _mm_addsub_ps(a, b);
  }

  inline vec4 hadd(const vec4 &a, const vec4 &b) {
    return _mm_hadd_ps(a, b);
  }

  inline vec4 hsub(const vec4 &a, const vec4 &b) {
    return _mm_hsub_ps(a, b);
  }

  //struct mat4;

  //mat4 inverse(const mat4 &mat);
  //inline mat4 operator-(const mat4 &mat1, const mat4 &mat2);
  //inline mat4 operator+(const mat4 &mat1, const mat4 &mat2);
  //inline mat4 operator/(const mat4 &mat1, const mat4 &mat2);
  //inline mat4 operator*(const mat4 &mat1, const mat4 &mat2);

  inline mat4::mat4() {
    value[0] = vec4(1.0f, 0.0f, 0.0f, 0.0f);
    value[1] = vec4(0.0f, 1.0f, 0.0f, 0.0f);
    value[2] = vec4(0.0f, 0.0f, 1.0f, 0.0f);
    value[3] = vec4(0.0f, 0.0f, 0.0f, 1.0f);
  }

  inline mat4::mat4(const float &a) {
    value[0] = vec4(   a, 0.0f, 0.0f, 0.0f);
    value[1] = vec4(0.0f,    a, 0.0f, 0.0f);
    value[2] = vec4(0.0f, 0.0f,    a, 0.0f);
    value[3] = vec4(0.0f, 0.0f, 0.0f,    a);
  }

  inline mat4::mat4(const vec4 &first, const vec4 &second, const vec4 &third, const vec4 &forth) {
    value[0] = first;
    value[1] = second;
    value[2] = third;
    value[3] = forth;
  }

  inline mat4::mat4(const float &a11, const float &a12, const float &a13, const float &a14,
                    const float &a21, const float &a22, const float &a23, const float &a24,
                    const float &a31, const float &a32, const float &a33, const float &a34,
                    const float &a41, const float &a42, const float &a43, const float &a44) {
    value[0] = vec4(a11, a12, a13, a14);
    value[1] = vec4(a21, a22, a23, a24);
    value[2] = vec4(a31, a32, a33, a34);
    value[3] = vec4(a41, a42, a43, a44);
  }

  inline mat4::mat4(const mat4 &mat) {
    value[0] = mat[0];
    value[1] = mat[1];
    value[2] = mat[2];
    value[3] = mat[3];
  }

  inline mat4::mat4(const quat &q) {
    *this = cast(q);
  }

  inline mat4::~mat4() {}

//   inline constexpr uint32_t mat4::length() const {
//     return 4;
//   }

  inline vec4 & mat4::operator[](const uint32_t &index) {
    return value[index];
  }

  inline const vec4 & mat4::operator[](const uint32_t &index) const {
    return value[index];
  }

  inline mat4 & mat4::operator=(const mat4 &mat) {
    value[0] = mat[0];
    value[1] = mat[1];
    value[2] = mat[2];
    value[3] = mat[3];
    return *this;
  }

  inline mat4 & mat4::operator=(const quat &q) {
    //mat4 tmp = cast(q);
    *this = cast(q);
    return *this;
  }

  inline mat4 & mat4::operator-=(const mat4 &mat) {
    value[0] -= mat.value[0];
    value[1] -= mat.value[1];
    value[2] -= mat.value[2];
    value[3] -= mat.value[3];
    return *this;
  }

  inline mat4 & mat4::operator-=(const float &a) {
    value[0] -= a;
    value[1] -= a;
    value[2] -= a;
    value[3] -= a;
    return *this;
  }

  inline mat4 & mat4::operator+=(const mat4 &mat) {
    value[0] += mat.value[0];
    value[1] += mat.value[1];
    value[2] += mat.value[2];
    value[3] += mat.value[3];
    return *this;
  }

  inline mat4 & mat4::operator+=(const float &a) {
    value[0] += a;
    value[1] += a;
    value[2] += a;
    value[3] += a;
    return *this;
  }

  inline mat4 & mat4::operator/=(const mat4 &mat) {
    *this *= inverse(mat);
    return *this;
  }

  inline mat4 & mat4::operator/=(const float &a) {
    value[0] /= a;
    value[1] /= a;
    value[2] /= a;
    value[3] /= a;
    return *this;
  }

  inline mat4 & mat4::operator*=(const mat4 &mat) {
    *this = *this * mat;
    return *this;
  }

  inline mat4 & mat4::operator*=(const float &a) {
    value[0] *= a;
    value[1] *= a;
    value[2] *= a;
    value[3] *= a;
    return *this;
  }

//   // только для трансформ матриц
//   mat4 inverse_transform(const mat4 &mat) {
//     #define SMALL_NUMBER (1.e-8f)
//
//     mat4 r;
//
//     // transpose 3x3, we know m03 = m13 = m23 = 0
//     __m128 t0 = movelh(mat[0], mat[1]);     // 00, 01, 10, 11
//     __m128 t1 = movehl(mat[0], mat[1]);     // 02, 03, 12, 13
//     r[0] = simd_shuffle(t0, mat[2], 0,2,0,3); // 00, 10, 20, 23(=0)
//     r[1] = simd_shuffle(t0, mat[2], 1,3,1,3); // 01, 11, 21, 23(=0)
//     r[2] = simd_shuffle(t1, mat[2], 0,2,2,3); // 02, 12, 22, 23(=0)
//
//     // (SizeSqr(mVec[0]), SizeSqr(mVec[1]), SizeSqr(mVec[2]), 0)
//     vec4 sizeSqr;
//     sizeSqr =  r[0] * r[0];
//     sizeSqr += r[1] * r[1];
//     sizeSqr += r[2] * r[2];
//
//     // optional test to avoid divide by 0
//     const vec4 one(1.f);
//     // for each component, if (sizeSqr < SMALL_NUMBER) sizeSqr = 1;
//     vec4 rSizeSqr = blendv(one / sizeSqr, one, sizeSqr < vec4(SMALL_NUMBER));
//
//     r[0] = r[0] * rSizeSqr;
//     r[1] = r[1] * rSizeSqr;
//     r[2] = r[2] * rSizeSqr;
//
//     // last line
//     r[3]  = r[0] * simd_swizzle1(mat[3], 0);
//     r[3] += r[1] * simd_swizzle1(mat[3], 1);
//     r[3] += r[2] * simd_swizzle1(mat[3], 2);
//     r[3]  = vec4(0.f, 0.f, 0.f, 1.f) - r[3];
//
//     return r;
//   }

  #define makeShuffleMask(x,y,z,w) (x | (y<<2) | (z<<4) | (w<<6))

  #define vecSwizzleMask(vec, mask) vec4(_mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(vec), mask)))
  #define vecSwizzle(vec, x, y, z, w) vecSwizzleMask(vec, makeShuffleMask(x, y, z, w))
  #define vecSwizzle1(vec, x) vecSwizzleMask(vec, makeShuffleMask(x, x, x, x))
  #define vecSwizzle_0022(vec) vec4(_mm_moveldup_ps(vec))
  #define vecSwizzle_1133(vec) vec4(_mm_movehdup_ps(vec))

  // return (vec1[x], vec1[y], vec2[z], vec2[w])
  #define vecShuffle(vec1, vec2, x,y,z,w)    vec4(_mm_shuffle_ps(vec1, vec2, makeShuffleMask(x,y,z,w)))
  // special shuffle
  #define vecShuffle_0101(vec1, vec2)        vec4(_mm_movelh_ps(vec1, vec2))
  #define vecShuffle_2323(vec1, vec2)        vec4(_mm_movehl_ps(vec1, vec2))

  inline vec4 mat2Mul(const vec4 &mat1, const vec4 &mat2) {
    return            mat1              * vecSwizzle(mat2, 0, 3, 0, 3) +
           vecSwizzle(mat1, 1, 0, 3, 2) * vecSwizzle(mat2, 2, 1, 2, 1);
  }

  inline vec4 mat2AdjMul(const vec4 &mat1, const vec4 &mat2) {
    return vecSwizzle(mat1, 3, 3, 0, 0) *         mat2 -
           vecSwizzle(mat1, 1, 1, 2, 2) * vecSwizzle(mat2, 2, 3, 0, 1);
  }

  inline vec4 mat2MulAdj(const vec4 &mat1, const vec4 &mat2) {
    return            mat1              * vecSwizzle(mat2, 0, 3, 0, 3) -
           vecSwizzle(mat1, 1, 0, 3, 2) * vecSwizzle(mat2, 2, 1, 2, 1);
  }

//   // для всех
//   mat4 inverse(const mat4 &mat) {
//     // use block matrix method
//     // A is a matrix, then i(A) or iA means inverse of A, A# (or A_ in code) means adjugate of A, |A| (or detA in code) is determinant, tr(A) is trace
//
//     // sub matrices
//     const vec4 A = movelh(mat[0], mat[1]);
//     const vec4 B = movehl(mat[0], mat[1]);
//     const vec4 C = movelh(mat[2], mat[3]);
//     const vec4 D = movehl(mat[2], mat[3]);
//
//     // я полагаю что в моем случае float версия будет медленее
// //     const vec4 detA = vec4(mat[0][0] * mat[1][1] - mat[0][1] * mat[1][0]);
// //     const vec4 detB = vec4(mat[0][2] * mat[1][3] - mat[0][3] * mat[1][2]);
// //     const vec4 detC = vec4(mat[2][0] * mat[3][1] - mat[2][1] * mat[3][0]);
// //     const vec4 detD = vec4(mat[2][2] * mat[3][3] - mat[2][3] * mat[3][2]);
//
//     // for determinant, float version is faster
//     // determinant as (|A| |B| |C| |D|)
//     const vec4 detSub = simd_shuffle(mat[0], mat[2], 0,2,0,2) * simd_shuffle(mat[1], mat[3], 1,3,1,3) -
//                         simd_shuffle(mat[0], mat[2], 1,3,1,3) * simd_shuffle(mat[1], mat[3], 0,2,0,2);
//     const vec4 detA = simd_swizzle1(detSub, 0);
//     const vec4 detB = simd_swizzle1(detSub, 1);
//     const vec4 detC = simd_swizzle1(detSub, 2);
//     const vec4 detD = simd_swizzle1(detSub, 3);
//
//     // let iM = 1/|M| * | X  Y |
//     //                  | Z  W |
//
//     // D#C
//     const vec4 D_C = mat2AdjMul(D, C);
//     // A#B
//     const vec4 A_B = mat2AdjMul(A, B);
//     // X# = |D|A - B(D#C)
//     vec4 X_ = detD * A - mat2Mul(B, D_C);
//     // W# = |A|D - C(A#B)
//     vec4 W_ = detA * D - mat2Mul(C, A_B);
//
//     // |M| = |A|*|D| + ... (continue later)
//     vec4 detM = detA * detD;
//
//     // Y# = |B|C - D(A#B)#
//     vec4 Y_ = detB * C - mat2MulAdj(D, A_B);
//     // Z# = |C|B - A(D#C)#
//     vec4 Z_ = detC * B - mat2MulAdj(A, D_C);
//
//     // |M| = |A|*|D| + |B|*|C| ... (continue later)
//     detM = detM - (detB * detC);
//
//     // tr((A#B)(D#C))
//     vec4 tr = A_B * simd_swizzle(D_C, 0,2,1,3);
//     tr = hadd(tr, tr);
//     tr = hadd(tr, tr);
//     // |M| = |A|*|D| + |B|*|C| - tr((A#B)(D#C)
//     detM = detM - tr;
//
//     const vec4 adjSignMask = vec4(1.f, -1.f, -1.f, 1.f);
//     // (1/|M|, -1/|M|, -1/|M|, 1/|M|)
//     const vec4 rDetM = adjSignMask / detM;
//
//     X_ *= rDetM;
//     Y_ *= rDetM;
//     Z_ *= rDetM;
//     W_ *= rDetM;
//
//     // apply adjugate and store, here we combine adjugate shuffle and store shuffle
//     return mat4(
//       simd_shuffle(X_, Y_, 3,1,3,1),
//       simd_shuffle(X_, Y_, 2,0,2,0),
//       simd_shuffle(Z_, W_, 3,1,3,1),
//       simd_shuffle(Z_, W_, 2,0,2,0)
//     );
//   }

  inline mat4 operator-(const mat4 &mat1, const mat4 &mat2) {
    return mat4(mat1[0] - mat2[0],
                mat1[1] - mat2[1],
                mat1[2] - mat2[2],
                mat1[3] - mat2[3]);
  }

  inline mat4 operator-(const mat4 &mat, const float &a) {
    return mat4(mat[0] - a,
                mat[1] - a,
                mat[2] - a,
                mat[3] - a);
  }

  inline mat4 operator-(const float &a, const mat4 &mat) {
    return mat4(a - mat[0],
                a - mat[1],
                a - mat[2],
                a - mat[3]);
  }

  inline mat4 operator+(const mat4 &mat1, const mat4 &mat2) {
    return mat4(mat1[0] + mat2[0],
                mat1[1] + mat2[1],
                mat1[2] + mat2[2],
                mat1[3] + mat2[3]);
  }

  inline mat4 operator+(const mat4 &mat, const float &a) {
    return mat4(mat[0] + a,
                mat[1] + a,
                mat[2] + a,
                mat[3] + a);
  }

  inline mat4 operator+(const float &a, const mat4 &mat) {
    return mat4(a + mat[0],
                a + mat[1],
                a + mat[2],
                a + mat[3]);
  }

  inline mat4 operator/(const mat4 &mat1, const mat4 &mat2) {
    return mat1 * inverse(mat2);
  }

  inline mat4 operator/(const mat4 &mat, const float &a) {
    return mat4(mat[0] / a,
                mat[1] / a,
                mat[2] / a,
                mat[3] / a);
  }

  inline mat4 operator/(const float &a, const mat4 &mat) {
    return mat4(a / mat[0],
                a / mat[1],
                a / mat[2],
                a / mat[3]);
  }

  // нашел здесь http://fhtr.blogspot.com/2010/02/4x4-float-matrix-multiplication-using.html
  inline mat4 operator*(const mat4 &mat1, const mat4 &mat2) {
    mat4 mat;
    for (uint32_t i = 0; i < 4; ++i) {
      float arr[4];
      mat1[i].store(arr);

      const vec4 ARx = vec4(arr[0]);
      const vec4 ARy = vec4(arr[1]);
      const vec4 ARz = vec4(arr[2]);
      const vec4 ARw = vec4(arr[3]);

      const vec4 X = ARx * mat2[0];
      const vec4 Y = ARy * mat2[1];
      const vec4 Z = ARz * mat2[2];
      const vec4 W = ARw * mat2[3];

      mat[i] = X + Y + Z + W;
    }

    return mat;
  }

  inline mat4 operator*(const mat4 &mat, const float &a) {
    return mat4(mat[0] * a,
                mat[1] * a,
                mat[2] * a,
                mat[3] * a);
  }

  inline mat4 operator*(const float &a, const mat4 &mat) {
    return mat4(a * mat[0],
                a * mat[1],
                a * mat[2],
                a * mat[3]);
  }

  inline vec4 operator*(const mat4 &mat, const vec4 &vec) {
    const vec4 v0 = simd_swizzle1(vec, 0);
    const vec4 v1 = simd_swizzle1(vec, 1);
    const vec4 v2 = simd_swizzle1(vec, 2);
    const vec4 v3 = simd_swizzle1(vec, 3);

    const vec4 m0 = mat[0] * v0;
    const vec4 m1 = mat[1] * v1; // _mm_mul_ps(m[1].data, v1);
    const vec4 a0 = m0 + m1; //_mm_add_ps(m0, m1);

    const vec4 m2 = mat[2] * v2; //_mm_mul_ps(m[2].data, v2);
    const vec4 m3 = mat[3] * v3; //_mm_mul_ps(m[3].data, v3);
    const vec4 a1 = m2 + m3; // _mm_add_ps(m2, m3);

    const vec4 a2 = a0 + a1; //_mm_add_ps(a0, a1);

    return a2;
  }

  inline vec4 operator*(const vec4 &vec, const mat4 &mat) {
#ifdef __SSE4_1__
    const vec4 prod1 = _mm_dp_ps(mat[0], vec, 0xFF);
    const vec4 prod2 = _mm_dp_ps(mat[1], vec, 0xFF);
    const vec4 prod3 = _mm_dp_ps(mat[2], vec, 0xFF);
    const vec4 prod4 = _mm_dp_ps(mat[3], vec, 0xFF);

    return simd_shuffle(movelh(prod1, prod2), movelh(prod3, prod4), 2, 0, 2, 0);
#else
    const vec4 prod1 = mat[0] * vec; //_mm_mul_ps(rows[0], v);
    const vec4 prod2 = mat[1] * vec; //_mm_mul_ps(rows[1], v);
    const vec4 prod3 = mat[2] * vec; //_mm_mul_ps(rows[2], v);
    const vec4 prod4 = mat[3] * vec; //_mm_mul_ps(rows[3], v);

    return hadd(hadd(prod1, prod2), hadd(prod3, prod4));
#endif
  }

  inline mat4 transpose(const mat4 &mat) {
    mat4 r(mat);
    _MM_TRANSPOSE4_PS(r[0], r[1], r[2], r[3]);

    return r;
  }

  inline mat4 translate(const mat4 &mat, const vec4 &vec) {
    mat4 r;
    float arr[4];
    vec.store(arr);
    r[3] = mat[0] * arr[0] + mat[1] * arr[1] + mat[2] * arr[2] + mat[3];
    return r;
  }

//   mat4 rotate(const mat4 &mat, const float &angle, const vec4 &axis) {
//     const float c = ::cos(angle);
//     const float s = ::sin(angle);
//
//     const vec4 a(normalize(axis));
//     const vec4 temp(angle * (1.0f - c));
//
//     mat4 rotate;
//     rotate[0][0] = c + temp[0] * axis[0];
//     rotate[0][1] = temp[0] * axis[1] + s * axis[2];
//     rotate[0][2] = temp[0] * axis[2] - s * axis[1];
//
//     rotate[1][0] = temp[1] * axis[0] - s * axis[2];
//     rotate[1][1] = c + temp[1] * axis[1];
//     rotate[1][2] = temp[1] * axis[2] + s * axis[0];
//
//     rotate[2][0] = temp[2] * axis[0] + s * axis[1];
//     rotate[2][1] = temp[2] * axis[1] - s * axis[0];
//     rotate[2][2] = c + temp[2] * axis[2];
//
//     mat4 result;
//     result[0] = mat[0] * rotate[0][0] + mat[1] * rotate[0][1] + mat[2] * rotate[0][2];
//     result[1] = mat[0] * rotate[1][0] + mat[1] * rotate[1][1] + mat[2] * rotate[1][2];
//     result[2] = mat[0] * rotate[2][0] + mat[1] * rotate[2][1] + mat[2] * rotate[2][2];
//     result[3] = mat[3];
//     return result;
//   }

  inline mat4 scale(const mat4 &mat, const vec4 &vec) {
    mat4 result(mat);
    float arr[4];
    vec.store(arr);
    result[0] *= arr[0];
    result[1] *= arr[1];
    result[2] *= arr[2];
    //result[3] = m[3];
    return result;
  }

  inline mat4 ortho(const float &left, const float &right, const float &bottom, const float &top, const float &zNear, const float &zFar) {
    mat4 result(1);
    result[0][0] = 2.0f / (right - left);
    result[1][1] = 2.0f / (top - bottom);
    result[3][0] = -(right + left) / (right - left);
    result[3][1] = -(top + bottom) / (top - bottom);

//#if GLM_DEPTH_CLIP_SPACE == GLM_DEPTH_ZERO_TO_ONE
      result[2][2] = -1.0f / (zFar - zNear);
      result[3][2] = -zNear / (zFar - zNear);
//#else
//      result[2][2] = - static_cast<T>(2) / (zFar - zNear);
//      result[3][2] = - (zFar + zNear) / (zFar - zNear);
//#endif

		return result;
  }

  inline mat4 ortho(const float &left, const float &right, const float &bottom, const float &top) {
    mat4 result(1);
    result[0][0] = 2.0f / (right - left);
    result[1][1] = 2.0f / (top - bottom);
    result[2][2] = -1.0f;
    result[3][0] = -(right + left) / (right - left);
    result[3][1] = -(top + bottom) / (top - bottom);
    return result;
  }

  inline mat4 frustum(const float &left, const float &right, const float &bottom, const float &top, const float &zNear, const float &zFar) {
    mat4 result(0);
    result[0][0] = (2.0f * zNear) / (right - left);
    result[1][1] = (2.0f * zNear) / (top - bottom);
    result[2][0] = (right + left) / (right - left);
    result[2][1] = (top + bottom) / (top - bottom);
    result[2][3] = -1.0f;

//#if GLM_DEPTH_CLIP_SPACE == GLM_DEPTH_ZERO_TO_ONE
      result[2][2] = zFar / (zNear - zFar);
      result[3][2] = -(zFar * zNear) / (zFar - zNear);
//#else
//      result[2][2] = - (zFar + zNear) / (zFar - zNear);
//      result[3][2] = - (static_cast<T>(2) * zFar * zNear) / (zFar - zNear);
//#endif

    return result;
  }

//   inline mat4 perspective(const float &fovy, const float &aspect, const float &near, const float &far) {
//     const float tanHalfFovy = std::tan(fovy / 2.0f);
//
// 	  const float far1 = far;
// 	  const float near1 = near;
//
//     mat4 result(0.0f);
//     result[0][0] = 1.0f / (aspect * tanHalfFovy);
//     result[1][1] = 1.0f / (tanHalfFovy);
//     result[2][3] = -1.0f;
//
// //#if GLM_DEPTH_CLIP_SPACE == GLM_DEPTH_ZERO_TO_ONE
//       result[2][2] = far1 / (near1 - far1);
//       result[3][2] = -(far1 * near1) / (far1 - near1);
// //#else
// //      result[2][2] = -(far + near) / (far - near);
// //      result[3][2] = -(2.0f * far * near) / (far - near);
// //#endif
//
//     return result;
//   }

//   inline mat4 perspectiveFov(const float &fov, const float &width, const float &height, const float &near, const float &far) {
//     const float rad = fov;
//     const float h = ::cos(0.5f * rad) / ::sin(0.5f * rad);
//     const float w = h * height / width; ///todo max(width , Height) / min(width , Height)?
//
//     mat4 result(0.0f);
//     result[0][0] = w;
//     result[1][1] = h;
//     result[2][3] = -1.0f;
//
// //#if GLM_DEPTH_CLIP_SPACE == GLM_DEPTH_ZERO_TO_ONE
//       result[2][2] = far / (near - far);
//       result[3][2] = -(far * near) / (far - near);
// //#else
// //      result[2][2] = -(far + near) / (far - near);
// //      result[3][2] = -(2.0f * far * near) / (far - near);
// //#endif
//
//     return result;
//   }

  inline vec4 project(const vec4 &obj, const mat4 &model, const mat4 &proj, const vec4 &viewport) {
    vec4 tmp = vec4(obj.x, obj.y, obj.z, 1.0f);
    tmp = model * tmp;
    tmp = proj * tmp;

    tmp /= tmp.w;

//#if GLM_DEPTH_CLIP_SPACE == GLM_DEPTH_ZERO_TO_ONE
      tmp.x = tmp.x * 0.5f + 0.5f;
      tmp.y = tmp.y * 0.5f + 0.5f;
//#else
//      tmp = tmp * 0.5 + 0.5;
//#endif

    tmp[0] = tmp[0] * float(viewport[2]) + float(viewport[0]);
    tmp[1] = tmp[1] * float(viewport[3]) + float(viewport[1]);

    return tmp;
  }

  inline vec4 unProject(const vec4 &win, const mat4 &model, const mat4 &proj, const vec4 &viewport) {
    const mat4 inverseMat = inverse(proj * model);

    vec4 tmp = vec4(win.x, win.y, win.y, 1.0f);
    tmp.x = (tmp.x - float(viewport[0])) / float(viewport[2]);
    tmp.y = (tmp.y - float(viewport[1])) / float(viewport[3]);
//#if GLM_DEPTH_CLIP_SPACE == GLM_DEPTH_ZERO_TO_ONE
      tmp.x = tmp.x * 2.0f - 1.0f;
      tmp.y = tmp.y * 2.0f - 1.0f;
//#else
//      tmp = tmp * 2.0f - 1.0f;
//#endif

    vec4 obj = inverseMat * tmp;
    obj /= obj.w;

    return obj;
  }

  inline mat4 pickMatrix(const float &centerX, const float &centerY, const float &deltaX, const float &deltaY, const vec4 &viewport) {
    ASSERT(deltaX > 0.0f && deltaY > 0.0f);
    mat4 result(1.0f);

    if (!(deltaX > 0.0f && deltaY > 0.0f)) return result; // Error

    const vec4 temp(
      (viewport[2] - 2.0f * (centerX - viewport[0])) / deltaX,
      (viewport[3] - 2.0f * (centerY - viewport[1])) / deltaY,
      0.0f, 0.0f
    );

    // Translate and scale the picked region to the entire window
    result = translate(result, temp);
    return scale(result, vec4(viewport[2] / deltaY, viewport[3] / deltaY, 1.0f, 1.0f));
  }

//   mat4 lookAt(const vec4 &eye, const vec4 &center, const vec4 &up) {
//     const vec4 f(normalize(center - eye));
//     const vec4 s(normalize(cross(f, up)));
//     const vec4 u(cross(s, f));
//
//     mat4 result(1.0f);
//     result[0][0] = s.x;
//     result[1][0] = s.y;
//     result[2][0] = s.z;
//     result[0][1] = u.x;
//     result[1][1] = u.y;
//     result[2][1] = u.z;
//     result[0][2] =-f.x;
//     result[1][2] =-f.y;
//     result[2][2] =-f.z;
//     result[3][0] =-dot(s, eye);
//     result[3][1] =-dot(vec4(u.x, u.y, u.z, 0.0f), eye);
//     result[3][2] = dot(f, eye);
//     return result;
//   }
#endif

#ifdef __SSSE3__
  inline ivec4 hadd(const ivec4 &a, const ivec4 &b) {
    return _mm_hadd_epi32(a, b);
  }

  inline ivec4 hsub(const ivec4 &a, const ivec4 &b) {
    return _mm_hsub_epi32(a, b);
  }

  inline ivec4 abs(const ivec4 &vec) {
    return _mm_abs_epi32(vec);
  }
#endif

#ifdef __SSE4_1__
  inline vec4 ceil(const vec4 &vec) {
    return _mm_ceil_ps(vec);
  }

  inline vec4 floor(const vec4 &vec) {
    return _mm_floor_ps(vec);
  }

  inline ivec4 max(const ivec4 &vec1, const ivec4 &vec2) {
    return _mm_max_epi32(vec1, vec2);
  }

  inline ivec4 min(const ivec4 &vec1, const ivec4 &vec2) {
    return _mm_min_epi32(vec1, vec2);
  }
#endif

}
