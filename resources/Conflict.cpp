#include "Conflict.h"

#include "Resource.h"

Conflict::Conflict(const CreateInfo &info) : chosenIndex(SIZE_MAX) {
  variants.push_back(info.firstResource);
}

ResourceID Conflict::id() const {
  return variants[0]->id();
}

void Conflict::setIndex(const size_t &index) {
  chosenIndex = index;
}

void Conflict::add(Resource* resource) {
  variants.push_back(resource);
}

size_t Conflict::index() const {
  return chosenIndex >= variants.size() ? variants.size()-1 : chosenIndex;
}

Resource* Conflict::getChosenVariant() const {
  return variants[index()];
}

const std::vector<Resource*> & Conflict::getVariants() const {
  return variants;
}