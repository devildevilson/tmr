#include "abilities_loader.h"

#include "Globals.h"
#include "state_loader.h"
#include "effects_loader.h"

#define EFFECTS_CONTAINER
#define STATES_CONTAINER
#include "game_resources.h"

#include <iostream>

namespace devils_engine {
  namespace resources {
    abilities_loader::abilities_loader(const create_info &info) : parser("abilities"), container(info.container), states(info.states), effects(info.effects) {}
    abilities_loader::~abilities_loader() {
      clear();
    }

    bool abilities_loader::validate(utils::problem_container<info::error> &errors, utils::problem_container<info::warning> &warnings) const {
      for (size_t i = 0; i < loading_data.size(); ++i) {
        {
          auto res = states->resource();
          if (res == nullptr) errors.add(mark(), ERROR_COULD_NOT_FIND_STATE, "Could not find state "+loading_data.at(i)->cast_state.name());
        }
        
        if (loading_data.at(i)->cost_effect.valid()) {
          auto res = effects->resource(loading_data.at(i)->cost_effect);
          if (res == nullptr) errors.add(mark(), ERROR_COULD_NOT_FIND_EFFECT, "Could not find effect "+loading_data.at(i)->cost_effect.name());
        }
      }
    }
    
    bool abilities_loader::load(const utils::id &id) {
      //auto abil = get_ability(id);
      auto abil = Global::get<game::abilities_container>()->get(id);
      if (abil != nullptr) return true;
      
      auto ptr = loading_data.get(id);
      if (ptr == nullptr) return false;
      
      {
        const bool res = states->load(ptr->cast_state);
        if (!res) throw std::runtime_error("Could not load state "+ptr->cast_state.name());
      }
      
      if (ptr->cost_effect.valid()) {
        const bool res = effects->load(ptr->cost_effect);
        if (!res) throw std::runtime_error("Could not load effect "+ptr->cost_effect.name());
      }
      
      // указатель на контейнер?
      auto res = container->create(ptr->id);
      res->id = ptr->id;
      res->name = ptr->name;
      res->description = ptr->description;
      res->cost = ptr->cost_effect.valid() ? Global::get<game::effects_container>()->get(ptr->cost_effect) : nullptr;
      res->cast = Global::get<game::states_container>()->get(ptr->cast_state);
      if (res->cast == nullptr) throw std::runtime_error("Could not find state "+ptr->cast_state.name());
      
      return true;
    }
    
    bool abilities_loader::unload(const utils::id &id) {
      return container->destroy(id);
    }
    
    void abilities_loader::end() {
      
    }
    
    void abilities_loader::clear() {
      
    }
    
    utils::id abilities_loader::check_json(const std::string &file_path, const std::string &file, const nlohmann::json &data, const size_t &mark, ability::load_data& info, utils::problem_container<info::error> &errors, utils::problem_container<info::warning> &warnings) const {
      (void)file_path; 
      (void)file;
      bool has_id = false, has_state = false; // по идее необходимость существует только в id и в состоянии
      
      const size_t error_count = errors.size();
      
      info.gpu_size = 0;
      info.mem_size = sizeof(game::ability_t);
      info.parser_mark = mark;
      info.mod = nullptr; // ???
      
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
        
        if (itr.value().is_string() && itr.key() == "state") {
          has_state = true;
          info.cast_state = utils::id::get(itr.value().get<std::string>());
          continue;
        }
        
        if (itr.value().is_string() && itr.key() == "cost") {
          info.cost_effect = utils::id::get(itr.value().get<std::string>());
          continue;
        }
      }
      
      if (!has_id) {
        errors.add(mark, abilities_loader::ERROR_ABILITY_MUST_HAVE_AN_ID, "Ability must have an id");
        return utils::id();
      }
      
      if (!has_state) {
        errors.add(mark, abilities_loader::ERROR_ABILITY_CAST_STATE_MUST_BE_SPECIFIED, "Ability "+info.id.name()+" must have a cast state");
        return utils::id();
      }
      
      (void)warnings;
      
      return error_count == errors.size() ? info.id : utils::id();
    }
  }
}
