#ifndef ANIMATION_COMPONENT_H
#define ANIMATION_COMPONENT_H

#include "AnimationSystem.h"
#include "Type.h"
//#include ""

//class StateController;
//class EventComponent;
class PhysicsComponent;
class TransformComponent;
class GraphicComponent;

// нужны данные об uv анимации
struct UVAnimation {
  enum class type {
    speed,
    uv,
    none
  };

  enum type type;
  glm::vec2 uvtransition; // смещение
  size_t time; // время применения смещения
};

class AnimationComponent {
public:
  static void setStateContainer(Container<AnimationState>* stateContainer);

  struct CreateInfo {
//    EventComponent* localEvents;
    TransformComponent* trans;
    PhysicsComponent* phys;
    GraphicComponent* graphics;
  };
  AnimationComponent(const CreateInfo &info);
  ~AnimationComponent();
  
//  void update(const uint64_t &time = 0) override;
  void update(const size_t &time);
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
//  void setAnimation(const Type &state, const ResourceID &id);

  // закольцевание анимации?
  struct PlayInfo {
    ResourceID id;
    float speedMod;
    size_t animationTime;
    bool looping;
  };
  void play(const PlayInfo &info);
  void apply(const UVAnimation &uvAnim);
  
//  size_t & getInternalIndex();
private:
  bool looping;
  bool direction;
  uint32_t currentAnimationIndex;
  uint32_t oldAnimationIndex;

  float speedMod;

  glm::vec2 uvtrans;
  UVAnimation uvAnim;

  size_t accumulatedTime;
  size_t animationTime;
//  size_t internalIndex;
  
//   StateController* controller;
//  EventComponent* localEvents;
  TransformComponent* trans;
  PhysicsComponent* phys; // скорость
  GraphicComponent* graphics;
  
  //AnimationSystem::AnimationUnitData data;
  // нафига мне нужен AnimationState??????? я могу сюда просто id анимации добавить
//   std::unordered_map<Type, AnimationState> states;
  
  static Container<AnimationState>* stateContainer;
  // это должно быть в стейт контроллере, время мы будем выставлять там
  //static ArrayInterface<AnimationState>* timeContainer; 
};

#endif
