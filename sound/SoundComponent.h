#ifndef SOUND_COMPONENT_H
#define SOUND_COMPONENT_H

#include "Type.h"
#include "ResourceID.h"

#define QUEUE_SOUND_DATA_COUNT 3

struct QueueSoundData;
class EventComponent;
class TransformComponent;
class PhysicsComponent;

// это звуки только от монстров? возможно
// тогда relative можно убрать

class SoundComponent {
public:
  struct CreateInfo {
    TransformComponent* transform;
    PhysicsComponent* physics;
//    EventComponent* events;
  };
  SoundComponent(const CreateInfo &info);
  ~SoundComponent();
  
  void update(const size_t &time);

  struct PlayInfo {
    const SoundData* sound;

    bool looping;
    bool memorised;
    bool relative;
    bool needVelocity;

    float scalar;

    float maxDist;

    float pitch; // по идее лучше генерировать случайным образом для почти каждого звука

    float rolloff; // полезная вещь, но нужно много проверять как именно работает
    float refDist; // полезная вещь, но нужно много проверять как именно работает
  };
  void play(const PlayInfo &info);
  void cancel();
private:
  struct SoundData {
    QueueSoundData* sound;
    bool needVelocity;
    float scalar;
  };
  
  TransformComponent* transform;
  PhysicsComponent* physics;

  SoundData queuedSounds[QUEUE_SOUND_DATA_COUNT]; // этого то поди несколько будет
  
  void updateSoundPosition(QueueSoundData* q, const float &scalar, const bool needVelocity, const bool relative);
};

#endif
