#ifndef MODIFICATION_H
#define MODIFICATION_H

#include <string>
#include <vector>
#include "ResourceID.h"

class Resource;

class Modification {
public:
  struct CreateInfo {
    std::string nameStr;
    std::string descriptionStr;
    std::string pathStr;
    std::string authorStr;
    std::string urlStr;

    size_t versionVar;
  };
  Modification(const CreateInfo &info);

  std::string name() const;
  std::string description() const;
  std::string path() const;
  std::string author() const;
  std::string url() const;

  size_t version() const;

  // изображение

  // сайз вычислять?

  void addResource(Resource* res);
  std::vector<Resource*> & resources();
  const std::vector<Resource*> & resources() const;
private:
  std::string nameStr;
  std::string descriptionStr;
  std::string pathStr;
  std::string authorStr;
  std::string urlStr;

  size_t size;
  size_t gpuSize;

  size_t versionVar;

  ResourceID imageRes;
  size_t imageIndexRes;

  std::vector<Resource*> relatedResources;
};

#endif //MODIFICATION_H
