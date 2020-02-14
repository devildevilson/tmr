#include "AbilityType.h"

#include "Globals.h"
#include "Effect.h"
#include "EffectComponent.h"
#include "AIComponent.h"
#include "AnimationComponent.h"
#include "EntityLoader.h"
#include "EntityCreator.h"
#include "global_components_indicies.h"
#include "Interactions.h"

#include "DelayedWorkSystem.h"

#define DEFAULT_THICKNESS 0.3f;

enum {
  ABILITY_INHERIT_TRANSFORM = (1<<0),
  ABILITY_INHERIT_EFFECTS = (1<<1),
  ABILITY_INHERIT_ATTRIBUTES = (1<<2),
  ABILITY_INHERIT_NO_CAST = (1<<3)
};

AbilityType::attribs::attribs() : container(0) {}
AbilityType::attribs::attribs(const bool inheritTransform, const bool inheritAttributes, const bool inheritEffects, const bool no_cast) : container(0) {
  make(inheritTransform, inheritAttributes, inheritEffects, no_cast);
}

void AbilityType::attribs::make(const bool inheritTransform, const bool inheritAttributes, const bool inheritEffects, const bool no_cast) {
  container = (uint32_t(inheritTransform)*ABILITY_INHERIT_TRANSFORM) | 
              (uint32_t(inheritAttributes)*ABILITY_INHERIT_EFFECTS) | 
              (uint32_t(inheritEffects)*ABILITY_INHERIT_ATTRIBUTES) | 
              (uint32_t(no_cast)*ABILITY_INHERIT_NO_CAST);
}

bool AbilityType::attribs::inheritTransform() const {
  return (container & ABILITY_INHERIT_TRANSFORM) == ABILITY_INHERIT_TRANSFORM;
}

bool AbilityType::attribs::inheritAttributes() const {
  return (container & ABILITY_INHERIT_EFFECTS) == ABILITY_INHERIT_EFFECTS;
}

bool AbilityType::attribs::inheritEffects() const {
  return (container & ABILITY_INHERIT_ATTRIBUTES) == ABILITY_INHERIT_ATTRIBUTES;
}

bool AbilityType::attribs::no_cast() const {
  return (container & ABILITY_INHERIT_NO_CAST) == ABILITY_INHERIT_NO_CAST;
}

AbilitiesComponent2::AbilitiesComponent2(const CreateInfo &info) : delayed(false), ent(info.ent), current(nullptr), abilityEnt(nullptr), defaultAnim(info.defaultAnim), removeEnt(info.removeEnt), abilities(info.abilities) {}

void AbilitiesComponent2::update() {
  ASSERT(current->send == nullptr && current->delay == 0);
  auto anims = ent->at<AnimationComponent>(ANIMATION_COMPONENT_INDEX);
  
  if ((current->delay == 0 && !delayed) || (anims.valid() && anims->current_time() >= current->delay && !delayed)) {
    if (current->effect != nullptr) {
      auto attribs = ent->at<AttributeComponent>(ATTRIBUTE_COMPONENT_INDEX);
      auto effects = ent->at<EffectComponent>(EFFECT_COMPONENT_INDEX);
      //auto entityAI = ent->at<AIComponent>(AI_COMPONENT_INDEX);
      
      ASSERT(attribs.valid() && effects.valid());
      
      ComputedEffectContainer cont(current->effect->baseValues().size());
      current->effect->compute(attribs->get_finder<FLOAT_ATTRIBUTE_TYPE>(), attribs->get_finder<INT_ATTRIBUTE_TYPE>(), &cont);
      current->effect->resist(attribs->get_finder<FLOAT_ATTRIBUTE_TYPE>(), attribs->get_finder<INT_ATTRIBUTE_TYPE>(), &cont);
      effects->addEffect(cont, current->effect, nullptr);
    }
    
    if (current->entityType.valid()) {
      const ObjectData data{
        {0.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 0.0f},
        0, 0, 0, current
      };
      auto local = Global::get<EntityLoader>()->create(current->entityType, ent, &data);
      if (!local->at<ImpactInteraction>(INTERACTION_INDEX).valid()) abilityEnt = local;
    }
    
    delayed = true;
  }
}

