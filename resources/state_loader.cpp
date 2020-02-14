#include "state_loader.h"

#include "Globals.h"
#include <fstream>
//#include "ImageLoader.h"
#include "image_loader.h"
#include "image_data.h"

namespace devils_engine {
  namespace resources {
    state_loader::state_loader(const create_info &info) : parser("states"), container(info.container), textures(info.textures), image_loader(info.image_loader), functions(info.functions) {}
    state_loader::~state_loader() {}

    bool state_loader::validate(utils::problem_container<info::error> &errors, utils::problem_container<info::warning> &warnings) const {
      (void)warnings;
      for (size_t i = 0; i < loading_data.size(); ++i) {
        for (const auto &image_data : loading_data.at(i)->image_datas) {
          auto res = image_loader->resource(image_data.image);
          if (res == nullptr) {
            errors.add(mark(), ERROR_STATE_IMAGE_IS_NOT_FOUND, "Could not find image "+image_data.image.name());
            return false;
          }
        }
        
        if (!loading_data.at(i)->next.valid()) continue;
        auto ptr = loading_data.get(loading_data.at(i)->next);
        if (ptr == nullptr) {
          errors.add(mark(), ERROR_NEXT_STATE_IS_NOT_FOUND, "Could not find state "+loading_data.at(i)->next.name());
          return false;
        }
      }
      return true;
    }
    
    bool state_loader::load(const utils::id &id) {
      {
        auto state = Global::get<game::states_container>()->get(id);
        if (state != nullptr) return true;
      }
      
      auto ptr = loading_data.get(id);
      if (ptr == nullptr) return false;
      
      auto state_ptr = container->create(ptr->id);
      auto func_itr = functions.find(ptr->func);
      if (func_itr != functions.end()) {
        state_ptr->action = func_itr->second;
      } else {
        state_ptr->action = nullptr;
      }
      
      state_ptr->time = ptr->time;
      state_ptr->next = nullptr;
      state_ptr->id = ptr->id;
      state_ptr->frame.texture_offset = textures->size();
      state_ptr->frame.images_count = ptr->image_datas.size();
      for (const auto &image_info : ptr->image_datas) {
        const bool res = image_loader->load(image_info.image);
        if (!res) throw std::runtime_error("Could not load image "+image_info.image.name());
        
        const auto id = Global::get<game::image_data_container>()->get(image_info.image);
        if (id == nullptr) throw std::runtime_error("Could not find image "+image_info.image.name());
        if (id->count <= image_info.index) throw std::runtime_error("Could not find image "+image_info.image.name());
        
        const Texture t{
          id->images[image_info.index],
          0,
          image_info.flipU ? -1.0f : 0.0f,
          image_info.flipV ? -1.0f : 0.0f
        };
        textures->push_back(t);
      }
      
      return true;
    }
    
    bool state_loader::unload(const utils::id &id) {
      // по всей видимости выгрузить будет сложно
      // для этого нужно удалить из массива текстурки
      throw std::runtime_error("Not implemented yet");
      
      return false;
    }
    
    void state_loader::end() {
      // нужен id
      
      for (size_t i = 0; i < loading_data.size(); ++i) {
        if (!loading_data.at(i)->next.valid()) continue;
        auto state = container->get(loading_data.at(i)->id);
        state->next = container->get(loading_data.at(i)->next);
      }
    }
    
    void state_loader::clear() {
      
    }
    
//     const core::state_t* state_loader::get(const utils::id& id) const {
//       return container.get(id);
//     }

    utils::id state_loader::check_json(const std::string &path_prefix, const std::string &file, const nlohmann::json &data, const size_t &mark, state::load_data& info, utils::problem_container<info::error> &errors, utils::problem_container<info::warning> &warnings) const {
      bool has_id = false, has_time = false, has_next = false;
      
      const size_t errors_size = errors.size();
      
      for (auto itr = data.begin(); itr != data.end(); ++itr) {
        if (itr.value().is_string() && itr.key() == "id") {
          has_id = true;
          info.id = utils::id::get(itr.value().get<std::string>());
          continue;
        }
        
        if (itr.value().is_number_integer() && itr.key() == "time") {
          has_time = true;
          const int64_t num = itr.value().get<int64_t>();
          if (num < 0) info.time = SIZE_MAX;
          else info.time = num;
          continue;
        }
        
        if (itr.value().is_array() && itr.key() == "frames") {
          state::load_data::image_data img;
          for (size_t i = 0; i < itr.value().size(); ++i) {
            if (!itr.value()[i].is_string()) {
              errors.add(mark, state_loader::ERROR_WRONG_FRAME_DATA, "Wrong frame data");
              break;
            }
            
            // парсим строку
            image_loader::parse_image_data_string(itr.value()[i].get<std::string>(), img.image, img.index, img.flipU, img.flipV);
            if (!img.image.valid()) throw std::runtime_error("state_loader parsing image string error");
            
            info.image_datas.push_back(img);
          }
          
          continue;
        }
        
        if (itr.key() == "next_state") {
          if (itr.value().is_null()) {
            has_next = true;
            info.next = utils::id();
          } else if (itr.value().is_string()) {
            has_next = true;
            info.next = utils::id::get(itr.value().get<std::string>());
          }
          
          continue;
        }
        
        if (itr.value().is_string() && itr.key() == "function") {
          info.func = itr.value().get<std::string>();
          continue;
        }
      }
      
      if (!has_id) {
        errors.add(mark, state_loader::ERROR_ID_IS_NOT_SPECIFIED, "State must have an id");
        return utils::id();
      }
      
      if (!has_time) {
        errors.add(mark, state_loader::ERROR_TIME_IS_NOT_FOUND, "State must have an explicit time");
        return utils::id();
      }
      
      if (!has_next) {
        errors.add(mark, state_loader::ERROR_NEXT_IS_NOT_FOUND, "State must have an explicit pointer to the next state");
        return utils::id();
      }
      
      return errors_size == errors.size() ? info.id : utils::id();
    }
  }
}
