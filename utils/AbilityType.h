#ifndef ABILITY_TYPE_H
#define ABILITY_TYPE_H

#include "Utility.h"
#include "Attributes.h"
#include "AttributesComponent.h"

#define MAX_ATTRIBUTES_COST_COUNT 3

namespace yacs {
  class entity;
}

class ItemType;
class Effect;
class InventoryComponent;

struct AbilityCost {
  // тип атрибута бы тоже сделать как тип итема, обращаться к нему по указателю
  const AttributeType<INT_ATTRIBUTE_TYPE>* type;
  INT_ATTRIBUTE_TYPE cost;
};

struct TransIn {
  simd::vec4 pos;
  simd::vec4 dir;
  simd::vec4 vel;
};

struct TransOut {
  simd::vec4 pos;
//  simd::vec4 dir;
  simd::vec4 vel;
};

struct AbilityTypeT {
  uint32_t container;

  AbilityTypeT();

  AbilityTypeT(const bool trans, const bool effects, const bool attribs, const bool tempWeapon);

  void make(const bool trans, const bool effects, const bool attribs, const bool tempWeapon);

  bool inheritTransform() const;

  bool inheritEffects() const;

  bool inheritAttributes() const;

  bool createTemporaryWeapon() const;

  bool eventModificator() const;
};

// предусмотреть копирование (можно отметить как UINT32_MAX)
template<typename T>
struct AbilityAttributeListElement {
  const AttributeType<T>* type;
  T baseValue;
};

// информация об интеракции может быть записана в создателе
// нам необходимо только указать delayTime (может быть разное оружее используещее одну и туже абилку), event (таже абилка делающаяя другие вещи)
// struct InteractionCreateInfo {
//   enum Interaction::type type;
// 
//   float plane[4]; // это по идее тоже переменная величина
//   // как сделать плоскость переменной? никак скорее всего, но она нужна чтобы посчитать где у нас происходит сейчас атака
//   //float distance; // это должно задаваться аттрибутом (дальность атаки)
// //   float attackAngle; // скорее всего очень многие вещи - это аттрибут
// //   uint32_t tickCount;
// 
//   // плоскость и тип можно легко указать в создателе
//   // коллизию тоже, делэй? эвент? аттрибуты?
//   // кроме делэя все указывается в создателе
//   // делэй, 
// 
//   uint32_t collisionGroup;
//   uint32_t collisionFilter;
// 
//   // скорее всего все аттрибуты обязаны быть
//   const AttributeType<FLOAT_ATTRIBUTE_TYPE>* attackDistance;
//   const AttributeType<FLOAT_ATTRIBUTE_TYPE>* attackSpeed;
//   const AttributeType<FLOAT_ATTRIBUTE_TYPE>* attackAngle;
//   const AttributeType<INT_ATTRIBUTE_TYPE>* tickCount;
//   const AttributeType<INT_ATTRIBUTE_TYPE>* tickTime;
//   size_t delayTime;
// 
//   Type event;
// };

// абилка это способ передать какую то информацию от одного энтити к другому
// разные абилки - разный способ, мне нужно учесть как можно больше всего
// первое что приходит на ум это файрбол и обычная атака

// можно ли АбилитиТайп превратить в ЭнтитиТайп? то есть чтобы обобщить абилки и энтити?
// можно, если разделить данные о стоимости и непосредственно создание энтити
// как создавать энтити?
class AbilityType {
public:
  using ComputeTransformFunction = std::function<TransOut(const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>> &, const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>> &, const TransIn &)>;
  using ComputeAttributesFunction = std::function<void(const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>> &, const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>> &, const std::vector<AbilityAttributeListElement<FLOAT_ATTRIBUTE_TYPE>> &, const std::vector<AbilityAttributeListElement<INT_ATTRIBUTE_TYPE>> &, std::vector<AttributeComponent::InitInfo<FLOAT_ATTRIBUTE_TYPE>> &, std::vector<AttributeComponent::InitInfo<INT_ATTRIBUTE_TYPE>> &)>;
  
  struct CreateInfo {
    Type abilityId;

    AbilityTypeT type;

    std::string abilityName;
    std::string abilityDesc;

    AbilityCost cost1;
    AbilityCost cost2;
    AbilityCost cost3;

    const ItemType* tempWeapon;

    Type impactEvent;

    size_t abilityCastTime; // должны быть переменные вещи зависящие от характеристик, пока так оставлю
    size_t abilityCooldown;

//     InteractionCreateInfo intCreateInfo;
    size_t delayTime;
    Type entityType;

    ComputeTransformFunction func;
    ComputeAttributesFunction attribsFunc;

    std::vector<const Effect*> impactEffectsList;

