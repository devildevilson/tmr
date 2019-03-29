#include "EntityAI.h"

// #include "EventComponent.h"

EntityAI::EntityAI() : aiData({0.0f, nullptr, nullptr, 0, 0, nullptr, nullptr}) {}
EntityAI::~EntityAI() {}

// r(0.0f), vertex(nullptr), lastVertex(nullptr), timeThreshold(0), currentTime(0), targetPtr(nullptr), groupPtr(nullptr), localEvents(nullptr)

event EntityAI::pushEvent(const Type &event, void* data) {
  // ранее я думал в таких эвентах как moveToTarget заполнять адишоналДата, например таргетом
  // но теперь я понятие не имею что тут можно придумать, хотя можно сделать вот что:
  // регламентировать определенную структуру под дополнительные данные и заполнять ее всякий раз
  // наверное так лучше всего сделать
  
  const EventData ed{
    &aiData,
    data
  };
  
  return localEvents->fireEvent(event, ed);
}

void EntityAI::setTarget(const EntityAI* target) {
  aiData.targetPtr = target;
}

const EntityAI* EntityAI::target() const {
  return aiData.targetPtr;
}

bool EntityAI::hasTarget() const {
  return aiData.targetPtr != nullptr;
}

glm::vec4 EntityAI::position() const {
  return pos;
}

glm::vec4 EntityAI::direction() const {
  return dir;
}

glm::vec4 EntityAI::velocity() const {
  return vel;
}

float EntityAI::raduis() const {
  return aiData.r;
}

bool EntityAI::hasGroup() const {
  return aiData.groupPtr != nullptr;
}

bool EntityAI::isGroupAI() const {
  return hasGroup() && aiData.groupPtr->ai() == this;
}

bool EntityAI::isGroupChief() const {
  return hasGroup() && aiData.groupPtr->chief() == this;
}

const AIGroup* EntityAI::group() const {
  return aiData.groupPtr;
}

event AIGroup::pushEvent(const EntityAI* member, const Type &event, void* data) {
  if (member == nullptr) {
    enum event ev = success;
    
    for (size_t i = 0; i < members.size(); ++i) {
      const enum event ret = members[i]->pushEvent(event, data);
      
      const uint32_t mask = ~static_cast<uint32_t>(can_be_deleted);
      const enum event tmp = static_cast<enum event>(mask & ret);
      ev = std::max(ev, tmp);
    }
    
    return ev;
  }
  
  EntityAI* realMember = nullptr;
  
  for (size_t i = 0; i < members.size(); ++i) {
    if (member == members[i]) {
      realMember = members[i];
      break;
    }
  }
  
  if (realMember == nullptr) return failure;
  
  return realMember->pushEvent(event, data);
}
  
const EntityAI* AIGroup::ai() const {
  return groupAI;
}

const EntityAI* AIGroup::member(const size_t &index) const {
  return members[index];
}

const EntityAI* AIGroup::chief() const {
  return groupChief;
}

size_t AIGroup::size() const {
  return members.size();
}

glm::vec4 AIGroup::position() const {
  return groupAI->position();
}

glm::vec4 AIGroup::direction() const {
  return groupAI->direction();
}

// пока непонятно что именно мне нужно
glm::vec4 AIGroup::extents() const {
  
}

// с этим тоже пока что ничего не понятно
// bool AIGroup::isGroupInBounds() const;
