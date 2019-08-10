#ifndef ANIMATION_COMPONENT_H
#define ANIMATION_COMPONENT_H

#include "AnimationSystem.h"
#include "Type.h"

//class StateController;
class EventComponent;
class TransformComponent;
class GraphicComponent;

class AnimationComponent {
public:
  static void setStateContainer(Container<AnimationState>* stateContainer);

  struct CreateInfo {
    EventComponent* localEvents;
    TransformComponent* trans;
    GraphicComponent* graphics;
  };
  AnimationComponent(const CreateInfo &info);
  ~AnimationComponent();
  
//  void update(const uint64_t &time = 0) override;
  void update(const uint64_t &time = 0);
//  void init(void* userData) override;
  
//   void addChild(Controller* c) override;
//   ControllerChildData changeState(const Type &state) override;
//   void reset() override;
//   void finishCallback() override;
//   
//   bool isFinished() const override;
//   bool isBlocking() const override;
//   bool isBlockingMovement() const override;
  
//   void precacheStateCount(const uint32_t &count);
  void setAnimation(const Type &state, const ResourceID &id);
  
//  size_t & getInternalIndex();
private:
  uint32_t currentAnimationIndex;
  uint32_t oldAnimationIndex;
  
  size_t accumulatedTime;
  size_t internalIndex;
  
//   StateController* controller;
  EventComponent* localEvents;
  TransformComponent* trans;
  GraphicComponent* graphics;
  
  //AnimationSystem::AnimationUnitData data;
  // нафига мне нужен AnimationState??????? я могу сюа просто id анимации добавить
//   std::unordered_map<Type, AnimationState> states;
  
  static Container<AnimationState>* stateContainer;
  // это должно быть в стейт контроллере, время мы будем выставлять там
  //static ArrayInterface<AnimationState>* timeContainer; 
};

#endif
