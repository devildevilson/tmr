#include "entity_loader.h"

#include "attributes_loader.h"
#include "state_loader.h"
#include "abilities_loader.h"
#include "shared_collision_constants.h"
#include "Globals.h"

namespace devils_engine {
  namespace resources {
    entity_loader::entity_loader(const create_info &info) : parser("entities"), abilities(info.abilities), attributes(info.attributes), states(info.states), container(info.container) {}
    entity_loader::~entity_loader() {
      clear();
    }

    bool entity_loader::validate(utils::problem_container<info::error> &errors, utils::problem_container<info::warning> &warnings) const {
      bool ret = true;
      for (size_t i = 0; i < loading_data.size(); ++i) {
        auto ptr = loading_data.at(i);
        
        for (const auto &attrib : ptr->attributes) {
          auto res = attributes->resource(attrib.first);
          if (res == nullptr) {
            errors.add(mark(), ERROR_COULD_NOT_FIND_ATTRIBUTE_RESOURCE, "Could not find attribute "+attrib.first.name());
            ret = false;
          }
        }
        
        for (const auto &ability : ptr->abilities) {
          auto res = abilities->resource(ability);
          if (res == nullptr) {
            errors.add(mark(), ERROR_COULD_NOT_FIND_ABILITY_RESOURCE, "Could not find attribute "+ability.name());
            ret = false;
          }
        }
        
        for (const auto &state : ptr->states) {
          auto res = states->resource(state);
          if (res == nullptr) {
            errors.add(mark(), ERROR_COULD_NOT_FIND_STATE_RESOURCE, "Could not find state "+state.name());
            ret = false;
          }
        }
      }
      
      (void)warnings;
      return ret;
    }
    
    bool entity_loader::load(const utils::id &id) {
      {
        auto itr = container->get(id);
        if (itr != nullptr) return true;
      }
      
      auto ptr = loading_data.get(id);
      if (ptr == nullptr) return false;
      
      std::vector<components::attributes::create_info::init<core::float_type>> float_init;
      std::vector<components::attributes::create_info::init<core::int_type>> int_init;
      
      for (const auto &attrib : ptr->attributes) {
        const bool res = attributes->load(attrib.first);
        if (!res) throw std::runtime_error("Could not load attribute "+attrib.first.name());
        
        {
          auto attrib_type = Global::get<game::float_attribute_types_container>()->get(attrib.first);
          if (attrib_type != nullptr) {
            float_init.push_back({static_cast<core::float_type>(attrib.second), attrib_type});
            continue;
          }
        }
        
        {
          auto attrib_type = Global::get<game::int_attribute_types_container>()->get(attrib.first);
          if (attrib_type != nullptr) {
            int_init.push_back({static_cast<core::int_type>(attrib.second), attrib_type});
            continue;
          }
        }
        
        throw std::runtime_error("Could not find attribute "+attrib.first.name());
      }
      
      std::vector<const game::ability_t*> ability_array;
      
      for (const auto &ability : ptr->abilities) {
        const bool res = abilities->load(ability);
        if (!res) throw std::runtime_error("Could not load ability "+ability.name());
        
        ability_array.push_back(Global::get<game::abilities_container>()->get(ability));
        if (ability_array.back() == nullptr) throw std::runtime_error("Could not find ability "+ability.name());
      }
      
      std::vector<const core::state_t*> states_array;
      for (const auto &state : ptr->states) {
        const bool res = states->load(state);
        if (!res) throw std::runtime_error("Could not load ability "+state.name());
        
        states_array.push_back(Global::get<game::states_container>()->get(state));
        if (states_array.back() == nullptr) throw std::runtime_error("Could not find ability "+state.name());
      }
      
      const core::state_t* default_state = nullptr;
      if (ptr->default_state.valid()) {
        const bool res = states->load(ptr->default_state);
        if (!res) throw std::runtime_error("Could not load ability "+ptr->default_state.name());
        
        default_state = Global::get<game::states_container>()->get(ptr->default_state);
        if (default_state == nullptr) throw std::runtime_error("Could not find ability "+ptr->default_state.name());
      }
      
      const core::entity_creator::pickup_data pickup{
        ptr->pickup.id,
        ptr->pickup.quantity
      };
      
      tb::BehaviorTree* tree = nullptr;
      if (ptr->intel.tree.valid()) {
        auto itr = behaviors.find(ptr->intel.tree);
        if (itr != behaviors.end()) {
          tree = itr->second;
        } else {
          throw std::runtime_error("tree == nullptr");
        }
      }
      
      const core::entity_creator::intelligence intel{
        tree,
        ptr->intel.func
      };
      
      const core::entity_creator::phys_data physics{
        ptr->physics.dynamic,
        ptr->physics.collisionGroup,
        ptr->physics.collisionFilter,
        ptr->physics.collisionTrigger,
        ptr->physics.stairHeight,
        ptr->physics.height,
        ptr->physics.width,
        ptr->physics.gravCoef,
        ptr->collision_property
      };
      
      const struct core::entity_creator::create_info info{
        float_init,
        int_init,
        ability_array,
        states_array,
        ptr->id,
        default_state,
        pickup,
        intel,
        physics,
        nullptr
      };
      auto creator_ptr = container->create(ptr->id, info);
      (void)creator_ptr;
      
      return true;
    }
    
