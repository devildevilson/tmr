#include "Type.h"

Type Type::get(const std::string &name) {
  const auto &itr = nameToType.find(name);
  if (itr != nameToType.end()) return itr->second;
  
  return Type(name);
}

bool Type::has(const std::string &name) {
  return nameToType.find(name) != nameToType.end();
}

size_t Type::getType() const {
  return type;
}

std::string Type::getName() const {
  return typeToName[*this];
}

Type & Type::operator=(const Type &other) {
  this->type = other.type;
  return *this;
}

bool Type::operator==(const Type &other) const {
  return getType() == other.getType();
}

bool Type::operator!=(const Type &other) const {
  return getType() != other.getType();
}
  
Type::Type() {}
Type::Type(const Type &type) : type(type.getType()) {}
Type::Type(const std::string& name) {
  type = newType;
  ++newType;
  
  nameToType[name] = *this;
  typeToName[*this] = name;
}

size_t Type::newType = 0;
std::unordered_map<std::string, Type> Type::nameToType;
std::unordered_map<Type, std::string> Type::typeToName;
