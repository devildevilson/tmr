#ifndef COMBAT_COMPONENT_H
#define COMBAT_COMPONENT_H

#include <glm/glm.hpp>

#include "EntityComponentSystem.h"
#include "Controller.h"
#include "State.h"

class TransformComponent;

// struct AttackType {
//   uint64_t attackTime = 0;
//   float angle = 0.0f;
//   glm::vec3 attackPlane;
// };

class AttackState : public State {
public:
  AttackState(const std::string &name, const uint64_t &attackTime, const float &distance) : State(name) {
    this->attackTime = attackTime;
    this->distance = distance;
  }
  
  virtual ~AttackState() {}
  
  void setTransform(TransformComponent* trans);
protected:
  float distance = 0.0f;
  TransformComponent* trans = nullptr;
  uint64_t attackTime = 0;
};

// эта атака оперирует Polygon'ом, наверное
class Slashing : public AttackState {
public:
  Slashing(const std::string &name, 
           const uint64_t &attackTime, 
           const float &distance, 
           const glm::vec3 &attackPlane, 
           const float &angle, 
           bool blocking = false, bool blockingMovement = false);
  
  virtual ~Slashing();
  
  void update(const uint64_t &time = 0) override;
  void reset() override;
  bool isFinished() const override;
  bool isBlocking() const override;
  bool isBlockingMovement() const override;
private:
  bool blocking = false;
  bool blockingMovement = false;
  bool finished = false;
  bool resetB = true;
  
  float angle = 0.0f;
  float accumAngle = 0.0f;
  uint64_t passedTime = 0;
  
  glm::vec3 attackPlane;
  glm::vec3 prevDir;
  glm::vec3 prevPos;
};

// эта атака оперирует OBB
class Stabbing : public AttackState {
public:
  Stabbing(const std::string &name, bool blocking = false, bool blockingMovement = false);
  virtual ~Stabbing();
  
  void update(const uint64_t &time = 0) override;
  void reset() override;
  bool isFinished() const override;
  bool isBlocking() const override;
  bool isBlockingMovement() const override;
private:
  uint64_t attackTime = 0;
  uint64_t passedTime = 0;
  
};

class Combat : public yacs::Component, public Controller {
public:
  Combat();
  virtual ~Combat();
  
  void update(const uint64_t &time = 0) override;
  void init(void* userData) override;
  
  void addChild(Controller* c) override;
  void changeState(const Type &type) override;
  void reset() override;
  
  bool isFinished() const override;
  bool isBlocking() const override;
  bool isBlockingMovement() const override;
  
  void addAttack(const Type &type, AttackState* attack);
private:
  AttackState* current = nullptr;
//   TransformComponent* trans = nullptr;
  std::unordered_map<size_t, AttackState*> states;
};

class InteractionComponent : public yacs::Component {
public:
  void attack(InteractionComponent* other);
  void use(InteractionComponent* other);
  void touch(InteractionComponent* other);
private:
  //Attributes, Events ... 
};

#endif
