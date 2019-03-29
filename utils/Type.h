#ifndef TYPE_H
#define TYPE_H

#include <string>
#include <vector>
#include <unordered_map>

class Type {
public:
  static Type get(const size_t &type);
  static Type get(const std::string &name);
  static bool has(const size_t &type);
  static bool has(const std::string &name);
  
  Type();
  
  size_t getType() const;
  std::string getName() const;
  
  bool operator==(const Type &other) const;
private:
  Type(const size_t &type);
  Type(const size_t &type, const std::string &name);
  
  size_t type = UINT64_MAX;
  std::string name;
  
  static std::vector<Type> types;
  static std::unordered_map<std::string, size_t> nameToType;
};

namespace std {
  template<> 
  struct hash<Type> {
    size_t operator() (const Type &t) const {
      return hash<size_t>()(t.getType());
    }
  };
}

#endif
