#ifndef SIMD_H
#define SIMD_H

/**
 * библиотечка для работы с simd
 * сделана для замены glm в некоторых местах
 * по идее такая структура должна быть хорошо копируема на гпу
 * я еще не разобрался как правильно обрабатывать булевы значения
 *
 */

#include <x86intrin.h>
#include <cstdint>

// нужно инлайны засунуть в отдельный дейфайн
// чтобы удобно было смотреть ассемблер

namespace simd {
#ifdef __SSE2__
  struct alignas(16) ivec4;
#endif

#ifdef __SSE3__
  struct mat4;
#endif

#ifdef __SSE__
  struct alignas(16) vec4 {
    union {
      struct {
        float x, y, z, w;
      };

      struct {
        float r, g, b, a;
      };

      float arr[4];

      __m128 native;
    };

    inline vec4();
    inline vec4(const vec4 &vec);
    inline vec4(const float &x, const float &y, const float &z, const float &w);
    inline vec4(const float &f1);
    inline vec4(const float* ptr);

#ifdef __SSE2__
    inline vec4(const ivec4 &vec);
#endif

    inline vec4(__m128 native);
    inline ~vec4();

    inline constexpr uint32_t length() const { return 4; }
    inline int movemask() const;
    inline float hsum() const;
    inline vec4 hsum_vec() const;
    inline vec4 aproximate_inverse();
    inline vec4 aproximate_inverse_sqrt();
    inline vec4 & abs();
    inline void load(const float* p);
    inline void store(float* p) const;
    inline vec4 & operator-= (const vec4 &rhs);
    inline vec4 & operator+= (const vec4 &rhs);
    inline vec4 & operator*= (const vec4 &rhs);
    inline vec4 & operator/= (const vec4 &rhs);
    inline vec4 & operator-=(const float &value);
    inline vec4 & operator+=(const float &value);
    inline vec4 & operator*=(const float &value);
    inline vec4 & operator/=(const float &value);
    inline vec4 & operator--();
    inline vec4 & operator++();
    inline vec4 operator--(int);
    inline vec4 operator++(int);
    inline vec4 & operator~();
    inline vec4 & operator&=(const vec4 &vec);
    inline vec4 & operator|=(const vec4 &vec);
    inline vec4 & operator^=(const vec4 &vec);
    inline operator __m128() const;
    inline vec4 & operator=(const vec4 &vec);

#ifdef __SSE2__
    inline vec4 & operator=(const ivec4 &vec);
#endif

    inline vec4 & operator=(__m128 x);
    inline float & operator[](const uint32_t &index);
    inline const float & operator[](const uint32_t &index) const;
  };

  inline vec4 operator-(const vec4 &vec);
  inline vec4 operator-(const vec4 &vec1, const vec4 &vec2);
  inline vec4 operator+(const vec4 &vec1, const vec4 &vec2);
  inline vec4 operator*(const vec4 &vec1, const vec4 &vec2);
  inline vec4 operator/(const vec4 &vec1, const vec4 &vec2);
  inline vec4 operator-(const vec4 &vec1, const float &value);
  inline vec4 operator+(const vec4 &vec1, const float &value);
  inline vec4 operator*(const vec4 &vec1, const float &value);
  inline vec4 operator/(const vec4 &vec1, const float &value);
  inline vec4 operator-(const float &value, const vec4 &vec1);
  inline vec4 operator+(const float &value, const vec4 &vec1);
  inline vec4 operator*(const float &value, const vec4 &vec1);
  inline vec4 operator/(const float &value, const vec4 &vec1);
  inline vec4 operator>(const vec4 &vec1, const vec4 &vec2);
  inline vec4 operator>=(const vec4 &vec1, const vec4 &vec2);
  inline vec4 operator<(const vec4 &vec1, const vec4 &vec2);
  inline vec4 operator<=(const vec4 &vec1, const vec4 &vec2);
  inline vec4 operator==(const vec4 &vec1, const vec4 &vec2);
  inline vec4 operator!=(const vec4 &vec1, const vec4 &vec2);
  inline vec4 operator&(const vec4 &vec1, const vec4 &vec2);
  inline vec4 operator|(const vec4 &vec1, const vec4 &vec2);
  inline vec4 operator^(const vec4 &vec1, const vec4 &vec2);

