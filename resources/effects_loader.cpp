#include "effects_loader.h"

#include "attributes_loader.h"

namespace devils_engine {
  namespace resources {
    effects_loader::effects_loader(const create_info &info) : parser("effects"), attribs(info.attribs), container(info.container) {}
    bool effects_loader::validate(utils::problem_container<info::error> &errors, utils::problem_container<info::warning> &warnings) const {
      for (size_t i = 0; i < loading_data.size(); ++i) {
        auto ptr = loading_data.at(i);
        for (size_t j = 0; j < game::effect_t::max_bonuses; ++j) {
          if (!ptr->container.bonuses[j].attrib.valid()) continue;
          
          auto id = ptr->container.bonuses[j].attrib;
          auto attrib = attribs->resource(id);
          if (attrib == nullptr) {
            errors.add(reinterpret_cast<size_t>(this), ERROR_COULD_NOT_FIND_ATTRIB, "Could not find attribute type "+id.name());
            return false;
          }
        }
      }
      
      (void)warnings;
      
      return true;
    }
    
    bool effects_loader::load(const utils::id &id) {
      {
        auto eff = container->get(id);
        if (eff != nullptr) return true;
      }
      
      auto ptr = loading_data.get(id);
      if (ptr == nullptr) return false;
      
      auto effect = container->create(ptr->id);
      effect->id = ptr->id;
      effect->name = ptr->name;
      effect->description = ptr->description;
      effect->type = ptr->type;
      effect->container = ptr->container;
      
      for (size_t i = 0; i < game::effect_t::max_bonuses; ++i) {
        if (!effect->container.bonuses[i].attrib.valid()) continue;
        
        const bool res = attribs->load(effect->container.bonuses[i].attrib);
        if (!res) throw std::runtime_error("Could not load attribute type "+effect->container.bonuses[i].attrib.name());
      }
      
      return true;
    }
    
    bool effects_loader::unload(const utils::id &id) {
      return container->destroy(id);
    }
    
    void effects_loader::end() {
      
    }
    
    void effects_loader::clear() {
      
    }
    
    utils::id effects_loader::check_json(const std::string &path_prefix, const std::string &file, const nlohmann::json &data, const size_t &mark, effect::load_data& info, utils::problem_container<info::error> &errors, utils::problem_container<info::warning> &warnings) const {
      bool has_id = false, has_remove = false, has_add = false;
      
      const size_t errors_count = errors.size();
      
      info.container.time = 0;
      info.container.period = 0;
      
      bool raw = true, add = true, remove = true, timer_reset = false, easy_stack = false, unique = false, update = false, complete_remove = true, periodic_add = true, periodic_increase_stack = false, timed_remove = true;
      
      for (auto itr = data.begin(); itr != data.end(); ++itr) {
        if (itr.value().is_string() && itr.key() == "id") {
          has_id = true;
          info.id = utils::id::get(itr.value().get<std::string>());
          continue;
        }
        
        if (itr.value().is_string() && itr.key() == "name") {
          info.name = itr.value().get<std::string>();
          continue;
        }
        
        if (itr.value().is_string() && itr.key() == "description") {
          info.description = itr.value().get<std::string>();
          continue;
        }
        
        if (itr.value().is_number_integer() && itr.key() == "time") {
          const int64_t val = itr.value().get<int64_t>();
          info.container.time = val < 0 ? SIZE_MAX : val;
          continue;
        }
        
        if (itr.value().is_number_integer() && itr.key() == "period") {
          const int64_t val = itr.value().get<int64_t>();
          info.container.period = val <= 0 ? SIZE_MAX : val;
          continue;
        }
        
        if (itr.value().is_object() && itr.key() == "attributes") {
          if (itr.value().size() >= game::effect_t::max_bonuses) {
            errors.add(mark, ERROR_TOO_MANY_ATTRIBUTE_BONUSES, 
                       info.id.valid() ? "Effect "+info.id.name()+" has more ("+std::to_string(itr.value().size())+") than maximum ("+std::to_string(game::effect_t::max_bonuses)+") attribute bonuses" : 
                       "Effect has more ("+std::to_string(itr.value().size())+") than maximum ("+std::to_string(game::effect_t::max_bonuses)+") attribute bonuses");
            break;
          }
          
          size_t size = 0;
          for (auto attrib = itr.value().begin(); attrib != itr.value().end(); ++attrib) {
            info.container.bonuses[size].attrib = utils::id::get(attrib.key());
            info.container.bonuses[size].bonus.add = 0.0f;
            info.container.bonuses[size].bonus.mul = 0.0f;
            
            for (auto attrib_data = attrib.value().begin(); attrib_data != attrib.value().end(); ++attrib_data) {
              if (attrib_data.value().is_number() && attrib_data.key() == "add") {
                info.container.bonuses[size].bonus.add = attrib_data.value().get<core::float_type>();
                continue;
              }
              
              if (attrib_data.value().is_number() && attrib_data.key() == "mul") {
                info.container.bonuses[size].bonus.mul = attrib_data.value().get<core::float_type>();
                continue;
              }
            }
            
            ++size;
          }
          
          //info.container.size = size;
          continue;
        }
        
        if (itr.value().is_boolean() && itr.key() == "is_final") {
          raw = !itr.value().get<bool>();
        }
        
        if (itr.value().is_boolean() && itr.key() == "add") {
          add = !itr.value().get<bool>();
        }
        
        if (itr.value().is_boolean() && itr.key() == "remove") {
          remove = itr.value().get<bool>();
        }
        
        if (itr.value().is_boolean() && itr.key() == "timer_reset") {
          timer_reset = itr.value().get<bool>();
        }
        
        if (itr.value().is_boolean() && itr.key() == "stackable") {
          easy_stack = itr.value().get<bool>();
        }
        
        if (itr.value().is_boolean() && itr.key() == "unique") {
          unique = itr.value().get<bool>();
        }
        
        if (itr.value().is_boolean() && itr.key() == "update") {
          update = itr.value().get<bool>();
        }
        
        if (itr.value().is_boolean() && itr.key() == "complete_remove") {
          complete_remove = itr.value().get<bool>();
        }
        
        if (itr.value().is_boolean() && itr.key() == "periodic_add") {
          has_add = true;
          periodic_add = itr.value().get<bool>();
        }
        
        if (itr.value().is_boolean() && itr.key() == "periodic_increase_stack") {
          periodic_increase_stack = itr.value().get<bool>();
        }
        
        if (itr.value().is_boolean() && itr.key() == "timed_remove") {
          has_remove = true;
          timed_remove = itr.value().get<bool>();
        }
      }
      
      struct game::effect_t::type t(raw, add, remove, timer_reset, easy_stack, unique, update, complete_remove, has_add ? periodic_add : add, periodic_increase_stack, has_remove ? timed_remove : remove);
      info.type = t;
      
      if (!has_id) {
        errors.add(mark, ERROR_EFFECT_MUST_HAVE_AN_ID, "Effect must have an id");
        return utils::id();
      }
      
      (void)path_prefix;
      (void)file;
      (void)warnings;
      
      return errors_count == errors.size() ? info.id : utils::id();
    }
  }
}