bool AbilitiesComponent2::casting_ability() const {
  return current != nullptr;
}

bool AbilitiesComponent2::cast(const Type &id) {
  if (casting_ability()) cancel();
  
  for (auto ability : abilities) {
    if (ability->id == id) {
      if (!checkCostAndSub(ability)) return false;
      
      current = ability;
      
      if (ability->send != nullptr) {
        auto anims = ent->at<AnimationComponent>(ANIMATION_COMPONENT_INDEX);
        
        const AnimationComponent::PlayInfoPtr info{
          current->send,
          false,
          1.0f // указать модификатор скорости в самой анимации? наверное
        };
        anims->play(info);
      }
      return true;
    }
  }
  
  return false;
}

const AbilityType* AbilitiesComponent2::current_ability() const {
  return current;
}

size_t AbilitiesComponent2::casting_time() const {
  if (!casting_ability()) return SIZE_MAX;
  
  auto anims = ent->at<AnimationComponent>(ANIMATION_COMPONENT_INDEX);
  return anims->time();
}

size_t AbilitiesComponent2::current_time() const {
  // вернуть время, скорее всего нужно взять их анимации
  if (!casting_ability()) return SIZE_MAX;
  
  auto anims = ent->at<AnimationComponent>(ANIMATION_COMPONENT_INDEX);
  return anims->current_time();
}

void AbilitiesComponent2::cancel() {
  if (current == nullptr) return;
  // нужно остановить анимацию и звук
  // как отменить созданный энтити? его нужно создать так чтобы я получил указатель
  // нужно учесть аттрибут скорости, или не нужно?
  
  // дефолтная анимация? она в любом случае будет указана при создании, можно ее добавить здесь
  // может ли у нас указатель abilityEnt отвалиться? может, в каких случаях? если anims->current_time() >= anims->time()
  
  auto anims = ent->at<AnimationComponent>(ANIMATION_COMPONENT_INDEX);
  if (anims.valid()) {
    if (anims->current_time() >= anims->time()) abilityEnt = nullptr;
    const AnimationComponent::PlayInfoPtr info{
      defaultAnim,
      true,
      1.0f
    };
    anims->play(info);
  }
  
  // нужно удалить abilityEnt, как это делается?
  // нужно учесть три вещи: оповестить что этот энтити скоро удалится в ИИ компонентах
  // понять был ли создан этот энтити с помощью абилки (для некоторых случаев)
  // с учетом всего этого добавить к удалению
  // абилку нужно доавить в юзер дату
  // удалить нам надо обязательно удостоверившись чтобы все перестали на этот объект ссылаться
  // для этого нам нужно отметить ии к удалению (ии будут скорее всего только у проджектайлов)
  // тут указатель только на те энтити у которых с вероятностью 99% не будет никакого ии
  if (abilityEnt != nullptr) {
    auto local = abilityEnt;
    abilityEnt = nullptr;
    removeEnt->add_work([local] () {
      Global::get<yacs::world>()->destroy_entity(local);
    });
  }
  
  current = nullptr;
  delayed = false;
}

bool AbilitiesComponent2::checkCostAndSub(const AbilityType* ability) {
  if (ability->cost == nullptr) return true;
  
  auto attribs = ent->at<AttributeComponent>(ATTRIBUTE_COMPONENT_INDEX);
  auto effects = ent->at<EffectComponent>(EFFECT_COMPONENT_INDEX);
  auto entityAI = ent->at<AIComponent>(AI_COMPONENT_INDEX);
  
  ComputedEffectContainer cont(ability->cost->baseValues().size());
  ability->cost->compute(attribs->get_finder<FLOAT_ATTRIBUTE_TYPE>(), attribs->get_finder<INT_ATTRIBUTE_TYPE>(), &cont);
  ability->cost->resist(attribs->get_finder<FLOAT_ATTRIBUTE_TYPE>(), attribs->get_finder<INT_ATTRIBUTE_TYPE>(), &cont);
  
  for (size_t i = 0; i < cont.bonusTypes.size; ++i) {
    {
      auto attrib = attribs->get<FLOAT_ATTRIBUTE_TYPE>(cont.bonusTypes.array[i].type);
      // хороший вопрос какой знак использовать, по идее все должно быть по аналогии с другими вещами
      if (attrib != nullptr && (attrib->value() + cont.bonusTypes.array[i].bonus.add <= 0.0f)) {
        cont.bonusTypes.clear();
        return false;
      }
    }
    
    {
      auto attrib = attribs->get<INT_ATTRIBUTE_TYPE>(cont.bonusTypes.array[i].type);
      // хороший вопрос какой знак использовать, по идее все должно быть по аналогии с другими вещами
      if (attrib != nullptr && (attrib->value() + cont.bonusTypes.array[i].bonus.add <= 0.0f)) {
        cont.bonusTypes.clear();
        return false;
      }
    }
  }
  
  effects->addEffect(cont, ability->cost, entityAI.get());
  return true;
}

