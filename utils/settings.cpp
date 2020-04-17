#include "settings.h"

#include <fstream>
#include <nlohmann/json.hpp>
#include <iomanip>
#include <iostream>
#include "GLFW/glfw3.h"

namespace devils_engine {
  namespace utils {
    settings::settings() : 
      graphics{
        true,
        1,
        1,
        0,
        60.0f
      }, sound{
        0.3f,
        1.0f,
        1.0f,
        1.0f
      }, controls{
        {
          false,
          5.0f,
          1.0f,
          1.0f
        },
        {}
      } {
        // ни в коем случае не делать это глобальными константами
        static const utils::id move_forward = utils::id::get("move_forward");
        static const utils::id move_backward = utils::id::get("move_backward");
        static const utils::id move_right = utils::id::get("move_right");
        static const utils::id move_left = utils::id::get("move_left");
        static const utils::id jump = utils::id::get("jump");
        static const utils::id escape = utils::id::get("escape");
        static const utils::id interface_focus = utils::id::get("interface_focus");
        
        static const utils::id screenshot = utils::id::get("screenshot");

        static const utils::id menu_next = utils::id::get("menu_next");
        static const utils::id menu_prev = utils::id::get("menu_prev");
        static const utils::id menu_increase = utils::id::get("menu_increase");
        static const utils::id menu_decrease = utils::id::get("menu_decrease");
        static const utils::id menu_choose = utils::id::get("menu_choose");
        
        controls.key_mapping = {
          {move_forward, GLFW_KEY_W, ""},
          {move_backward, GLFW_KEY_S, ""},
          {move_right, GLFW_KEY_D, ""},
          {move_left, GLFW_KEY_A, ""},
          {jump, GLFW_KEY_SPACE, ""},
          {escape, GLFW_KEY_ESCAPE, ""},
          {interface_focus, GLFW_KEY_LEFT_ALT, ""},
          {menu_next, GLFW_KEY_DOWN, ""},
          {menu_prev, GLFW_KEY_UP, ""},
          {menu_increase, GLFW_KEY_RIGHT, ""},
          {menu_decrease, GLFW_KEY_LEFT, ""},
          {menu_choose, GLFW_KEY_ENTER, ""},
          {screenshot, GLFW_KEY_F11, ""}
        };
      }
    
    bool settings::load(const std::string &path) {
      std::fstream file(path);
      if (!file) return false;
      
      nlohmann::json j;
      try {
        file >> j;
      } catch (const std::exception &e) {
        std::cout << e.what() << "\n";
        return false;
      }
      
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
            
            if (g.value().is_number_unsigned() && g.key() == "video_mode") {
              graphics.video_mode = g.value().get<uint32_t>();
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
        
        if (itr.value().is_object() && itr.key() == "controls") {
          for (auto c = itr.value().begin(); c != itr.value().end(); ++c) {
            if (c.value().is_object() && c.key() == "mouse") {
              for (auto m = c.value().begin(); m != c.value().end(); ++m) {
                if (m.value().is_boolean() && m.key() == "inverted") {
                  controls.mouse.inverted = m.value().get<bool>();
                  continue;
                }
                
                if (m.value().is_number() && m.key() == "sens") {
                  controls.mouse.sens = m.value().get<float>();
                  continue;
                }
                
                if (m.value().is_number() && m.key() == "sens_x") {
                  controls.mouse.sens_x = m.value().get<float>();
                  continue;
                }
                
                if (m.value().is_number() && m.key() == "sens_y") {
                  controls.mouse.sens_x = m.value().get<float>();
                  continue;
                }
              }
              continue;
            }
            
            if (c.value().is_object() && c.key() == "keys_map") {
              controls.key_mapping.clear();
              for (auto k = c.value().begin(); k != c.value().end(); ++k) {
                if (k.value().is_object()) {
                  key_event ke;
                  ke.event = utils::id::get(k.key());
                  ke.key = UINT32_MAX;
                  for (auto o = itr.value().begin(); o != itr.value().end(); ++o) {
                    if (o.value().is_number_integer() && o.key() == "key") {
                      const int64_t key_id = o.value().get<int64_t>();
                      ke.key = key_id < 0 ? UINT32_MAX : key_id;
                      continue;
                    }
                    
                    if (o.value().is_string() && o.key() == "description") {
                      ke.description = o.value().get<std::string>();
                      continue;
                    }
                  }
                  
                  controls.key_mapping.push_back(ke);
                }
                
                if (k.value().is_number_integer()) {
                  key_event ke;
                  ke.event = utils::id::get(k.key());
                  const int64_t key_id = k.value().get<int64_t>();
                  ke.key = key_id < 0 ? UINT32_MAX : key_id;
//                   std::cout << ke.event.name() << " " << ke.key << "\n";
                  controls.key_mapping.push_back(ke);
                }
              }
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
      j["graphics"]["video_mode"] = graphics.video_mode;
      j["graphics"]["fov"] = graphics.fov;
      
      j["sound"]["master"] = sound.master;
      j["sound"]["music"] = sound.music;
      j["sound"]["menu"] = sound.menu;
      j["sound"]["sounds"] = sound.sounds;
      
      j["controls"]["mouse"]["inverted"] = controls.mouse.inverted;
      j["controls"]["mouse"]["sens"] = controls.mouse.sens;
      j["controls"]["mouse"]["sens_x"] = controls.mouse.sens_x;
      j["controls"]["mouse"]["sens_y"] = controls.mouse.sens_y;
      
      for (const auto &key : controls.key_mapping) {
//         std::cout << key.event.name() << " " << key.key << "\n";
        if (key.description.empty()) {
          j["controls"]["keys_map"][key.event.name()] = int64_t(key.key == UINT32_MAX ? -1 : key.key);
        } else {
          j["controls"]["keys_map"][key.event.name()]["key"] = int64_t(key.key == UINT32_MAX ? -1 : key.key);
          j["controls"]["keys_map"][key.event.name()]["description"] = key.description;
        }
      }
      
      //std::fstream file(path);
      std::ofstream file(path, std::ofstream::out | std::ofstream::trunc);
      file << std::setw(2) << j;
      return true;
    }
  }
}
