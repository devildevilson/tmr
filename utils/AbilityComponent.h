#ifndef ABILITY_COMPONENT_H
#define ABILITY_COMPONENT_H

#include "Type.h"
#include "AbilitiesStruct.h"
#include "Engine.h"
#include "ThreadPool.h"

#include <vector>

// короче нужно добавить возможность разблокировать абилки
// например подобрав свиток, хотя тут вот что
// может быть необходимо совместить абилки и оружее
// для того чтобы было проще ориентироваться, и
// для того чтобы был единый интерфейс для этого
// как тогда быть с атакой на кнопку мыши?
// нужно определить абилки+состояние которые запускаются при взятии оружия
// нужно еще понять когда происходит смена оружия
// одна абилка от оружия для атаки, одна абилка при смене
// видимо текущее оружее все же мы должны выбрать
// возможно нужно разделить айтем и оружие (нет, можно не делить)
// использовать этот класс как интерфейс для такого поведения?
// это возможно, абилка что так что так должна запускать каст если 
// у нее определено время каста, что означает кулдаун?
// мы не можем воспользоваться этим оружием какое то время

// сделаю это на основе оружия

// разблокировку можно сделать на основе оружия
// но нужно оставить возможность запускать какие нибудь заданные абилки

class AbilityType;
class AttributeComponent;
class InventoryComponent;
class StateController;
class EffectComponent;
class DelayedTimedWorkSystem;
class EntityAI;
class WeaponsComponent;
class ItemType;

// ???
enum class cast_state {
  not_found,
  not_enough_resource,
  casting,
  
};

class AbilityComponent {
public:
  struct CreateInfo {
    const AbilityType* openAbility;
    
    WeaponsComponent* weapons;
    AttributeComponent* attribs;
    EffectComponent* effects;
    InventoryComponent* inventory;
    const EntityAI* entityAI;
    const Abilities* abilities;
    
    // должно быть оружие по умолчанию (не здесь)
  };
  AbilityComponent(const CreateInfo &info);
  
  void update();
  
  // полезно запускать абилку из пула
  cast_state cast(const size_t &slotIndex);
  cast_state cast(const Type &type);
  
  // мне нужен способ отменить текущую абилку
  void cancel();
  const AbilityType* castingAbility() const;
  size_t castingTime() const;
  
  bool takeWeapon(const size_t &slotIndex);
  
  bool attack();
  bool open(); // тут должна быть специальная абилка
  
  bool isCastPossible(const size_t &slotIndex) const;
  bool isCastPossible(const Type &type) const;
private:
  const ItemType* currentWeapon;
  const ItemType* lastWeapon;
  const AbilityType* ability;
  Type currentState;
  Type nextState;
  
  const AbilityType* openAbility;
  
  WeaponsComponent* weapons;
  AttributeComponent* attribs;
  EffectComponent* effects;
  InventoryComponent* inventory;
  StateController* controller;
  DelayedTimedWorkSystem* workSystem;
  const EntityAI* entityAI;
  
  const Abilities* abilities;
  
  bool checkCostAndSub(const AbilityType* ability);
  bool checkCost(const AbilityType* ability) const;
  void setStateAndCast(const AbilityType* ability, const Type &state);
  void castAbility(const AbilityType* ability);
};

class AbilityComponentSystem : public Engine {
public:
  struct CreateInfo {
    dt::thread_pool* pool;
  };
  AbilityComponentSystem(const CreateInfo &info);
  
  void update(const size_t &time) override;
private:
  dt::thread_pool* pool;
};

#endif
