#ifndef MATERIAL_COMPONENT_H
#define MATERIAL_COMPONENT_H

#include "EntityComponentSystem.h"
#include "Type.h"

class EventComponent;

// мы должны обращаться к этому компоненту, когда нам нужно 
// передать материалу взаимодействие с поверхностью
// с другой стороны, зачем нам вообще нужен этот компонент?
// мы можем добавить в эвенты взаимодействие с материалом
// наверное так и нужно сделать
class MaterialComponent : public yacs::Component {
public:
  MaterialComponent();
  ~MaterialComponent();
  
  void update(const size_t &time) override;
  void init(void* userData) override;
  
  void registerMaterialEvent(const Type &type);
  
private:
  EventComponent* events;
  
  yacs::Entity* materialEntity;
  EventComponent* materialEvents;
};

#endif
