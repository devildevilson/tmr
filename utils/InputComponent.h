#ifndef INPUT_COMPONENT_H
#define INPUT_COMPONENT_H

#include "Utility.h"
#include "ArrayInterface.h"
#include "PhysicsTemporary.h"
#include "TransformComponent.h"

class InputComponent {
public:
  static void setContainer(Container<InputData>* container);

  InputComponent();
  ~InputComponent();

  simd::vec4 & front();
  simd::vec4 & up();
  simd::vec4 & right();
  simd::vec4 & movementData();
  const simd::vec4 & front() const;
  const simd::vec4 & up() const;
  const simd::vec4 & right() const;
  const simd::vec4 & movementData() const;

  void setMovement(const float &front, const float &up, const float &right);

  uint32_t inputIndex;

  static Container<InputData>* container;
};

class UserInputComponent : public InputComponent {
public:
  struct CreateInfo {
    TransformComponent* trans;
  };
  UserInputComponent(const CreateInfo &info);
  ~UserInputComponent();

  void mouseMove(const float &horisontalAngle, const float &verticalAngle, const simd::mat4 &transform);

  void forward();
  void backward();
  void left();
  void right();
  void jump();
private:
  float horisontalAngleSum, verticalAngleSum;

//  StateController* states;
//   InputComponent* input = nullptr;
  TransformComponent* trans;

  glm::vec3 rotation;
};

#endif //INPUT_COMPONENT_H
