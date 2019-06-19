#ifndef EVENT_COMPONENT_H
#define EVENT_COMPONENT_H

#include "EntityComponentSystem.h"
#include "Type.h"
#include <functional>

#include "Event.h"

class EventComponent : public yacs::Component {
public:
  CLASS_TYPE_DECLARE
  
  EventComponent();
  ~EventComponent();
  
  void update(const uint64_t &time) override;
  void init(void* userData) override;
  
  void registerEvent(const Type &type, const std::function<event(const Type &, const EventData &)> &func);
  bool hasEvent(const Type &type) const;
  event fireEvent(const Type &type, const EventData &data);
private:
  std::unordered_map<Type, std::vector<std::function<event(const Type &, const EventData &)>>> events;
};

#endif
