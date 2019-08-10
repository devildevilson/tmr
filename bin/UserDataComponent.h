#ifndef USER_DATA_COMPONENT_H
#define USER_DATA_COMPONENT_H

class TransformComponent;
class GraphicComponent;
class PhysicsComponent;
class AnimationComponent;
class DecalContainerComponent;
class EntityAI;
class vertex_t;
class EventComponent;

namespace yacs {
  class entity;
}

struct UserDataComponent {
  yacs::entity* entity;
  TransformComponent* trans;
  GraphicComponent* graphic;
  PhysicsComponent* phys;
  AnimationComponent* anim;
  DecalContainerComponent* decalContainer;
  EntityAI* aiComponent;
  vertex_t* vertex;
  EventComponent* events;
};

#endif //USER_DATA_COMPONENT_H
