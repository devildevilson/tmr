#ifndef WEAPON_H
#define WEAPON_H

#include "id.h"
#include <cstddef>

namespace devils_engine {
  namespace core {
    struct state_t;
  }
  
  namespace game {
    struct effect_t;
    struct ability_t;
    
    struct weapon_t {
      inline ~weapon_t() {
        delete [] states;
        delete [] abilitites;
      }
      
      utils::id id;
      const effect_t* cost;
      size_t states_count;
      size_t abilities_count;
      const core::state_t** states;
      const ability_t** abilitites;
    };
  }
}

// нужен ли оружию тип? для быстрых проверок возможно

#endif
