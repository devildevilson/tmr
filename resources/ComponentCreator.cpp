#include "ComponentCreator.h"

#include "EntityComponentSystem.h"
#include "Globals.h"

DataIdentifier::DataIdentifier(const char* name) : num(0) {
  ASSERT(strlen(name) <= sizeof(size_t)+1);
  
  const size_t copySize = std::min(strlen(name), sizeof(size_t));
  memcpy(str, name, copySize);
}

DataIdentifier::DataIdentifier(const DataIdentifier &id) : num(id.num) {}

bool DataIdentifier::operator==(const DataIdentifier &another) const {
  return this->num == another.num;
}

DataIdentifier & DataIdentifier::operator=(const DataIdentifier &id) {
  this->num = id.num;
  return *this;
}

UniversalDataContainer::UniversalDataContainer(const CreateInfo &info) : dataSize(0), data(nullptr) {
  for (const auto &userData : info.datas) {
    dataSize += userData.size;
  }
  
  data = new char[dataSize];
  
  size_t offset = 0;
  for (const auto &userData : info.datas) {
    memcpy(&data[offset], userData.data, userData.size);
    placements.push_back({userData.id, offset, userData.size});
    offset += userData.size;
  }
}

UniversalDataContainer::UniversalDataContainer(const UniversalDataContainer &cont) : dataSize(cont.dataSize), data(new char[dataSize]), placements(cont.placements) {
  memcpy(data, cont.data, dataSize);
}

UniversalDataContainer::~UniversalDataContainer() {
  delete [] data;
}

char* UniversalDataContainer::get_data(const DataIdentifier &id) const {
  for (const auto &placement : placements) {
    if (placement.id == id) return &data[placement.offset];
  }
  
  return nullptr;
}

size_t UniversalDataContainer::get_data_size(const DataIdentifier &id) const {
  for (const auto &placement : placements) {
    if (placement.id == id) return placement.size;
  }
  
  return SIZE_MAX;
}

size_t UniversalDataContainer::size() const {
  return dataSize;
}

UniversalDataContainer & UniversalDataContainer::operator=(const UniversalDataContainer &cont) {
  delete [] data;
  
  dataSize = cont.dataSize;
  data = new char[dataSize];
  placements = cont.placements;
  memcpy(data, cont.data, dataSize);
  
  return *this;
}

CreatorComponent::CreatorComponent(const CreateInfo &info) : creators(info.creators) {}
  
void CreatorComponent::create(yacs::entity* parent, yacs::entity* ent, const UniversalDataContainer* container) const {
  for (auto creator : creators) {
    creator->create(parent, ent, container);
  }
}

// yacs::entity* EntityCreator::create(const Type &type, yacs::entity* parent) {
//   auto itr = types.find(type);
//   if (itr == types.end()) throw std::runtime_error("Creator with type "+type.name()+" doesnt exist");
//   
//   auto creatorEnt = itr->second;
//   auto creatorComp = creatorEnt->get<CreatorComponent>();
//   
//   auto ent = Global::world()->create_entity();
//   creatorComp->create(parent, ent);
//   
//   return ent;
// }
