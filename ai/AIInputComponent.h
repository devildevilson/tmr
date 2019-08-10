#ifndef AI_INPUT_COMPONENT_H
#define AI_INPUT_COMPONENT_H

#include "Utility.h"
#include "TransformComponent.h"
#include "PhysicsComponent.h"
#include "InputComponent.h"

class RawPath;

class AIInputComponent : public InputComponent {
public:
  struct CreateInfo {
    PhysicsComponent* physics;
    TransformComponent* trans;
  };
  AIInputComponent(const CreateInfo &info);
  ~AIInputComponent();

  simd::vec4 predictPos(const size_t &predictionTime) const;

  void seek(const simd::vec4 &target);
  void flee(const simd::vec4 &target);
  simd::vec4 followPath(const size_t &predictionTime, const RawPath* path, const size_t &currentPathSegmentIndex);
  void stayOnPath(const size_t &predictionTime, const RawPath* path);

  void setPhysicsComponent(PhysicsComponent* physics);
private:
  PhysicsComponent* physics;
  TransformComponent* trans;
};

#endif //AIINPUTCOMPONENT_H
