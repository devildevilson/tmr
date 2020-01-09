#include "AbilityType.h"

#define DEFAULT_THICKNESS 0.3f;

enum {
  ABILITY_INHERIT_TRANSFORM = (1<<0),
  ABILITY_INHERIT_EFFECTS = (1<<1),
  ABILITY_INHERIT_ATTRIBUTES = (1<<2),
  ABILITY_CREATE_TEMP_WEAPON = (1<<3),
  ABILITY_MODIFICATOR = (1<<4),
};

AbilityTypeT::AbilityTypeT() : container(0) {}
AbilityTypeT::AbilityTypeT(const bool trans, const bool effects, const bool attribs, const bool tempWeapon) : container(0) {
  make(trans, effects, attribs, tempWeapon);
}

void AbilityTypeT::make(const bool trans, const bool effects, const bool attribs, const bool tempWeapon) {
  container |= (uint32_t(trans)*ABILITY_INHERIT_TRANSFORM) |
               (uint32_t(effects)*ABILITY_INHERIT_EFFECTS) |
               (uint32_t(attribs)*ABILITY_INHERIT_ATTRIBUTES) |
               (uint32_t(tempWeapon)*ABILITY_CREATE_TEMP_WEAPON) |
               (uint32_t(false)*ABILITY_MODIFICATOR);
}

bool AbilityTypeT::inheritTransform() const {
  return (container & ABILITY_INHERIT_TRANSFORM) == ABILITY_INHERIT_TRANSFORM;
}

bool AbilityTypeT::inheritEffects() const {
  return (container & ABILITY_INHERIT_EFFECTS) == ABILITY_INHERIT_EFFECTS;
}

bool AbilityTypeT::inheritAttributes() const {
  return (container & ABILITY_INHERIT_ATTRIBUTES) == ABILITY_INHERIT_ATTRIBUTES;
}

bool AbilityTypeT::createTemporaryWeapon() const {
  return (container & ABILITY_CREATE_TEMP_WEAPON) == ABILITY_CREATE_TEMP_WEAPON;
}

bool AbilityTypeT::eventModificator() const {
  return (container & ABILITY_MODIFICATOR) == ABILITY_MODIFICATOR;
}

AbilityType::AbilityType(const CreateInfo &info)
  : abilityId(info.abilityId),
    type(info.type),
    abilityName(info.abilityName),
    abilityDesc(info.abilityDesc),
    abilityState(info.abilityState),
    abilityCooldown(info.abilityCooldown),
    abilityCost(info.abilityCost),
    nextAbility(nullptr),
    abilityEffect(info.abilityEffect),
    delayTime(info.delayTime),
    entityType(info.entityType),
    func(info.func),
    attribsFunc(info.attribsFunc),
    intAttribs(info.intAttribs),
    floatAttribs(info.floatAttribs) {}

AbilityType::~AbilityType() {}

Type AbilityType::id() const {
  return abilityId;
}

std::string AbilityType::name() const {
  return abilityName;
}

std::string AbilityType::description() const {
  return abilityDesc;
}

bool AbilityType::inheritTransform() const {
  return type.inheritTransform();
}

bool AbilityType::inheritEffects() const {
  return type.inheritEffects();
}

bool AbilityType::inheritAttributes() const {
  return type.inheritAttributes();
}

bool AbilityType::createTemporaryWeapon() const {
  return type.createTemporaryWeapon();
}

// const ItemType* AbilityType::temporaryWeapon() const {
//   return tempWeapon;
// }

// Type AbilityType::event() const {
//   return impactEvent;
// }

// size_t AbilityType::castTime() const {
//   return abilityCastTime;
// }

Type AbilityType::state() const {
  return abilityState;
}

size_t AbilityType::cooldown() const {
  return abilityCooldown;
}

const Effect* AbilityType::cost() const {
  return abilityCost;
}

const AbilityType* AbilityType::next() const {
  return nextAbility;
}

void AbilityType::setNext(const AbilityType* a) {
  nextAbility = a;
}

const Effect* AbilityType::effect() const {
  return abilityEffect;
}

Type AbilityType::entityCreatorType() const {
  return entityType;
}

size_t AbilityType::delay() const {
  return delayTime;
}

TransOut AbilityType::computeTransform(const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>> &float_finder, const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>> &int_finder, const TransIn &transform) const {
  return func(float_finder, int_finder, transform);
}

void AbilityType::computeAttributes(const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>> &float_finder, const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>> &int_finder, std::vector<AttributeComponent::InitInfo<FLOAT_ATTRIBUTE_TYPE>> &float_attribs, std::vector<AttributeComponent::InitInfo<INT_ATTRIBUTE_TYPE>> &int_attribs) const {
  attribsFunc(float_finder, int_finder, floatAttribs, intAttribs, float_attribs, int_attribs);
}

bool AbilityType::hasTransformFunc() const {
  return bool(func);
}

bool AbilityType::hasComputeFunc() const {
  return bool(attribsFunc);
}

const std::vector<AbilityAttributeListElement<FLOAT_ATTRIBUTE_TYPE>> & AbilityType::floatAttributes() const {
  return floatAttribs;
}

const std::vector<AbilityAttributeListElement<INT_ATTRIBUTE_TYPE>> & AbilityType::intAttributes() const {
  return intAttribs;
}

// const std::vector<AbilityType::EntityCreateInfo> & AbilityType::entityInfos() const {
//   return entityCreateInfos;
// }
// 
// const std::vector<const Effect*> & AbilityType::effects() const {
//   return abilityEffects;
// }