AbilityComponentSystem::AbilityComponentSystem(const CreateInfo &info) : pool(info.pool) {}
void AbilityComponentSystem::update(const size_t &time) {
  (void)time;
  
  const size_t count = Global::world()->count_components<AbilitiesComponent>();
  const size_t one_thread_count = std::ceil(float(count) / float(pool->size()+1));
  size_t start = 0;
  for (uint32_t i = 0; i < pool->size()+1; ++i) {
    const size_t jobCount = std::min(one_thread_count, count-start);
    if (jobCount == 0) break;
    
    pool->submitbase([start, jobCount] () {
      for (size_t index = start; index < start+jobCount; ++index) {
        auto handle = Global::world()->get_component<AbilitiesComponent>(index);
        handle->update();
      }
    });
    start += jobCount;
  }
  
  pool->compute();
  pool->wait();
}

// несколько простых функций (в хедере писал об этом)
// все функции которые я тут вызываю должны быть по возможности O(const)
// все функции здесь я должен мочь запустить параллельно (ну то есть синхронизация глобальных объектов должна быть)
// как избежать циклических зависимостей? возникает когда мы меняем стейт на стейт с нулевым временем
// который в свою очередь меняет на стейт опять с нулевым временем, а тот обратно на предыдущий
// здесь по идее мы можем ограничить количество смен стейта с помощью функции (ну или вообще)
// неким числом, например 10, если больше 10 то вылетаем с ошибкой
// счетчик мы должны обновлять каждый кадр
// нужно строго определить взаимодействия между объектами

struct graphics_component {
  // текстура + трансформа
  float movx;
  float movy;
  float alpha; 
  // потребуется интерполировать
  // либо опускать не плавно
  // у нас есть константа времени по которому будут обновляться lower_weapon
  
  void displace(const float &mx, const float &my);
  void update(const size_t &time);
};

struct input {
  float x, y, z; // для некоторых вещей скорость должна быть константной
};

#define MAX_BONUSES 32

struct effect_t {
  struct bonus_t {
    float mul;
    float add;
  };
  
  struct bonus_container {
    Type attrib; // тут бы тоже использовать индекс, проблема в том что не у всех объектов аттрибуты расположены на одинаковых местах
    bonus_t bonus;
  };
  
  uint32_t count;
  bonus_container bonuses[MAX_BONUSES];
};

struct effects_component {
  struct computed_effect {
    const effect_t* effect;
    size_t time;
    effect_t container;
  };
  
  std::vector<computed_effect> effects;
};

#define MAX_INT_ATTRIB 16
#define MAX_FLOAT_ATTRIB 16

struct attribute_type_t {
  std::string name;
  std::function<float()> compute_f; // нужно наверное все же перевычислять каждое некоторое время
};

struct attributes_component {
  size_t float_count;
  Attribute<FLOAT_ATTRIBUTE_TYPE> float_attribs[MAX_FLOAT_ATTRIB];
};