  inline vec4 andnot(const vec4 &vec1, const vec4 &vec2);
  inline vec4 max(const vec4 &vec1, const vec4 &vec2);
  inline vec4 min(const vec4 &vec1, const vec4 &vec2);
  inline vec4 abs(const vec4 &vec);
  inline vec4 sqrt(const vec4 &vec);
  
  inline vec4 equal(const vec4 &a, const vec4 &b, const float &epsilon);
  inline bool all(const vec4 &vec);
  inline bool any(const vec4 &vec);
  inline bool all_xyz(const vec4 &vec);
  inline bool any_xyz(const vec4 &vec);
  
  inline float hsum(const vec4 &vec);
  inline vec4 hsum_vec(const vec4 &vec);
  
  inline vec4 mix(const vec4 &x, const vec4 &y, const float &a);
//   inline vec4 lerp(const vec4 &x, const vec4 &y, const float &a);
  
  // 12 инструкций
  inline vec4 cross(const vec4 &a, const vec4 &b);

  inline float dot(const vec4 &vec1, const vec4 &vec2);

  // 33 инструкции (ох уж этот _MM_TRANSPOSE4_PS)
  inline vec4 dot4(vec4 &vec11, vec4 &vec12, vec4 &vec13, vec4 &vec14,
                   vec4 &vec21, vec4 &vec22, vec4 &vec23, vec4 &vec24);

  inline vec4 dot4(const vec4 &xxxx1, const vec4 &yyyy1, const vec4 &zzzz1,
                   const vec4 &xxxx2, const vec4 &yyyy2, const vec4 &zzzz2);

  // 8 инструкций
  inline vec4 dot4_no_transpose(const vec4 &mat11, const vec4 &mat12, const vec4 &mat13, const vec4 &mat14,
                                const vec4 &mat21, const vec4 &mat22, const vec4 &mat23, const vec4 &mat24);
  // 8 инструкций
  inline float length(const vec4 &vec);
  // 9 инструкций
  inline vec4 length_vec(const vec4 &vec);
  // 7 инструкций
  inline float length2(const vec4 &vec);
  // 8 инструкций
  inline vec4 length2_vec(const vec4 &vec);
  // 9 инструкций
  inline vec4 length4(const vec4 &vec1, const vec4 &vec2, const vec4 &vec3, const vec4 &vec4);
  inline vec4 length4_no_transpose(const vec4 &vec1, const vec4 &vec2, const vec4 &vec3, const vec4 &vec4);
  inline vec4 length4(const vec4 &xxxx, const vec4 &yyyy, const vec4 &zzzz);

  inline vec4 length24(const vec4 &vec1, const vec4 &vec2, const vec4 &vec3, const vec4 &vec4);
  inline vec4 length24_no_transpose(const vec4 &vec1, const vec4 &vec2, const vec4 &vec3, const vec4 &vec4);
  inline vec4 length24(const vec4 &xxxx, const vec4 &yyyy, const vec4 &zzzz);

  inline vec4 normalize(const vec4 &vec);
  inline void normalize4(vec4 &xxxx, vec4 &yyyy, vec4 &zzzz); // тут по идее сгенерятся инструкции для ссылок
  struct norm_output { vec4 xxxx; vec4 yyyy; vec4 zzzz; };
  inline norm_output normalize4(const vec4 &xxxx, const vec4 &yyyy, const vec4 &zzzz);

  inline float distance(const vec4 &a, const vec4 &b);
  inline vec4 distance4(const vec4 &xxxx1, const vec4 &yyyy1, const vec4 &zzzz1,
                        const vec4 &xxxx2, const vec4 &yyyy2, const vec4 &zzzz2);

  inline float distance2(const vec4 &a, const vec4 &b);
  inline vec4 distance24(const vec4 &xxxx1, const vec4 &yyyy1, const vec4 &zzzz1,
                         const vec4 &xxxx2, const vec4 &yyyy2, const vec4 &zzzz2);

  // normalize4 по всей видимости будет медленее чем 4 normalize подряд

  inline int movemask(const vec4 &vec);
  // как назвать это дело?
  // 0xffffffff если оба не НаН
  inline vec4 notnan(const vec4 &a, const vec4 &b);
  // 0xffffffff если хотя бы один НаН
  inline vec4 nan(const vec4 &a, const vec4 &b);
  inline vec4 aproximate_inverse(const vec4 &vec);
  inline vec4 aproximate_inverse_sqrt(const vec4 &vec);
  inline vec4 movehl(const vec4 &a, const vec4 &b);
  inline vec4 movelh(const vec4 &a, const vec4 &b);

//   inline constexpr vec4 shuffle(const vec4 &a, const vec4 &b, const uint32_t &x, const uint32_t &y, const uint32_t &z, const uint32_t &w) {
//     return vec4(_mm_shuffle_ps(a, b, _MM_SHUFFLE(x, y, z, w)));
//   }

