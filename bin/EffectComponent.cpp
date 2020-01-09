#include "EffectComponent.h"

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
  // нужен мьютекс
  
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
//   static const auto func = [&] (const size_t &start, const size_t &count, const size_t &time) {
//     for (size_t i = start; i < start+count; ++i) {
// //      components[i]->update(time);
//       auto handle = Global::world()->get_component<EffectComponent>(i);
//       handle->update(time);
//     }
//   };

  const size_t componentsCount = Global::world()->count_components<EffectComponent>();
  const size_t count = std::ceil(float(componentsCount) / float(pool->size()+1));
  size_t start = 0;
  for (uint32_t i = 0; i < pool->size()+1; ++i) {
    const size_t jobCount = std::min(count, componentsCount-start);
    if (jobCount == 0) break;

    pool->submitbase([start, count, time] () {
      for (size_t i = start; i < start+count; ++i) {
        auto handle = Global::world()->get_component<EffectComponent>(i);
        handle->update(time);
      }
    });
    
//     pool->submitnr(func, start, jobCount, time);

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
