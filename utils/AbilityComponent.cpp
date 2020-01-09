#include "AbilityComponent.h"

#include "StateController.h"
#include "AbilityType.h"
#include "AttributesComponent.h"
#include "Effect.h"
#include "EffectComponent.h"
#include "DelayedTimedWorkSystem.h"
#include "InventoryComponent.h"

#include "Globals.h"
#include "EntityComponentSystem.h"

AbilityComponent::AbilityComponent(const CreateInfo &info) : currentWeapon(nullptr), lastWeapon(nullptr), ability(nullptr), openAbility(info.openAbility), weapons(info.weapons), attribs(info.attribs), effects(info.effects), inventory(info.inventory), abilities(info.abilities) {}

// может быть вообще не потребуется
void AbilityComponent::update() {
  if (!currentWeapon->whenAttack().empty()) return;
  if (currentWeapon->whenUse().ability == nullptr) return; // такое возможно в условиях когда у нас абилка? нет, но у нас может быть сейчас временное оружее
  if (!currentWeapon->whenUse().ability->state().valid()) return;
  
  
  
  if (ability != nullptr && ability->state().valid() && currentWeapon->whenUse().ability == ability && ((controller->state() == currentState && controller->isFinished()) || (controller->state() == nextState))) {
    if (nextState.valid()) {
      controller->setState(nextState);
      currentState = nextState;
    } else {
      currentState = lastWeapon->whenUse().state.valid() ? lastWeapon->whenUse().state : lastWeapon->whenUse().ability->state();
      controller->setState(currentState);
      nextState = controller->nextState();
      std::swap(currentWeapon, lastWeapon);
    }
    
    castAbility(ability);
    ability = nullptr;
    
    return;
  }
  
  if (((controller->state() == currentState && controller->isFinished()) || controller->looping()) || (controller->state() == nextState)) {
    currentState = lastWeapon->whenUse().state.valid() ? lastWeapon->whenUse().state : lastWeapon->whenUse().ability->state();
    controller->setState(currentState);
    nextState = controller->nextState();
    std::swap(currentWeapon, lastWeapon);
  }
}

cast_state AbilityComponent::cast(const size_t &slotIndex) {
  auto type = weapons->get(slotIndex);
  if (type == nullptr) return cast_state::not_found;
  setStateAndCast(type->whenUse().ability, type->whenUse().state);
  
  for (const auto &slot : abilities->slots) {
    if (slot.index == slotIndex) {
      const bool ret = checkCost(slot.type);
      if (!ret) return cast_state::not_enough_resource;
      
      setStateAndCast(slot.type, slot.state);
      
      return cast_state::casting;
    }
  }
  
  return cast_state::not_found;
}

cast_state AbilityComponent::cast(const Type &type) {
  // сначала должны обойти инвентарь в поисках абилки
  
  for (const auto &slot : abilities->slots) {
    if (slot.type->id() == type) {
      const bool ret = checkCost(slot.type);
      if (!ret) return cast_state::not_enough_resource;
      
      setStateAndCast(slot.type, slot.state);
      
      return cast_state::casting;
    }
  }
  
  return cast_state::not_found;
}

void AbilityComponent::cancel() {
  if (currentWeapon->whenUse().ability != nullptr && lastWeapon->whenUse().ability->state() == controller->state()) {
    controller->setState(lastWeapon->whenUse().state.valid() ? lastWeapon->whenUse().state : lastWeapon->whenUse().ability->state());
    std::swap(currentWeapon, lastWeapon);
  }
}

const AbilityType* AbilityComponent::castingAbility() const {
  return currentState == controller->state() || nextState == controller->state() ? ability : nullptr;
}

size_t AbilityComponent::castingTime() const {
  return controller->time();
}

bool AbilityComponent::takeWeapon(const size_t &slotIndex) {
  if (controller->blocking()) return false;
  
  auto type = weapons->get(slotIndex);
  if (type == nullptr) return false;
  if (currentWeapon == type) return true;
  
  if (type->whenUse().ability != nullptr) {
    if (!checkCostAndSub(type->whenUse().ability))  return false;
    
    setStateAndCast(type->whenUse().ability, type->whenUse().ability->state());
  } else {
//     currentState = type->whenUse().state;
//     if (currentState.valid()) {
//       const bool ret = controller->setState(currentState);
//       if (!ret) throw std::runtime_error("Could not set state "+currentState.name());
//       nextState = controller->nextState();
//     }
    controller->changeWeaponState(type->whenUse().state);
  }
  
  // мы должны выставить текущее оружее
  if (!type->whenAttack().empty()) {
    lastWeapon = currentWeapon;
    currentWeapon = type;
  }
  
  return true;
}

bool AbilityComponent::attack() {
  if (currentWeapon == nullptr) throw std::runtime_error("currentWeapon == nullptr");
  if (currentWeapon->whenAttack().empty()) throw std::runtime_error("currentWeapon in not weapon");
  
  if (controller->blocking()) return false;
  
  for (auto & data : currentWeapon->whenAttack()) {
    if (controller->state() == data.state) {
      if (!checkCostAndSub(data.ability)) return false;
      
      setStateAndCast(data.ability, data.ability->state()); // стейт с абилки
      return true;
    }
  }
  
  return false;
}

