#ifndef EVENT_H
#define EVENT_H

#include <string>
#include <unordered_map>
#include <vector>
#include <functional>

#include "Type.h"

#define EVENT_UNDEFINED 0

enum event : uint32_t {
  success = 0, // 00
  running = 1, // 01
  failure = 2, // 10
  can_be_deleted = (1<<2) // 100
};

struct EventData {
  void* additionalData;
  void* userData;
};

// как тут можно сделать обращение к этому классу в мультитрединге?
// тут ничего особенно сделать не выйдет (но мьютекс в принципе можно добавить, скорее всего еще к каждому вектору)
// для чего глобальные эвенты могут нам пригодиться?
// пока не знаю
class EventsContainer {
public:
  EventsContainer();
  ~EventsContainer();
  
  // как удалять функцию? (можно к каждой функции добавлять уникальный айдишник, и по нему искать)
  void registerEvent(const Type &type, const std::function<event(const Type &, const EventData &)> &func);
  bool hasEvent(const Type &type) const;
  event fireEvent(const Type &type, const EventData &data);
protected:
  std::unordered_map<Type, std::vector<std::function<event(const Type &, const EventData &)>>> events;
};

#endif
