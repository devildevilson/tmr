#ifndef TYPE_H
#define TYPE_H

#include <string>
#include <unordered_map>

class Type {
public:
  static Type get(const std::string &name);
  static bool has(const std::string &name);
  
  Type();
  Type(const Type &type) = default;
  Type(const size_t &type);
//  Type(const std::string &name);

  bool valid() const;
  size_t type() const;
  std::string name() const;
  
  Type & operator=(const Type &other) = default;
private:
  size_t m_type;
  
  static size_t newType;
  static std::unordered_map<std::string, size_t> nameToType;
};

bool operator==(const Type &first, const Type &second);
bool operator!=(const Type &first, const Type &second);
bool operator<(const Type &first, const Type &second);
bool operator>(const Type &first, const Type &second);
bool operator<=(const Type &first, const Type &second);
bool operator>=(const Type &first, const Type &second);

namespace std {
  template<> 
  struct hash<Type> {
    size_t operator() (const Type &t) const {
      return hash<size_t>()(t.type());
    }
  };
}

#endif
