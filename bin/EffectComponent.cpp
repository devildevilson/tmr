#include "EffectComponent.h"

enum EffectTypeEnum : uint32_t {
  EFFECT_TYPE_RAW = (1 << 0),
  EFFECT_TYPE_ADD = (1 << 1),
  EFFECT_TYPE_REMOVE = (1 << 2),
  EFFECT_TYPE_PERIODICALY_APPLY = (1 << 3),
  EFFECT_TYPE_COMPUTE_EFFECT = (1 << 4),
  EFFECT_TYPE_CAN_RESIST = (1 << 5),
  EFFECT_TYPE_ONE_TIME_EFFECT = (1 << 6),
  EFFECT_TYPE_RESET_TIMER = (1 << 7),
  EFFECT_TYPE_STACKABLE = (1 << 8),
  EFFECT_TYPE_EASY_STACK = (1 << 9)
};

EffectType::EffectType() : container(0) {}
EffectType::EffectType(const EffectType &type) : container(type.container) {}
EffectType::EffectType(const bool raw, const bool add, const bool remove, const bool periodically_apply, const bool compute_effect, const bool resist, const bool one_time_effect, const bool timer_reset, const bool stackable, const bool easy_stack) : container(0) {
  make(raw, add, remove, periodically_apply, compute_effect, resist, one_time_effect, timer_reset, stackable, easy_stack);
}

