#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>
#include <unordered_map>

// class SettingsEditor {
// public:
//   
// };

struct SettingVar {
  // хранить здесь json?
  
  bool isInt() const;
};

class Settings {
public:
  void load(const std::string &path);
  void save();
  void save(const std::string &path);
  
//   int64_t & getInt(const std::string &name);
//   float & getFloat(const std::string &name);
//   std::string & getString(const std::string &name);
  template<typename T>
  T & get(const std::string &name);
  template<typename T>
  bool has(const std::string &name, T* data = nullptr);
private:
  std::unordered_map<std::string, int64_t> intData;
  std::unordered_map<std::string, float> floatData;
  std::unordered_map<std::string, std::string> stringData;
  
  // нужно лучше продумать как загружать скрытые настройки
  // и как их сохранять
  // может быть просто несколько объектов с настройками?
};

#endif
