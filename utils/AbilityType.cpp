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
    impactEvent(info.impactEvent),
    type(info.type),
    abilityName(info.abilityName),
    abilityDesc(info.abilityDesc),
    tempWeapon(info.tempWeapon),
    abilityCastTime(info.abilityCastTime),
    abilityCooldown(info.abilityCooldown),
    costs{info.cost1, info.cost2, info.cost3},
//     intCreateInfo(info.intCreateInfo),
    interactionDelayTime(info.delayTime),
    entityType(info.entityType),
    func(info.func),
    attribsFunc(info.attribsFunc),
    impactEffectsList(info.impactEffectsList),
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

const ItemType* AbilityType::temporaryWeapon() const {
  return tempWeapon;
}

Type AbilityType::event() const {
  return impactEvent;
}

size_t AbilityType::castTime() const {
  return abilityCastTime;
}

size_t AbilityType::cooldown() const {
  return abilityCooldown;
}

AbilityCost AbilityType::cost(const size_t &index) const {
  ASSERT(index < MAX_ATTRIBUTES_COST_COUNT);
  return costs[index];
}

// InteractionCreateInfo AbilityType::interactionInfo() const {
//   return intCreateInfo;
// }

Type AbilityType::entityCreateType() const {
  return entityType;
}

TransOut AbilityType::computeTransform(const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>> &float_finder, const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>> &int_finder, const TransIn &parentData) const {
  return func(float_finder, int_finder, parentData);
}

void AbilityType::computeAttributes(const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>> &float_finder,
                                    const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>> &int_finder,
//                                    const std::vector<AbilityAttributeListElement<FLOAT_ATTRIBUTE_TYPE>> &floatAttribs,
//                                    const std::vector<AbilityAttributeListElement<INT_ATTRIBUTE_TYPE>> &intAttribs,
                                    std::vector<AttributeComponent::InitInfo<FLOAT_ATTRIBUTE_TYPE>> &floatInit,
                                    std::vector<AttributeComponent::InitInfo<INT_ATTRIBUTE_TYPE>> &intInit) const {
  attribsFunc(float_finder, int_finder, floatAttribs, intAttribs, floatInit, intInit);
}

const std::vector<const Effect*> & AbilityType::effectsList() const {
  return impactEffectsList;
}

const std::vector<AbilityAttributeListElement<INT_ATTRIBUTE_TYPE>> & AbilityType::intAttributesList() const {
  return intAttribs;
}

const std::vector<AbilityAttributeListElement<FLOAT_ATTRIBUTE_TYPE>> & AbilityType::floatAttributesList() const {
  return floatAttribs;
}
