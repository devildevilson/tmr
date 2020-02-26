#ifndef STATE_LOADER_H
#define STATE_LOADER_H

#include "id.h"
#include "resource.h"
#include "resource_parser.h"
#include "resource_container.h"
#define STATES_CONTAINER
#include "game_resources.h"
#include "ArrayInterface.h"
#include "RenderStructures.h"

namespace yacs {
  class entity;
}

namespace devils_engine {
  namespace resources {
    class image_loader;
    
    namespace state {
      struct load_data : public core::resource {
        struct image_data {
          utils::id image;
          size_t index;
          bool flipV;
          bool flipU;
        };
        
        std::vector<image_data> image_datas;
        size_t time;
        std::string func;
        utils::id next;
      };
    }
    
    class state_loader : public parser<state::load_data, 30>, public validator, public loader {
    public:
      enum errors {
        ERROR_FILE_NOT_FOUND,
        ERROR_WRONG_FRAME_DATA,
        ERROR_ID_IS_NOT_SPECIFIED,
        ERROR_TIME_IS_NOT_FOUND,
        ERROR_NEXT_IS_NOT_FOUND,
        ERROR_STATE_IMAGE_IS_NOT_FOUND,
        ERROR_NEXT_STATE_IS_NOT_FOUND
      };
      
      struct create_info {
        game::states_container_load* container;
        ArrayInterface<Texture>* textures;
        class image_loader* image_loader;
        std::unordered_map<std::string, std::function<void(yacs::entity*, const size_t &)>> functions;
      };
      state_loader(const create_info &info);
      ~state_loader();

      bool validate(utils::problem_container<info::error> &errors, utils::problem_container<info::warning> &warnings) const override;
      
      bool load(const utils::id &id) override;
      bool unload(const utils::id &id) override;
      void end() override;
      void clear() override;
      
      //const core::state_t* get(const utils::id& id) const;
    private:
      game::states_container_load* container;
      ArrayInterface<Texture>* textures;
      class image_loader* image_loader;
      std::unordered_map<std::string, std::function<void(yacs::entity*, const size_t &)>> functions;
      
      utils::id check_json(const std::string &path_prefix, const std::string &file, const nlohmann::json &data, const size_t &mark, state::load_data& info, utils::problem_container<info::error> &errors, utils::problem_container<info::warning> &warnings) const override;
    };
  }
}

#endif
