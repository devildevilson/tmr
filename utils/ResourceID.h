#ifndef RESOURCE_ID_H
#define RESOURCE_ID_H

#include <cstdint>
#include <string>
#include <unordered_map>

class ResourceID {
public:
  static ResourceID get(const std::string &name);
  static bool has(const std::string &name);
  
  ResourceID();
  ResourceID(const std::string &name);
  ResourceID(const ResourceID &other);
  
  size_t id() const;
  std::string name() const;
  
  ResourceID & operator=(const ResourceID &other);
  bool operator==(const ResourceID &other) const;
  bool operator!=(const ResourceID &other) const;
private:
  ResourceID(const size_t &id);
  
  size_t resourceId;
  
  static size_t nextId;
  static std::unordered_map<std::string, size_t> idx;
  static std::unordered_map<size_t, std::string> names;
};

namespace std {
  template<>
  struct hash<ResourceID> {
    size_t operator() (const ResourceID &resourceId) const {
      return hash<size_t>()(resourceId.id());
    }
  };
}

#endif
