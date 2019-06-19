#include "MaterialSystem.h"

#include "Globals.h"

MaterialSystem::MaterialSystem() {}
MaterialSystem::~MaterialSystem() {}

yacs::Entity* MaterialSystem::create(const Type &type) {
  auto itr = materials.find(type);
  if (itr != materials.end()) throw std::runtime_error("Material " + type.getName() + " is already exist");
  
  yacs::Entity* ent = Global::world()->createEntity();
  materials[type] = ent;
  
  return ent;
}

yacs::Entity* MaterialSystem::get(const Type &type) {
  auto itr = materials.find(type);
  if (itr == materials.end()) throw std::runtime_error("Material " + type.getName() + " is not exist");
  
  return itr->second;
}

void MaterialSystem::clear() {
  for (auto & material : materials) {
    Global::world()->removeEntity(material.second);
  }
  materials.clear();
}
