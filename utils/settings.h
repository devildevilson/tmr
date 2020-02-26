#ifndef SETTINGS_H
#define SETTINGS_H

#include <cstdint>
#include <string>

namespace devils_engine {
  namespace utils {
    struct settings {
      struct graphics {
        bool fullscreen;
        uint32_t width;
        uint32_t height;
        float fov;
      };
      
      struct key_mapping {
        std::pair<std::string, uint32_t> action_key;
      };
      
      struct sound {
        float master;
        float music;
        float menu;
        float sounds;
      };
      
      struct graphics graphics;
      struct sound sound;
      
      settings();
      bool load(const std::string &path);
      bool dump(const std::string &path);
    };
    
    typedef const settings const_settings;
  }
}

#endif
