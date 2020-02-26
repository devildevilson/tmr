#ifndef MAP_DATA_H
#define MAP_DATA_H

#include "id.h"
#include <vector>

namespace yacs {
  class entity;
}

namespace yavf {
  class Buffer;
}

namespace devils_engine {
  namespace game {
    enum class difficulty {
      baby,
      easy,
      medium,
      hard,
      nightmare
    };
    
    struct map_data {
      utils::id current;
      utils::id episod;
      utils::id next_map;
      utils::id next_secret_map; // можем ли мы перепрыгуть на другой эпизод? наверное стандартными средствами нет
      enum difficulty difficulty;
      std::vector<std::pair<uint32_t, yacs::entity*>> tagged_entities;
      yavf::Buffer* vertices;
    };
    
    using map_data_container = const map_data;
    using map_data_container_load = map_data;
  }
}

#endif
