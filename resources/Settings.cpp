#include "Settings.h"

#include <iostream>
#include <fstream>

#include "nlohmann/json.hpp"
#include <iomanip>

#include "Globals.h"

void iterateThroughSettings(const std::string &prefix,
                            const nlohmann::json &json, 
                            std::unordered_map<std::string, int64_t> &intData, 
                            std::unordered_map<std::string, float> &floatData, 
                            std::unordered_map<std::string, std::string> &stringData) {
  for (auto it = json.begin(); it != json.end(); ++it) {
    if (it.value().is_object()) {
      iterateThroughSettings(prefix + "." + it.key(), it.value(), intData, floatData, stringData);
    }
    
    if (it.value().is_number_integer()) {
      intData.insert(std::make_pair(prefix + "." + it.key(), it.value().get<int64_t>()));
    }
    
    if (it.value().is_boolean()) {
      intData.insert(std::make_pair(prefix + "." + it.key(), it.value().get<bool>()));
    }
    
    if (it.value().is_number_float()) {
      floatData.insert(std::make_pair(prefix + "." + it.key(), it.value().get<float>()));
    }
    
    if (it.value().is_string()) {
      stringData.insert(std::make_pair(prefix + "." + it.key(), it.value().get<std::string>()));
    }
  }
}

void parse_setting_name(const std::string &name, std::vector<std::string> &data) {
  if (name.empty()) return;
  
  size_t start = 0;
  size_t pos = 0;
  
  while (pos < name.size()) {
    pos = name.find('.', pos);
    const std::string &str = name.substr(start, pos == std::string::npos ? name.size()-start : pos-start);
    data.push_back(str);
    
    if (pos >= name.size()) break;
    
    ++pos;
    start = pos;
  }
  
  if (data.empty()) throw std::runtime_error("data empty()");
}

void Settings::load(const std::string &name, const std::string &path) {
  std::ifstream i(path);
  
  nlohmann::json j;
  
  i >> j;
  
//   std::cout << std::setw(2) << j << "\n";
  
  iterateThroughSettings(name, j, intData, floatData, stringData);
  
  fileName[name] = path;
}

void Settings::save() {
  struct Temp {
    std::string fileName;
    nlohmann::json j;
  };
  
  std::unordered_map<std::string, Temp> files;
  
  for (const auto &pair : fileName) {
    files[pair.first] = Temp{pair.second, nlohmann::json{}};
  }
  
  std::vector<std::string> strings;
  for (const auto &pair : intData) {
    parse_setting_name(pair.first, strings);
    
    nlohmann::json* tmp = &files[strings[0]].j;
    for (size_t i = 1; i < strings.size()-1; ++i) {
      auto itr = tmp->find(strings[i]);
      if (itr == tmp->end()) {
        itr = tmp->emplace(strings[i], nlohmann::json{}).first;
      }
      tmp = &itr.value();
    }
    
    tmp->emplace(strings[strings.size()-1], pair.second);
    strings.clear();
  }
  
  for (const auto &pair : floatData) {
    parse_setting_name(pair.first, strings);
    
    nlohmann::json* tmp = &files[strings[0]].j;
    for (size_t i = 1; i < strings.size()-1; ++i) {
      auto itr = tmp->find(strings[i]);
      if (itr == tmp->end()) {
        itr = tmp->emplace(strings[i], nlohmann::json{}).first;
      }
      tmp = &itr.value();
    }
    
    tmp->emplace(strings[strings.size()-1], pair.second);
    strings.clear();
  }
  
  for (const auto &pair : stringData) {
    parse_setting_name(pair.first, strings);
    
    nlohmann::json* tmp = &files[strings[0]].j;
    for (size_t i = 1; i < strings.size()-1; ++i) {
      auto itr = tmp->find(strings[i]);
      if (itr == tmp->end()) {
        itr = tmp->emplace(strings[i], nlohmann::json{}).first;
      }
      tmp = &itr.value();
    }
    
    tmp->emplace(strings[strings.size()-1], pair.second);
    strings.clear();
  }
  
  // здесь дампим json на диск
  for (const auto &pair : files) {
    std::ofstream file(pair.second.fileName);
    file << std::setw(2) << pair.second.j << "\n";
  }
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
  return stringData[name];
}

template<>
const int64_t & Settings::get(const std::string &name) const {
  return intData.at(name);
}

template<>
const float & Settings::get(const std::string &name) const {
  return floatData.at(name);
}

template<>
const std::string & Settings::get(const std::string &name) const {
  return stringData.at(name);
}

template<>
bool Settings::has(const std::string &name, int64_t* data) const {
  auto itr = intData.find(name);
  if (itr == intData.end()) return false;
  
  if (data != nullptr) *data = itr->second;
  return true;
}

template<>
bool Settings::has(const std::string &name, float* data) const {
  auto itr = floatData.find(name);
  if (itr == floatData.end()) return false;
  
  if (data != nullptr) *data = itr->second;
  return true;
}

template<>
bool Settings::has(const std::string &name, std::string* data) const {
  auto itr = stringData.find(name);
  if (itr == stringData.end()) return false;
  
  if (data != nullptr) *data = itr->second;
  return true;
}

void Settings::setFileName(const std::string &name, const std::string &path) {
  fileName[name] = path;
}
