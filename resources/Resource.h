#ifndef RESOURCE_H
#define RESOURCE_H

#include <cstddef>
#include <vector>
#include "ResourceID.h"

class Modification;
class Conflict;
class ResourceParser;

// как добавлять депенденси? по идее ресурс у меня должен определяться только правильно заданным ResourceID
// использую ResourceID

// необходимо наследовать от этого класса
class Resource {
public:
  struct CreateInfo {
    ResourceID resId;
    std::string pathStr;
    size_t resSize;
    size_t resGPUSize;

    const ResourceParser* parsedBy;
    const Modification* mod;
//    const Conflict* relatedConflict;
  };
  Resource(const CreateInfo &info);
  virtual ~Resource();

  ResourceID id() const;
  std::string path() const;

  size_t size() const;
  size_t gpuSize() const;

  const ResourceParser* parser() const;
  const Modification* modification() const;
//  const Conflict* conflict() const;
//
//  void addDependency(const Conflict* conflict);
//  const std::vector<const Conflict*> & getDependencies() const;

  void addDependency(const ResourceID &id);
  const std::vector<ResourceID> & dependencies() const;
protected:
  ResourceID resId;
  std::string pathStr;
  size_t resSize;
  size_t resGPUSize;

  const ResourceParser* parsedBy;
  const Modification* mod;
//  const Conflict* relatedConflict;
//  std::vector<const Conflict*> dependency;
  std::vector<ResourceID> dependency;
};

#endif //RESOURCE_H
