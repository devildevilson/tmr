#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include <cstddef>

class Optimizer {
public:
  virtual ~Optimizer() {}
  
  virtual void optimize() = 0;
  virtual void clear() = 0;
  virtual size_t size() const = 0;
};

#endif
