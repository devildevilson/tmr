#ifndef PHYSICS_TEMPORARY_H
#define PHYSICS_TEMPORARY_H

#include <cstdint>
// #include <glm/glm.hpp>

#include "Utility.h"

#define EPSILON 0.000001f

#define PI   3.1415926535897932384626433832795
#define PI_2 6.28318530717958647693
#define PI_H 1.57079632679489661923
#define PI_Q 0.78539816339744830962
#define PI_E (PI / 8)

#define MCS_TO_SEC(dt) (float(dt) / 1000000.0f)

#define DEG_TO_RAD(deg) ((deg * PI) / 180.0f)
#define RAD_TO_DEG(rad) ((rad * 180.0f) / PI)

struct RotationData {
  glm::vec3 anchorDir;
  float anchorDist;
  // или тут все же позиция якоря должна быть?
  
  glm::vec4 rotationNormal;

  float currentAngle;
  float maxAngle; // нужен ли минимальный угол? или он всегда 0?
  float rotationSpeed;
  uint32_t stepTime; // нужно ли?

  glm::mat4 matrix;
};

struct Transform {
  glm::vec4 pos;
  //glm::mat4 orn;
  glm::vec4 rot;
  glm::vec4 scale;
};

// наверное лучше если мы отдельно сделаем, либо одно копирование
// либо +1 bool
// struct TransformStates {
//   Transform prev;
//   Transform curr;
// };

struct InputData {
  glm::vec4 right;
  glm::vec4 up;
  glm::vec4 front;
  glm::vec4 moves;
};

struct BroadphasePair {
  // что должно быть здесь?
  uint32_t firstIndex;
  uint32_t secondIndex;
  float dist;
  uint32_t islandIndex;
  // мне кажется этого достаточно для перехода от броадфазы к наровфазе
};

struct PlayerData {
  glm::vec4 pos;
  //glm::vec4 rot;
};

#endif // !PHYSICS_TEMPORARY_H
