#ifndef EVENT_FUNCTOR_H
#define EVENT_FUNCTOR_H

#include <cstdint>

#include "Type.h"

namespace yacs {
  class entity;
}

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

class EventFunctor {
public:
  virtual ~EventFunctor() = default;

  virtual event call(const Type &type, const EventData &data, yacs::entity* entity) = 0;
};

#endif //EVENT_FUNCTOR_H
