#ifndef ENGINE_H
#define ENGINE_H

#include <cstdint>

class Engine {
public:
  virtual ~Engine() {}
  
  virtual void update(const uint64_t &time) = 0;
  // должен появиться метод clean для того чтобы подготовиться к следуюшей карте
};

#endif
