#ifndef PATHFINDING_SYSTEM_H
#define PATHFINDING_SYSTEM_H

#include <vector>
#include "id.h"
#include "Utility.h"

namespace devils_engine {
  namespace components {
    class vertex;
  }
  
  namespace path {
    enum class find_state {
      exist,
      not_exist,
      delayed
    };
    
    struct container {
      // этого достаточно для того чтобы описать путь
      // думаю что тут может еще пригодиться способ узнать вершину по которой мы сейчас проходим
      struct data {
        simd::vec4 pos;
        simd::vec4 dir;
      };
      
      std::vector<data> array;
    };
    
    struct request {
      utils::id id;
      components::vertex* start;
      components::vertex* end;
    };
    
    struct response {
      find_state state;
      container* path;
      size_t counter;
    };
  }
  
  namespace systems {
    class pathfinding {
      
    };
  }
}

#endif
