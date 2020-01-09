#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>
#include <unordered_map>

#include "MemoryPool.h"

// по большому счету настройки это структура глобальная
// к которой все обращаются когда необходимо получить 
// какие то строго определенные переменные
// как сделать доступ к ним константным для всей игры, 
// но с возможностью изменять в определенном месте?
// константный укзатель?

// есть ли необходимость делать собираемые настройки, как это я пытался сделать раньше?
// вряд ли, настройки от игры к игре будут отличаться не так уж сильно
// единственно что будет действительно сильно отличаться это привязка клавиш

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
      
      void load(const std::string &path);
      void dump(const std::string &path);
    };
    
    typedef const settings const_settings;
  }
}

class Settings {
public:
  void load(const std::string &name, const std::string &path);
  void save();

  template<typename T>
  T & get(const std::string &name);
  template<typename T>
  const T & get(const std::string &name) const;
  template<typename T>
  bool has(const std::string &name, T* data = nullptr) const;
  
  void setFileName(const std::string &name, const std::string &path);
private:
  std::unordered_map<std::string, int64_t> intData;
  std::unordered_map<std::string, float> floatData;
  std::unordered_map<std::string, std::string> stringData;
  
  std::unordered_map<std::string, std::string> fileName;
  
  // нужно лучше продумать как загружать скрытые настройки
  // и как их сохранять
  // может быть просто несколько объектов с настройками?
};

#endif
