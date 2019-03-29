#include "Type.h"

Type Type::get(const size_t &type) {
  if (type >= types.size()) return Type(type);
  
  return types[type];
}

Type Type::get(const std::string &name) {
  const auto &itr = nameToType.find(name);
  if (itr != nameToType.end()) return types[itr->second];
  
  types.push_back(Type(types.size(), name));
  nameToType[name] = types.back().getType();
  
  return types.back();
}

bool Type::has(const size_t &type) {
  if (type >= types.size()) return false;
  
  return true;
}

bool Type::has(const std::string &name) {
  if (nameToType.find(name) == nameToType.end()) return false;
  
  return true;
}

size_t Type::getType() const {
  return type;
}

std::string Type::getName() const {
  return name;
}

bool Type::operator==(const Type &other) const {
  return getType() == other.getType();
}
  
Type::Type() {}

Type::Type(const size_t &type) {
  this->type = type;
}

Type::Type(const size_t &type, const std::string &name) {
  this->type = type;
  this->name = name;
}

std::vector<Type> Type::types;
std::unordered_map<std::string, size_t> Type::nameToType;
