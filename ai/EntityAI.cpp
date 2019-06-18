#include "EntityAI.h"

// #include "EventComponent.h"
#include "Globals.h"
#include "Physics.h"
#include "UserData.h"

enum StateFlagsEnum {
  STATE_BLOCKING = (1<<0),
  STATE_BLOCKING_MOVEMENT = (1<<1),
  STATE_HAS_PATH = (1<<2),
  STATE_ON_GROUND = (1<<3)
};

StateFlagsContainer::StateFlagsContainer() : container(0) {}
  
bool StateFlagsContainer::isBlocking() const {
  return (container & STATE_BLOCKING) == STATE_BLOCKING;
}

bool StateFlagsContainer::isBlockingMovement() const {
  return (container & STATE_BLOCKING_MOVEMENT) == STATE_BLOCKING_MOVEMENT;
}

bool StateFlagsContainer::hasPath() const {
  return (container & STATE_HAS_PATH) == STATE_HAS_PATH;
}

bool StateFlagsContainer::isOnGround() const {
  return (container & STATE_ON_GROUND) == STATE_ON_GROUND;
}

void StateFlagsContainer::setBlocking(const bool value) {
  container = value ? container | STATE_BLOCKING : container & ~(STATE_BLOCKING);
}

void StateFlagsContainer::setBlockingMovement(const bool value) {
  container = value ? container | STATE_BLOCKING_MOVEMENT : container & ~(STATE_BLOCKING_MOVEMENT);
}

void StateFlagsContainer::setPathExisting(const bool value) {
  container = value ? container | STATE_HAS_PATH : container & ~(STATE_HAS_PATH);
}

void StateFlagsContainer::setOnGround(const bool value) {
  container = value ? container | STATE_ON_GROUND : container & ~(STATE_ON_GROUND);
}

EntityAI::EntityAI() : pos{0.0f, 0.0f, 0.0f, 0.0f}, dir{0.0f, 0.0f, 0.0f, 0.0f}, vel{0.0f, 0.0f, 0.0f, 0.0f}, objectIndex(UINT32_MAX), aiData({0.0f, nullptr, nullptr, nullptr, nullptr}), collidingIndex(0), localEvents(nullptr) {}
EntityAI::~EntityAI() {}

event EntityAI::pushEvent(const Type &event, void* data) {
  // надо запомнить что поиск пути и следование пути это разные вещи
  // + добавляется следование за целью и движение в пользовательскую сторону
  // как сделать ховл? вообще я так полагаю в думе из-за того что не были
  // сделаны стороны у этой анимации поэтому выглядело будто он все время смотрит на игрока
  // думаю что это меньшая проблема
  // состояние? с этим гораздо сложнее стало
  // но мне нужно как то при атаке не идти никуда
  // я так полагаю мы должны понять блокирующая ли у нас анимация (причем наверное только анимация)
  // хотя наверное даже не так
  // блокирующиее ли у нас что-то?
  // то есть не в анимации флажок специальный ставить, а в стейт контроллере
  // там мы просто выставляем флажок блокировки (если нужно) когда начинаем какое-нибудь действие
  // а когда заканчиваем действие то выставляем обратно
  // например атака или скилл, мы попадаем в апдейт компонента где принимаем решение атаковать или нет
  // если атака, то выставляем в состоянии блокировку движения, например
  // затем в следующем кадре пытаемся двигаться, чекаем флажок состояния, отправляем фейл если не можем
  
  lastEvent = event;
  
  const EventData ed{
    &aiData,
    data
  };
  
  return localEvents->fireEvent(event, ed);
}

void EntityAI::setTarget(const EntityAI* target) {
  aiData.targetPtr = target;
}

void EntityAI::addTag(const Type &type) {
  tags.insert(type);
}

void EntityAI::removeTag(const Type &type) {
  tags.erase(type);
}

size_t & EntityAI::getBlackboardValue(const Type &type) {
  auto itr = localBlackboard.find(type);
  if (itr == localBlackboard.end()) {
    itr = localBlackboard.insert(std::make_pair(type, SIZE_MAX)).first;
  }
  
  return itr->second;
}

const EntityAI* EntityAI::target() const {
  return aiData.targetPtr;
}

bool EntityAI::hasTarget() const {
  return aiData.targetPtr != nullptr;
}

