#ifndef SOUND_LOADER_H
#define SOUND_LOADER_H

#include "resource.h"
#include "resource_parser.h"
#include "resource_container.h"
#include "sound_data.h"

#include <string>

namespace devils_engine {
  namespace sound {
    struct load_data : public core::resource {
      enum type type;
      std::string path;
      bool forced_mono;
      bool cached;
      size_t pcm_size;
    };
  }
  
  namespace resources {
    class sound_loader : public parser<sound::load_data, 50>, public validator, public loader {
    public:
      enum errors {
        SOUND_TYPE_IS_NOT_SUPPORTED,
        COULD_NOT_LOAD_FILE,
        COULD_NOT_DECODE_FILE,
        MISSED_SOUND_ID,
        
      };
      
      enum warnings {
        FLAC_TYPE_MAY_LOSE_PRECISION
      }
      
      struct create_info {
        game::sounds_container_load* container;
      };
      sound_loader(const create_info &info);
      
      bool validate(utils::problem_container<info::error> &errors, utils::problem_container<info::warning> &warnings) const override;
      bool load(const utils::id &id) override;
      bool unload(const utils::id &id) override;
      void end() override;
      void clear() override;
    private:
      game::sounds_container_load* container;
      
      utils::id check_json(const std::string &path_prefix, const std::string &file, const nlohmann::json &data, const size_t &mark, sound::load_data& info, utils::problem_container<info::error> &errors, utils::problem_container<info::warning> &warnings) const override;
    };
  }
}

#endif
