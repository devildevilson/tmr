#include "EffectComponent.h"

enum EffectTypeEnum {
  EFFECT_TYPE_RAW = (1 << 0),
  EFFECT_TYPE_ADD = (1 << 1),
  EFFECT_TYPE_REMOVE = (1 << 2),
  EFFECT_TYPE_PERIODICALY_APPLY = (1 << 3),
  EFFECT_TYPE_COMPUTE_EFFECT = (1 << 4),
  EFFECT_TYPE_CAN_RESIST = (1 << 5),
};

EffectType::EffectType() : container(0) {}
EffectType::EffectType(const EffectType &type) : container(type.container) {}
EffectType::EffectType(const bool raw, const bool add, const bool remove, const bool periodicaly_apply, const bool compute_effect, const bool resist) : container(0) {
  make(raw, add, remove, periodicaly_apply, compute_effect, resist);
}

void EffectType::make(const bool raw, const bool add, const bool remove, const bool periodicaly_apply, const bool compute_effect, const bool resist) {
  container |= (raw * EFFECT_TYPE_RAW) | 
               (add * EFFECT_TYPE_ADD) | 
               (remove * EFFECT_TYPE_REMOVE) | 
               (periodicaly_apply * EFFECT_TYPE_PERIODICALY_APPLY) | 
               (compute_effect * EFFECT_TYPE_COMPUTE_EFFECT) | 
               (resist * EFFECT_TYPE_CAN_RESIST);
}

bool EffectType::raw() const {
  return (container & EFFECT_TYPE_RAW) == EFFECT_TYPE_RAW;
}

bool EffectType::add() const {
  return (container & EFFECT_TYPE_ADD) == EFFECT_TYPE_ADD;
}

bool EffectType::remove() const {
  return (container & EFFECT_TYPE_REMOVE) == EFFECT_TYPE_REMOVE;
}

bool EffectType::periodicaly_apply() const {
  return (container & EFFECT_TYPE_PERIODICALY_APPLY) == EFFECT_TYPE_PERIODICALY_APPLY;
}

bool EffectType::compute_effect() const {
  return (container & EFFECT_TYPE_COMPUTE_EFFECT) == EFFECT_TYPE_COMPUTE_EFFECT;
}

bool EffectType::can_be_resisted() const {
  return (container & EFFECT_TYPE_CAN_RESIST) == EFFECT_TYPE_CAN_RESIST;
}

Effect::Effect(const CreateInfo &info) 
 : baseTime(info.baseEffectTime), 
   base_period_time(info.basePeriodTime), 
   typeVar(info.effectType), 
   effectType(info.id), 
   nameVar(info.name), 
   descriptionVar(info.description), 
   bonusTypes(info.types), 
   computefunc(info.compute),
   resistfunc(info.resist) {}

Effect::~Effect() {}

EffectType Effect::type() const {
  return typeVar;
}

std::vector<BonusType> Effect::baseValues() const {
  return bonusTypes;
}

size_t Effect::baseEffectTime() const {
  return baseTime;
}

size_t Effect::basePeriodTime() const {
  return base_period_time;
}

std::string Effect::name() const {
  return nameVar;
}

std::string Effect::description() const {
  return descriptionVar;
}

Type Effect::id() const {
  return effectType;
}

EffectFuncRet Effect::compute(const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>> &float_attrib, const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>> &int_attrib) const {
  return computefunc(baseTime, base_period_time, bonusTypes, float_attrib, int_attrib);
}

EffectFuncRet Effect::resist(const size_t &time, const size_t &period_time, const std::vector<BonusType> &bonuses, const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>> &float_attribs, const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>> &int_attribs) {
  return resistfunc(time, period_time, bonuses, float_attribs, int_attribs);
}

CLASS_TYPE_DEFINE_WITH_NAME(EffectComponent, "EffectComponent")

EffectComponent::EffectComponent(const CreateInfo &info) : systemIndex(0), system(info.system), attribs(nullptr), counter(0) {
  system->addEffectComponent(this);
}

EffectComponent::~EffectComponent() {
  system->removeEffectComponent(this);
}

