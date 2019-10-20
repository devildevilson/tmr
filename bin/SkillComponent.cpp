#include "SkillComponent.h"

#include "TransformComponent.h"
#include "PhysicsComponent.h"
#include "EffectComponent.h"
#include "Interactions.h"

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
    intCreateInfo(info.intCreateInfo),
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

InteractionCreateInfo AbilityType::interactionInfo() const {
  return intCreateInfo;
}

const yacs::entity* AbilityType::entityCreateType() const {
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

// это скорее всего будет супер долгая операция, чтобы было быстрее видимо придется хардкодить
// yacs::entity* createAbilityEntity(const AbilityType* abilityType, yacs::entity* parent) {
//   const yacs::entity* entType = abilityType->entityCreateType();
//   auto attr = parent->get<AttributeComponent>();
//   auto attackDist = attr->get(abilityType->interactionInfo().attackDistance);
//   auto attackSpeed = attr->get(abilityType->interactionInfo().attackSpeed);
// 
//   auto ent = Global::world()->create_entity();
// 
//   // тут же должно быть создание физического объекта, я поторопился, интеракции не должны самостоятельно создавать физический объект
//   // по крайней мере в случае когда он нужен (лучи наверное пусть сами создают)
//   TransformComponent* abilityTransform = nullptr;
//   if (abilityType->inheritTransform()) {
//     auto trans = parent->get<TransformComponent>();
//     ent->set(trans);
//     abilityTransform = trans.get();
//   } else {
//     auto phys = parent->get<PhysicsComponent>();
//     auto transParent = parent->get<TransformComponent>();
// 
//     const TransIn in{
//       transParent->pos(),
//       transParent->rot(),
//       phys->getVelocity()
//     };
// 
//     const TransOut out = abilityType->computeTransform(attr->get_finder<FLOAT_ATTRIBUTE_TYPE>(), attr->get_finder<INT_ATTRIBUTE_TYPE>(), in);
// 
//     abilityTransform = ent->add<TransformComponent>(out.pos, transParent->rot(), simd::vec4(1.0f, 1.0f, 1.0f, 0.0f)).get();
//   }
// 
//   PhysicsComponent* abilityPhys = nullptr;
//   switch (abilityType->interactionInfo().type) {
//     case Interaction::type::slashing:
//     case Interaction::type::stabbing:
//     case Interaction::type::impact: {
//       // создаем здесь
// 
//       // должна быть возможность установить скорость по умолчанию
//       const PhysicsObjectCreateInfo phys_info{
//         false,
//         PhysicsType(false, SPHERE_TYPE, false, true, false, false),
//         abilityType->interactionInfo().collisionGroup, // это задается из абилки
//         abilityType->interactionInfo().collisionFilter,
//         0.0f,
//         0.0f,
//         0.0f,
//         attackDist->value(),                 // это по идее какой то аттрибут или что то вроде
//         UINT32_MAX,
//         abilityTransform->index(),       // нужно создать транс, до создания физики
//         UINT32_MAX,
// //     info.matrixIndex,
// //     info.rotationIndex,
//         UINT32_MAX,
//         UINT32_MAX,
//         Type()
//       };
// 
//       abilityPhys = ent->add<PhysicsComponent>(phys_info).get();
//       break;
//     }
//     default:
//       abilityPhys = nullptr;
//   }
// 
//   if (abilityType->inheritAttributes()) {
//     ent->set(attr);
//   } else {
//     // абилка наверное не потребует этого компонента
//     //auto attr = entType->get<AttributeCreatorComponent>();
//     //if (attr.valid()) {
//     // создаем этот компонент энтити
//     std::vector<AttributeComponent::InitInfo<FLOAT_ATTRIBUTE_TYPE>> floatInit(abilityType->floatAttributesList().size());
//     std::vector<AttributeComponent::InitInfo<INT_ATTRIBUTE_TYPE>> intInit(abilityType->intAttributesList().size());
// 
//     abilityType->computeAttributes(attr->get_finder<FLOAT_ATTRIBUTE_TYPE>(), attr->get_finder<INT_ATTRIBUTE_TYPE>(), floatInit, intInit);
// 
//     // TODO: физика здесь nullptr, это может вызвать ряд проблем
//     ent->add<AttributeComponent>(AttributeComponent::CreateInfo{abilityPhys, nullptr, SIZE_MAX, floatInit, intInit});
//     //}
//   }
// 
//   if (abilityType->inheritEffects()) {
//     auto effects = parent->get<EffectComponent>();
//     ent->set(effects);
//   } else {
//     auto effects = ent->add<EffectComponent>();
//     for (size_t i = 0; i < abilityType->effectsList().size(); ++i) {
//       effects->addEventEffect(abilityType->event(), abilityType->effectsList()[i]);
//     }
//   }
// 
//   switch (abilityType->interactionInfo().type) {
//     case Interaction::type::target: {
//       ent->add<TargetInteraction>(TargetInteraction::CreateInfo{}); // как это создавать? мне нужно откуда то взять таргет
//       break;
//     }
//     case Interaction::type::ray: {
//       ent->add<RayInteraction>(RayInteraction::CreateInfo{
//         RayInteraction::type::first_min,
//         std::numeric_limits<float>::infinity(),
//         0.0f,
//         parent->get<PhysicsComponent>()->getIndexContainer().objectDataIndex,
//         1,
//         abilityType->interactionInfo().delayTime,
//         ent,
//         abilityTransform,
//         abilityType->event()
//       });
//       break;
//     }
//     case Interaction::type::slashing: {
//       ent->add<SlashingInteraction>(SlashingInteraction::CreateInfo{
//         abilityType->interactionInfo().delayTime,
//         , // attackTime
//         abilityType->interactionInfo().tickTime,
//         abilityType->interactionInfo().tickCount,
//         0, // ticklessObj
//         DEFAULT_THICKNESS,
//         , // angle
//         attackDist->value(),
//         attackSpeed->value(), // speed
//         abilityType->interactionInfo().plane,
//         ent,
//         abilityPhys,
//         abilityTransform,
//         abilityType->event()
//       });
//       break;
//     }
//     case Interaction::type::stabbing: {
// 
//       break;
//     }
//     case Interaction::type::impact: {
// 
//       break;
//     }
// 
//     default:
//       throw std::runtime_error("Bad interaction type");
//   }
// 
//   // далее находим следующий креатор
// 
//   // в конце нужно сохранить как нибудь информацию о создании
// }

bool AbilitySystemComponent::cast(const Type &id) {
  for (size_t i = 0; i < abilities.size(); ++i) {
    if (abilities[i].type->id() == id) {


      return true;
    }
  }

  return false;
}

Type AbilitySystemComponent::casted_ability() const {

}

size_t AbilitySystemComponent::casted(const Type &id) const {

}
