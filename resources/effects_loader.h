#ifndef EFFECTS_LOADER_H
#define EFFECTS_LOADER_H

#include "id.h"
#include "resource.h"
#include "resource_parser.h"
#include "resource_container.h"
#define EFFECTS_CONTAINER
#include "game_resources.h"

namespace devils_engine {
  namespace effect {
    struct load_data : public core::resource {
      std::string name;
      std::string description;
      struct game::effect_t::type type;
      struct game::effect_t::container container;
    };
  }
  
  namespace resources {
    class attributes_loader;
    
    class effects_loader : public parser<effect::load_data, 30>, public loader, public validator {
    public:
      enum errors {
        ERROR_EFFECT_MUST_HAVE_AN_ID,
        ERROR_COULD_NOT_FIND_ATTRIB,
        ERROR_TOO_MANY_ATTRIBUTE_BONUSES
      };
      
      struct create_info {
        attributes_loader* attribs;
        game::effects_container_load* container;
      };
      effects_loader(const create_info &info);
      
      bool validate(utils::problem_container<info::error> &errors, utils::problem_container<info::warning> &warnings) const override;

      bool load(const utils::id &id) override;
      bool unload(const utils::id &id) override;
      void end() override;
      
      void clear() override;
    private:
      attributes_loader* attribs;
      game::effects_container_load* container;
      
      utils::id check_json(const std::string &path_prefix, const std::string &file, const nlohmann::json &data, const size_t &mark, effect::load_data& info, utils::problem_container<info::error> &errors, utils::problem_container<info::warning> &warnings) const override;
    };
  }
}

#endif
