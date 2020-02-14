#ifndef ABILITY_TYPE_H
#define ABILITY_TYPE_H

#include "Utility.h"
#include "Attributes.h"
#include "AttributesComponent.h"
#include "shared_memory_constants.h"

#define MAX_ATTRIBUTES_COST_COUNT 3

// короч, я посмотрел как сделано в думе
// в думе у оружия определены все анимации состояний
// мне бы тоже нужно определить состояния оружия и абилок
// абилка: каст, непосредственно отправка заклинания, состояние готовности, upstate?, downstate?
// оружее: готовность, атака, перезарядка?, upstate?, downstate?
// патроны? основная задача проверить не кончились ли, а так можно использовать все что угодно
// как сделать последующий урон? то есть от какой то магии "заряженным" оружием
// наносить урон не заряженным оружием, как?
// где хранить аттрибуты? по идее нормально их держать именно у основного энтити
// 

// анимация смерти, после окончания этой анимации удаляем объект
// как ее сделать? по идее нам нужно несколько данных: анимация смерти и абилка
// выпадающий айтем тоже

namespace yacs {
  class entity;
}

class ItemType;
class Effect;
class InventoryComponent;
class Animation;
class DelayedWorkSystem;

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

// не помешало бы все же отказаться от состояний
struct AbilityType {
  struct attribs {
    uint32_t container;
    
    attribs();
    attribs(const bool inheritTransform, const bool inheritAttributes, const bool inheritEffects, const bool no_cast);
    void make(const bool inheritTransform, const bool inheritAttributes, const bool inheritEffects, const bool no_cast);
    bool inheritTransform() const;
    bool inheritAttributes() const;
    bool inheritEffects() const;
    bool no_cast() const;
  };
  
  template <typename T>
  struct AttributeListElement {
    const AttributeType<T>* type;
    T baseValue;
  };
  
  using ComputeTransformFunction = std::function<TransOut(const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>> &, const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>> &, const TransIn &)>;
  using ComputeAttributesFunction = std::function<void(const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>> &, const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>> &, const std::vector<AttributeListElement<FLOAT_ATTRIBUTE_TYPE>> &, const std::vector<AttributeListElement<INT_ATTRIBUTE_TYPE>> &, std::vector<AttributeComponent::InitInfo<FLOAT_ATTRIBUTE_TYPE>> &, std::vector<AttributeComponent::InitInfo<INT_ATTRIBUTE_TYPE>> &)>;
  
  Type id;
  std::string name;
  std::string description;
  struct attribs attribs;
//   Type cast;
//   Type send;
//   Type ready;
  // в связи с тем о чем я написал ниже анимаций, списка аттрибутов, функций может не быть
  // хотя какие то данные все же должны быть: стейты, тип энтити?, эффект?
  // + что то для интерфейса
  const Animation* cast;
  const Animation* send;
  const Animation* ready;
  size_t cooldown;
  const Effect* cost; // ???
  size_t delay;
  const Effect* effect;
  Type entityType;
  ComputeTransformFunction transformFunc;
  ComputeAttributesFunction attribsFunc;
  std::vector<AttributeListElement<INT_ATTRIBUTE_TYPE>> intAttribs;
  std::vector<AttributeListElement<FLOAT_ATTRIBUTE_TYPE>> floatAttribs;
};

// этот компонент для всех
// нужно ли добавлять/удалять абилки? нет
class AbilitiesComponent {
public:
  struct CreateInfo {
    yacs::entity* ent;
    const Animation* defaultAnim;
    DelayedWorkSystem* removeEnt;
    std::vector<const AbilityType*> abilities;
  };
  AbilitiesComponent(const CreateInfo &info);
  
  void update();
  
  bool cast(const Type &id);
  bool casting_ability() const;
  const AbilityType* current_ability() const;
  size_t casting_time() const;
  size_t current_time() const;
  void cancel();
private:
  bool delayed;
  yacs::entity* ent;
  const AbilityType* current;
  yacs::entity* abilityEnt;
  const Animation* defaultAnim;
  DelayedWorkSystem* removeEnt;
  std::vector<const AbilityType*> abilities;
  