void EffectComponent::update(const size_t &time) {
  // резист от эффекта как и где расчитывать? по идее нужно расчитывать при добавлении в массив
  // так как потом будет мало слишком данных, как расчитывать? по идее сейчас здесь у нас усиленные бонусы
  // и есть тип к которму мы применяем это дело, где хранить? функцию можно хранить опять в эфектах
  // она будет очень похожа на то что было: время, период, бонусы и типы, аттрибуты
  // мы не можем перерасчитывать эфект так как мы не знаем че удалять будем 
  // то есть резист срабатывает только один раз при добавлении
  
  for (size_t i = 0; i < effects.size(); ++i) {
    effects[i].currentTime += time;
    
    if (effects[i].currentTime >= effects[i].effectData.time) {
      if (effects[i].effect->type().remove()) {
        const std::vector<BonusType> &v = effects[i].effectData.bonusTypes;
        
        for (size_t j = 0; j < v.size(); ++j) {
          const AttribChangeData ad{
            AttribChangeType(effects[i].effect->type().raw(), false),
            v[j].bonus,
            v[j].type,
            effects[i].entity
          };
          attribs->change_attribute(ad);
        }
      }
      
      std::swap(effects[i], effects.back());
      effects.pop_back();
      --i;
    }
    
    const bool period = (effects[i].currentTime % effects[i].effectData.period_time) <= time;
    //effects[i].currentTime >= effects[i].effectData.period_time
    if (effects[i].effect->type().periodicaly_apply() && period) {
      const std::vector<BonusType> &v = effects[i].effectData.bonusTypes;
        
      for (size_t j = 0; j < v.size(); ++j) {
        const AttribChangeData ad{
          AttribChangeType(effects[i].effect->type().raw(), true),
          v[j].bonus,
          v[j].type,
          effects[i].entity
        };
        attribs->change_attribute(ad);
      }
    }
  }
}

void EffectComponent::init(void* userData) {
  (void)userData;
  
  attribs = getEntity()->get<AttributeComponent>().get();
  if (attribs == nullptr) {
    Global::console()->printE("Could not create EffectComponent without attributes");
    throw std::runtime_error("Could not create EffectComponent without attributes");
  }
}

void EffectComponent::addEffect(const EffectFuncRet &effectData, Effect* effect, EntityAI* entity) {
  const EffectFuncRet &resistedData = effect->type().can_be_resisted() ? 
                                          effect->resist(effectData.time, 
                                                         effectData.period_time, 
                                                         effectData.bonusTypes, 
                                                         attribs->get_finder<FLOAT_ATTRIBUTE_TYPE>(), 
                                                         attribs->get_finder<INT_ATTRIBUTE_TYPE>()) : effectData;
  
  if (effect->type().add()) {
    for (size_t j = 0; j < resistedData.bonusTypes.size(); ++j) {
      const AttribChangeData ad{
        AttribChangeType(effect->type().raw(), true),
        resistedData.bonusTypes[j].bonus,
        resistedData.bonusTypes[j].type,
        entity
      };
      attribs->change_attribute(ad);
    }
  }
  
  effects.push_back(TimeEffect{0, resistedData, effect, entity});
}

void EffectComponent::removeEffect(Effect* effect) {
  for (size_t i = 0; i < effects.size(); ++i) {
    if (effects[i].effect == effect) {
      if (effects[i].effect->type().remove()) {
        const std::vector<BonusType> &v = effects[i].effectData.bonusTypes;
      
        for (size_t j = 0; j < v.size(); ++j) {
          const AttribChangeData ad{
            AttribChangeType(effects[i].effect->type().raw(), false),
            v[j].bonus,
            v[j].type,
            effects[i].entity
          };
          attribs->change_attribute(ad);
        }
      }
      
      std::swap(effects[i], effects.back());
      effects.pop_back();
      break;
    }
  }
}

void EffectComponent::addEventEffect(const Type &event, Effect* effect) {
  eventsEffects.push_back(EffectData{event, effect});
}

void EffectComponent::removeEventEffect(const Type &event, Effect* effect) {
  for (size_t i = 0; i < eventsEffects.size(); ++i) {
    if (eventsEffects[i].event == event && eventsEffects[i].effect == effect) {
      std::swap(eventsEffects[i], eventsEffects.back());
      eventsEffects.pop_back();
      break;
    }
  }
}

Effect* EffectComponent::getNextEventEffect(const Type &event) {
  Effect* effect = nullptr;
  while (effect == nullptr) {
    if (counter >= eventsEffects.size()) {
      counter = 0;
      break;
    }
    
    if (eventsEffects[counter].event == event) effect = eventsEffects[counter].effect;
    
    ++counter;
  }
  
  return effect;
}

size_t & EffectComponent::index() {
  return systemIndex;
}

EffectSystem::EffectSystem(const CreateInfo &info) : pool(info.pool) {}
EffectSystem::~EffectSystem() {}

void EffectSystem::update(const uint64_t &time) {
  static const auto func = [&] (const size_t &start, const size_t &count) {
    for (size_t i = start; i < start+count; ++i) {
      components[i]->update(time);
    }
  };
  
  const size_t count = std::ceil(float(components.size()) / float(pool->size()+1));
  size_t start = 0;
  for (uint32_t i = 0; i < pool->size()+1; ++i) {
    const size_t jobCount = std::min(count, components.size()-start);
    if (jobCount == 0) break;

    pool->submitnr(func, start, jobCount);

    start += jobCount;
  }
  
  pool->compute();
  pool->wait();
}

void EffectSystem::addEffectComponent(EffectComponent* comp) {
  comp->index() = components.size();
  components.push_back(comp);
}

void EffectSystem::removeEffectComponent(EffectComponent* comp) {
  components.back()->index() = comp->index();
  std::swap(components[comp->index()], components.back());
  components.pop_back();
}