void EffectType::make(const bool raw, const bool add, const bool remove, const bool periodically_apply, const bool compute_effect, const bool resist, const bool one_time_effect, const bool timer_reset, const bool stackable, const bool easy_stack) {
  container |= (raw * EFFECT_TYPE_RAW) |
               (add * EFFECT_TYPE_ADD) |
               (remove * EFFECT_TYPE_REMOVE) |
               (periodically_apply * EFFECT_TYPE_PERIODICALY_APPLY) |
               (compute_effect * EFFECT_TYPE_COMPUTE_EFFECT) |
               (resist * EFFECT_TYPE_CAN_RESIST) |
               (one_time_effect * EFFECT_TYPE_ONE_TIME_EFFECT) |
               (timer_reset * EFFECT_TYPE_RESET_TIMER) |
               (stackable * EFFECT_TYPE_STACKABLE) |
               (easy_stack * EFFECT_TYPE_EASY_STACK);
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

bool EffectType::periodically_apply() const {
  return (container & EFFECT_TYPE_PERIODICALY_APPLY) == EFFECT_TYPE_PERIODICALY_APPLY;
}

bool EffectType::compute_effect() const {
  return (container & EFFECT_TYPE_COMPUTE_EFFECT) == EFFECT_TYPE_COMPUTE_EFFECT;
}

bool EffectType::can_be_resisted() const {
  return (container & EFFECT_TYPE_CAN_RESIST) == EFFECT_TYPE_CAN_RESIST;
}

bool EffectType::one_time_effect() const {
  return (container & EFFECT_TYPE_ONE_TIME_EFFECT) == EFFECT_TYPE_ONE_TIME_EFFECT;
}

bool EffectType::timer_reset() const {
  return (container & EFFECT_TYPE_RESET_TIMER) == EFFECT_TYPE_RESET_TIMER;
}

bool EffectType::stackable() const {
  return (container & EFFECT_TYPE_STACKABLE) == EFFECT_TYPE_STACKABLE;
}

bool EffectType::easy_stack() const {
  return (container & EFFECT_TYPE_EASY_STACK) == EFFECT_TYPE_EASY_STACK;
}

BonusTypesContainer::BonusTypesContainer(const size_t &size) : array(size == 0 ? nullptr : new BonusType[size]), size(size) {}

void BonusTypesContainer::clear() {
  delete [] array;
}

ComputedEffectContainer::ComputedEffectContainer(const size_t &size) : time(0), period_time(0), bonusTypes(size) {}

Effect::Effect(const CreateInfo &info) 
 : baseTime(info.baseEffectTime), 
   base_period_time(info.basePeriodTime), 
   typeVar(info.effectType), 
   effectType(info.id), 
   nameVar(info.name), 
   descriptionVar(info.description), 
   bonusTypes(info.types),
   mods(info.mods),
   computefunc(info.compute),
   resistfunc(info.resist) {}

Effect::~Effect() {}

EffectType Effect::type() const {
  return typeVar;
}

const std::vector<BonusType> & Effect::baseValues() const {
  return bonusTypes;
}

const std::vector<Effect::EventModificator> & Effect::modificators() const {
  return mods;
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

void Effect::compute(const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>> &float_attrib, const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>> &int_attrib, ComputedEffectContainer* container) const {
  ASSERT(container->bonusTypes.size == bonusTypes.size());
  ASSERT(typeVar.compute_effect() == bool(computefunc));

  if (typeVar.compute_effect()) computefunc(this, float_attrib, int_attrib, container);
  else memcpy(container->bonusTypes.array, bonusTypes.data(), bonusTypes.size()*sizeof(bonusTypes[0]));
}

void Effect::resist(const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>> &float_attribs, const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>> &int_attribs, ComputedEffectContainer* container) const {
  ASSERT(container->bonusTypes.size == bonusTypes.size());
  ASSERT(typeVar.can_be_resisted() == bool(resistfunc));

  if (typeVar.can_be_resisted()) resistfunc(this, float_attribs, int_attribs, container);
}

//event Effect::call(const Type &type, const EventData &data, yacs::entity* entity) {
//  return eventFunc(type, data, entity);
//}

//CLASS_TYPE_DEFINE_WITH_NAME(EffectComponent, "EffectComponent")

EffectComponent::EffectComponent(const CreateInfo &info) : attribs(info.attribs) {
//  system->addEffectComponent(this);
}

EffectComponent::~EffectComponent() {
//  system->removeEffectComponent(this);
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
        const AttribChanging type = effects[i].effect->type().raw() ? ATTRIB_BONUS_TYPE_RAW_REMOVE : ATTRIB_BONUS_TYPE_FINAL_REMOVE;
        addAttribChanges(type, effects[i].effectData, effects[i].effect, effects[i].entity);
      }

      effects[i].effectData.bonusTypes.clear();
      for (const auto & mod : effects[i].effect->modificators()) {
        removeEventEffect(mod.event, mod.effect);
      }
      std::swap(effects[i], effects.back());
      effects.pop_back();
      --i;
    }
    
    const bool period = (effects[i].currentTime % effects[i].effectData.period_time) <= time;
    //effects[i].currentTime >= effects[i].effectData.period_time
    if (effects[i].effect->type().periodically_apply() && period) {
      const AttribChanging type = effects[i].effect->type().raw() ? ATTRIB_BONUS_TYPE_RAW_ADD : ATTRIB_BONUS_TYPE_FINAL_ADD;
      addAttribChanges(type, effects[i].effectData, effects[i].effect, effects[i].entity);
    }
  }
}

void EffectComponent::addEffect(const ComputedEffectContainer &effectData, const Effect* effect, const EntityAI* entity) {
  if (effect->type().add()) {
    const AttribChanging type = effect->type().raw() ? ATTRIB_BONUS_TYPE_RAW_ADD : ATTRIB_BONUS_TYPE_FINAL_ADD;
    addAttribChanges(type, effectData, effect, entity);
  }

  if (effect->type().one_time_effect()) return;

  effects.push_back(TimeEffect{0, effectData, effect, entity});
  for (const auto & mod : effect->modificators()) {
    eventsEffects.push_back(mod);
  }
}

bool EffectComponent::hasEffect(const Type &effectId) const {
  return findIndex(0, effectId) != SIZE_MAX;
}

bool EffectComponent::resetEffectTimer(const Type &effectId) {
  bool ret = false;
  size_t index = findIndex(0, effectId);
  while (index != SIZE_MAX) {
    effects[index].currentTime = 0;
    ret = true;

    index = findIndex(index+1, effectId);
  }

  return ret;
}