  #define simd_shuffle(a, b, x, y, z, w) simd::vec4(_mm_shuffle_ps(a, b, _MM_SHUFFLE(x, y, z, w)))
  #define simd_swizzle(a, x, y, z, w)    simd::vec4(_mm_shuffle_ps(a, a, _MM_SHUFFLE(x, y, z, w)))
  #define simd_swizzle1(a, x)            simd::vec4(_mm_shuffle_ps(a, a, _MM_SHUFFLE(x, x, x, x)))

  template<uint32_t x, uint32_t y, uint32_t z, uint32_t w>
  inline vec4 shuffle(const vec4 &v1, const vec4 &v2) {
    return vec4(_mm_shuffle_ps(v1, v2, _MM_SHUFFLE(x, y, z, w)));
  }

  template<uint32_t x, uint32_t y, uint32_t z, uint32_t w>
  inline vec4 swizzle(const vec4 &v) {
    return vec4(_mm_shuffle_ps(v, v, _MM_SHUFFLE(x, y, z, w)));
  }

  template<uint32_t x>
  inline vec4 swizzle1(const vec4 &v) {
    return vec4(_mm_shuffle_ps(v, v, _MM_SHUFFLE(x, x, x, x)));
  }

  inline vec4 blendv(const vec4 &a, const vec4 &b, const vec4 &i);

  struct alignas(16) quat {
    union {
      struct {
        float w, x, y, z;
      };

      float arr[4];

      __m128 native;
    };

    inline quat();
    inline quat(const quat &q);

#ifdef __SSE3__
    inline quat(const mat4 &mat);
#endif

    inline quat(const float &w, const float &x, const float &y, const float &z);
    inline quat(const vec4 &a, const vec4 &b);
    inline quat(const float* ptr);
    inline quat(__m128 native);
//     inline ~quat();

    inline constexpr uint32_t length() const { return 4; }

    inline void load(const float* p);
    inline void store(float* p) const;

    inline quat & operator-=(const quat &rhs);
    inline quat & operator+=(const quat &rhs);
    inline quat & operator*=(const quat &rhs);
    inline quat & operator*=(const float &value);
    inline quat & operator/=(const float &value);

    inline operator __m128() const;
#ifdef __SSE3__
    inline operator mat4() const;
#endif

    inline quat & operator=(const quat &q);
#ifdef __SSE3__
    inline quat & operator=(const mat4 &mat);
#endif
    inline quat & operator=(__m128 x);
    inline float & operator[](const uint32_t &index);
    inline const float & operator[](const uint32_t &index) const;
  };

  // можно ли так обозначить conjugate? в глм никто так не делает
  inline quat operator-(const quat &q);
  inline quat operator-(const quat &a, const quat &b);
  inline quat operator+(const quat &a, const quat &b);
  inline quat operator*(const quat &a, const quat &b);
  inline quat operator*(const quat &a, const float &value);
  inline quat operator/(const quat &a, const float &value);
  inline quat operator*(const float &value, const quat &a);
  inline quat operator/(const float &value, const quat &a);
  inline vec4 operator*(const quat &q, const vec4 &v);
  inline vec4 operator*(const vec4 &v, const quat &q);

  inline quat conjugate(const quat &q);
  inline quat inverse(const quat &q);
  inline float dot(const quat &a, const quat &b);
  inline float length(const quat &q);
  inline quat normalize(const quat &q);

  quat mix(const quat &x, const quat &y, const float &a);
  inline quat lerp(const quat &x, const quat &y, const float &a);
  quat slerp(const quat &x, const quat &y, const float &a);

  quat rotate(const quat &q, const float &angle, const vec4 &axis);
  inline vec4 eulerAngles(const quat &q);

  float roll(const quat &q);
  float pitch(const quat &q);
  float yaw(const quat &q);

  inline float angle(const quat &q);
  vec4 axis(const quat &q);
  quat angleAxis(const float &angle, const vec4 &axis);

  mat4 cast(const quat &q);
  quat cast(const mat4 &mat);

