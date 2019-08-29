#ifndef SOUND_COMPONENT_H
#define SOUND_COMPONENT_H

#include "Type.h"
#include "ResourceID.h"

#define QUEUE_SOUND_DATA_COUNT 3

struct QueueSoundData;
class EventComponent;
class TransformComponent;
class PhysicsComponent;

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
  
//  void setSound(const Type &type, const ResourceID &soundId, const float &maxDist);

  struct PlayInfo {
    ResourceID soundId;
    size_t time;

    // делэй нужно еще сделать

    bool looping;
    bool memorised;
    bool relative;
    bool needVelocity;

    float relativePos[4];

    float maxDist;

    float pitch; // по идее лучше генерировать случайным образом для почти каждого звука

    float rolloff; // полезная вещь, но нужно много проверять как именно работает
    float refDist; // полезная вещь, но нужно много проверять как именно работает

    // должен быть способ создать звук за которым не нужно следить
    // то есть у этого звука: константное положение, скорость, направление
  };
  void play(const PlayInfo &info);
  void cancel();
private:
  struct SoundData {
    QueueSoundData* sound;
    size_t time;
    float relativePos[4];
    bool needVelocity;
  };

  TransformComponent* transform;
  PhysicsComponent* physics;
//  EventComponent* events;

  SoundData queuedSounds[QUEUE_SOUND_DATA_COUNT]; // этого то поди несколько будет
};

#endif
