#ifndef ENGINE_H
#define ENGINE_H

#include <cstdint>

class Engine {
public:
  virtual ~Engine() {}
  
  virtual void update(const uint64_t &time) = 0;
};

#endif
