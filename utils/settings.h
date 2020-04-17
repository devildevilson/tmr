#ifndef SETTINGS_H
#define SETTINGS_H

#include <cstdint>
#include <string>
#include "id.h"

namespace devils_engine {
  namespace utils {
    struct settings {
      struct graphics {
        bool fullscreen;
        uint32_t width;
        uint32_t height;
        uint32_t video_mode;
        float fov;
      };
      
      struct mouse {
        bool inverted;
        float sens;
        float sens_x;
        float sens_y;
        // smooth? 
      };
      
      struct sound {
        float master;
        float music;
        float menu;
        float sounds;
      };
      
      struct key_event {
        utils::id event;
        uint32_t key;
        std::string description;
      };
      
      struct controls {
        struct mouse mouse;
        std::vector<key_event> key_mapping;
      };
      
      struct graphics graphics;
      struct sound sound;
      struct controls controls;
      
      settings();
      bool load(const std::string &path);
      bool dump(const std::string &path);
    };
    
    typedef const settings const_settings;
  }
}

#endif