// для того чтобы правильно сделать атаку мечем нужно 
// два состояния атаки, два состояния готовности, два довн стейта
// короче говоря нужно сделать просто массив с состояниями
// + нужно придумать механизм простого доступа к ним 
// (то есть какие то глобальные константы, я знаю как сделать это в луа, нужно ли париться по этому в с++? не думаю)
// Type??? 
// нужно как то отделять одно оружие от другого? у нас указатель есть
// а как брать нужное оружие? индекс, не запутаюсь ли я при загрузке? по идее даже в этом случе не нужно
struct state_t;
struct weapon_t {
  const effect_t* cost;
  size_t size;
  const state_t** states;
//   const state_t* attack;
//   const state_t* ready;
//   const state_t* reload;
//   const state_t* down_state;
//   const state_t* up_state;
};

struct weapons_component {
  const weapon_t* current;
  const weapon_t* pending;
  
  bool has() const; // ?
  
};

// нам бы проверять жизнь энтити
// в думе энтити можно было удалить 2-мя способами:
// выставить state в null и обнулить указатель на функцию "думальщика"
// в моем случае можно наверное просто состояние обнулить
struct lifetime_component {
  
};

// да и все, из стейта получаем какое то взаимодействие
// скорее всего нужно сделать компонент с текущей абилкой
// ко всему прочему нужно обрабатывать кулдаун
// абилки могут зависеть от оружия
// независимые абилки будут только у мага,
// можно все абилки сделать зависимыми от оружия
// но у мага каждое оружие обладает всеми абилками
// ну кстати идея норм, кулдауны? монстрам они нахер не нужны по идее
// игрок будет и так стеснен анимацией каста
struct ability_t {
  Type id;
  std::string name;
  std::string description;
  const state_t* cast;
  size_t cooldown;
  const effect_t* cost;
};

struct item_t {
  Type id;
  std::string name;
  std::string description;
  const ability_t* ability;
};

void check_collision(yacs::entity* entity, yacs::entity* obj) {
  obj->get<pickup>();
  remove(obj);
}

bool check_ammo(yacs::entity* ent) {
  // тут в моем случае считаем эффект и смотрим на аттрибуты
  // но в думе патроны вычитались позже по вызовам
  // как сделать в моем случае чтобы не перевычислять?
  // можно вычитать сразу
  
  return false;
}

#define ATTACK_STATE_1 3

void fire_weapon(yacs::entity* ent) {
  // функция которая: проверяет есть ли у меня патроны, делает шум, меняет состояние
  // а уже видимо из состояния запускается звук
  // мы сначало выставляем новое состояние а уже из него запускаем звук и видимо непосредственно обрабатываем выстрел
  
  // если патронов нет, то ничего не делаем больше, 
  // по идее там уже сменилось состояние
  if (!check_ammo(ent)) return;
  
  // если патроны есть то делаем базовые функции
  // в думе меняется стейт у mobj_t и сразу же запускается следующая функция
  // функции может не быть, стейт разделяется для игрока в мире и игрока на экране
  
  // в том числе с помощью стейтов выставляем свет, поднимаем/опускаем оружее
  // например чтобы поменять оружие нам нужно: 
  // переменная для следующего оружия, стейт опускания, стейт поднимания
  // 
  
  auto weapons = ent->get<weapons_component>();
  auto states = ent->get<struct states>();
  states->set_state(weapons->current->states[ATTACK_STATE_1]);
}

#define LOWER_SPEED 0.1f
#define LOWERED 0.5f

// но тут скорее не энтити
void lower_weapon(yacs::entity* ent) {
  float v = 1.0f; // берем из энтити инфу
  v -= LOWER_SPEED;
  
  if (v > LOWERED) return;
  
  // если игрок умер то мы ему оружие должны убрать с экрана и запретить им пользоваться
  // то есть он должен попасть в этот стейт при смерти
  
  const state_t* new_state = nullptr;
  auto states = ent->get<struct states>();
  states->set_state(new_state);
  // берем у оружия новый стейт, который будет стейтом подъема
  // выставляем его игроку и действуем по аналогии с этим стейтом 
  
  // set_state - функция базовая и единственое что должна делать это присваивать стейт переменной
  // все это похоже на бехавиор три, только не бехавиор три управляет стейтами а стейты деревом
  
  // понятное дело нужно задизайнить так чтобы я мог использовать эту же функцию для того чтобы
  // делать особую анимацию 
}