    std::vector<AbilityAttributeListElement<INT_ATTRIBUTE_TYPE>> intAttribs;
    std::vector<AbilityAttributeListElement<FLOAT_ATTRIBUTE_TYPE>> floatAttribs;
  };

  AbilityType(const CreateInfo &info);

  ~AbilityType();

  Type id() const;

  std::string name() const;

  std::string description() const;

  // я уже несколько раз повторял что нужно разделить контейнер эффектов которые мы передаем от остального

  bool inheritTransform() const;  // если не наследует трансформ, то нужно понять где создать, наиболее сложная задача видимо
  bool inheritEffects() const;    // если не наследует эффекты, то нужно какие то эффекты добавить, тоже список по идее
  bool inheritAttributes() const; // с атрибутами проще, список атрибутов мы можем легко задать

  bool createTemporaryWeapon() const;

  const ItemType* temporaryWeapon() const;

  Type event() const;             // ???

  size_t castTime() const;

  size_t cooldown() const;
  
  size_t delayTime() const;

  // требование по мане, или по другим атрибутам
  AbilityCost cost(const size_t &index) const;

//   InteractionCreateInfo interactionInfo() const;

  Type entityCreateType() const;

  // основной вопрос где создавать абилку, мне нужно на основе атрибутов каким то образом вычислить положение, скорость,
  // направление, но только в случае когда трансформа не наследуется
  // хотя направление по идее берется из скорости, но на вход направление поди должно быть подано
  TransOut computeTransform(const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>> &float_finder,
                            const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>> &int_finder,
                            const TransIn &parentData) const;

  void computeAttributes(const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>> &float_finder,
                         const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>> &int_finder,
//                         const std::vector<AbilityAttributeListElement<FLOAT_ATTRIBUTE_TYPE>> &floatAttribs,
//                         const std::vector<AbilityAttributeListElement<INT_ATTRIBUTE_TYPE>> &intAttribs,
                         std::vector<AttributeComponent::InitInfo<FLOAT_ATTRIBUTE_TYPE>> &floatInit,
                         std::vector<AttributeComponent::InitInfo<INT_ATTRIBUTE_TYPE>> &intInit) const;

  // должны быть еще сопутствующие частицы, но они скорее должны исходить от самой абилки

  // переключение состояния? при касте, при применении, при владении должны быть состояния
  // у меня же еще абилка должна оружее менять, точнее наверное она должна делать это по умолчанию
  // то есть это скорее абилка по призыву оружия
  // у меня оружее сейчас это не энтити, поэтому создать как энтити не получится
  // в принципе при атаке огненным шаром - это тоже абилка
  // то получается что у нас абилка должна либо создавать энтити либо менять оружее
  // как сделано в других играх? заклинание действует практически мгновенно без каста (например в морре)

  // модификаторы на атаку как сделать? к эффектам у энтити нужно добавить модификатор, ко всему прочему нужно какой то графический эффект проиграть
  //

  // это эффекты для передачи другому объекту, они должны передаваться по эвенту указаному здесь же видимо
  const std::vector<const Effect*> & effectsList() const;

  const std::vector<AbilityAttributeListElement<INT_ATTRIBUTE_TYPE>> & intAttributesList() const;

  const std::vector<AbilityAttributeListElement<FLOAT_ATTRIBUTE_TYPE>> & floatAttributesList() const;

  // собственно что мне нужно для создания интеракции, тип, данные для типа (какие?)
  // мне нужно будет указать плоскость,
  // указать AbilityType для проджектайлов
  // у меня теперь подход то меняется, теперь я проджектайлы создаю при любых обстоятельствах
  // но тем не менее нужно указать логику взаимодействия
  // я кстати теперь могу вызывать взаимодействия внутри этого класса (хотя может лучше их как то группировать)
private:
  Type abilityId;
  Type impactEvent;

  AbilityTypeT type;

  std::string abilityName;
  std::string abilityDesc;

  const ItemType* tempWeapon;

  size_t abilityCastTime;
  size_t abilityCooldown;

  AbilityCost costs[MAX_ATTRIBUTES_COST_COUNT];

//   InteractionCreateInfo intCreateInfo;
  size_t interactionDelayTime; // проблема - delay задает не абилка а оружее

  //const EntityCreateType* entityCreateType;
  Type entityType;

  ComputeTransformFunction func;
  ComputeAttributesFunction attribsFunc;

  std::vector<const Effect*> impactEffectsList;

  std::vector<AbilityAttributeListElement<INT_ATTRIBUTE_TYPE>> intAttribs;
  std::vector<AbilityAttributeListElement<FLOAT_ATTRIBUTE_TYPE>> floatAttribs;
};

#endif
