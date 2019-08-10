#ifndef CAMERA_COMPONENT_H
#define CAMERA_COMPONENT_H

class TransformComponent;
class InputComponent;
class PhysicsComponent;

class CameraComponent {
public:
  struct CreateInfo {
    TransformComponent* trans;
    InputComponent* input;
    PhysicsComponent* phys;
  };
  CameraComponent(const CreateInfo &info);
  ~CameraComponent();

  void update();
private:
  TransformComponent* trans;
  InputComponent* input;
  PhysicsComponent* phys;
};

#endif //CAMERA_COMPONENT_H
