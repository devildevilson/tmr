#ifndef CONFLICT_H
#define CONFLICT_H

#include <cstddef>
#include <vector>

#include "ResourceID.h"

class Resource;

// const ??????

class Conflict {
public:
  struct CreateInfo {
    Resource* firstResource;
  };
  Conflict(const CreateInfo &info);

  ResourceID id() const;

  void setIndex(const size_t &index);
  void add(Resource* resource);

  size_t index() const;
  Resource* getChosenVariant() const;
  const std::vector<Resource*> & getVariants() const;
private:
  size_t chosenIndex;
  std::vector<Resource*> variants;
};

#endif //CONFLICT_H