void EffectComponent::removeEffect(const Type &effectId) {
  size_t index = findIndex(0, effectId);
  while (index != SIZE_MAX) {
    if (effects[index].effect->type().remove()) {
      const AttribChanging type = effects[index].effect->type().raw() ? ATTRIB_BONUS_TYPE_RAW_REMOVE : ATTRIB_BONUS_TYPE_FINAL_REMOVE;
      addAttribChanges(type, effects[index].effectData, effects[index].effect, effects[index].entity);
    }

    effects[index].effectData.bonusTypes.clear();
    for (const auto & mod : effects[index].effect->modificators()) {
      removeEventEffect(mod.event, mod.effect);
    }
    std::swap(effects[index], effects.back());
    effects.pop_back();

    index = findIndex(index, effectId);
  }
}

void EffectComponent::addEventEffect(const Type &event, const Effect* effect) {
  eventsEffects.push_back(Effect::EventModificator{event, effect});
}

void EffectComponent::removeEventEffect(const Type &event, const Effect* effect) {
  for (size_t i = 0; i < eventsEffects.size(); ++i) {
    if (eventsEffects[i].event == event && eventsEffects[i].effect == effect) {
      std::swap(eventsEffects[i], eventsEffects.back());
      eventsEffects.pop_back();
      break;
    }
  }
}

const Effect* EffectComponent::getNextEventEffect(const Type &event, size_t &counter) const {
  const Effect* effect = nullptr;
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

size_t EffectComponent::findIndex(const size_t &start, const Type &effectId) const {
  for (size_t i = start; i < effects.size(); ++i) {
    if (effects[i].effect->id() == effectId) return i;
  }

  return SIZE_MAX;
}

void EffectComponent::addAttribChanges(const AttribChanging &type, const ComputedEffectContainer &effectData, const Effect* effect, const EntityAI* entity) {
  for (size_t j = 0; j < effectData.bonusTypes.size; ++j) {
    const AttribChangeData ad{
//            AttribChangeType(effects[i].effect->type().raw(), false),
      type,
      effectData.bonusTypes.array[j].bonus,
      effectData.bonusTypes.array[j].type,
      effect,
      entity
    };
    attribs->change_attribute(ad);
  }
}

EffectSystem::EffectSystem(const CreateInfo &info) : pool(info.pool) {}
EffectSystem::~EffectSystem() {}

void EffectSystem::update(const size_t &time) {
  static const auto func = [&] (const size_t &start, const size_t &count, const size_t &time) {
    for (size_t i = start; i < start+count; ++i) {
//      components[i]->update(time);
      auto handle = Global::world()->get_component<EffectComponent>(i);
      handle->update(time);
    }
  };

  const size_t &componentsCount = Global::world()->count_components<EffectComponent>();
  const size_t count = std::ceil(float(componentsCount) / float(pool->size()+1));
  size_t start = 0;
  for (uint32_t i = 0; i < pool->size()+1; ++i) {
    const size_t jobCount = std::min(count, componentsCount-start);
    if (jobCount == 0) break;

    pool->submitnr(func, start, jobCount, time);

    start += jobCount;
  }
  
  pool->compute();
  pool->wait();
}

const Effect* EffectSystem::get(const Type &type) const {
  auto itr = effects.find(type);
  if (itr == effects.end()) return nullptr;

  return itr->second;
}

const Effect* EffectSystem::create(const Effect::CreateInfo &info) {
  auto itr = effects.find(info.id);
  if (itr != effects.end()) throw std::runtime_error("Effect with type " + info.id.name() + " is already created");

  Effect* effect = effectsPool.newElement(info);
  effects[info.id] = effect;
  return effect;
}

void EffectSystem::destroy(const Type &type) {
  auto itr = effects.find(type);
  if (itr == effects.end()) return;

  effectsPool.deleteElement(itr->second);
  effects.erase(itr);
}