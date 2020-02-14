#include "EntityAI.h"

// #include "EventComponent.h"
#include "Globals.h"
#include "Physics.h"
#include "UserData.h"
#include "global_components_indicies.h"
#include "EntityComponentSystem.h"
#include "StateController.h"
#include "MovementComponent.h"
#include "InventoryComponent.h"
#include "AttributesComponent.h"
#include "EffectComponent.h"
#include "AbilityComponent.h"
#include "global_components_indicies.h"

// enum StateFlagsEnum {
//   STATE_BLOCKING = (1<<0),
//   STATE_BLOCKING_MOVEMENT = (1<<1),
//   STATE_HAS_PATH = (1<<2),
//   STATE_ON_GROUND = (1<<3)
// };

// StateFlagsContainer::StateFlagsContainer() : container(0) {}
//   
// bool StateFlagsContainer::isBlocking() const {
//   return (container & STATE_BLOCKING) == STATE_BLOCKING;
// }
// 
// bool StateFlagsContainer::isBlockingMovement() const {
//   return (container & STATE_BLOCKING_MOVEMENT) == STATE_BLOCKING_MOVEMENT;
// }
// 
// bool StateFlagsContainer::hasPath() const {
//   return (container & STATE_HAS_PATH) == STATE_HAS_PATH;
// }
// 
// bool StateFlagsContainer::isOnGround() const {
//   return (container & STATE_ON_GROUND) == STATE_ON_GROUND;
// }
// 
// void StateFlagsContainer::setBlocking(const bool value) {
//   container = value ? container | STATE_BLOCKING : container & ~(STATE_BLOCKING);
// }
// 
// void StateFlagsContainer::setBlockingMovement(const bool value) {
//   container = value ? container | STATE_BLOCKING_MOVEMENT : container & ~(STATE_BLOCKING_MOVEMENT);
// }
// 
// void StateFlagsContainer::setPathExisting(const bool value) {
//   container = value ? container | STATE_HAS_PATH : container & ~(STATE_HAS_PATH);
// }
// 
// void StateFlagsContainer::setOnGround(const bool value) {
//   container = value ? container | STATE_ON_GROUND : container & ~(STATE_ON_GROUND);
// }

void Blackboard::setPtr(const Type &id, const void* ptr) {
  Data d;
  d.ptr = ptr;
  map[id] = d;
}

void Blackboard::setValueF(const Type &id, const double &value) {
  Data d;
  d.valueF = value;
  map[id] = d;
}

void Blackboard::setValue(const Type &id, const size_t &value) {
  Data d;
  d.value = value;
  map[id] = d;
}

const void* Blackboard::getPtr(const Type &id) const {
  auto itr = map.find(id);
  if (itr != map.end()) return itr->second.ptr;
  
  return nullptr;
}

double Blackboard::getValueF(const Type &id) const {
  auto itr = map.find(id);
  if (itr != map.end()) return itr->second.valueF;
  
  return 0.0;
}

size_t Blackboard::getValue(const Type &id) const {
  auto itr = map.find(id);
  if (itr != map.end()) return itr->second.value;
  
  return 0;
}

EntityAI::EntityAI(const CreateInfo &info)
 : pos{0.0f, 0.0f, 0.0f, 0.0f}, 
   dir{0.0f, 0.0f, 0.0f, 0.0f}, 
   vel{0.0f, 0.0f, 0.0f, 0.0f}, 
   objectIndex(UINT32_MAX), 
   entType(info.entType), 
   r(info.r), 
   m_vertex(info.m_vertex), 
   lastVertex(info.m_vertex), 
   targetPtr(nullptr), 
   groupPtr(nullptr), 
   ent(info.ent) {}
//    usrData(info.usrData), 
//    mv(info.mv), 
//    abilityComponent(info.abilityComponent), 
//    inventoryComponent(info.inventoryComponent), 
//    weaponsComponent(info.weaponsComponent), 
//    attribs(info.attribs), 
//    effectsComp(info.effectsComp), 
//    states(info.states) {}
EntityAI::~EntityAI() {}

void EntityAI::target(const EntityAI* target) {
  targetPtr = target;
}

void EntityAI::removeSelf(const bool fullRemove) {
  // мы должны передать энтити на удаление
  // нужно ли как то помечать энтити которое готовится к удалению?
  // вообще нужно какой то фидбек все же получать при создании/удалении
  // как сделать фидбек? нужен он для чего? чтобы отменить создание например
  // на самом деле тут проблема кроется немного в другом
  // мне нужно было определить состояние и список действий которые в это состояние происходят
  // в том числе запуск абилки, а абилка должна просто описывать как создать энтити
  // в таком виде достаточно отменить состояние
  
  // теперь есть указатель на энтити, передадим его на удаление
}

