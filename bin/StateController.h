#ifndef STATE_CONTROLLER_H
#define STATE_CONTROLLER_H

#include "Type.h"
#include "ResourceID.h"
#include "EventFunctor.h"
#include "Engine.h"
#include "EntityComponentSystem.h"
#include "MemoryPool.h"
#include "ThreadPool.h"
#include "Interaction.h"

#include "StateControllerType.h"

#include <vector>

// у меня набирается уже куча object type классов
// можно ли как то их собрать в какой нибудь фактори класс?
// и таким образом производить энтити

class EventComponent;
class AnimationComponent;
class SoundComponent;
class InteractionComponent;
class InventoryComponent;
class AttributesComponent;

// состояния то не уникальны
class StateController : public EventFunctor {
public:
  struct CreateInfo {
    AnimationComponent* animations;
    SoundComponent* sounds;
    InteractionComponent* interactions;
    AttributesComponent* attribs;
    InventoryComponent* inventory;
    const StateControllerType* controllerType;
//    size_t resourceContainerSize;
    // данные для создания состояний
  };
  StateController(const CreateInfo &info);

  void update(const size_t &time);

  bool blocking() const;
  bool blockingMovement() const;
  bool looping() const;
  bool usedWeapon() const;
  bool usedItem() const;

  Type state() const;
  Type nextState() const;

  size_t time() const;
  size_t stateTime() const;

  const StateControllerType* type() const;

  event call(const Type &type, const EventData &data, yacs::entity* entity) override;

  void callDefaultState();
private:
  AnimationComponent* animations;
  SoundComponent* sounds;
  InteractionComponent* interactions;
  AttributesComponent* attribs;
  InventoryComponent* inventory;

  size_t currentTime;
  const StateData* currentState;
  const StateData* prevState;
  const StateControllerType* controllerType;
};

class StateControllerSystem : public Engine, public yacs::system {
public:
  struct CreateInfo {
    dt::thread_pool* pool;
  };
  StateControllerSystem(const CreateInfo &info);
  ~StateControllerSystem();

  void update(const size_t &time) override;

  void create(const Type &type, const StateControllerType::CreateInfo &info);
  const StateControllerType* get(const Type &type) const;
private:
  dt::thread_pool* pool;

  std::unordered_map<Type, StateControllerType*> types;
  MemoryPool<StateControllerType, sizeof(StateControllerType)*20> typePool;
};

#endif //STATE_CONTROLLER_H
