#ifndef EFFECT_COMPONENT_H
#define EFFECT_COMPONENT_H

#include "EntityComponentSystem.h"
#include "AttributesComponent.h"
#include "MemoryPool.h"
#include "EventFunctor.h"
#include "Effect.h"

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
    AttributeComponent* attribs;
  };
  EffectComponent(const CreateInfo &info);
  ~EffectComponent();
  
  void update(const size_t &time = 0);

  // стаки?
  void addEffect(const ComputedEffectContainer &effectData, const Effect* effect, const EntityAI* entity);
  bool hasEffect(const Type &effectId) const;
  bool resetEffectTimer(const Type &effectId);
  void removeEffect(const Type &effectId);

  void addEventEffect(const Type &event, const Effect* effect);
  void removeEventEffect(const Type &event, const Effect* effect);

  const Effect* getNextEventEffect(const Type &event, size_t &counter) const;
private:
  // тут еще добавятся стаки, по сути это просто еще одно добавление вычисленных вещей в аттрибуты
  // а потом удаление из аттрибутов несколько раз
  struct TimeEffect {
    size_t currentTime;
    ComputedEffectContainer effectData;
    const Effect* effect;
    // указатель на EntityAI? наверное наиболее адекватный выбор
    const EntityAI* entity;
  };
  
//  struct EffectData {
//    Type event;
//    const Effect* effect;
//  };
  
//  size_t systemIndex;
//  EffectSystem* system;
  AttributeComponent* attribs;
  std::vector<TimeEffect> effects;

  std::vector<Effect::EventModificator> eventsEffects;

  size_t findIndex(const size_t &start, const Type &effectId) const;
  void addAttribChanges(const AttribChanging &type, const ComputedEffectContainer &effectData, const Effect* effect, const EntityAI* entity);
};

class EffectSystem : public Engine, public yacs::system {
public:
  struct CreateInfo {
    dt::thread_pool* pool;
  };
  EffectSystem(const CreateInfo &info);
  ~EffectSystem();
  
  void update(const size_t &time) override;

  // создание удаление эффектов будет здесь наверное
  const Effect* get(const Type &type) const;
  const Effect* create(const Effect::CreateInfo &info);
  void destroy(const Type &type);
private:
  dt::thread_pool* pool;

  std::unordered_map<Type, Effect*> effects;
  MemoryPool<Effect, sizeof(Effect)*20> effectsPool;
//  std::vector<EffectComponent*> components;
};

#endif
