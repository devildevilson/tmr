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
      false,
      0.0f
      //{0.0f, 0.0f, 0.0f, 0.0f}
    };
  }
}

SoundComponent::~SoundComponent() {}

void SoundComponent::update(const size_t &time) {
  (void)time;

  for (uint32_t i = 0; i < QUEUE_SOUND_DATA_COUNT; ++i) {
    if (queuedSounds[i].sound == nullptr) continue;

    const float pos = queuedSounds[i].sound->playingPosition();
    //const size_t timePos = pos * queuedSounds[i].time;

    if (pos >= 1.0f) {
      Global::sound()->unqueueSound(queuedSounds[i].sound);
      queuedSounds[i].sound = nullptr;
//       queuedSounds[i].time = SIZE_MAX;
      continue;
    }

    updateSoundPosition(queuedSounds[i].sound, queuedSounds[i].scalar, queuedSounds[i].needVelocity, false);
  }
}

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

  QueueSoundData* q = Global::get<SoundSystem>()->queueSound(info.sound);

  if (info.memorised) {
    queuedSounds[index].sound = q;
    //queuedSounds[index].time = info.time;
    //memcpy(queuedSounds[index].relativePos, info.relativePos, sizeof(float)*4);
    queuedSounds[index].needVelocity = info.needVelocity;
    queuedSounds[index].scalar = info.scalar;
    
  }

  q->type = QueueSoundType(info.relative, true, true, info.looping);

  q->maxDist = info.maxDist;
  q->pitch = info.pitch;
  q->rolloff = info.rolloff;
  q->refDist = info.refDist;
  
  updateSoundPosition(q, info.scalar, info.needVelocity, info.relative);
}

void SoundComponent::cancel() {

}

void SoundComponent::updateSoundPosition(QueueSoundData* q, const float &scalar, const bool needVelocity, const bool relative) {
  if (transform != nullptr) {
    const auto pos = float(!relative)*transform->pos() + transform->rot()*scalar;
    const auto rot = transform->rot();
    pos.storeu(q->pos);
    rot.storeu(q->dir);
  }
  
  if (physics != nullptr && needVelocity) {
    physics->getVelocity().storeu(q->vel);
  }
}
