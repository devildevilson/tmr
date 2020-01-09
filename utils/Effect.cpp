#include "Effect.h"

#include "Utility.h"

#include <cstring>

enum EffectTypeEnum : uint32_t {
  EFFECT_TYPE_RAW = (1 << 0),
  EFFECT_TYPE_ADD = (1 << 1),
  EFFECT_TYPE_REMOVE = (1 << 2),
  EFFECT_TYPE_PERIODICALY_APPLY = (1 << 3),
  EFFECT_TYPE_COMPUTE_EFFECT = (1 << 4),
  EFFECT_TYPE_CAN_RESIST = (1 << 5),
  EFFECT_TYPE_ONE_TIME_EFFECT = (1 << 6),
  EFFECT_TYPE_RESET_TIMER = (1 << 7),
  EFFECT_TYPE_STACKABLE = (1 << 8),
  EFFECT_TYPE_EASY_STACK = (1 << 9)
};

EffectType::EffectType() : container(0) {}
EffectType::EffectType(const EffectType &type) : container(type.container) {}
EffectType::EffectType(const bool raw, const bool add, const bool remove, const bool periodically_apply, const bool compute_effect, const bool resist, const bool one_time_effect, const bool timer_reset, const bool stackable, const bool easy_stack) : container(0) {
  make(raw, add, remove, periodically_apply, compute_effect, resist, one_time_effect, timer_reset, stackable, easy_stack);
}

void EffectType::make(const bool raw, const bool add, const bool remove, const bool periodically_apply, const bool compute_effect, const bool resist, const bool one_time_effect, const bool timer_reset, const bool stackable, const bool easy_stack) {
  container |= (raw * EFFECT_TYPE_RAW) |
               (add * EFFECT_TYPE_ADD) |
               (remove * EFFECT_TYPE_REMOVE) |
               (periodically_apply * EFFECT_TYPE_PERIODICALY_APPLY) |
               (compute_effect * EFFECT_TYPE_COMPUTE_EFFECT) |
               (resist * EFFECT_TYPE_CAN_RESIST) |
               (one_time_effect * EFFECT_TYPE_ONE_TIME_EFFECT) |
               (timer_reset * EFFECT_TYPE_RESET_TIMER) |
               (stackable * EFFECT_TYPE_STACKABLE) |
               (easy_stack * EFFECT_TYPE_EASY_STACK);
}

bool EffectType::raw() const {
  return (container & EFFECT_TYPE_RAW) == EFFECT_TYPE_RAW;
}

bool EffectType::add() const {
  return (container & EFFECT_TYPE_ADD) == EFFECT_TYPE_ADD;
}

bool EffectType::remove() const {
  return (container & EFFECT_TYPE_REMOVE) == EFFECT_TYPE_REMOVE;
}

bool EffectType::periodically_apply() const {
  return (container & EFFECT_TYPE_PERIODICALY_APPLY) == EFFECT_TYPE_PERIODICALY_APPLY;
}

bool EffectType::compute_effect() const {
  return (container & EFFECT_TYPE_COMPUTE_EFFECT) == EFFECT_TYPE_COMPUTE_EFFECT;
}

bool EffectType::can_be_resisted() const {
  return (container & EFFECT_TYPE_CAN_RESIST) == EFFECT_TYPE_CAN_RESIST;
}

bool EffectType::one_time_effect() const {
  return (container & EFFECT_TYPE_ONE_TIME_EFFECT) == EFFECT_TYPE_ONE_TIME_EFFECT;
}

bool EffectType::timer_reset() const {
  return (container & EFFECT_TYPE_RESET_TIMER) == EFFECT_TYPE_RESET_TIMER;
}

bool EffectType::stackable() const {
  return (container & EFFECT_TYPE_STACKABLE) == EFFECT_TYPE_STACKABLE;
}

bool EffectType::easy_stack() const {
  return (container & EFFECT_TYPE_EASY_STACK) == EFFECT_TYPE_EASY_STACK;
}

BonusTypesContainer::BonusTypesContainer(const size_t &size) : array(size == 0 ? nullptr : new BonusType[size]), size(size) {}

void BonusTypesContainer::clear() {
  delete [] array;
}

// ComputedEffectContainer::ComputedEffectContainer() : time(0), period_time(0), bonusTypes(0) {}
ComputedEffectContainer::ComputedEffectContainer(const size_t &size) : time(0), period_time(0), bonusTypes(size) {}

Effect::Effect(const CreateInfo &info) 
 : baseTime(info.baseEffectTime), 
   base_period_time(info.basePeriodTime), 
   typeVar(info.effectType), 
   effectType(info.id), 
   nameVar(info.name), 
   descriptionVar(info.description), 
   bonusTypes(info.types),
//    mods(info.mods),
   computefunc(info.compute),
   resistfunc(info.resist) {}

Effect::~Effect() {}

EffectType Effect::type() const {
  return typeVar;
}

const std::vector<BonusType> & Effect::baseValues() const {
  return bonusTypes;
}

const std::vector<Effect::EventModificator> & Effect::modificators() const {
  return mods;
}

size_t Effect::baseEffectTime() const {
  return baseTime;
}

size_t Effect::basePeriodTime() const {
  return base_period_time;
}

std::string Effect::name() const {
  return nameVar;
}

std::string Effect::description() const {
  return descriptionVar;
}

Type Effect::id() const {
  return effectType;
}

void Effect::compute(const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>> &float_attrib, const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>> &int_attrib, ComputedEffectContainer* container) const {
  ASSERT(container->bonusTypes.size == bonusTypes.size());
  ASSERT(typeVar.compute_effect() == bool(computefunc));

  if (typeVar.compute_effect()) computefunc(this, float_attrib, int_attrib, container);
  else memcpy(container->bonusTypes.array, bonusTypes.data(), bonusTypes.size()*sizeof(bonusTypes[0]));
}

void Effect::resist(const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>> &float_attribs, const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>> &int_attribs, ComputedEffectContainer* container) const {
  ASSERT(container->bonusTypes.size == bonusTypes.size());
  ASSERT(typeVar.can_be_resisted() == bool(resistfunc));

  if (typeVar.can_be_resisted()) resistfunc(this, float_attribs, int_attribs, container);
}

void Effect::setModificators(const std::vector<EventModificator> &mods) {
  this->mods = mods;
}

//event Effect::call(const Type &type, const EventData &data, yacs::entity* entity) {
//  return eventFunc(type, data, entity);
//}
