#ifndef ANIMATION_COMPONENT_H
#define ANIMATION_COMPONENT_H

#include "EntityComponentSystem.h"
#include "Controller.h"
#include "AnimationSystem.h"

class StateController;
class EventComponent;

class AnimationComponent : public yacs::Component/*, public Controller*/ {
public:
  CLASS_TYPE_DECLARE
  
  static void setStateContainer(Container<AnimationState>* stateContainer);
  
  AnimationComponent();
  ~AnimationComponent();
  
  void update(const uint64_t &time = 0) override;
  void init(void* userData) override;
  
//   void addChild(Controller* c) override;
//   ControllerChildData changeState(const Type &state) override;
//   void reset() override;
//   void finishCallback() override;
//   
//   bool isFinished() const override;
//   bool isBlocking() const override;
//   bool isBlockingMovement() const override;
  
//   void precacheStateCount(const uint32_t &count);
  void setAnimation(const Type &state, const std::string &animName);
private:
  uint32_t animStateCurrentIndex;
  uint32_t animationUnitIndex;
  
  StateController* controller;
  EventComponent* localEvents;
  
  //AnimationSystem::AnimationUnitData data;
  // нафига мне нужен AnimationState??????? я могу сюа просто id анимации добавить
  std::unordered_map<Type, AnimationState> states;
  
  static Container<AnimationState>* stateContainer;
  // это должно быть в стейт контроллере, время мы будем выставлять там
  //static ArrayInterface<AnimationState>* timeContainer; 
};

#endif
