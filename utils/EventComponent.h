#ifndef EVENT_COMPONENT_H
#define EVENT_COMPONENT_H

#include "Type.h"
#include "EventFunctor.h"

#include <vector>
#include <functional>
#include <mutex>

// для того чтобы сократить количество памяти используемой std::function (у нас EventComponent создается для почти каждого объекта)
// нужно сделать специальный объект-функтор, в нем будет все вызываться
// важно что этот объект может быть не уникален для каждого компонента

class BasicEventFunctor : public EventFunctor {
public:
  struct CreateInfo {
    std::function<event(const Type &, const EventData &)> func;
  };
  BasicEventFunctor(const CreateInfo &info);

  event call(const Type &type, const EventData &data, yacs::entity* entity) override;
private:
  std::function<event(const Type &, const EventData &)> func;
};

class EventComponent {
public:
  struct CreateInfo {
    yacs::entity* entity;
  };
  EventComponent(const CreateInfo &info);
  ~EventComponent();
  
//  void update(const uint64_t &time) override;
//  void init(void* userData) override;
  
  void registerEvent(const Type &type, EventFunctor* func);
  void removeEvent(EventFunctor* func);
  bool hasEvent(const Type &type) const;
  // логично передавать все же энтити от которого пришел вызов эвента на константной основе
  event fireEvent(const Type &type, const EventData &data);
  
  void registerEvent_save(const Type &type, EventFunctor* func);
  bool hasEvent_save(const Type &type) const;
  event fireEvent_save(const Type &type, const EventData &data);
private:
  yacs::entity* entity;

  // тут нужно добавить мьютекс 
  // и добавить безопасные функции которые будут вызывать мьютекс
  mutable std::mutex local_mutex;
  //std::unordered_map<Type, std::vector<std::function<event(const Type &, const EventData &)>>> events;
  //std::unordered_map<Type, std::vector<EventFunctor*>> events;
  // я хочу поменять на просто std::vector
  std::vector<EventFunctor*> events;
};

#endif
