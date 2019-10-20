#ifndef USER_DATA_COMPONENT_H
#define USER_DATA_COMPONENT_H

// нужно сделать несколько структур с несколькими указателями
// для того чтобы сократить количество поисков внутри энтити
// например: данные для физики (графика + контейнер декалей + ???), данные для взаимодействий (атрибуты, эфекты, и тд)
// для некоторых компонентов и так и сяк придется хранить указатели

#include <mutex>

class TransformComponent;
class GraphicUpdater;
class PhysicsComponent;
class AnimationComponent;
class DecalContainerComponent;
class EntityAI;
class vertex_t;
class EventComponent;

class AttributesComponent;
class EffectComponent;
class InventoryComponent; // ???
class UsableComponent;

namespace yacs {
  class entity;
}

// сократить
struct UserDataComponent {
  yacs::entity* entity;
  TransformComponent* trans;
  GraphicUpdater* graphic;
  PhysicsComponent* phys;
  AnimationComponent* anim;
  DecalContainerComponent* decalContainer;
  EntityAI* aiComponent;
  vertex_t* vertex;
  EventComponent* events;
};

// будем брать эту структуру вместо поиска всех означенных
struct InteractionData {
  yacs::entity* entity;
  AttributesComponent* attribs;
  EffectComponent* effects;
  UsableComponent* usable; // для того чтобы обрабатывать эффект использовать
};

// пригодится если я планирую сделать мультипоток
struct MutexContainer {
  std::mutex mutex;
};

#endif //USER_DATA_COMPONENT_H
