#include "settings.h"

#include <fstream>
#include <nlohmann/json.hpp>
#include <iomanip>

namespace devils_engine {
  namespace utils {
    settings::settings() : 
      graphics{
        true,
        1,
        1,
        60.0f
      }, sound{
        0.3f,
        1.0f,
        1.0f,
        1.0f
      } {}
    
    bool settings::load(const std::string &path) {
      std::fstream file(path);
      if (!file) return false;
      
      nlohmann::json j;
      file >> j;
      
      for (auto itr = j.begin(); itr != j.end(); ++itr) {
        if (itr.value().is_object() && itr.key() == "graphics") {
          for (auto g = itr.value().begin(); g != itr.value().end(); ++g) {
            if (g.value().is_boolean() && g.key() == "fullscreen") {
              graphics.fullscreen = g.value().get<bool>();
              continue;
            }
            
            if (g.value().is_number_unsigned() && g.key() == "width") {
              graphics.width = g.value().get<uint32_t>();
              continue;
            }
            
            if (g.value().is_number_unsigned() && g.key() == "height") {
              graphics.height = g.value().get<uint32_t>();
              continue;
            }
            
            if (g.value().is_number() && g.key() == "fov") {
              graphics.fov = g.value().get<float>();
              continue;
            }
          }
          continue;
        }
        
        if (itr.value().is_object() && itr.key() == "sound") {
          for (auto s = itr.value().begin(); s != itr.value().end(); ++s) {
            if (s.value().is_number() && s.key() == "master") {
              sound.master = s.value().get<float>();
              continue;
            }
            
            if (s.value().is_number() && s.key() == "music") {
              sound.music = s.value().get<float>();
              continue;
            }
            
            if (s.value().is_number() && s.key() == "menu") {
              sound.menu = s.value().get<float>();
              continue;
            }
            
            if (s.value().is_number() && s.key() == "sounds") {
              sound.sounds = s.value().get<float>();
              continue;
            }
          }
          continue;
        }
      }
      
      return true;
    }
    
    bool settings::dump(const std::string &path) {
      nlohmann::json j;
      j["graphics"]["fullscreen"] = graphics.fullscreen;
      j["graphics"]["width"] = graphics.width;
      j["graphics"]["height"] = graphics.height;
      j["graphics"]["fov"] = graphics.fov;
      
      j["sound"]["master"] = sound.master;
      j["sound"]["music"] = sound.music;
      j["sound"]["menu"] = sound.menu;
      j["sound"]["sounds"] = sound.sounds;
      
      std::fstream file(path);
      file << std::setw(2) << j;
      return true;
    }
  }
}
