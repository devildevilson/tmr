#ifndef EVENT_COMPONENT_H
#define EVENT_COMPONENT_H

#include "Type.h"
#include <functional>

#include "Event.h"

#include <mutex>

class EventComponent {
public:
  EventComponent();
  ~EventComponent();
  
//  void update(const uint64_t &time) override;
//  void init(void* userData) override;
  
  void registerEvent(const Type &type, const std::function<event(const Type &, const EventData &)> &func);
  bool hasEvent(const Type &type) const;
  event fireEvent(const Type &type, const EventData &data);
  
  void registerEvent_save(const Type &type, const std::function<event(const Type &, const EventData &)> &func);
  bool hasEvent_save(const Type &type) const;
  event fireEvent_save(const Type &type, const EventData &data);
private:
  // тут нужно добавить мьютекс 
  // и добавить безопасные функции которые будут вызывать мьютекс
  mutable std::mutex local_mutex;
  std::unordered_map<Type, std::vector<std::function<event(const Type &, const EventData &)>>> events;
};

#endif
