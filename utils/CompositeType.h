#ifndef COMPOSITE_TYPE_H
#define COMPOSITE_TYPE_H

#include <cstdint>
#include <string>
#include <unordered_map>
#include <cassert>

template <typename Container = uint32_t>
class CompositeType {
public:
  CompositeType() {}
  CompositeType(const Container &raw) : type(raw) {}
  CompositeType(const std::string &name) {
    auto itr = names.find(name);
    
    if (itr == names.end()) {
      assert(sizeof(Container)*8 >= nextTypeBit);
      
      type = 1<<nextTypeBit;
      names[name] = type;
      typeToName[type] = name;
      ++nextTypeBit;
    } else {
      type = itr->second;
    }
  }
  
  CompositeType(const CompositeType<Container> &type) : type(type.type) {}
  
  Container raw() const { return type; }
  std::string name() const {
    auto itr = typeToName.find(type);
    if (itr == typeToName.end()) return "";
    
    return itr->second;
  }
  
  CompositeType<Container> & operator|=(const CompositeType<Container> &type) {
    this->type |= type.type;
    return *this;
  }
  
  CompositeType<Container> & operator&=(const CompositeType<Container> &type) {
    this->type &= type.type;
    return *this;
  }
  
  CompositeType<Container> & operator^=(const CompositeType<Container> &type) {
    this->type ^= type.type;
    return *this;
  }
  
  CompositeType<Container> & operator= (const CompositeType<Container> &type) {
    this->type = type.type;
    return *this;
  }
  
  bool operator==(const CompositeType<Container> &type) const {
    return this->type == type.type;
  }
  
  bool operator!=(const CompositeType<Container> &type) const {
    return this->type != type.type;
  }
  
  bool operator==(const Container &raw) const {
    return this->type == raw;
  }
  
  bool operator!=(const Container &raw) const {
    return this->type != raw;
  }
  
  operator bool() {
    return type != 0;
  }
private:
  Container type;
  
  static uint32_t nextTypeBit;
  static std::unordered_map<std::string, Container> names;
  static std::unordered_map<Container, std::string> typeToName;
};

template <typename Container = uint32_t>
CompositeType<Container> operator&(const CompositeType<Container> &type1, const CompositeType<Container> &type2) {
  const Container tmp = type1.raw() & type2.raw();
  return CompositeType<Container>(tmp);
}

template <typename Container = uint32_t>
CompositeType<Container> operator|(const CompositeType<Container> &type1, const CompositeType<Container> &type2) {
  const Container tmp = type1.raw() | type2.raw();
  return CompositeType<Container>(tmp);
}

template <typename Container = uint32_t>
CompositeType<Container> operator^(const CompositeType<Container> &type1, const CompositeType<Container> &type2) {
  const Container tmp = type1.raw() ^ type2.raw();
  return CompositeType<Container>(tmp);
}

template <typename Container = uint32_t>
CompositeType<Container> operator~(const CompositeType<Container> &type1) {
  const Container tmp = ~type1.raw();
  return CompositeType<Container>(tmp);
}

template <typename Container>
uint32_t CompositeType<Container>::nextTypeBit = 0;

// static std::unordered_map<std::string, Container> names;
// static std::unordered_map<Container, std::string> typeToName;

#endif
