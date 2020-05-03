#include "decals_container_component.h"
#include "EntityComponentSystem.h"
#include "state.h"
#include <cstring>
#include "core_funcs.h"
#include "TransformComponent.h"
#include "global_components_indicies.h"
#include "stages.h"
#include "Globals.h"

namespace devils_engine {
  namespace components {
    decals_container::decals_container(yacs::entity* ent) : ent(ent) {}
    decals_container::~decals_container() {
      // нужно почистить указатели, придется обойти всех ремувить так
      for (auto &decal : decals) {
        auto decal_data = decal.decal_ent->get<struct decal_data>();
        decal_data->remove(ent);
      }
    }
    
    void decals_container::draw() {
      if (core::deleted_state(ent)) return;
      
      auto trans = ent->at<TransformComponent>(game::entity::transform);
      
      for (const auto &decal : decals) {
        const render::decal_gbuffer::decal_data data{
          decal.vertices.data(),
          decal.vertices.size(),
          trans.valid() ? trans->index() : UINT32_MAX,
          UINT32_MAX,
          UINT32_MAX,
          decal.state->frame.texture_offset
        };
        ASSERT(Global::get<render::decal_gbuffer>() != nullptr);
        Global::get<render::decal_gbuffer>()->add(data);
      }
    }
    
    // использую вектор, нужно придумать что то другое
    void decals_container::add(const decal &data) {
      decals.push_back(data);
    }
    
    void decals_container::remove(yacs::entity* decal_ent) {
      for (size_t i = 0; i < decals.size(); ++i) {
        if (decals[i].decal_ent == decal_ent) {
          std::swap(decals[i], decals.back());
          decals.pop_back();
          break;
        }
      }
    }
    
    decal_data::decal_data(yacs::entity* ent) : ent(ent) {}
    decal_data::~decal_data() {
      for (auto ent : entities) {
        auto container = ent->at<decals_container>(game::wall::decal_container);
        container->remove(this->ent);
      }
    }
    
    void decal_data::add(yacs::entity* ent) {
      entities.push_back(ent);
    }
    
    void decal_data::remove(yacs::entity* ent) {
      for (size_t i = 0; i < entities.size(); ++i) {
        if (entities[i] == ent) {
          std::swap(entities[i], entities.back());
          entities.pop_back();
          break;
        }
      }
    }
  }
}