  template<uint32_t x, uint32_t y, uint32_t z, uint32_t w>
  inline quat shuffle(const quat &v1, const quat &v2) {
    return quat(_mm_shuffle_ps(v1, v2, _MM_SHUFFLE(x, y, z, w)));
  }

  template<uint32_t x, uint32_t y, uint32_t z, uint32_t w>
  inline quat swizzle(const quat &v) {
    return quat(_mm_shuffle_ps(v, v, _MM_SHUFFLE(x, y, z, w)));
  }

  template<uint32_t x>
  inline quat swizzle1(const quat &v) {
    return quat(_mm_shuffle_ps(v, v, _MM_SHUFFLE(x, x, x, x)));
  }

  // судя по предыдущим записям здесь сгерерируется 12 инструкций
  inline bool collision_aabb(const vec4 &pos1, const vec4 &extents1, const vec4 &pos2, const vec4 &extents2);
#endif

#ifdef __SSE2__
  struct alignas(16) ivec4 {
    union {
      struct {
        int32_t x, y, z, w;
      };

      struct {
        int32_t r, g, b, a;
      };

      int32_t arr[4];

      __m128i native;
    };

    inline ivec4();
    inline ivec4(const int32_t &x, const int32_t &y, const int32_t &z, const int32_t &w);
    inline ivec4(const int32_t &x);
    inline ivec4(const ivec4 &vec);
    inline ivec4(const vec4 &vec);
    inline ivec4(__m128i native);
    inline ~ivec4();

    inline constexpr uint32_t length() const { return 4; }

#ifdef __SSSE3__
    inline ivec4 & hadd(const ivec4 &vec);
    inline ivec4 & hsub(const ivec4 &vec);
    inline ivec4 & abs();
#endif

    inline ivec4 & operator-();
    inline ivec4 & operator-=(const ivec4 &rhs);
    inline ivec4 & operator+=(const ivec4 &rhs);
    inline ivec4 & operator*=(const ivec4 &rhs);

    // деления целых чисел нет :(
//     inline ivec4 & operator/= (const ivec4 &rhs)

    inline ivec4 & operator-=(const int32_t &value);
    inline ivec4 & operator+=(const int32_t &value);
    inline ivec4 & operator*=(const int32_t &value);

    // деления целых чисел нет :(
//     inline ivec4 & operator/= (const int32_t &value);

    inline ivec4 & operator--();
    inline ivec4 & operator++();
    inline ivec4 operator--(int);
    inline ivec4 operator++(int);
    inline ivec4 & operator~();
    inline ivec4 & operator|=(const ivec4 &vec);
    inline ivec4 & operator^=(const ivec4 &vec);
    inline ivec4 & operator&=(const ivec4 &vec);
    inline ivec4 & operator<<=(const int32_t &shift);
    inline ivec4 & operator>>=(const int32_t &shift);
    inline operator __m128i() const;
    inline ivec4 & operator=(const ivec4 &vec);
    inline ivec4 & operator=(const vec4 &vec);
    inline ivec4 & operator=(__m128i x);
    inline int32_t & operator[](const uint32_t &index);
    inline const int32_t & operator[](const uint32_t &index) const;
  };

  inline ivec4 operator-(const ivec4 &vec1, const ivec4 &vec2);
  inline ivec4 operator+(const ivec4 &vec1, const ivec4 &vec2);
  inline ivec4 operator*(const ivec4 &vec1, const ivec4 &vec2);
  // деления целых чисел нет :(
//     inline ivec4 operator/(const ivec4 &vec) const;
  inline ivec4 operator-(const ivec4 &vec, const int32_t &value);
  inline ivec4 operator+(const ivec4 &vec, const int32_t &value);
  inline ivec4 operator*(const ivec4 &vec, const int32_t &value);

  inline ivec4 operator-(const int32_t &value, const ivec4 &vec);
  inline ivec4 operator+(const int32_t &value, const ivec4 &vec);
  inline ivec4 operator*(const int32_t &value, const ivec4 &vec);

  inline ivec4 div(const ivec4 &vec, const int32_t &value);
  inline ivec4 div(const ivec4 &vec1, const ivec4 &vec2);

