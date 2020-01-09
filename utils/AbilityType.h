#ifndef ABILITY_TYPE_H
#define ABILITY_TYPE_H

#include "Utility.h"
#include "Attributes.h"
#include "AttributesComponent.h"
#include "shared_memory_constants.h"

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
  simd::vec4 dir;
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

// абилка - это и есть действие
// нужно продумать несколько действий внутри абилки
// абилка до каста, абилка сразу после каста, абилка после использования абилки
// как то вот так

// сколько не думаю не могу нормально продумать абилки
// вечно какой то идиотизм
// что я понял до сих пор (26.11.2019):
// абилка должна производить некое действие,
// где действие - это создание энтити, которое собственно и совершает действие 
// (действие задается либо деревом поведения либо просто функцией)
// это мне более менее понятно, как к этому подойти?
// мне необходимо при использовании абилки во первых отнять 
// какое то количество ресурса, а во вторых проиграть некую анимацию каста
// видимо все же абилка должна быть от состояния, а не наоборот
// что с ресурсом? если абилка от состояния значит что состояние не должно проигрываться
// если абилке не хватает ресурса, и вот проблема
// ко всему прочему мне бы еще сделать цепочку абилок

// отнимаем ресурс -> анимация каста (если есть) -> состояние непосредственного использования -> абилка через какое то время начинает действовать
// две анимации идут друг за другом, че с оружием? у оружия определена абилка при атаке, нужно сделать наверное вот как
// нужно сделать цепь абилок, абилка должна быть задана так: стоимость, состояние, данные для создания энтити или эффекты (можно один), кулдаун? (пока не знаю) и следующая абилка в цепи

class AbilityType {
public:
  using ComputeTransformFunction = std::function<TransOut(const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>> &, const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>> &, const TransIn &)>;
  using ComputeAttributesFunction = std::function<void(const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>> &, const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>> &, const std::vector<AbilityAttributeListElement<FLOAT_ATTRIBUTE_TYPE>> &, const std::vector<AbilityAttributeListElement<INT_ATTRIBUTE_TYPE>> &, std::vector<AttributeComponent::InitInfo<FLOAT_ATTRIBUTE_TYPE>> &, std::vector<AttributeComponent::InitInfo<INT_ATTRIBUTE_TYPE>> &)>;
  
  struct EntityCreateInfo {
    size_t delayTime;
    Type entityType;

    ComputeTransformFunction func;
    ComputeAttributesFunction attribsFunc;
    
    //std::vector<const Effect*> impactEffectsList;
    
    // мне нужны где какие то данные о интеракции
    // логично их расположить при создании энтити
    // да лучше добавить это в энтити
  };
  
  struct CreateInfo {
    Type abilityId;

    AbilityTypeT type;

    std::string abilityName;
    std::string abilityDesc;

    // это у нас либо каст, либо непосредственное применение
    // то есть огненный шар: сначало проходит этот стейт, затем мы в руки получаем оружие
    // либо непосредственно абилка: кастуется, через делэй создается энтити
    Type abilityState;
    size_t abilityCooldown;

    const Effect* abilityCost;
    
    // может ли это быть эффектом который мы передаем другому объекту?
    // мы можем этот список указать в энтити, как добавлять/удалять из этого списка?
    // по идее модификаторы приходящие из эффектов должны иметь какое то влияние на этот список
    const Effect* abilityEffect;
    size_t delayTime;
    Type entityType;
    ComputeTransformFunction func;
    ComputeAttributesFunction attribsFunc;
    
    // должны ли мы использовать строго фиксированное количество аттрибутов?
    // если мы зададим число вроде 256, то этого скорее всего будет достаточно для всего
    // думаю будет достаточно даже если мы зададим 64 или даже 32
    std::vector<AbilityAttributeListElement<INT_ATTRIBUTE_TYPE>> intAttribs;
    std::vector<AbilityAttributeListElement<FLOAT_ATTRIBUTE_TYPE>> floatAttribs;
    
    // скорее всего здесь нужно указать импакт эффекты
    // так еще больше обобщим энтити
//     std::vector<const Effect*> impactEffects;
//     const Effect* impactEffects[IMPACT_EFFECT_MAX_COUNT];
    // правда невозможно добавить какие то эффекты дополнительно
    // как добавлять? полезно оставить все как было
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

//   const ItemType* temporaryWeapon() const;

//   Type event() const;             // должно ли это быть здесь?

  //size_t castTime() const;
  Type state() const;
  size_t cooldown() const;
//   size_t delayTime() const;

  const Effect* cost() const;
  
  // проблема этого подхода в том, что мы должны заранее знать какие абилки следуют за какими
  // в принципе это на этапе создания нам известно
  const AbilityType* next() const;
  void setNext(const AbilityType* a);

  const Effect* effect() const;
  Type entityCreatorType() const;
  size_t delay() const;
  
  // скорее всего теперь один энтити и один эффект
//   const std::vector<EntityCreateInfo> & entityInfos() const;
//   const std::vector<const Effect*> & effects() const;
  
  TransOut computeTransform(const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>> &float_finder, const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>> &int_finder, const TransIn &transform) const;
  void computeAttributes(const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>> &float_finder, const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>> &int_finder, std::vector<AttributeComponent::InitInfo<FLOAT_ATTRIBUTE_TYPE>> &float_attribs, std::vector<AttributeComponent::InitInfo<INT_ATTRIBUTE_TYPE>> &int_attribs) const;
  
  const std::vector<AbilityAttributeListElement<FLOAT_ATTRIBUTE_TYPE>> & floatAttributes() const;
  const std::vector<AbilityAttributeListElement<INT_ATTRIBUTE_TYPE>> & intAttributes() const;
  
  bool hasTransformFunc() const;
  bool hasComputeFunc() const;
private:
  Type abilityId;
//   Type impactEvent;

  AbilityTypeT type;

  std::string abilityName;
  std::string abilityDesc;

//   const ItemType* tempWeapon;

  // пока не понятно что в цепочке должно происходить с состояниями?
  // по идее они должны тоже проигрываться, должны ли они проигрываться сразу после текущего стейта?
  // или же если у них стейта нет, то сразу срабатывать? это вопрос, 
  // но для того чтобы сделать модификатор атаки нужно чтобы абилка без стейта срабатывала сразу
  Type abilityState; // если стейт не определен, то сразу делаем энтити
  size_t abilityCooldown;

  // каждая абилка в цепочке должна проверить 
  // хватает ли ей ресурса, цепочка останавливается
  // когда какой то из абилок не хватает ресурсов
  // кост может быть не задан
  const Effect* abilityCost;
  
  const AbilityType* nextAbility;
  
//   std::vector<EntityCreateInfo> entityCreateInfos;
//   std::vector<const Effect*> abilityEffects;
  
  const Effect* abilityEffect;
  size_t delayTime;
  Type entityType;
  ComputeTransformFunction func;
  ComputeAttributesFunction attribsFunc;
  
  // дополнительные вещи для того чтобы посчитать аттрибуты энтити
  std::vector<AbilityAttributeListElement<INT_ATTRIBUTE_TYPE>> intAttribs;
  std::vector<AbilityAttributeListElement<FLOAT_ATTRIBUTE_TYPE>> floatAttribs;
};

#endif
