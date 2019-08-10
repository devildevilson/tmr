#ifndef SOUND_COMPONENT_H
#define SOUND_COMPONENT_H

#include "Type.h"
#include "ResourceID.h"

struct QueueSoundData;
class EventComponent;
class TransformComponent;
class PhysicsComponent;

class SoundComponent {
public:
  struct CreateInfo {
    TransformComponent* transform;
    PhysicsComponent* physics;
    EventComponent* events;
  };
  SoundComponent(const CreateInfo &info);
  ~SoundComponent();
  
  void update(const size_t &time);
//  void init(void* userData) override;
  
  void setSound(const Type &type, const ResourceID &soundId, const float &maxDist);
private:
  TransformComponent* transform;
  PhysicsComponent* physics;
  EventComponent* events;
  
  QueueSoundData* queuedSound; // этого то поди несколько будет
  
  // макс дист? наверное в сет саунд уйдет
};

#endif