void EntityAI::sleep(const size_t &time) {
  // что здесь? нам возможно потребуется некое время сна 
  // если время сна задано, то ничего не делаем все это время
  // как пробуждать? устанавливать время сна в 0
}

void EntityAI::addTag(const Type &type) {
  tags.insert(type);
}

void EntityAI::removeTag(const Type &type) {
  tags.erase(type);
}

Blackboard & EntityAI::blackboard() {
  return localBlackboard;
}

void EntityAI::addStimulus(const Type &type, const EntityAI* source) {
  stimulus.push_back({type, source});
}

EntityAI::Stimulus EntityAI::getStimulus() {
  if (stimulus.empty()) return {Type(), nullptr};
  auto s = stimulus.front();
  stimulus.pop_front();
  return s;
}

MovementComponent* EntityAI::movement() {
  return ent->at<MovementComponent>(MOVEMENT_COMPONENT_INDEX).get();
}

AbilityComponent* EntityAI::actions() {
  return ent->at<AbilityComponent>(ABILITIES_COMPONENT_INDEX).get();
}

StateController* EntityAI::state() {
  return ent->at<StateController>(STATE_CONTROLLER_INDEX).get();
}

const InventoryComponent* EntityAI::inventory() const {
  return ent->at<InventoryComponent>(INVENTORY_COMPONENT_INDEX).get();
}

const WeaponsComponent* EntityAI::weapons() const {
  return ent->at<WeaponsComponent>(WEAPONS_COMPONENT_INDEX).get();
}

const AttributeComponent* EntityAI::attributes() const {
  return ent->at<AttributeComponent>(ATTRIBUTE_COMPONENT_INDEX).get();
}

const EffectComponent* EntityAI::effects() const {
  return ent->at<EffectComponent>(EFFECT_COMPONENT_INDEX).get();
}

const StateController* EntityAI::state() const {
  return ent->at<StateController>(STATE_CONTROLLER_INDEX).get();
}

const EntityAI* EntityAI::target() const {
  return targetPtr;
}

bool EntityAI::hasTarget() const {
  return targetPtr != nullptr;
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
  return r;
}

const vertex_t* EntityAI::vertex() const {
  const vertex_t* vert = m_vertex == nullptr ? lastVertex : m_vertex;
  return vert;
}

bool EntityAI::hasGroup() const {
  return groupPtr != nullptr;
}

// bool EntityAI::isGroupAI() const {
//   return hasGroup() && aiData.groupPtr->ai() == this;
// }

bool EntityAI::isGroupChief() const {
  return hasGroup() && groupPtr->chief() == this;
}

const AIGroup* EntityAI::group() const {
  return groupPtr;
}

entity_type EntityAI::entityType() const {
  return entType;
}

std::vector<const EntityAI*> EntityAI::getObjectsInRadius(const float &radius) const {
  std::vector<const EntityAI*> vector;
  
  const vertex_t* vert = vertex();
  float finalRadius = radius;
  
  while (finalRadius > 0.0f) {
//     for (size_t v = 0; v < vert->size(); ++v) {
//       
//     }
    
    // тут наверное неплохо сработает рекурсия
  }
  
  return vector;
}

std::vector<const EntityAI*> EntityAI::getObjectsInRadius(const entity_type &type, const float &radius) const {
  
}

const EntityAI* EntityAI::getNextCollider(size_t &index) const {
  const EntityAI* nextEntity = nullptr;
  while (nextEntity == nullptr && index < Global::physics()->getOverlappingDataSize()) {
    auto arr = Global::physics()->getOverlappingPairsData();
    
    if (arr->at(index).firstIndex == objectIndex) {
      auto cont = Global::physics()->getIndexContainer(arr->at(index).secondIndex);
      PhysUserData* data = reinterpret_cast<PhysUserData*>(cont->userData);
      nextEntity = data->aiComponent;
    }
    
    if (arr->at(index).secondIndex == objectIndex) {
      auto cont = Global::physics()->getIndexContainer(arr->at(index).firstIndex);
      PhysUserData* data = reinterpret_cast<PhysUserData*>(cont->userData);
      nextEntity = data->aiComponent;
    }
    
    ++index;
  }
  
  return nextEntity;
}

bool EntityAI::hasTag(const Type &tag) const {
  auto itr = tags.find(tag);
  return itr != tags.end();
}

bool EntityAI::grounded() const {
  return m_vertex != nullptr; // булева переменная или не нужна?
}

const yacs::entity* EntityAI::getEntity() const {
  return ent;
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
