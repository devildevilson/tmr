#include "Type.h"

Type Type::get(const std::string &name) {
  const auto &itr = nameToType.find(name);
  if (itr != nameToType.end()) return Type(itr->second);

  const size_t type = newType;
  ++newType;
  nameToType[name] = type;

  return Type(type);
}

bool Type::has(const std::string &name) {
  return nameToType.find(name) != nameToType.end();
}

bool Type::valid() const {
  return m_type != SIZE_MAX;
}

size_t Type::type() const {
  return m_type;
}

std::string Type::name() const {
  for (const auto &pair : nameToType) {
    if (pair.second == m_type) return pair.first;
  }
  
  return "";
}

//Type & Type::operator=(const Type &other) {
//  this->type = other.type;
//  return *this;
//}

//bool Type::operator==(const Type &other) const {
//  return getType() == other.getType();
//}
//
//bool Type::operator!=(const Type &other) const {
//  return getType() != other.getType();
//}
  
Type::Type() : m_type(SIZE_MAX) {}
//Type::Type(const Type &type) : type(type.getType()) {}
Type::Type(const size_t &type) : m_type(type) {}
//Type::Type(const std::string& name) {
//  m_type = newType;
//  ++newType;
//
//  nameToType[name] = *this;
//}

size_t Type::newType = 0;
std::unordered_map<std::string, size_t> Type::nameToType;

bool operator==(const Type &first, const Type &second) {
  return first.type() == second.type();
}

bool operator!=(const Type &first, const Type &second) {
  return first.type() != second.type();
}

bool operator<(const Type &first, const Type &second) {
  return first.type() < second.type();
}

bool operator>(const Type &first, const Type &second) {
  return first.type() > second.type();
}

bool operator<=(const Type &first, const Type &second) {
  return first.type() <= second.type();
}

bool operator>=(const Type &first, const Type &second) {
  return first.type() >= second.type();
}