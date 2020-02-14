#ifndef ATTRIBUTES_LOADER_H
#define ATTRIBUTES_LOADER_H

#include "id.h"
#include "resource.h"
#include "resource_parser.h"
#include "resource_container.h"
#define ATTRIBUTE_TYPES_CONTAINER
#include "game_resources.h"

namespace devils_engine {
  namespace attribute_type {
    struct load_data : public core::resource {
      bool floattype;
      std::string name;
      std::string description;
      std::string func_name;
    };
  }
  
  namespace resources {
    class attributes_loader : public loader, public parser<attribute_type::load_data, 50>, public validator {
    public:
      enum ErrorTypes {
        ERROR_FILE_NOT_FOUND = 0,
        ERROR_BAD_TYPE_VALUE,
        ERROR_ATTRIBUTE_ID_NOT_FOUND,
        ERROR_ATTRIBUTE_TYPE_IS_NOT_SPECIFIED
      };
      
      struct create_info {
        game::float_attribute_types_container_load* float_container;
        game::int_attribute_types_container_load* int_container;
        std::unordered_map<std::string, core::attribute_t<core::float_type>::type::func_type> float_compute_funcs;
        std::unordered_map<std::string, core::attribute_t<core::int_type>::type::func_type> int_compute_funcs;
      };
      attributes_loader(const create_info &info);
      ~attributes_loader();
      
      bool validate(utils::problem_container<info::error> &errors, utils::problem_container<info::warning> &warnings) const override;

      bool load(const utils::id &id) override;
      bool unload(const utils::id &id) override;
      void end() override;
      
      void clear() override;
      
//       const struct core::attribute_t<core::float_type>::type* get_float_type(const utils::id &id) const;
//       const struct core::attribute_t<core::int_type>::type* get_int_type(const utils::id &id) const;
    private:
      game::float_attribute_types_container_load* float_container;
      game::int_attribute_types_container_load* int_container;
      
      std::unordered_map<std::string, core::attribute_t<core::float_type>::type::func_type> float_compute_funcs;
      std::unordered_map<std::string, core::attribute_t<core::int_type>::type::func_type> int_compute_funcs;
      
      utils::id check_json(const std::string &path_prefix, const std::string &file, const nlohmann::json &data, const size_t &mark, attribute_type::load_data& info, utils::problem_container<info::error> &errors, utils::problem_container<info::warning> &warnings) const override;
    };
  }
}

#endif
