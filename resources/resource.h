#ifndef RESOURCE_H
#define RESOURCE_H

#include "id.h"

namespace devils_engine {
  namespace resources {
    template <typename T>
    class modification;
    //class parser;
  }
  
  namespace core {
    struct resource {
      utils::id id;
      size_t mem_size;
      size_t gpu_size;
      resources::modification<resource>* mod;
      size_t parser_mark;
    };
  }
}

#endif
