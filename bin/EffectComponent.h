#ifndef EFFECT_COMPONENT_H
#define EFFECT_COMPONENT_H

#include "EntityComponentSystem.h"
#include "AttributesComponent.h"

// все эффекты по идее должны быть зарегистрированы заранее
// эффект должен включать в себя иконку, название и описание как минимум
// а лучше еще и автоматически генерируемое описание изменений

struct EffectType {
  uint32_t container;
  
  EffectType();
  EffectType(const bool raw, const bool add, const bool remove, const bool periodicaly_apply, const bool compute_effect);
  void make(const bool raw, const bool add, const bool remove, const bool periodicaly_apply, const bool compute_effect);
  
  bool raw() const;
  bool add() const;
  bool remove() const;
  bool periodicaly_apply() const;
  bool compute_effect() const;
};

struct BonusType {
  Bonus bonus;
  TypelessAttributeType type;
};

struct EffectFuncRet {
  size_t time;
  size_t period_time;
  std::vector<BonusType> bonusTypes;
};

class Effect {
public:
  struct CreateInfo {
    EffectType effectType;
    size_t baseEffectTime;
    size_t basePeriodTime;
    Type id;
    std::string name;
    std::string description;
    std::vector<BonusType> types;
    std::function<EffectFuncRet(const size_t&, const size_t&, const std::vector<BonusType>&, const AttributeFinder<FLOAT_ATTRIBUTE_TYPE>&, const AttributeFinder<INT_ATTRIBUTE_TYPE>&)> func;
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
  
  EffectFuncRet compute(const AttributeFinder<FLOAT_ATTRIBUTE_TYPE> &float_attrib, const AttributeFinder<INT_ATTRIBUTE_TYPE> &int_attrib) const;
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
  std::function<EffectFuncRet(const size_t&, const size_t&, const std::vector<BonusType>&, const AttributeFinder<FLOAT_ATTRIBUTE_TYPE>&, const AttributeFinder<INT_ATTRIBUTE_TYPE>&)> func;
  
  // иконка - просто текстурка, мы ее будем рисовать в специальном месте
};

// как передать эффект от одного объекта к другому? 
// эффектов которые мы передаем может быть несколько 
// могут ли в процессе игры добавиться эффекты которые мы передаем? да, конечно
// насколько эффективно будет использовать анордеред_мап + сортируемый вектор?
// нужно будет как то определять наличие изменений, сортировать, обходить мапу и обновлять данные в ней
// скорее всего накладываемых эффектов будет не то чтобы очень много, и в принципе работать скорее всего это будет быстро

class EffectComponent : public yacs::Component {
public:
  EffectComponent();
  ~EffectComponent();
  
  void update(const size_t &time = 0) override;
  void init(void* userData) override;
  
  void add_effect(const Type &type);
  void remove_effect(const Type &type);
private:
  struct TimeEffect {
    size_t currentTime;
    EffectFuncRet effectData;
    Effect* effect;
  };
  
  AttributeComponent* attribs;
  std::vector<Effect*> effects;
};

#endif
