#ifndef ENTITY_LOADER_H
#define ENTITY_LOADER_H

#include "id.h"
#include "resource.h"
#include "resource_parser.h"
#include "resource_container.h"
#include "RenderStructures.h"
#include "ArrayInterface.h"
#include "entity_creator_resources.h"
#include "entity_creator.h"

namespace yacs {
  class entity;
}

//class AttributesLoader;

namespace devils_engine {
  namespace core {
    struct state_t;
  }
  
  namespace game {
    struct ability_t;
  }
  
  namespace resources {
    class abilities_loader;
    class attributes_loader;
    class state_loader;
    
    namespace enitity {
      struct load_data : public core::resource {
        struct pickup_data {
          utils::id id;
          size_t quantity;
        };
        
        struct intelligence {
          utils::id tree;
          utils::id func;
        };
        
        struct phys_data {
          bool dynamic;
          uint32_t collisionGroup;
          uint32_t collisionFilter;
          float stairHeight;
          float height;
          float width;
          float gravCoef;
        };
        
        pickup_data pickup;
        utils::id drop_item; // нужно ли это вообще? можно обойтись и без этого
        intelligence intel;
        utils::id default_state;
        phys_data physics;
        
        std::vector<std::pair<utils::id, double>> attributes;
        std::vector<utils::id> abilities;
        std::vector<utils::id> states;
      };
    }
    
    class entity_loader : public parser<enitity::load_data, 30>, public loader, public validator {
    public:
      enum errors {
        ERROR_FILE_NOT_FOUND,
        ERROR_BAD_ATTRIBUTE_TYPE,
        ERROR_BAD_HEIGHT_WIDTH_PHYSICS_DATA,
        ERROR_COULD_NOT_FIND_ATTRIBUTE_RESOURCE,
        ERROR_COULD_NOT_FIND_ABILITY_RESOURCE,
        ERROR_COULD_NOT_FIND_STATE_RESOURCE
      };
      
      enum warnings {
        WARNING_BAD_COLLISION_GROUP,
        WARNING_FOUND_INTELLIGENCE_TREE_AND_FUNC,
      };
      
      struct create_info {
        abilities_loader* abilities;
        attributes_loader* attributes;
        state_loader* states;
        game::entity_creators_container_load* container;
        std::unordered_map<utils::id, tb::BehaviorTree*> behaviors;
      };
      entity_loader(const create_info &info);
      ~entity_loader();
      
      bool validate(utils::problem_container<info::error> &errors, utils::problem_container<info::warning> &warnings) const override;

      bool load(const utils::id &id) override;
      bool unload(const utils::id &id) override;
      void end() override;
      
      void clear() override;
    private:
      abilities_loader* abilities;
      attributes_loader* attributes;
      state_loader* states;
      game::entity_creators_container_load* container;
      std::unordered_map<utils::id, tb::BehaviorTree*> behaviors;
      
      utils::id check_json(const std::string &path_prefix, const std::string &file, const nlohmann::json &data, const size_t &mark, enitity::load_data& info, utils::problem_container<info::error> &errors, utils::problem_container<info::warning> &warnings) const override;
    };
  }
}

#endif
