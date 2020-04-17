#ifndef SHARED_MATHEMATICAL_CONSTANTS_H
#define SHARED_MATHEMATICAL_CONSTANTS_H

#define PI   3.1415926535897932384626433832795
#define PI_2 (PI * 2.0)     /* 6.28318530717958647693 */
#define PI_H (PI / 2.0)     /* 1.57079632679489661923 */
#define PI_Q (PI / 4.0)     /* 0.78539816339744830962 */
#define PI_E (PI / 8.0)

#define DEG_TO_RAD(deg) ((deg * PI) / 180.0)
#define RAD_TO_DEG(rad) ((rad * 180.0) / PI)

#define EPSILON 0.000001f
#define PASSABLE_ANGLE_DEGREE 45.0f
//#define PASSABLE_ANGLE glm::radians(PASSABLE_ANGLE_DEGREE)
#define PASSABLE_ANGLE DEG_TO_RAD(PASSABLE_ANGLE_DEGREE)

#ifndef UINT32_MAX
  #define UINT32_MAX 0xFFFFFFFF
#endif

//#ifdef __cplusplus
//
//#endif

#endif
