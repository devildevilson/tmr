#ifndef EVENT_TASK_SYSTEM_H
#define EVENT_TASK_SYSTEM_H

#include "Engine.h"

// мы должны откладывать выполнение определенных эвентов
// причем некоторые эвенты мы можем выполнить во время отрисовки и сна
class EventTaskSystem : public Engine {
public:
  EventTaskSystem();
  ~EventTaskSystem();
  
  void update(const uint64_t & time) override;
private:
  
};

#endif
