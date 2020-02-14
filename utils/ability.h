#ifndef ABILITY_H
#define ABILITY_H

#include <string>
#include "id.h"

namespace devils_engine {
  namespace core {
    struct state_t;
  }
  
  namespace game {
    struct effect_t;
    
    struct ability_t {
      utils::id id;
      std::string name;
      std::string description;
      const core::state_t* cast;
      const effect_t* cost;
      //size_t cooldown;
    };
  }
}

// да и все, из стейта получаем какое то взаимодействие
// скорее всего нужно сделать компонент с текущей абилкой
// ко всему прочему нужно обрабатывать кулдаун
// абилки могут зависеть от оружия
// независимые абилки будут только у мага,
// можно все абилки сделать зависимыми от оружия
// но у мага каждое оружие обладает всеми абилками
// ну кстати идея норм, кулдауны? монстрам они нахер не нужны по идее
// игрок будет и так стеснен анимацией каста

#endif
