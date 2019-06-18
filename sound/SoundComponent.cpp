#include "SoundComponent.h"

#include "SoundSystem.h"
#include "EventComponent.h"
#include "Components.h"
#include "Globals.h"

CLASS_TYPE_DEFINE_WITH_NAME(SoundComponent, "SoundComponent")

SoundComponent::SoundComponent() {}
SoundComponent::~SoundComponent() {
  Global::sound()->removeComponent(this);
}

void SoundComponent::update(const size_t &time) {
  // тут мы по идее будем в будущем обновлять звук
  // позицию и прочее
  // каким объектам это требуется?
  // думаю что огненный шар например может вполне издавать звук пока летит
  // скорее всего я пойму больше только к тому моменту как займусь скилами и прочим
//   if (queuedSound != nullptr) {
//     
//   }
}

void SoundComponent::init(void* userData) {
  transform = getEntity()->get<TransformComponent>().get();
  physics = getEntity()->get<PhysicsComponent2>().get();
  
  events = getEntity()->get<EventComponent>().get();
  if (events == nullptr) {
    Global::console()->printE("Trying to create SoundComponent without events");
    throw std::runtime_error("Trying to create SoundComponent without events");
  }
  
  Global::sound()->addComponent(this);
}

void SoundComponent::setSound(const Type &type, const ResourceID &soundId, const float &maxDist) {
  static const auto eventFunc = [&] (const Type type, const EventData &data, const ResourceID &soundId, const float &maxDist) {
    // тут нам нужно запихать данные для воспроизведения
    // большинство звуков которые мы отсюда стартуем, будут проигрываться лишь единожды
    // следовательно нет необходимости держать указатель на это дело
    // в каких случаях нам нужен указатель?
    
    QueueSoundData* q = Global::sound()->queueSound(soundId);
    q->maxDist = maxDist;
    
    if (transform != nullptr) {
      transform->pos().storeu(q->pos);
      transform->rot().storeu(q->dir);
    }
    
    if (physics != nullptr) {
      physics->getVelocity().storeu(q->vel);
    }
    
    // pitch? на форумах говорят что нужно брать какое то случайное число
    
    q->updateSource();
    return success;
  };
  
  events->registerEvent(type, std::bind(eventFunc, std::placeholders::_1, std::placeholders::_2, soundId, maxDist));
}
