#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>
#include <unordered_map>

#include "MemoryPool.h"

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
