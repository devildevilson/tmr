#ifndef ENTITY_COMPONENT_SYSTEM_H
#define ENTITY_COMPONENT_SYSTEM_H

#include <unordered_map>

struct ComponentType {
  ComponentType() {}
  
  ComponentType(const size_t &type) {
    yacsType = type;
  }
  
  ComponentType(const size_t &type, const std::string &name) {
    yacsType = type;
    this->name = name;
    stringToType[name] = type;
  }
      
  size_t get() const {
    return yacsType;
  }
  
  void set(const size_t &type) {
    yacsType = type;
    stringToType[name] = type;
  }
  
  bool has(const std::string &name) {
    if (stringToType.find(name) == stringToType.end()) return false;
    
    return true;
  }
  
  static ComponentType get(const std::string &name) {
    auto itr = stringToType.find(name);
    if (itr == stringToType.end()) return ComponentType(SIZE_MAX);
    
    return ComponentType(itr->second);
  }
  
  size_t yacsType;
  std::string name;
  static std::unordered_map<std::string, size_t> stringToType;
};

#define USER_DEFINED_COMPONENT_TYPE
#define CLASS_TYPE_DECLARE static ComponentType yacsType;
#define CLASS_TYPE_DEFINE(name) ComponentType name::yacsType(SIZE_MAX);
#define CLASS_TYPE_DEFINE_WITH_NAME(typeName, name) ComponentType typeName::yacsType(SIZE_MAX, name);
#define YACS_UPDATE_TYPE const size_t &time = 0
#define YACS_UPDATE_CALL time
#define YACS_SYSTEM_UPDATE const size_t &time
#define YACS_SYSTEM_FUNC const size_t &time
#define YACS_SYSTEM_UPDATE_CALL time
#define YACS_DEFAULT_COMPONENTS_COUNT 1000
#define YACS_DEFAULT_ENTITY_COUNT 1000
//#include "YACS.h"
#include "yacs.h"

// std::ostream& operator<<(std::ostream& stream, const glm::vec3 &vec);

#endif

// #ifdef YACS_DEFINE_EVENT_TYPE
//   std::unordered_map<std::string, uint64_t> ComponentType::stringToType;
// #endif
