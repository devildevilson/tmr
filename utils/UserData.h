#ifndef USER_DATA_H
#define USER_DATA_H

namespace yacs {
  class entity;
}

class DecalContainerComponent;
class GraphicComponent;
class EntityAI;
class vertex_t;
class EventComponent;

struct PhysUserData {
  yacs::entity* ent;
  DecalContainerComponent* decalContainer;
  GraphicComponent* graphicComponent;
  EntityAI* aiComponent;
  vertex_t* vertex;
  EventComponent* events;
};

#endif
