#ifndef SOUND_COMPONENT_H
#define SOUND_COMPONENT_H

#include "EntityComponentSystem.h"

#include "Type.h"
#include "ResourceID.h"

struct QueueSoundData;
class EventComponent;
class TransformComponent;
class PhysicsComponent2;

class SoundComponent : public yacs::Component {
public:
  CLASS_TYPE_DECLARE
  
  SoundComponent();
  ~SoundComponent();
  
  void update(const size_t &time) override;
  void init(void* userData) override;
  
  void setSound(const Type &type, const ResourceID &soundId, const float &maxDist);
private:
  TransformComponent* transform;
  PhysicsComponent2* physics;
  EventComponent* events;
  
  QueueSoundData* queuedSound; // этого то поди несколько будет
  
  // макс дист? наверное в сет саунд уйдет
};

#endif
