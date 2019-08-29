#include "Modification.h"

Modification::Modification(const CreateInfo &info) : nameStr(info.nameStr), descriptionStr(info.descriptionStr), pathStr(info.pathStr), authorStr(info.authorStr), urlStr(info.urlStr), versionVar(info.versionVar) {}

std::string Modification::name() const {
  return nameStr;
}

std::string Modification::description() const {
  return descriptionStr;
}

std::string Modification::path() const {
  return pathStr;
}

std::string Modification::author() const {
  return authorStr;
}

std::string Modification::url() const {
  return urlStr;
}

size_t Modification::version() const {
  return versionVar;
}

void Modification::addResource(Resource* res) {
  relatedResources.push_back(res);
}

std::vector<Resource*> & Modification::resources() {
  return relatedResources;
}

const std::vector<Resource*> & Modification::resources() const {
  return relatedResources;
}
