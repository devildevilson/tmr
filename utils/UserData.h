#ifndef USER_DATA_H
#define USER_DATA_H

namespace yacs {
  class Entity;
}

class DecalContainerComponent;
class GraphicComponent;

struct PhysUserData {
  yacs::Entity* ent;
  DecalContainerComponent* decalContainer;
  GraphicComponent* graphicComponent;
};

#endif