void fireball(yacs::entity* ent) {
  // может быть стоит задать еще какие нибудь вещи
  // вообще неплохо было бы тогда выкинуть расчет скорости/направления/позиции из функций в абилке
  // а сделать так чтобы игрок мог задать эту функцию из луа и сделать расчет самостоятельно
  // то же самое и с аттрибутами, эффекты можно добавить по вкусу так сказать
  // урон оформить бы как эффект, хотя можно передать эффект без аттрибутов
  // в общем задать аттрибуты, эффекты, трансформу
  // ent create_entity (type, parent, ability?, data?)
}

void big_sword_melee_attack(yacs::entity* ent) {
  // тут начинаются проблемы: в думе ближний бой оформлен как луч (кастится и проверяет дальность)
  // я же хочу атаку оформить примерно как взмах реального меча
  // для этого мне по крайней мере нужно создать физический объект-коллайдер
  // хотя если сделать функцию, которая создаст и проследит время жизни интеракции
  
  // state slashing_attack (ent, effect, distance, angle, time, speed_mod, plane, tick_count, tick_time)
  
  // функция создает энтити с трансформой игрока + физика + интеракция, если энтити уже создан
  // то корректируем данные, если функция один раз не будет вызвана, энтити удалится
  // если стейт == доне то меняем стейт энтити
  // ну выглядит не так уж пососно
  
  // поиск пути? скорее всего дерево поведения никуда не девается
}

void gun_fire(yacs::entity* ent) {
  // еще у нас есть потребность прокинуть луч
  // луч мы используем для того чтобы посчитать атаку 
  // и для того чтобы открыть дверь
  // я вполне могу сделать так чтобы игроку возвращался указатель на объект
  // который мы задели лучем
  // что потом?
  // для начала нужно почекать как открытие дверей работает, но судя по всему это 
  // стейт с функцией которая изменяет положение
}

// удаление энтити мы должны сделать так чтобы сначало оповестить все энтити
// о том что нас скоро удалят и убрать у них указатели
void remove(yacs::entity* ent) {
  // выставим флажок, а потом добавим к удалению энтити
  // когда конкретно удалить? у меня должен быть механизм валидации подобных объектов
  // который работает независимо от запуска дерева поведения
}

