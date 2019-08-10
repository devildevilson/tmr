#ifndef PHYSICS_TEMPORARY_H
#define PHYSICS_TEMPORARY_H

#include <cstdint>

#include "Utility.h"

struct RotationData {
  glm::vec3 anchorDir;
  float anchorDist;
  // или тут все же позиция якоря должна быть?
  
  simd::vec4 rotationNormal;

  float currentAngle;
  float maxAngle; // нужен ли минимальный угол? или он всегда 0?
  float rotationSpeed;
  uint32_t stepTime; // нужно ли?

  simd::mat4 matrix;
};

struct Transform {
  simd::vec4 pos;
  //simd::mat4 orn;
  simd::vec4 rot;
  simd::vec4 scale;
};

// наверное лучше если мы отдельно сделаем, либо одно копирование
// либо +1 bool
// struct TransformStates {
//   Transform prev;
//   Transform curr;
// };

struct InputData {
  simd::vec4 right;
  simd::vec4 up;
  simd::vec4 front;
  simd::vec4 moves;
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
  simd::vec4 pos;
  //simd::vec4 rot;
};

#endif // !PHYSICS_TEMPORARY_H
