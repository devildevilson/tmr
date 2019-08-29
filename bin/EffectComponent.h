#ifndef EFFECT_COMPONENT_H
#define EFFECT_COMPONENT_H

#include "EntityComponentSystem.h"
#include "AttributesComponent.h"
#include "MemoryPool.h"
#include "EventFunctor.h"

// все эффекты по идее должны быть зарегистрированы заранее
// эффект должен включать в себя иконку, название и описание как минимум
// а лучше еще и автоматически генерируемое описание изменений

struct EffectType {
  uint32_t container;
  
  EffectType();
  EffectType(const EffectType &type);
  EffectType(const bool raw, const bool add, const bool remove, const bool periodically_apply, const bool compute_effect, const bool resist);
  void make(const bool raw, const bool add, const bool remove, const bool periodically_apply, const bool compute_effect, const bool resist);
  
  bool raw() const;
  bool add() const;
  bool remove() const;
  bool periodically_apply() const;
  bool compute_effect() const;
  bool can_be_resisted() const;

  bool one_time_effect() const; // нужно очень быстро это обработать
};

struct BonusType {
  Bonus bonus;
  TypelessAttributeType type;
};

struct EffectFuncRet {
  size_t time;
  size_t period_time;
  std::vector<BonusType> bonusTypes; // копирование std vector портит вообще все
};

class Effect : public EventFunctor {
public:
  struct CreateInfo {
    Type id;

    EffectType effectType;
    size_t baseEffectTime;
    size_t basePeriodTime;

    std::string name;
    std::string description;
    std::vector<BonusType> types;
    std::function<EffectFuncRet(const size_t&, const size_t&, const std::vector<BonusType>&, const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>>&, const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>>&)> compute;
    std::function<EffectFuncRet(const size_t&, const size_t&, const std::vector<BonusType>&, const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>>&, const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>>&)> resist;
    std::function<event(const Type&, const EventData&, yacs::entity* entity)> eventFunc;
  };
  Effect(const CreateInfo &info);
  ~Effect();
  
  EffectType type() const;
//   Bonus baseBonus() const;
  std::vector<BonusType> baseValues() const;
  size_t baseEffectTime() const;
  size_t basePeriodTime() const;
  std::string name() const;
  std::string description() const;
  Type id() const;
  
  EffectFuncRet compute(const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>> &float_attrib, const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>> &int_attrib) const;
  EffectFuncRet resist(const size_t &time, const size_t &period_time, const std::vector<BonusType> &bonuses, const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>> &float_attribs, const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>> &int_attribs);

  event call(const Type &type, const EventData &data, yacs::entity* entity) override;
private:
  // тип: рав не рав, инт не инт, нужно ли его удалять наверное, что еще? переодическое использование 
  // как сделать силу эффекта зависимую от характеристик? то есть изменения коснуться Bonus'а и времени эффекта
  // а значит по крайней мере эти данные должны быть сохранены отдельно
  // функция от характеристик? скорее всего, нужно ли делать обновление данных каждое некоторое время?
  // было бы неплохо
  
//   Bonus base;
  size_t baseTime;
  size_t base_period_time;
  
  EffectType typeVar;
  Type effectType;
//   size_t attribType;
  
  // эффект воздействует только на один аттрибут? по хорошему нет
  // разные бонусы на каждый тип аттрибута? полезная особенность
  // а вот висит эффект одно время и период видимо тоже один
  
  std::string nameVar;
  std::string descriptionVar;
  
  std::vector<BonusType> bonusTypes;
  
  // тут наверное нужно вернуть тупл чтоб было удобнее
  // нужно ли таким функциям знать у кого они вызываются?
  // а, и чьи характеристики брать при вычислении всего этого, по идее нужно брать и те и те характеристики
  // даже не так, нужно вычислять воздействие эффекта каждый раз, а вычислить силу эффекта можно и один раз
  // воздействие эффекта вычислить каждый раз невозможно при существующей системе
  std::function<EffectFuncRet(const size_t&, const size_t&, const std::vector<BonusType>&, const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>>&, const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>>&)> computefunc;
  std::function<EffectFuncRet(const size_t&, const size_t&, const std::vector<BonusType>&, const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>>&, const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>>&)> resistfunc;
  
  // иконка - просто текстурка, мы ее будем рисовать в специальном месте

  // может быть мне здесь нужно подписываться на интеракцию
  // почему бы и нет
  std::function<event(const Type&, const EventData&, yacs::entity* entity)> eventFunc;
};

// как передать эффект от одного объекта к другому? 
// эффектов которые мы передаем может быть несколько 
// могут ли в процессе игры добавиться эффекты которые мы передаем? да, конечно
// насколько эффективно будет использовать анордеред_мап + сортируемый вектор?
// нужно будет как то определять наличие изменений, сортировать, обходить мапу и обновлять данные в ней
// скорее всего накладываемых эффектов будет не то чтобы очень много, и в принципе работать скорее всего это будет быстро

class EntityAI;
//class EffectSystem;

class EffectComponent {
public:
  struct CreateInfo {
//    EffectSystem* system;
    AttributeComponent* attribs;
  };
  EffectComponent(const CreateInfo &info);
  ~EffectComponent();
  
  void update(const size_t &time = 0);
  void init(void* userData);
  
  void addEffect(const EffectFuncRet &effectData, Effect* effect, EntityAI* entity);
  void removeEffect(Effect* effect);
  
  void addEventEffect(const Type &event, Effect* effect);
  void removeEventEffect(const Type &event, Effect* effect);
  
  Effect* getNextEventEffect(const Type &event, size_t &counter);
  
//  size_t & index();
private:
  struct TimeEffect {
    size_t currentTime;
    EffectFuncRet effectData;
    Effect* effect;
    // указатель на EntityAI? наверное наиболее адекватный выбор
    EntityAI* entity;
  };
  
  struct EffectData {
    Type event;
    Effect* effect;
  };
  
//  size_t systemIndex;
//  EffectSystem* system;
  AttributeComponent* attribs;
  std::vector<TimeEffect> effects;

  std::vector<EffectData> eventsEffects;
};

class EffectSystem : public Engine, public yacs::system {
public:
  struct CreateInfo {
    dt::thread_pool* pool;
  };
  EffectSystem(const CreateInfo &info);
  ~EffectSystem();
  
  void update(const size_t &time) override;
  
//  void addEffectComponent(EffectComponent* comp);
//  void removeEffectComponent(EffectComponent* comp);

  // создание удаление эффектов будет здесь наверное
  Effect* get(const Type &type) const;
  Effect* create(const Effect::CreateInfo &info);
  void destroy(const Type &type);
private:
  dt::thread_pool* pool;

  std::unordered_map<Type, Effect*> effects;
  MemoryPool<Effect, sizeof(Effect)*20> effectsPool;
//  std::vector<EffectComponent*> components;
};

#endif
