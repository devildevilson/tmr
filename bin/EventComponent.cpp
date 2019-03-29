#include "EventComponent.h"

CLASS_TYPE_DEFINE_WITH_NAME(EventComponent, "EventComponent")

EventComponent::EventComponent() {}
EventComponent::~EventComponent() {}

void EventComponent::update(const uint64_t &time)  { (void)time; }
void EventComponent::init(void* userData) { (void)userData; }

void EventComponent::registerEvent(const Type &type, const std::function<event(const Type &, const EventData &)> &func) {
  events[type].push_back(func);
}

bool EventComponent::hasEvent(const Type &type) const {
  auto itr = events.find(type);
  if (itr == events.end()) return false;
  
  return !itr->second.empty();
}

event EventComponent::fireEvent(const Type &type, const EventData &data) {
  auto itr = events.find(type);
  if (itr == events.end()) return failure;
  
  event val = success;
  
  for (size_t i = 0; i < itr->second.size(); ++i) {
    const event ret = itr->second[i](type, data);
    
    if ((ret & can_be_deleted) == can_be_deleted) {
      std::swap(itr->second[i], itr->second.back());
      itr->second.pop_back();
      --i;
    }
    
    const uint32_t mask = ~static_cast<uint32_t>(can_be_deleted);
    const event tmp = static_cast<event>(mask & ret);
    val = std::max(val, tmp);
  }
  
  if (itr->second.empty()) events.erase(itr);
  
  return val;
}
