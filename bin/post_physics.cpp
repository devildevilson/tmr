#include "post_physics.h"

#include "Globals.h"
#include "ThreadPool.h"
#include "EntityComponentSystem.h"
#include "global_components_indicies.h"
#include "graphics_component.h"
#include "type_info_component.h"
#include "Physics.h"
#include "core_funcs.h"
#include "decals_container_component.h"

namespace devils_engine {
  namespace post_physics {
    void func(const ArrayInterface<BroadphasePair>* pairs, const size_t &start, const size_t &count) { //, yacs::entity* player
      for (size_t i = start; i < start+count; ++i) {
        const uint32_t index = pairs->at(i+1).secondIndex;
        //if (index == 5097) PRINT("drawing complex box")
        auto ent = reinterpret_cast<yacs::entity*>(Global::get<PhysicsEngine>()->getUserData(index));
//         if (ent == player) continue;
        if (core::is_player(ent)) continue;
        //auto type = ent->at<components::type_info>(game::entity::type_info);
        //static const utils::id wall = utils::id::get("wall");
        
        {
          auto graphics = ent->at<components::sprite_graphics>(game::entity::graphics);
          if (graphics.valid()) graphics->draw();
        }
        
        {
          auto graphics = ent->at<components::indexed_graphics>(game::entity::graphics);
          //ASSERT(type->id == wall && graphics.valid());
          if (graphics.valid()) {
//             std::cout << "index " << index << "\n";
            graphics->draw();
          }
        }
        
        {
          auto graphics = ent->at<components::complex_indices_graphics>(game::entity::graphics);
          if (graphics.valid()) graphics->draw();
        }
        
        {
          // скорее всего это будет единственный доступный свет
          auto light = ent->at<components::point_light>(game::monster::light);
          if (light.valid()) light->draw();
        }
        
        // декали
        {
          auto decal_container = ent->at<components::decals_container>(game::wall::decal_container);
          if (decal_container.valid()) decal_container->draw();
        }
        // дебаг
      }
    }
    
    void update(dt::thread_pool* pool) { //, yacs::entity* player
      // рисуем игрока здесь?
      
      const uint32_t frustumOutputCount = Global::get<PhysicsEngine>()->getFrustumTestSize();
      //std::cout << "frustumOutputCount " << frustumOutputCount << "\n";
      const auto frustumPairs = Global::get<PhysicsEngine>()->getFrustumPairs();
      
      const size_t count = std::ceil(float(frustumOutputCount) / float(pool->size()+1));
      size_t start = 0;
      for (uint32_t i = 0; i < pool->size()+1; ++i) {
        const size_t jobCount = std::min(count, frustumOutputCount-start);
        if (jobCount == 0) break;

        pool->submitbase([frustumPairs, start, jobCount] () {
          func(frustumPairs, start, jobCount);
        });

        start += jobCount;
      }

      pool->compute();
      pool->wait();
    }
  }
}
