#ifndef ABILITIES_STRUCT_H
#define ABILITIES_STRUCT_H

#include "Type.h"
#include <vector>

class AbilityType;

struct Abilities {
  struct Slot {
    size_t index;
    const AbilityType* type;
    Type state;
  };
  
  std::vector<Slot> slots;
};

#endif
