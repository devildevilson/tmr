#ifndef DECALS_CONTAINER_COMPONENTS_H
#define DECALS_CONTAINER_COMPONENTS_H

#include "Utility.h"
#include <vector>
#include "shared_structures.h"

namespace yacs {
  class entity;
}

namespace devils_engine {
  namespace core {
    struct state_t;
  }
  
  namespace components {
    struct decals_container {
      struct decal {
        yacs::entity* decal_ent;
        //simd::vec4* points;
        //size_t points_count;
        std::vector<render::vertex> vertices;
        //uint32_t texture_index; // опять же стейт?
        const core::state_t* state;
      };
      
      yacs::entity* ent;
      std::vector<decal> decals;
      
      decals_container(yacs::entity* ent);
      ~decals_container();
      
      void draw();
      void add(const decal &data);
      void remove(yacs::entity* decal_ent);
    };
    
    struct decal_data {
      yacs::entity* ent;
      std::vector<yacs::entity*> entities;
      
      decal_data(yacs::entity* ent);
      ~decal_data();
      
      void add(yacs::entity* ent);
      void remove(yacs::entity* ent);
    };
  }
}

#endif
