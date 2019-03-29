#ifndef EDITABLE_H
#define EDITABLE_H

#include <cstdint>

class Editable {
public:
  virtual void uiDraw() {};
protected:
  size_t graphicIndex = 0;
};

#endif