  bool checkCostAndSub(const AbilityType* ability);
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

// думаю что нужно разнести все же компонент для игрока и компонент для монстров
// у игрока есть потребность тесного взаимодействия абилок и оружия
// есть просто абилки которые могут быть, есть абилки которые определяются оружием,
// есть абилки которые дают игроку оружие, так что для игрока важно сделать видимо совместный компонент

// как сделать атаку война? 
// размашистый удар, заканчивается анимацией выставлением рук в нормальное положение
// после чего из этой позиции такой же удар идет в другую сторону
// ультимативно бы сделать тогда еще анимацию прерывания
// как сделать модификатор к атаке? я думал на счет этого, последовательный вызов абилок
// но как сделать модифицируемый последовательный вызов абилок? 
// или даже как сделать правильно модификатор именно к урону?
// по идее одноразовый модификатор - это неплохая тема, 
// но тогда нам модификаторы все равно надо вытащить из непосредственно абилок
// 
struct WeaponType {
  struct attribs {
    uint32_t container;
    
    attribs();
    bool two_state_weapon() const;
  };
  
//   Type ready[2];
//   Type attack[2];
  Type attackFailed[2];
  Type reloading;
  const AbilityType* ability[2];
};

// это только игрока
class WeaponComponent2 {
public:
  const WeaponType* get(const size_t &index) const;
  void add(const WeaponType* weapon);
  bool has(const WeaponType* type) const;
private:
  // нужна проверка на наличие оружее
  const WeaponType** weapons;
};

struct ItemType2 {
  Type entityType;
  const AbilityType* ability;
};

// либо мы берем объект в инвентарь, либо применяем эффект
struct PickupItem2 {
  const ItemType2* type;
  size_t quantity;
  const Effect* effect;
};

// я так подозреваю что это только для игрока
class InventoryComponent2 {
public:
  size_t add(const ItemType2* type, const size_t &quantity);
  size_t remove(const ItemType2* type, const size_t &quantity);
  size_t has(const ItemType2* type);
  
  // сортировка? + доступ к вектору
private:
  struct Item {
    const ItemType2* type;
    size_t quantity;
  };
  