bool AbilityComponent::open() {
  // спец абилка, одна скорее всего на все энтити
  // может ли использование быть заблокированным? вряд ли
  
  ASSERT(openAbility != nullptr);
  
  castAbility(openAbility);
  return true;
}

bool AbilityComponent::isCastPossible(const size_t &slotIndex) const {
  // сначала должны обойти инвентарь в поисках абилки
  
  for (const auto &slot : abilities->slots) {
    if (slot.type->id() == slotIndex) {
      return checkCost(slot.type);
    }
  }
  
  return false;
}

bool AbilityComponent::isCastPossible(const Type &type) const {
  for (const auto &slot : abilities->slots) {
    if (slot.type->id() == type) {
      return checkCost(slot.type);
    }
  }
  
  return false;
}

bool AbilityComponent::checkCostAndSub(const AbilityType* ability) {
  if (ability->cost() == nullptr) return true;
  
  ComputedEffectContainer cont(ability->cost()->baseValues().size());
  ability->cost()->compute(attribs->get_finder<FLOAT_ATTRIBUTE_TYPE>(), attribs->get_finder<INT_ATTRIBUTE_TYPE>(), &cont);
  ability->cost()->resist(attribs->get_finder<FLOAT_ATTRIBUTE_TYPE>(), attribs->get_finder<INT_ATTRIBUTE_TYPE>(), &cont);
  
  for (size_t i = 0; i < cont.bonusTypes.size; ++i) {
    if (cont.bonusTypes.array[i].type.float_type()) {
      auto attrib = attribs->get<FLOAT_ATTRIBUTE_TYPE>(cont.bonusTypes.array[i].type);
      // хороший вопрос какой знак использовать, по идее все должно быть по аналогии с другими вещами
      if (attrib->value() + cont.bonusTypes.array[i].bonus.add <= 0.0f) {
        cont.bonusTypes.clear();
        return false;
      }
    }
  }
  
  effects->addEffect(cont, ability->cost(), entityAI);
  return true;
}

bool AbilityComponent::checkCost(const AbilityType* ability) const {
  if (ability->cost() == nullptr) return true;
  
  ComputedEffectContainer cont(ability->cost()->baseValues().size());
  ability->cost()->compute(attribs->get_finder<FLOAT_ATTRIBUTE_TYPE>(), attribs->get_finder<INT_ATTRIBUTE_TYPE>(), &cont);
  ability->cost()->resist(attribs->get_finder<FLOAT_ATTRIBUTE_TYPE>(), attribs->get_finder<INT_ATTRIBUTE_TYPE>(), &cont);
  
  for (size_t i = 0; i < cont.bonusTypes.size; ++i) {
    if (cont.bonusTypes.array[i].type.float_type()) {
      auto attrib = attribs->get<FLOAT_ATTRIBUTE_TYPE>(cont.bonusTypes.array[i].type);
      // хороший вопрос какой знак использовать, по идее все должно быть по аналогии с другими вещами
      if (attrib->value() + cont.bonusTypes.array[i].bonus.add <= 0.0f) {
        cont.bonusTypes.clear();
        return false;
      }
    }
  }
  
  return true;
}

void AbilityComponent::setStateAndCast(const AbilityType* ability, const Type &state) {
  currentState = state;
  if (state.valid()) {
    const bool ret = controller->setState(currentState);
    if (!ret) throw std::runtime_error("Could not set state "+currentState.name());
    nextState = controller->nextState();
  }
  
  this->ability = ability;
  if (ability != nullptr) castAbility(ability);
}

void AbilityComponent::castAbility(const AbilityType* ability) {
  if (!ability->entityInfos().empty()) {
    for (size_t i = 0; i < ability->entityInfos().size(); ++i) {
      const size_t index = i;
      const size_t finalDelay = currentState.valid() ? ability->entityInfos()[i].delayTime / controller->speedModificator() : ability->entityInfos()[i].delayTime;
      workSystem->add_work(finalDelay, [ability, index] () {
        const auto &info = ability->entityInfos()[index];
        // создаем энтити
      });
    }
  }
  
  for (auto effect : ability->effects()) {
    ComputedEffectContainer cont(effect->baseValues().size());
    effect->compute(attribs->get_finder<FLOAT_ATTRIBUTE_TYPE>(), attribs->get_finder<INT_ATTRIBUTE_TYPE>(), &cont);
    effect->resist(attribs->get_finder<FLOAT_ATTRIBUTE_TYPE>(), attribs->get_finder<INT_ATTRIBUTE_TYPE>(), &cont);
    
    effects->addEffect(cont, effect, entityAI);
  }
}

AbilityComponentSystem::AbilityComponentSystem(const CreateInfo &info) : pool(info.pool) {}
void AbilityComponentSystem::update(const size_t &time) {
  (void)time;
  
  const size_t count = Global::world()->count_components<AbilityComponent>();
  const size_t one_thread_count = std::ceil(float(count) / float(pool->size()+1));
  size_t start = 0;
  for (uint32_t i = 0; i < pool->size()+1; ++i) {
    const size_t jobCount = std::min(one_thread_count, count-start);
    if (jobCount == 0) break;
    
    pool->submitbase([start, jobCount] () {
      for (size_t index = start; index < start+jobCount; ++index) {
        auto handle = Global::world()->get_component<AbilityComponent>(index);
        handle->update();
      }
    });
    start += jobCount;
  }
  
  pool->compute();
  pool->wait();
}