simd::vec4 EntityAI::position() const {
  return simd::vec4(pos);
}

simd::vec4 EntityAI::direction() const {
  return simd::vec4(dir);
}

simd::vec4 EntityAI::velocity() const {
  return simd::vec4(vel);
}

float EntityAI::radius() const {
  return aiData.r;
}

const vertex_t* EntityAI::vertex() const {
  const vertex_t* vert = aiData.vertex == nullptr ? aiData.lastVertex : aiData.vertex;
  return vert;
}

bool EntityAI::hasGroup() const {
  return aiData.groupPtr != nullptr;
}

// bool EntityAI::isGroupAI() const {
//   return hasGroup() && aiData.groupPtr->ai() == this;
// }

bool EntityAI::isGroupChief() const {
  return hasGroup() && aiData.groupPtr->chief() == this;
}

const AIGroup* EntityAI::group() const {
  return aiData.groupPtr;
}

const EntityAI* EntityAI::getNextCollider() const {
  const EntityAI* nextEntity = nullptr;
  while (nextEntity == nullptr && collidingIndex < Global::physics()->getOverlappingDataSize()) {
    auto arr = Global::physics()->getOverlappingPairsData();
    
    if (arr->at(collidingIndex).firstIndex == objectIndex) {
      auto cont = Global::physics()->getIndexContainer(arr->at(collidingIndex).secondIndex);
      PhysUserData* data = reinterpret_cast<PhysUserData*>(cont->userData);
      nextEntity = data->aiComponent;
    }
    
    if (arr->at(collidingIndex).secondIndex == objectIndex) {
      auto cont = Global::physics()->getIndexContainer(arr->at(collidingIndex).firstIndex);
      PhysUserData* data = reinterpret_cast<PhysUserData*>(cont->userData);
      nextEntity = data->aiComponent;
    }
    
    ++collidingIndex;
  }
  
  if (nextEntity == nullptr) collidingIndex = 0;
  
  return nextEntity;
}

bool EntityAI::hasTag(const Type &tag) const {
  auto itr = tags.find(tag);
  return itr != tags.end();
}

Type EntityAI::getLastEventType() const {
  return lastEvent;
}

bool EntityAI::blocked() const {
  return states.isBlocking();
}

bool EntityAI::movementBlocked() const {
  return states.isBlockingMovement();
}

bool EntityAI::hasPath() const {
  return states.hasPath();
}

bool EntityAI::grounded() const {
  return states.isOnGround();
}

// event AIGroup::pushEvent(const EntityAI* member, const Type &event, void* data) {
//   if (member == nullptr) {
//     enum event ev = success;
//     
//     for (size_t i = 0; i < members.size(); ++i) {
//       const enum event ret = members[i]->pushEvent(event, data);
//       
//       const uint32_t mask = ~static_cast<uint32_t>(can_be_deleted);
//       const enum event tmp = static_cast<enum event>(mask & ret);
//       ev = std::max(ev, tmp);
//     }
//     
//     return ev;
//   }
//   
//   EntityAI* realMember = nullptr;
//   
//   for (size_t i = 0; i < members.size(); ++i) {
//     if (member == members[i]) {
//       realMember = members[i];
//       break;
//     }
//   }
//   
//   if (realMember == nullptr) return failure;
//   
//   return realMember->pushEvent(event, data);
// }
// 
// event AIGroup::pushEvent(const Type &event, void* data) {
//   return groupAI->pushEvent(event, data);
// }
// 
// void AIGroup::setTarget(const EntityAI* target) {
//   groupAI->setTarget(target);
// }
//   
// const EntityAI* AIGroup::ai() const {
//   return groupAI;
// }

const EntityAI* AIGroup::member(const size_t &index) const {
  return members[index];
}

const EntityAI* AIGroup::chief() const {
  return groupChief;
}

size_t AIGroup::size() const {
  return members.size();
}

// simd::vec4 AIGroup::position() const {
//   return groupAI->position();
// }
// 
// simd::vec4 AIGroup::direction() const {
//   return groupAI->direction();
// }

// пока непонятно что именно мне нужно
simd::vec4 AIGroup::extents() const {
  
}

// с этим тоже пока что ничего не понятно
// bool AIGroup::isGroupInBounds() const;