    bool entity_loader::unload(const utils::id &id) {
      return container->destroy(id);
    }
    
    void entity_loader::end() {
      
    }
    
    void entity_loader::clear() {
      // здесь может контейнер почистить
    }
    
    utils::id entity_loader::check_json(const std::string &path_prefix, const std::string &file, const nlohmann::json &data, const size_t &mark, enitity::load_data& info, utils::problem_container<info::error> &errors, utils::problem_container<info::warning> &warnings) const {
      bool has_id = false, has_physics = false;
      
      const size_t errors_size = errors.size();
      
      for (auto itr = data.begin(); itr != data.end(); ++itr) {
        if (itr.value().is_string() && itr.key() == "id") {
          has_id = true;
          info.id = utils::id::get(itr.value().get<std::string>());
          continue;
        }
        
        if (itr.value().is_object() && itr.key() == "attributes") {
          for (auto attrib = itr.value().begin(); attrib != itr.value().end(); ++attrib) {
            if (!attrib.value().is_number()) {
              errors.add(mark, entity_loader::ERROR_BAD_ATTRIBUTE_TYPE, "Bad attribute type");
              break;
            }
            
            info.attributes.push_back(std::make_pair(utils::id::get(attrib.key()), attrib.value().get<double>()));
          }
          
          continue;
        }
        
        if (itr.value().is_object() && itr.key() == "pickup_item") {
          for (auto pickup = itr.value().begin(); pickup != itr.value().end(); ++pickup) {
            if (pickup.value().is_string() && pickup.key() == "id") {
              info.pickup.id = utils::id::get(pickup.value().get<std::string>());
              continue;
            }
            
            if (pickup.value().is_number_unsigned() && pickup.key() == "quantity") {
              info.pickup.quantity = pickup.value().get<size_t>();
              continue;
            }
          }
          continue;
        }
        
        if (itr.value().is_array() && itr.key() == "abilities") {
          for (size_t i = 0; i < itr.value().size(); ++i) {
            info.abilities.push_back(utils::id::get(itr.value()[i].get<std::string>()));
          }
          continue;
        }
        
        if (itr.value().is_object() && itr.key() == "intelligence") {
          bool has_tree = false, has_func = false;
          for (auto intel = itr.value().begin(); intel != itr.value().end(); ++intel) {
            if (intel.value().is_string() && intel.key() == "behaviour_tree") {
              has_tree = true;
              info.intel.tree = utils::id::get(intel.value().get<std::string>());
              continue;
            }
            
            if (intel.value().is_string() && intel.key() == "behaviour_func") {
              has_func = true;
              
              continue;
            }
            
            if (intel.value().is_string() && intel.key() == "pathfinder_type") {
              info.intel.func = utils::id::get(intel.value().get<std::string>());
              continue;
            }
          }
          
          if (has_tree && has_func) {
            warnings.add(mark, WARNING_FOUND_INTELLIGENCE_TREE_AND_FUNC, "Founded intelligence tree id and func id simultaneously. Using tree id");
          }
          
          continue;
        }
        
        if (itr.value().is_array() && itr.key() == "states") {
          for (size_t i = 0; i < itr.value().size(); ++i) {
            info.states.push_back(utils::id::get(itr.value()[i].get<std::string>()));
          }
          continue;
        }
        
        if (itr.value().is_string() && itr.key() == "default_state") {
          info.default_state = utils::id::get(itr.value().get<std::string>());
          continue;
        }
        
        // нам так или иначе нужно указать функцию коллизии, с другой стороны у нас может быть глобальная функция колизии, а тут указать просто какой нибудь тип
        // 
        if (itr.value().is_string() && itr.key() == "collision_property") {
          
        }
        
        if (itr.value().is_object() && itr.key() == "physics") {
          has_physics = true;
          bool hasHeight = false, hasWidth = false, hasStairHeight = false;;
          for (auto physData = itr.value().begin(); physData != itr.value().end(); ++physData) {
            if (physData.value().is_number() && physData.key() == "height") {
              hasHeight = true;
              info.physics.height = physData.value().get<float>();
              continue;
            }
            
            if (physData.value().is_number() && physData.key() == "width") {
              hasWidth = true;
              info.physics.width = physData.value().get<float>();
              continue;
            }
            
            if (physData.value().is_number() && physData.key() == "stair_height") {
              hasStairHeight = true;
              info.physics.stairHeight = physData.value().get<float>();
              continue;
            }
            
            if (physData.value().is_number() && physData.key() == "gravitational_coefficient") {
              info.physics.gravCoef = physData.value().get<float>();
              continue;
            }
            
            if (physData.value().is_boolean() && physData.key() == "dynamic") {
              info.physics.dynamic = physData.value().get<bool>();
              continue;
            }
            
            if (physData.value().is_string() && physData.key() == "collision_group") {
              const std::string &str = physData.value().get<std::string>();
              
              if (str == "monster") {
                info.physics.collisionGroup = MONSTER_COLLISION_TYPE;
                info.physics.collisionFilter = monster_collision_filter;
                info.physics.collisionTrigger = monster_trigger_filter;
              } else if (str == "small_decoration") {
                info.physics.collisionGroup = SMALL_DECORATION_COLLISION_TYPE;
                info.physics.collisionFilter = small_decoration_collision_filter;
                info.physics.collisionTrigger = small_decoration_trigger_filter;
              } else if (str == "big_decoration") {
                info.physics.collisionGroup = BIG_DECORATION_COLLISION_TYPE;
                info.physics.collisionFilter = big_decoration_collision_filter;
                info.physics.collisionTrigger = big_decoration_trigger_filter;
              } else if (str == "ghost") {
                info.physics.collisionGroup = GHOST_COLLISION_TYPE;
                info.physics.collisionFilter = ghost_collision_filter;
                info.physics.collisionTrigger = ghost_trigger_filter;
              } else if (str == "item") {
                info.physics.collisionGroup = ITEM_COLLISION_TYPE;
                info.physics.collisionFilter = item_collision_filter;
                info.physics.collisionTrigger = item_trigger_filter;
              } else if (str == "interaction") {
                info.physics.collisionGroup = INTERACTION_COLLISION_TYPE;
                info.physics.collisionFilter = interaction_collision_filter;
                info.physics.collisionTrigger = interaction_trigger_filter;
              } else {
                warnings.add(mark, entity_loader::WARNING_BAD_COLLISION_GROUP, "Bad collision group. Using default (monster)");
                
                info.physics.collisionGroup = MONSTER_COLLISION_TYPE;
                info.physics.collisionFilter = monster_collision_filter;
                info.physics.collisionTrigger = monster_trigger_filter;
              }
              
              continue;
            }
            
            // триггер груп по идее будет сильно ограничен, но нужен он чтобы например айтем пересекался только со стенами, но при этом триггерился от игрока
            // этот функционал можно добавить в коллизию
//             if (physData.value().is_string() && physData.key() == "trigger_group") {
//               const std::string &str = physData.value().get<std::string>();
//               
//             }
            
            if (physData.value().is_string() && physData.key() == "collision_property") {
              info.collision_property = utils::id::get(physData.value().get<std::string>());
              continue;
            }
          }
          
          if (!(hasHeight && hasWidth)) {
            errors.add(mark, entity_loader::ERROR_BAD_HEIGHT_WIDTH_PHYSICS_DATA, "Could not find height and width data");
          }
          
          if (!hasStairHeight) {
            info.physics.stairHeight = info.physics.height * 0.333f;
          }
          continue;
        }
      }
      
      if (!has_id) {
        errors.add(mark, entity_loader::ERROR_ENTITY_MUST_HAVE_AN_ID, "Could not find entity id");
        return utils::id();
      }
      
      if (!has_physics) {
        errors.add(mark, entity_loader::ERROR_ENTITY_MUST_HAVE_A_PHYSICS_DATA, "Physics data must be specified. Entity: "+info.id.name());
      }
      
      (void)file;
      (void)path_prefix;
      return errors.size() == errors_size ? info.id : utils::id();
    }
  }
}
