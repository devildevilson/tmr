#include "ResourceID.h"

ResourceID ResourceID::get(const std::string &name) {
  auto itr = idx.find(name);
  if (itr == idx.end()) {
    ResourceID id(nextId);
    idx[name] = nextId;
    names[nextId] = name;
    ++nextId;
    return id;
  }
  
  return ResourceID(itr->second);
}

bool ResourceID::has(const std::string &name) {
  auto itr = idx.find(name);
  return itr != idx.end();
}

ResourceID::ResourceID() : resourceId(SIZE_MAX) {}

ResourceID::ResourceID(const std::string &name) {
  auto itr = idx.find(name);
  if (itr == idx.end()) {
    resourceId = nextId;
    idx[name] = nextId;
    names[nextId] = name;
    ++nextId;
  } else {
    resourceId = itr->second;
  }
}

ResourceID::ResourceID(const ResourceID &other) {
  resourceId = other.resourceId;
}

size_t ResourceID::id() const {
  return resourceId;
}

std::string ResourceID::name() const {
  return names[resourceId];
}

ResourceID & ResourceID::operator=(const ResourceID &other) {
  resourceId = other.resourceId;
  return *this;
}

bool ResourceID::operator==(const ResourceID &other) const {
  return resourceId == other.resourceId;
}

bool ResourceID::operator!=(const ResourceID &other) const {
  return resourceId != other.resourceId;
}

ResourceID::ResourceID(const size_t &id) : resourceId(id) {}

size_t ResourceID::nextId = 0;
std::unordered_map<std::string, size_t> ResourceID::idx;
std::unordered_map<size_t, std::string> ResourceID::names;
