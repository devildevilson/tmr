#ifndef TYPE_H
#define TYPE_H

#include <string>
#include <unordered_map>

class Type {
public:
  static Type get(const std::string &name);
  static bool has(const std::string &name);
  
  Type();
  Type(const Type &type);
  Type(const std::string &name);
  
  size_t getType() const;
  std::string getName() const;
  
  Type & operator=(const Type &other);
  bool operator==(const Type &other) const;
  bool operator!=(const Type &other) const;
private:
  size_t type;
  
  static size_t newType;
  static std::unordered_map<std::string, Type> nameToType;
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