  // деления целых чисел нет :(
//     inline ivec4 operator/(const int32_t &value) const;
  inline ivec4 operator|(const ivec4 &vec1, const ivec4 &vec2);
  inline ivec4 operator>(const ivec4 &vec1, const ivec4 &vec2);
  inline ivec4 operator>=(const ivec4 &vec1, const ivec4 &vec2);
  inline ivec4 operator<(const ivec4 &vec1, const ivec4 &vec2);
  // этого оператора нет (сработает ли это?)
  inline ivec4 operator<=(const ivec4 &vec1, const ivec4 &vec2);
  inline ivec4 operator==(const ivec4 &vec1, const ivec4 &vec2);
  inline ivec4 operator!=(const ivec4 &vec1, const ivec4 &vec2);
  inline ivec4 operator&(const ivec4 &vec1, const ivec4 &vec2);
  inline ivec4 operator^(const ivec4 &vec1, const ivec4 &vec2);
  inline ivec4 operator<<(const ivec4 &vec, const int32_t &shift);
  inline ivec4 operator>>(const ivec4 &vec, const int32_t &shift);

  inline ivec4 andnot(const ivec4 &vec1, const ivec4 &vec2);

  inline vec4 cast(const ivec4 &vec);
  inline ivec4 cast(const vec4 &vec);
  inline ivec4 truncate(const vec4 &vec);

  // 56 строк
  vec4 log(const vec4 &vec);
  // 45 строк
  vec4 exp(const vec4 &vec);
  // 62 строки
  vec4 sin(const vec4 &vec);
  // 66 строк
  vec4 cos(const vec4 &vec);
  // 63 строки (меньше чем у косинуса?)
  void sincos(const vec4 &input, vec4 &s, vec4 &c);
#endif

#ifdef __SSE3__
  inline vec4 addsub(const vec4 &a, const vec4 &b);
  inline vec4 hadd(const vec4 &a, const vec4 &b);
  inline vec4 hsub(const vec4 &a, const vec4 &b);

  struct alignas(16) mat4 {
  public:
    inline mat4();
    inline mat4(const float &a);

    inline mat4(const vec4 &first, const vec4 &second, const vec4 &third, const vec4 &forth);
    inline mat4(const float &a11, const float &a12, const float &a13, const float &a14,
                const float &a21, const float &a22, const float &a23, const float &a24,
                const float &a31, const float &a32, const float &a33, const float &a34,
                const float &a41, const float &a42, const float &a43, const float &a44);

    inline mat4(const mat4 &mat);
    inline mat4(const quat &q);
    inline ~mat4();

    inline constexpr uint32_t length() const { return 4; }
    inline vec4 & operator[](const uint32_t &index);
    inline const vec4 & operator[](const uint32_t &index) const;

    inline mat4 & operator=(const mat4 &mat);
    inline mat4 & operator=(const quat &q);

    inline mat4 & operator-=(const mat4 &mat);
    inline mat4 & operator-=(const float &a);
    inline mat4 & operator+=(const mat4 &mat);
    inline mat4 & operator+=(const float &a);
    inline mat4 & operator/=(const mat4 &mat);
    inline mat4 & operator/=(const float &a);
    inline mat4 & operator*=(const mat4 &mat);
    inline mat4 & operator*=(const float &a);
  private:
    vec4 value[4];
  };

  inline mat4 operator-(const mat4 &mat1, const mat4 &mat2);
  inline mat4 operator-(const mat4 &mat, const float &a);
  inline mat4 operator-(const float &a, const mat4 &mat);
  inline mat4 operator+(const mat4 &mat1, const mat4 &mat2);
  inline mat4 operator+(const mat4 &mat, const float &a);
  inline mat4 operator+(const float &a, const mat4 &mat);
  inline mat4 operator/(const mat4 &mat1, const mat4 &mat2);
  inline mat4 operator/(const mat4 &mat, const float &a);
  inline mat4 operator/(const float &a, const mat4 &mat);

  // нашел здесь http://fhtr.blogspot.com/2010/02/4x4-float-matrix-multiplication-using.html
  inline mat4 operator*(const mat4 &mat1, const mat4 &mat2);
  inline mat4 operator*(const mat4 &mat, const float &a);
  inline mat4 operator*(const float &a, const mat4 &mat);

  inline vec4 operator*(const mat4 &mat, const vec4 &vec);
  inline vec4 operator*(const vec4 &vec, const mat4 &mat);

  // только для трансформ матриц (56 строк)
  mat4 inverse_transform(const mat4 &mat);
  inline vec4 mat2Mul(const vec4 &mat1, const vec4 &mat2);
  inline vec4 mat2AdjMul(const vec4 &mat1, const vec4 &mat2);
  inline vec4 mat2MulAdj(const vec4 &mat1, const vec4 &mat2);