// enum {
//   ABILITY_INHERIT_TRANSFORM = (1<<0),
//   ABILITY_INHERIT_EFFECTS = (1<<1),
//   ABILITY_INHERIT_ATTRIBUTES = (1<<2),
//   ABILITY_CREATE_TEMP_WEAPON = (1<<3),
//   ABILITY_MODIFICATOR = (1<<4),
// };
// 
// AbilityTypeT::AbilityTypeT() : container(0) {}
// AbilityTypeT::AbilityTypeT(const bool trans, const bool effects, const bool attribs, const bool tempWeapon) : container(0) {
//   make(trans, effects, attribs, tempWeapon);
// }
// 
// void AbilityTypeT::make(const bool trans, const bool effects, const bool attribs, const bool tempWeapon) {
//   container |= (uint32_t(trans)*ABILITY_INHERIT_TRANSFORM) |
//                (uint32_t(effects)*ABILITY_INHERIT_EFFECTS) |
//                (uint32_t(attribs)*ABILITY_INHERIT_ATTRIBUTES) |
//                (uint32_t(tempWeapon)*ABILITY_CREATE_TEMP_WEAPON) |
//                (uint32_t(false)*ABILITY_MODIFICATOR);
// }
// 
// bool AbilityTypeT::inheritTransform() const {
//   return (container & ABILITY_INHERIT_TRANSFORM) == ABILITY_INHERIT_TRANSFORM;
// }
// 
// bool AbilityTypeT::inheritEffects() const {
//   return (container & ABILITY_INHERIT_EFFECTS) == ABILITY_INHERIT_EFFECTS;
// }
// 
// bool AbilityTypeT::inheritAttributes() const {
//   return (container & ABILITY_INHERIT_ATTRIBUTES) == ABILITY_INHERIT_ATTRIBUTES;
// }
// 
// bool AbilityTypeT::createTemporaryWeapon() const {
//   return (container & ABILITY_CREATE_TEMP_WEAPON) == ABILITY_CREATE_TEMP_WEAPON;
// }
// 
// bool AbilityTypeT::eventModificator() const {
//   return (container & ABILITY_MODIFICATOR) == ABILITY_MODIFICATOR;
// }
// 
// AbilityType::AbilityType(const CreateInfo &info)
//   : abilityId(info.abilityId),
//     type(info.type),
//     abilityName(info.abilityName),
//     abilityDesc(info.abilityDesc),
//     abilityState(info.abilityState),
//     abilityCooldown(info.abilityCooldown),
//     abilityCost(info.abilityCost),
//     nextAbility(nullptr),
//     abilityEffect(info.abilityEffect),
//     delayTime(info.delayTime),
//     entityType(info.entityType),
//     func(info.func),
//     attribsFunc(info.attribsFunc),
//     intAttribs(info.intAttribs),
//     floatAttribs(info.floatAttribs) {}
// 
// AbilityType::~AbilityType() {}
// 
// Type AbilityType::id() const {
//   return abilityId;
// }
// 
// std::string AbilityType::name() const {
//   return abilityName;
// }
// 
// std::string AbilityType::description() const {
//   return abilityDesc;
// }
// 
// bool AbilityType::inheritTransform() const {
//   return type.inheritTransform();
// }
// 
// bool AbilityType::inheritEffects() const {
//   return type.inheritEffects();
// }
// 
// bool AbilityType::inheritAttributes() const {
//   return type.inheritAttributes();
// }
// 
// bool AbilityType::createTemporaryWeapon() const {
//   return type.createTemporaryWeapon();
// }
// 
// // const ItemType* AbilityType::temporaryWeapon() const {
// //   return tempWeapon;
// // }
// 
// // Type AbilityType::event() const {
// //   return impactEvent;
// // }
// 
// // size_t AbilityType::castTime() const {
// //   return abilityCastTime;
// // }
// 
// Type AbilityType::state() const {
//   return abilityState;
// }
// 
// size_t AbilityType::cooldown() const {
//   return abilityCooldown;
// }
// 
// const Effect* AbilityType::cost() const {
//   return abilityCost;
// }
// 
// const AbilityType* AbilityType::next() const {
//   return nextAbility;
// }
// 
// void AbilityType::setNext(const AbilityType* a) {
//   nextAbility = a;
// }
// 
// const Effect* AbilityType::effect() const {
//   return abilityEffect;
// }
// 
// Type AbilityType::entityCreatorType() const {
//   return entityType;
// }
// 
// size_t AbilityType::delay() const {
//   return delayTime;
// }
// 
// TransOut AbilityType::computeTransform(const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>> &float_finder, const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>> &int_finder, const TransIn &transform) const {
//   return func(float_finder, int_finder, transform);
// }
// 
// void AbilityType::computeAttributes(const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>> &float_finder, const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>> &int_finder, std::vector<AttributeComponent::InitInfo<FLOAT_ATTRIBUTE_TYPE>> &float_attribs, std::vector<AttributeComponent::InitInfo<INT_ATTRIBUTE_TYPE>> &int_attribs) const {
//   attribsFunc(float_finder, int_finder, floatAttribs, intAttribs, float_attribs, int_attribs);
// }
// 
// bool AbilityType::hasTransformFunc() const {
//   return bool(func);
// }
// 
// bool AbilityType::hasComputeFunc() const {
//   return bool(attribsFunc);
// }
// 
// const std::vector<AbilityAttributeListElement<FLOAT_ATTRIBUTE_TYPE>> & AbilityType::floatAttributes() const {
//   return floatAttribs;
// }
// 
// const std::vector<AbilityAttributeListElement<INT_ATTRIBUTE_TYPE>> & AbilityType::intAttributes() const {
//   return intAttribs;
// }
// 
// // const std::vector<AbilityType::EntityCreateInfo> & AbilityType::entityInfos() const {
// //   return entityCreateInfos;
// // }
// // 
// // const std::vector<const Effect*> & AbilityType::effects() const {
// //   return abilityEffects;
// // }
