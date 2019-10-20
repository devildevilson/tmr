#include "SoundComponent.h"

#include "SoundSystem.h"
#include "EventComponent.h"
//#include "Components.h"
#include "Globals.h"
#include "TransformComponent.h"
#include "PhysicsComponent.h"

SoundComponent::SoundComponent(const CreateInfo &info) : transform(info.transform), physics(info.physics) {
  for (uint32_t i = 0; i < QUEUE_SOUND_DATA_COUNT; ++i) {
    queuedSounds[i] = {
      nullptr,
      SIZE_MAX,
      {0.0f, 0.0f, 0.0f, 0.0f},
      false
    };
  }
}

SoundComponent::~SoundComponent() {}

void SoundComponent::update(const size_t &time) {
  (void)time;

  for (uint32_t i = 0; i < QUEUE_SOUND_DATA_COUNT; ++i) {
    if (queuedSounds[i].sound == nullptr) continue;

    const float pos = queuedSounds[i].sound->playingPosition();
    const size_t timePos = pos * queuedSounds[i].time;

    if (timePos >= queuedSounds[i].time) {
      Global::sound()->unqueueSound(queuedSounds[i].sound);
      queuedSounds[i].sound = nullptr;
      queuedSounds[i].time = SIZE_MAX;
      continue;
    }

    if (transform != nullptr) {
      (transform->pos() + simd::vec4(queuedSounds[i].relativePos)).storeu(queuedSounds[i].sound->pos);
      transform->rot().storeu(queuedSounds[i].sound->dir);
    }

    if (physics != nullptr && queuedSounds[i].needVelocity) {
      physics->getVelocity().storeu(queuedSounds[i].sound->vel);
    }
  }
}

//void SoundComponent::init(void* userData) {
//  (void)userData;
//
//  transform = getEntity()->get<TransformComponent>().get();
//  physics = getEntity()->get<PhysicsComponent2>().get();
//
//  events = getEntity()->get<EventComponent>().get();
//  if (events == nullptr) {
//    Global::console()->printE("Trying to create SoundComponent without events");
//    throw std::runtime_error("Trying to create SoundComponent without events");
//  }
//
//  Global::sound()->addComponent(this);
//}

//void SoundComponent::setSound(const Type &type, const ResourceID &soundId, const float &maxDist) {
//  static const auto eventFunc = [&] (const Type type, const EventData &data, const ResourceID &soundId, const float &maxDist) {
//    (void)type;
//    (void)data;
//    // тут нам нужно запихать данные для воспроизведения
//    // большинство звуков которые мы отсюда стартуем, будут проигрываться лишь единожды
//    // следовательно нет необходимости держать указатель на это дело
//    // в каких случаях нам нужен указатель?
//
//    QueueSoundData* q = Global::sound()->queueSound(soundId);
//    q->maxDist = maxDist;
//
//    if (transform != nullptr) {
//      transform->pos().storeu(q->pos);
//      transform->rot().storeu(q->dir);
//    }
//
//    if (physics != nullptr) {
//      physics->getVelocity().storeu(q->vel);
//    }
//
//    // pitch? на форумах говорят что нужно брать какое то случайное число
//
//    q->updateSource();
//    return success;
//  };
//
//  events->registerEvent(type, std::bind(eventFunc, std::placeholders::_1, std::placeholders::_2, soundId, maxDist));
//}

void SoundComponent::play(const PlayInfo &info) {
  uint32_t index = 0;
  for (; index < QUEUE_SOUND_DATA_COUNT; ++index) {
    if (queuedSounds[index].sound == nullptr) break;
  }

  // если все указатели сейчас заняты, такое не должно произойти при нормальных обстоятельствах
  if (index >= QUEUE_SOUND_DATA_COUNT && info.memorised) {
    // пока что будем вылетать
    throw std::runtime_error("Every sounds is busy now");
  }

  QueueSoundData* q = Global::sound()->queueSound(info.soundId);

  if (info.memorised) {
    queuedSounds[index].sound = q;
    queuedSounds[index].time = info.time;
    memcpy(queuedSounds[index].relativePos, info.relativePos, sizeof(float)*4);
    queuedSounds[index].needVelocity = info.needVelocity;
  }

  q->type = QueueSoundType(info.relative, true, true, info.looping);

  q->maxDist = info.maxDist;
  q->pitch = info.pitch;
  q->rolloff = info.rolloff;
  q->refDist = info.refDist;

  if (transform != nullptr) {
    (transform->pos() + simd::vec4(info.relativePos)).storeu(q->pos);
    transform->rot().storeu(q->dir);
  } else {
    memcpy(q->pos, info.relativePos, sizeof(float)*4);
  }

  if (physics != nullptr && info.needVelocity) {
    physics->getVelocity().storeu(q->vel);
  } else {
    memset(q->vel, 0, sizeof(float)*4);
  }
}

void SoundComponent::cancel() {

}