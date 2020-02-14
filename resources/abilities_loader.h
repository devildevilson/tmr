#ifndef ABILITIES_LOADER_H
#define ABILITIES_LOADER_H

#include "resource.h"
#include "resource_parser.h"
#include "resource_container.h"
//#include "ability.h"
#define ABILITIES_CONTAINER
#include "game_resources.h"

namespace devils_engine {
  namespace game {
    struct ability_t;
  }
  
  namespace resources {
    class state_loader;
    class effects_loader;
    
    namespace ability {
      struct load_data : public core::resource {
//         utils::id id;
        std::string name;
        std::string description;
        utils::id cast_state;
        utils::id cost_effect;
      };
    }
    
    class abilities_loader : public parser<ability::load_data, 30>, public loader, public validator {
    public:
      //using data_container = utils::resource_container_array<utils::id, game::ability_t, 30>;
      
      enum errors {
        ERROR_FILE_NOT_FOUND = 0,
        ERROR_ABILITY_MUST_HAVE_AN_ID,
        ERROR_ABILITY_CAST_STATE_MUST_BE_SPECIFIED,
        ERROR_COULD_NOT_FIND_STATE,
        ERROR_COULD_NOT_FIND_EFFECT,
      };
      
      struct create_info {
        game::abilities_container_load* container;
        state_loader* states;
        effects_loader* effects;
      };
      abilities_loader(const create_info &info);
      ~abilities_loader();

      bool validate(utils::problem_container<info::error> &errors, utils::problem_container<info::warning> &warnings) const override;
      
      bool load(const utils::id &id) override;
      bool unload(const utils::id &id) override;
      void end() override;
      void clear() override;
      
//       const game::ability_t* get_ability(const utils::id &id) const;
    protected:
      game::abilities_container_load* container;
      state_loader* states;
      effects_loader* effects;
      
      utils::id check_json(const std::string &file_path, const std::string &file, const nlohmann::json &data, const size_t &mark, ability::load_data& info, utils::problem_container<info::error> &errors, utils::problem_container<info::warning> &warnings) const override;
    };
  }
}

#endif
