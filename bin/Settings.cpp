#include "Settings.h"

#include <iostream>
#include <fstream>
#include "nlohmann/json.hpp"

// enum class ResType {
//   CLIENT,
//   SERVER
// };
// 
// struct ResourceInfo {
//   ResType type;
//   std::string name;
//   std::string description;
//   size_t texturesSize;
//   size_t soundsSize;
//   size_t scriptsSize;
// };
// 
// struct TextureData {
//   uint32_t width;
//   uint32_t height;
//   std::string path;
//   std::string name;
//   size_t count;
//   size_t rows;
//   size_t columns;
// };
// 
// class ResourceManager;
// 
// class Loader {
// public:
//   virtual bool load(const nlohmann::json &data, const ResourceManager* manager) = 0;
// };
// 
// class ResourceManager {
// public:
//   void addLoader(Loader* loader);
//   void loadMain(const std::string &path);
// private:
//   std::vector<Loader*> loaders;
//   std::unordered_map<std::string, ResourceInfo> loadedResources;
// };
// 
// class TexturesLoader : public Loader {
// public:
//   void* getPixels(size_t &size);
//   bool load(const nlohmann::json &data, const ResourceManager* manager) override;
//   void clear();
// private:
//   std::vector<TextureData> datas;
//   uint8_t* data;
//   size_t size;
//   
// };
// 
// class TextureLoader {
// public:
//   virtual void load(const std::string &prefix, const std::string &path, uint32_t &width, uint32_t &height, uint32_t &channels) = 0;
//   virtual void clear() = 0;
// };
// 
// class TextureResource {
// public:
//   
// private:
//   TextureLoader* tex;
//   uint32_t width;
//   uint32_t height;
//   size_t count;
//   size_t rows;
//   size_t columns;
// };
// 
// class StbiLoader : public TextureLoader {
// public:
//   uint8_t* getPixels(size_t &size);
//   uint32_t getWidth();
//   uint32_t getHeight();
//   uint32_t getChannels();
// private:
//   uint8_t* data;
//   size_t size;
//   uint32_t width;
//   uint32_t height;
//   uint32_t channels;
//   
// };

void iterateThroughSettings(const std::string &prefix,
                            const nlohmann::json &json, 
                            std::unordered_map<std::string, int64_t> &intData, 
                            std::unordered_map<std::string, float> &floatData, 
                            std::unordered_map<std::string, std::string> &stringData) {
  for (auto it = json.begin(); it != json.end(); ++it) {
    if (it.value().is_number_integer()) {
      /*const auto i = */intData.insert(std::make_pair(prefix + "." + it.key(), it.value().get<int64_t>()));
      //std::cout << i.first->first << ": " << i.first->second << "\n";
    }
    
    if (it.value().is_boolean()) {
      /*const auto i = */intData.insert(std::make_pair(prefix + "." + it.key(), it.value().get<bool>()));
      //std::cout << i.first->first << ": " << i.first->second << "\n";
    }
    
    if (it.value().is_number_float()) {
      /*const auto i = */floatData.insert(std::make_pair(prefix + "." + it.key(), it.value().get<float>()));
      //std::cout << i.first->first << ": " << i.first->second << "\n";
    }
    
    if (it.value().is_string()) {
      /*const auto i = */stringData.insert(std::make_pair(prefix + "." + it.key(), it.value().get<std::string>()));
      //std::cout << i.first->first << ": " << i.first->second << "\n";
    }
  }
}

void Settings::load(const std::string &path) {
  std::ifstream i(path);
  
  nlohmann::json j;
  
  i >> j;
  
  //std::cout << std::setw(2) << j << "\n";
  
  // тут нужно будет потом добавить обход по секциям
  // как быть с настройками модов? в принципе ничего сложного по идее
  // обходим, название секции это сокращенное название мода
  // нужно только сделать отдельную функцию для этого
  for (auto it = j.begin(); it != j.end(); ++it) {
    std::string prefix = "game";
    
    if (it.value().is_object()) {
      iterateThroughSettings(prefix + "." + it.key(), it.value(), intData, floatData, stringData);
    }
  }
  
//   for (nlohmann::json::iterator it = j.begin(); it != j.end(); ++it) {
//     if (it.value().is_number_integer()) {
//       /*const auto i = */intData.insert(std::make_pair(it.key(), it.value().get<int64_t>()));
//       //std::cout << i.first->first << ": " << i.first->second << "\n";
//     }
//     
//     if (it.value().is_boolean()) {
//       /*const auto i = */intData.insert(std::make_pair(it.key(), it.value().get<bool>()));
//       //std::cout << i.first->first << ": " << i.first->second << "\n";
//     }
//     
//     if (it.value().is_number_float()) {
//       /*const auto i = */floatData.insert(std::make_pair(it.key(), it.value().get<float>()));
//       //std::cout << i.first->first << ": " << i.first->second << "\n";
//     }
//     
//     if (it.value().is_string()) {
//       /*const auto i = */stringData.insert(std::make_pair(it.key(), it.value().get<std::string>()));
//       //std::cout << i.first->first << ": " << i.first->second << "\n";
//     }
//   }
}

void Settings::save() {
  
}

void Settings::save(const std::string &path) {
  
}

template<>
int64_t & Settings::get(const std::string &name) {
  return intData[name];
}

template<>
float & Settings::get(const std::string &name) {
  return floatData[name];
}

template<>
std::string & Settings::get(const std::string &name) {
//   auto itr = stringData.find(name);
//   if (itr == stringData.end()) {
//     stringData[name] = "";
//   }
  
  return stringData[name];
}

template<>
bool Settings::has(const std::string &name, int64_t* data) {
  auto itr = intData.find(name);
  if (itr == intData.end()) return false;
  
  if (data != nullptr) *data = itr->second;
  return true;
}

template<>
bool Settings::has(const std::string &name, float* data) {
  auto itr = floatData.find(name);
  if (itr == floatData.end()) return false;
  
  if (data != nullptr) *data = itr->second;
  return true;
}

template<>
bool Settings::has(const std::string &name, std::string* data) {
  auto itr = stringData.find(name);
  if (itr == stringData.end()) return false;
  
  if (data != nullptr) *data = itr->second;
  return true;
}

// int64_t & Settings::getInt(const std::string &name) {
//   auto itr = intData.find(name);
//   if (itr == intData.end()) {
//     intData[name] = 0;
//   }
//   
//   return intData[name];
// }
// 
// float & Settings::getFloat(const std::string &name) {
//   auto itr = floatData.find(name);
//   if (itr == floatData.end()) {
//     floatData[name] = 0.0f;
//   }
//   
//   return floatData[name];
// }
// 
// std::string & Settings::getString(const std::string &name) {
//   auto itr = stringData.find(name);
//   if (itr == stringData.end()) {
//     stringData[name] = "";
//   }
//   
//   return stringData[name];
// }
