#include "Resource.h"

Resource::Resource(const CreateInfo &info) : resId(info.resId), pathStr(info.pathStr), resSize(info.resSize), resGPUSize(info.resGPUSize), parsedBy(info.parsedBy), mod(info.mod) {}
Resource::~Resource() {}

ResourceID Resource::id() const {
  return resId;
}

std::string Resource::path() const {
  return pathStr;
}

size_t Resource::size() const {
  return resSize;
}

size_t Resource::gpuSize() const {
  return resGPUSize;
}

const ResourceParser* Resource::parser() const {
  return parsedBy;
}

const Modification* Resource::modification() const {
  return mod;
}

//const Conflict* Resource::conflict() const {
//  return relatedConflict;
//}
//
//void Resource::addDependency(const Conflict* conflict) {
//  dependency.push_back(conflict);
//}
//
//const std::vector<const Conflict*> & Resource::getDependencies() const {
//  return dependency;
//}

void Resource::addDependency(const ResourceID &id) {
  dependency.push_back(id);
}

const std::vector<ResourceID> & Resource::dependencies() const {
  return dependency;
}