#include "InputComponent.h"

#include "Globals.h"

void InputComponent::setContainer(Container<InputData>* container) {
  InputComponent::container = container;
}

InputComponent::InputComponent() : inputIndex(container->insert({})) {}
InputComponent::~InputComponent() {
  container->erase(inputIndex);
}

simd::vec4 & InputComponent::front() {
  return container->at(inputIndex).front;
}

simd::vec4 & InputComponent::up() {
  return container->at(inputIndex).up;
}

simd::vec4 & InputComponent::right() {
  return container->at(inputIndex).right;
}

simd::vec4 & InputComponent::movementData() {
  return container->at(inputIndex).moves;
}

const simd::vec4 & InputComponent::front() const {
  return container->at(inputIndex).front;
}

const simd::vec4 & InputComponent::up() const {
  return container->at(inputIndex).up;
}

const simd::vec4 & InputComponent::right() const {
  return container->at(inputIndex).right;
}

const simd::vec4 & InputComponent::movementData() const {
  return container->at(inputIndex).moves;
}

void InputComponent::setMovement(const float &front, const float &up, const float &right) {
  container->at(inputIndex).moves = simd::vec4(right, up, front, 0.0f);
}

Container<InputData>* InputComponent::container = nullptr;

UserInputComponent::UserInputComponent(const CreateInfo &info) : horisontalAngleSum(0.0f), verticalAngleSum(0.0f), trans(info.trans), rotation(0.0f) {
  const simd::vec4 &startingDir = trans->rot();
  cartesianToSpherical(startingDir, horisontalAngleSum, verticalAngleSum);
}

UserInputComponent::~UserInputComponent() {}

void UserInputComponent::mouseMove(const float &horisontalAngle, const float &verticalAngle, const simd::mat4 &transform) {
  float x, y;
  {
    // RegionLog rl("angle compute");

    horisontalAngleSum += horisontalAngle;
    verticalAngleSum += verticalAngle;

    if (horisontalAngleSum > 360.0f) horisontalAngleSum = horisontalAngleSum - 360.0f;
    if (horisontalAngleSum < 0.0f)   horisontalAngleSum = horisontalAngleSum + 360.0f;

    if (verticalAngleSum > 180.0f - 0.01f) verticalAngleSum = 180.0f - 0.01f;
    if (verticalAngleSum < 0.0f   + 0.01f) verticalAngleSum = 0.0f   + 0.01f;

    x = -glm::radians(horisontalAngleSum); // азимут
    y = -glm::radians(verticalAngleSum); // зенит

    rotation.y = x; // yaw
    rotation.x = y; // pitch
  }

  simd::vec4 front;
  {
    // RegionLog rl("front");

    front = simd::vec4(
            glm::sin(y) * glm::cos(x), // r = 1.0f
            -glm::cos(y),
            glm::sin(y) * glm::sin(x),
            0.0f
    );
    //   front.x = glm::sin(y) * glm::cos(x); // r = 1.0f
    //   front.y = -glm::cos(y);
    //   front.z = glm::sin(y) * glm::sin(x);
    //   front.w = 0.0f;

    front = simd::normalize(front);
  }

  simd::vec4 right;
  {
    // RegionLog rl("right");

    right = simd::vec4(
            glm::cos(x - glm::half_pi<float>()),
            0.0f,
            glm::sin(x - glm::half_pi<float>()),
            0.0f
    );
    //   right.x = glm::cos(x - glm::half_pi<float>());
    //   right.y = 0.0f;
    //   right.z = glm::sin(x - glm::half_pi<float>());
    //   right.w = 0.0f;

    right = simd::normalize(right);
  }

  simd::vec4 up;
  {
    // RegionLog rl("up");
    up = simd::normalize(simd::cross(right, front));
  }

  {
    // RegionLog rl("transform");
    // transform я должен получать из вне
//    const simd::mat4 &transform = Global::physics()->getOrientation();
    front = transform * front;
    right = transform * right;
    up = transform * up;
    this->front() = front;
    this->InputComponent::right() = right;
    this->up() = up;
    playerRotation = rotation;
    {
      Global g;
      g.setPlayerRot(glm::vec4(rotation, 0.0f));
    }
  }

  trans->rot() = front;

  setMovement(0.0f, 0.0f, 0.0f);
}

void UserInputComponent::forward() {
  movementData().z = 1.0f;
}

void UserInputComponent::backward() {
  movementData().z = -1.0f;
}

void UserInputComponent::left() {
  movementData().x = -1.0f;
}

void UserInputComponent::right() {
  movementData().x = 1.0f;
}

void UserInputComponent::jump() {
  movementData().y = 1.0f;
}