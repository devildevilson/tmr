#ifndef EDITABLE_H
#define EDITABLE_H

#include <cstdint>
#include <cstddef>

class Editable {
public:
  Editable() = default;
  virtual void uiDraw() {};
protected:
  size_t graphicIndex = 0;
};

#endif
