#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "Type.h"

struct ControllerChildData {
  uint32_t cont;
  
  ControllerChildData();
  ControllerChildData(const bool &changed, const bool &blocking, const bool &blockingMovement);
  void make(const bool &changed, const bool &blocking, const bool &blockingMovement);
  
  bool changed() const;
  bool blocking() const;
  bool blockngMovement() const;
};

class Controller {
public:
  virtual ~Controller() {}
  
  virtual void addChild(Controller* c) = 0;
  virtual ControllerChildData changeState(const Type &type) = 0;
  virtual void reset() = 0;
  virtual void finishCallback() = 0;
  
  virtual bool isFinished() const = 0;
  virtual bool isBlocking() const = 0;
  virtual bool isBlockingMovement() const = 0;
};

#endif
