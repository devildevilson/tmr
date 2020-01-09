#ifndef EFFECT_H
#define EFFECT_H

// все эффекты по идее должны быть зарегистрированы заранее
// эффект должен включать в себя иконку, название и описание как минимум
// а лучше еще и автоматически генерируемое описание изменений

#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

#include "Attributes.h"

struct EffectType {
  uint32_t container;
  
  EffectType();
  EffectType(const EffectType &type);
  EffectType(const bool raw, const bool add, const bool remove, const bool periodically_apply, const bool compute_effect, const bool resist, const bool one_time_effect, const bool timer_reset, const bool stackable, const bool easy_stack);
  void make(const bool raw, const bool add, const bool remove, const bool periodically_apply, const bool compute_effect, const bool resist, const bool one_time_effect, const bool timer_reset, const bool stackable, const bool easy_stack);
  
  bool raw() const;
  bool add() const;
  bool remove() const;
  bool periodically_apply() const;
  bool compute_effect() const;
  bool can_be_resisted() const;

  bool one_time_effect() const; // мы просто должны добавить изменения аттрибутов, но не добавлять в буфер компонента эффектов
  bool timer_reset() const; // что мы должны сделать если добавляем второй раз?
  bool stackable() const;
  bool easy_stack() const; // увеличиваем счетчик стаков + добавляем изменения аттрибутов
  // нам бы конечно уникальное время бы для каждого стака, видимо это не изи стаки тогда
};

struct BonusType {
  Bonus bonus;
  //TypelessAttributeType type;
  Type type;
};

struct BonusTypesContainer {
  BonusTypesContainer(const size_t &size);

  void clear();

  BonusType* array;
  size_t size;
};

// стаки эффекта можно сделать с помощью дополнительного добавления бонусов
struct ComputedEffectContainer {
  ComputedEffectContainer(const size_t &size);

  size_t time;
  size_t period_time;
  //std::vector<BonusType> bonusTypes; // копирование std vector портит вообще все
  BonusTypesContainer bonusTypes;
};

//: public EventFunctor
class Effect {
public:
  using FuncType = std::function<void(const Effect*, const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>>&, const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>>&, ComputedEffectContainer*)>;
  
  // зачем мне это нужно?
  struct EventModificator {
    Type event;
    const Effect* effect;
  };

  struct CreateInfo {
    Type id;

    EffectType effectType;
    size_t baseEffectTime;
    size_t basePeriodTime;

    std::string name;
    std::string description;
    std::vector<BonusType> types;
//     std::vector<EventModificator> mods;
    FuncType compute;
    FuncType resist;
//    std::function<event(const Type&, const EventData&, yacs::entity* entity)> eventFunc;
  };
  Effect(const CreateInfo &info);
  ~Effect();
  
  EffectType type() const;
//   Bonus baseBonus() const;
  const std::vector<BonusType> & baseValues() const;
  const std::vector<EventModificator> & modificators() const;
  size_t baseEffectTime() const;
  size_t basePeriodTime() const;
  std::string name() const;
  std::string description() const;
  Type id() const;
  
  void compute(const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>> &float_attribs, const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>> &int_attribs, ComputedEffectContainer* container) const;
  void resist (const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>> &float_attribs, const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>> &int_attribs, ComputedEffectContainer* container) const;
  
  void setModificators(const std::vector<EventModificator> &mods);

  // не помню для чего я это делал, но в случае Effect нужно делать эту функцию константной
  // теперь вероятно не потребуется
//  event call(const Type &type, const EventData &data, yacs::entity* entity) override;
private:
  // тип: рав не рав, инт не инт, нужно ли его удалять наверное, что еще? переодическое использование 
  // как сделать силу эффекта зависимую от характеристик? то есть изменения коснуться Bonus'а и времени эффекта
  // а значит по крайней мере эти данные должны быть сохранены отдельно
  // функция от характеристик? скорее всего, нужно ли делать обновление данных каждое некоторое время?
  // было бы неплохо, но с этим есть большие проблемы в данный момент
  
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

  // эффект может добавлять эвент эффекты, то есть например улучшать атаку на определенное время
  // просто последовательно добавлять? или здесь можно что то придумать другое?
  // модификаторов будет явно очень мало, но они скорее всего будут очень нужны
  // некоторые модификаторы атаки лучше всего сделать отдельным эффектом, чтобы потом легче было отделить одно от другого
  std::vector<EventModificator> mods;

  std::function<void(const Effect*, const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>>&, const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>>&, ComputedEffectContainer*)> computefunc;
  std::function<void(const Effect*, const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>>&, const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>>&, ComputedEffectContainer*)> resistfunc;
  
  // иконка - просто текстурка, мы ее будем рисовать в специальном месте

  // может быть мне здесь нужно подписываться на интеракцию
  // почему бы и нет
//  std::function<event(const Type&, const EventData&, yacs::entity* entity)> eventFunc;
};

#endif
