#ifndef USER_DATA_H
#define USER_DATA_H

namespace yacs {
  class Entity;
}

class DecalContainerComponent;
class GraphicComponent;
class EntityAI;
class vertex_t;

struct PhysUserData {
  yacs::Entity* ent;
  DecalContainerComponent* decalContainer;
  GraphicComponent* graphicComponent;
  EntityAI* aiComponent;
  vertex_t* vertex;
};

#endif