  std::vector<Item> items;
};

// actionf_t отвечает видимо за все взаимодействия, то есть существует стейт
// выбор оружия, actionf_t это A_Raise, который вызывается каждый кадр с некой 
// переменной, которая прибавляется к высоте оружия, пока не дойдет до какого то 
// определенного значения, после чего стейт изменится на Ready, где будет вызываться 
// функция A_WeaponReady, она в свою очередь отвечает за покачивание оружия
// при перемещении, проверяет нажатие клавиш, в файле p_pspr.c описание функций
// все функции выглядят как очень логично разбитые базовые действия применяемые
// к игроку (некоторые функции только для игрока, остальные для mobj_t (базовый класс бля всех объектов кажется))
// мне необходимо сделать то же самое, а что самое главное можно все сделать так
// чтобы и в луа перенести и в мультипотоке легко это работало, осталось только 
// понять что такое pspdef_t (подписано как Overlay view sprites)
// связано со спрайтами игрока (то есть со спрайтами рисующимися непосредственно на экране)
// что это более точно я пока не знаю
// тем не менее теперь нужно понять как мне в свою очередь сделать actionf_t
// это функции в которых объекты получают непосредственно "инструкции" к действию
// мне необходимо определить базовый набор инструкций 
// с помощью которых я уже буду моделировать более сложное поведение
// в основном это касается переключения состояний, запуск звуков, эффекты?, 
// интеракции? + ко всему эти функции касаются ли колизии?

// struct AbilityCost {
//   // тип атрибута бы тоже сделать как тип итема, обращаться к нему по указателю
//   const AttributeType<INT_ATTRIBUTE_TYPE>* type;
//   INT_ATTRIBUTE_TYPE cost;
// };
// 
// struct AbilityTypeT {
//   uint32_t container;
// 
//   AbilityTypeT();
// 
//   AbilityTypeT(const bool trans, const bool effects, const bool attribs, const bool tempWeapon);
// 
//   void make(const bool trans, const bool effects, const bool attribs, const bool tempWeapon);
// 
//   bool inheritTransform() const;
// 
//   bool inheritEffects() const;
// 
//   bool inheritAttributes() const;
// 
//   bool createTemporaryWeapon() const;
// 
//   bool eventModificator() const;
// };
// 
// // предусмотреть копирование (можно отметить как UINT32_MAX)
// template<typename T>
// struct AbilityAttributeListElement {
//   const AttributeType<T>* type;
//   T baseValue;
// };
// 
// // информация об интеракции может быть записана в создателе
// // нам необходимо только указать delayTime (может быть разное оружее используещее одну и туже абилку), event (таже абилка делающаяя другие вещи)
// // struct InteractionCreateInfo {
// //   enum Interaction::type type;
// // 
// //   float plane[4]; // это по идее тоже переменная величина
// //   // как сделать плоскость переменной? никак скорее всего, но она нужна чтобы посчитать где у нас происходит сейчас атака
// //   //float distance; // это должно задаваться аттрибутом (дальность атаки)
// // //   float attackAngle; // скорее всего очень многие вещи - это аттрибут
// // //   uint32_t tickCount;
// // 
// //   // плоскость и тип можно легко указать в создателе
// //   // коллизию тоже, делэй? эвент? аттрибуты?
// //   // кроме делэя все указывается в создателе
// //   // делэй, 
// // 
// //   uint32_t collisionGroup;
// //   uint32_t collisionFilter;
// // 
// //   // скорее всего все аттрибуты обязаны быть
// //   const AttributeType<FLOAT_ATTRIBUTE_TYPE>* attackDistance;
// //   const AttributeType<FLOAT_ATTRIBUTE_TYPE>* attackSpeed;
// //   const AttributeType<FLOAT_ATTRIBUTE_TYPE>* attackAngle;
// //   const AttributeType<INT_ATTRIBUTE_TYPE>* tickCount;
// //   const AttributeType<INT_ATTRIBUTE_TYPE>* tickTime;
// //   size_t delayTime;
// // 
// //   Type event;
// // };
// 
// // абилка - это и есть действие
// // нужно продумать несколько действий внутри абилки
// // абилка до каста, абилка сразу после каста, абилка после использования абилки
// // как то вот так
// 
// // сколько не думаю не могу нормально продумать абилки
// // вечно какой то идиотизм
// // что я понял до сих пор (26.11.2019):
// // абилка должна производить некое действие,
// // где действие - это создание энтити, которое собственно и совершает действие 
// // (действие задается либо деревом поведения либо просто функцией)
// // это мне более менее понятно, как к этому подойти?
// // мне необходимо при использовании абилки во первых отнять 
// // какое то количество ресурса, а во вторых проиграть некую анимацию каста
// // видимо все же абилка должна быть от состояния, а не наоборот
// // что с ресурсом? если абилка от состояния значит что состояние не должно проигрываться
// // если абилке не хватает ресурса, и вот проблема
// // ко всему прочему мне бы еще сделать цепочку абилок
// 
// // отнимаем ресурс -> анимация каста (если есть) -> состояние непосредственного использования -> абилка через какое то время начинает действовать
// // две анимации идут друг за другом, че с оружием? у оружия определена абилка при атаке, нужно сделать наверное вот как
// // нужно сделать цепь абилок, абилка должна быть задана так: стоимость, состояние, данные для создания энтити или эффекты (можно один), кулдаун? (пока не знаю) и следующая абилка в цепи
// 
// class AbilityType {
// public:
//   using ComputeTransformFunction = std::function<TransOut(const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>> &, const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>> &, const TransIn &)>;
//   using ComputeAttributesFunction = std::function<void(const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>> &, const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>> &, const std::vector<AbilityAttributeListElement<FLOAT_ATTRIBUTE_TYPE>> &, const std::vector<AbilityAttributeListElement<INT_ATTRIBUTE_TYPE>> &, std::vector<AttributeComponent::InitInfo<FLOAT_ATTRIBUTE_TYPE>> &, std::vector<AttributeComponent::InitInfo<INT_ATTRIBUTE_TYPE>> &)>;
//   
//   struct EntityCreateInfo {
//     size_t delayTime;
//     Type entityType;
// 
//     ComputeTransformFunction func;
//     ComputeAttributesFunction attribsFunc;
//     
//     //std::vector<const Effect*> impactEffectsList;
//     
//     // мне нужны где какие то данные о интеракции
//     // логично их расположить при создании энтити
//     // да лучше добавить это в энтити
//   };
//   
//   struct CreateInfo {
//     Type abilityId;
// 
//     AbilityTypeT type;
// 
//     std::string abilityName;
//     std::string abilityDesc;
// 
//     // это у нас либо каст, либо непосредственное применение
//     // то есть огненный шар: сначало проходит этот стейт, затем мы в руки получаем оружие
//     // либо непосредственно абилка: кастуется, через делэй создается энтити
//     Type abilityState;
//     size_t abilityCooldown;
// 
//     const Effect* abilityCost;
//     
//     // может ли это быть эффектом который мы передаем другому объекту?
//     // мы можем этот список указать в энтити, как добавлять/удалять из этого списка?
//     // по идее модификаторы приходящие из эффектов должны иметь какое то влияние на этот список
//     const Effect* abilityEffect;
//     size_t delayTime;
//     Type entityType;
//     ComputeTransformFunction func;
//     ComputeAttributesFunction attribsFunc;
//     
//     // должны ли мы использовать строго фиксированное количество аттрибутов?
//     // если мы зададим число вроде 256, то этого скорее всего будет достаточно для всего
//     // думаю будет достаточно даже если мы зададим 64 или даже 32
//     std::vector<AbilityAttributeListElement<INT_ATTRIBUTE_TYPE>> intAttribs;
//     std::vector<AbilityAttributeListElement<FLOAT_ATTRIBUTE_TYPE>> floatAttribs;
//     
//     // скорее всего здесь нужно указать импакт эффекты
//     // так еще больше обобщим энтити
// //     std::vector<const Effect*> impactEffects;
// //     const Effect* impactEffects[IMPACT_EFFECT_MAX_COUNT];
//     // правда невозможно добавить какие то эффекты дополнительно
//     // как добавлять? полезно оставить все как было
//   };
// 
//   AbilityType(const CreateInfo &info);
//   ~AbilityType();
//   
//   Type id() const;
//   std::string name() const;
//   std::string description() const;
// 
//   // я уже несколько раз повторял что нужно разделить контейнер эффектов которые мы передаем от остального
// 
//   bool inheritTransform() const;  // если не наследует трансформ, то нужно понять где создать, наиболее сложная задача видимо
//   bool inheritEffects() const;    // если не наследует эффекты, то нужно какие то эффекты добавить, тоже список по идее
//   bool inheritAttributes() const; // с атрибутами проще, список атрибутов мы можем легко задать
// 
//   bool createTemporaryWeapon() const;
// 
// //   const ItemType* temporaryWeapon() const;
// 
// //   Type event() const;             // должно ли это быть здесь?
// 
//   //size_t castTime() const;
//   Type state() const;
//   size_t cooldown() const;
// //   size_t delayTime() const;
// 
//   const Effect* cost() const;
//   
//   // проблема этого подхода в том, что мы должны заранее знать какие абилки следуют за какими
//   // в принципе это на этапе создания нам известно
//   const AbilityType* next() const;
//   void setNext(const AbilityType* a);
// 
//   const Effect* effect() const;
//   Type entityCreatorType() const;
//   size_t delay() const;
//   
//   // скорее всего теперь один энтити и один эффект
// //   const std::vector<EntityCreateInfo> & entityInfos() const;
// //   const std::vector<const Effect*> & effects() const;
//   
//   TransOut computeTransform(const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>> &float_finder, const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>> &int_finder, const TransIn &transform) const;
//   void computeAttributes(const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>> &float_finder, const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>> &int_finder, std::vector<AttributeComponent::InitInfo<FLOAT_ATTRIBUTE_TYPE>> &float_attribs, std::vector<AttributeComponent::InitInfo<INT_ATTRIBUTE_TYPE>> &int_attribs) const;
//   
//   const std::vector<AbilityAttributeListElement<FLOAT_ATTRIBUTE_TYPE>> & floatAttributes() const;
//   const std::vector<AbilityAttributeListElement<INT_ATTRIBUTE_TYPE>> & intAttributes() const;
//   
//   bool hasTransformFunc() const;
//   bool hasComputeFunc() const;
// private:
//   Type abilityId;
// //   Type impactEvent;
// 
//   AbilityTypeT type;
// 
//   std::string abilityName;
//   std::string abilityDesc;
// 
// //   const ItemType* tempWeapon;
// 
//   // пока не понятно что в цепочке должно происходить с состояниями?
//   // по идее они должны тоже проигрываться, должны ли они проигрываться сразу после текущего стейта?
//   // или же если у них стейта нет, то сразу срабатывать? это вопрос, 
//   // но для того чтобы сделать модификатор атаки нужно чтобы абилка без стейта срабатывала сразу
//   Type abilityState; // если стейт не определен, то сразу делаем энтити
//   size_t abilityCooldown;
// 
//   // каждая абилка в цепочке должна проверить 
//   // хватает ли ей ресурса, цепочка останавливается
//   // когда какой то из абилок не хватает ресурсов
//   // кост может быть не задан
//   const Effect* abilityCost;
//   
//   const AbilityType* nextAbility;
//   
// //   std::vector<EntityCreateInfo> entityCreateInfos;
// //   std::vector<const Effect*> abilityEffects;
//   
//   const Effect* abilityEffect;
//   size_t delayTime;
//   Type entityType;
//   ComputeTransformFunction func;
//   ComputeAttributesFunction attribsFunc;
//   
//   // дополнительные вещи для того чтобы посчитать аттрибуты энтити
//   std::vector<AbilityAttributeListElement<INT_ATTRIBUTE_TYPE>> intAttribs;
//   std::vector<AbilityAttributeListElement<FLOAT_ATTRIBUTE_TYPE>> floatAttribs;
// };

#endif