  // для всех, нашел здесь (https://lxjk.github.io/2017/09/03/Fast-4x4-Matrix-Inverse-with-SSE-SIMD-Explained.html)
  // 121 строка
  mat4 inverse(const mat4 &mat);

  inline mat4 transpose(const mat4 &mat);

  inline mat4 translate(const mat4 &mat, const vec4 &vec);
  // rotate скорее всего получится неоправданно большим
  // 106 строк, и все в основном потому что получение одного числа из регистра это дорого
  mat4 rotate(const mat4 &mat, const float &angle, const vec4 &axis);
  inline mat4 scale(const mat4 &mat, const vec4 &vec);
  
  inline mat4 orientation(const vec4 &normal, const vec4 &up);
  
  inline mat4 ortho(const float &left, const float &right, const float &bottom, const float &top, const float &zNear, const float &zFar);
//   mat4 orthoLH(const float &left, const float &right, const float &bottom, const float &top, const float &zNear, const float &zFar);
//   mat4 orthoRH(const float &left, const float &right, const float &bottom, const float &top, const float &zNear, const float &zFar);
  inline mat4 ortho(const float &left, const float &right, const float &bottom, const float &top);
  inline mat4 frustum(const float &left, const float &right, const float &bottom, const float &top, const float &zNear, const float &zFar);
//   mat4 frustumLH(const float &left, const float &right, const float &bottom, const float &top, const float &zNear, const float &zFar);
//   mat4 frustumRH(const float &left, const float &right, const float &bottom, const float &top, const float &zNear, const float &zFar);

  // TODO: bug with 'far' and 'near' on windows g++
  inline mat4 perspective(const float &fovy, const float &aspect, const float &near, const float &far) {
    const float tanHalfFovy = std::tan(fovy / 2.0f);
  
	  const float far1 = far;
	  const float near1 = near;
  
    mat4 result(0.0f);
    result[0][0] = 1.0f / (aspect * tanHalfFovy);
    result[1][1] = 1.0f / (tanHalfFovy);
    result[2][3] = -1.0f;
    result[2][2] = far1 / (near1 - far1);
    result[3][2] = -(far1 * near1) / (far1 - near1);
  
    return result;
  }
  
  inline mat4 perspectiveFov(const float &fov, const float &width, const float &height, const float &near, const float &far) {
    const float rad = fov;
    const float h = ::cos(0.5f * rad) / ::sin(0.5f * rad);
    const float w = h * height / width; ///todo max(width , Height) / min(width , Height)?
  
    mat4 result(0.0f);
    result[0][0] = w;
    result[1][1] = h;
    result[2][3] = -1.0f;
    result[2][2] = far / (near - far);
    result[3][2] = -(far * near) / (far - near);
  
    return result;
  }

  inline vec4 project(const vec4 &obj, const mat4 &model, const mat4 &proj, const vec4 &viewport);
  inline vec4 unProject(const vec4 &win, const mat4 &model, const mat4 &proj, const vec4 &viewport);

  inline mat4 pickMatrix(const float &centerX, const float &centerY, const float &deltaX, const float &deltaY, const vec4 &viewport);

  // взятие из симда одной составляющей преобразуется обычно в какой-то кошмар
  // думаю что здесь от инлайна надо отказаться (82 инструкции)
  mat4 lookAt(const vec4 &eye, const vec4 &center, const vec4 &up);
#endif

#ifdef __SSSE3__
  inline ivec4 hadd(const ivec4 &a, const ivec4 &b);
  inline ivec4 hsub(const ivec4 &a, const ivec4 &b);
  inline ivec4 abs(const ivec4 &vec);
#endif

#ifdef __SSE4_1__
  inline vec4 ceil(const vec4 &vec);
  inline vec4 floor(const vec4 &vec);
  inline ivec4 max(const ivec4 &vec1, const ivec4 &vec2);
  inline ivec4 min(const ivec4 &vec1, const ivec4 &vec2);
#endif

  // sse4.2 не добавила особо полезных инструкций
  // дальше идут avx инструкции, в чем их польза?
  // они добавляют 256 бит числа (вектор из 8 float'ов или 4 даубла)
  // есть еще FMA инструкции, которые добавляют несколько хороших арифметических инструкций
  // avx512 добавляет соответственно 512 бит число, котрое может быть матрицей 4х4
  // это полезно
}

#include "simd.inl"

#endif

#ifdef SIMD_IMPLEMENTATION
#include "simd.impl.inl"
#endif
