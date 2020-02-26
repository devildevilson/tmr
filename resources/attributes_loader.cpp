#include "attributes_loader.h"

#include "Globals.h"

#include <fstream>
#include <iostream>

namespace devils_engine {
  namespace resources {
    //attributes_loader::load_data::load_data(const Resource::CreateInfo &info) : Resource(info) {}
    
    attributes_loader::attributes_loader(const create_info &info) : parser("attributes"), float_container(info.float_container), int_container(info.int_container), float_compute_funcs(info.float_compute_funcs), int_compute_funcs(info.int_compute_funcs) {}
    attributes_loader::~attributes_loader() {
      clear();
    }

    bool attributes_loader::validate(utils::problem_container<info::error> &errors, utils::problem_container<info::warning> &warnings) const {
      const size_t errors_count = errors.size();
      
      for (size_t i = 0; i < loading_data.size(); ++i) {
        if (loading_data.at(i)->func_name.empty()) continue;
        
        {
          auto itr = float_compute_funcs.find(loading_data.at(i)->func_name);
          if (itr == float_compute_funcs.end()) errors.add(mark(), ERROR_COULD_NOT_FIND_ATTRIBUTE_TYPE_FUNCTION, "Could not find attribute type "+loading_data.at(i)->id.name()+" function");
        }
        
        {
          auto itr = int_compute_funcs.find(loading_data.at(i)->func_name);
          if (itr == int_compute_funcs.end()) errors.add(mark(), ERROR_COULD_NOT_FIND_ATTRIBUTE_TYPE_FUNCTION, "Could not find attribute type "+loading_data.at(i)->id.name()+" function");
        }
      }
      
      (void)warnings;
      return errors_count == errors.size();
    }

    bool attributes_loader::load(const utils::id &id) {
      {
        auto ftype = Global::get<game::float_attribute_types_container>()->get(id);
        if (ftype != nullptr) return true;
      }
      
      {
        auto itype = Global::get<game::int_attribute_types_container>()->get(id);
        if (itype != nullptr) return true;
      }
      
      auto ptr = loading_data.get(id);
      if (ptr == nullptr) return false;
      
      if (ptr->floattype) {
        auto itr = float_compute_funcs.find(ptr->func_name);
        if (itr == float_compute_funcs.end()) throw std::runtime_error("Could not find function for attribute "+ptr->id.name());
        
        auto type = float_container->create(ptr->id);
        type->id = ptr->id;
        type->name = ptr->name;
        type->description = ptr->description;
        type->compute = itr->second;
      } else {
        auto itr = int_compute_funcs.find(ptr->func_name);
        if (itr == int_compute_funcs.end()) throw std::runtime_error("Could not find function for attribute "+ptr->id.name());
        
        auto type = int_container->create(ptr->id);
        type->id = ptr->id;
        type->name = ptr->name;
        type->description = ptr->description;
        type->compute = itr->second;
      }
      
      return true;
    }
    
    bool attributes_loader::unload(const utils::id &id) {
      bool ret = false;
      ret = ret || float_container->destroy(id);
      ret = ret || int_container->destroy(id);
      return ret;
    }
    
    void attributes_loader::end() {
      
    }
    
    void attributes_loader::clear() {}
    
//     const struct core::attribute_t<core::float_type>::type* attributes_loader::get_float_type(const utils::id &id) const {
//       return float_container.get(id);
//     }
//     
//     const struct core::attribute_t<core::int_type>::type* attributes_loader::get_int_type(const utils::id &id) const {
//       return int_container.get(id);
//     }
    
    utils::id attributes_loader::check_json(const std::string &path_prefix, const std::string &file, const nlohmann::json &data, const size_t &mark, attribute_type::load_data& info, utils::problem_container<info::error> &errors, utils::problem_container<info::warning> &warnings) const {
      bool hasId = false, hasType = false;
      
      info.gpu_size = 0;
      info.mem_size = sizeof(struct core::attribute_t<core::float_type>::type);
      info.mod = nullptr;
      info.parser_mark = mark;
      
      const size_t errorsCount = errors.size();
      
      for (auto itr = data.begin(); itr != data.end(); ++itr) {
        if (itr.value().is_string() && itr.key() == "id") {
          hasId = true;
          info.id = utils::id::get(itr.value().get<std::string>());
        }
        
        if (itr.value().is_string() && itr.key() == "type") {
          hasType = true;
          const bool rightFloat = itr.value().get<std::string>() == "fractional";
          const bool rightInt = itr.value().get<std::string>() == "integer";
          if (!rightFloat && !rightInt) {
            errors.add(mark, attributes_loader::ERROR_BAD_TYPE_VALUE, "Variable with key \"type\" must be equal \"fractional\" or \"integer\"");
            continue;
          }
          
          info.floattype = rightFloat;
        }
        
        if (itr.value().is_string() && itr.key() == "name") {
          info.name = itr.value().get<std::string>();
        }
        
        if (itr.value().is_string() && itr.key() == "description") {
          info.description = itr.value().get<std::string>();
        }
        
        if (itr.value().is_string() && itr.key() == "compute_function") {
          info.func_name = itr.value().get<std::string>();
        }
      }
      
      if (!hasId) {
        errors.add(mark, attributes_loader::ERROR_ATTRIBUTE_ID_NOT_FOUND, "Attribute description must have an id");
        return utils::id();
      }
      
      if (!hasType) {
        errors.add(mark, attributes_loader::ERROR_ATTRIBUTE_TYPE_IS_NOT_SPECIFIED, "Attribute description must have a type");
        return utils::id();
      }
      
      (void)path_prefix;
      (void)file;
      (void)warnings;
      
      return errorsCount == errors.size() ? info.id : utils::id();
    }
  }
}
