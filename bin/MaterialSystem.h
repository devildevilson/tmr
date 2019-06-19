#ifndef MATERIAL_SYSTEM_H
#define MATERIAL_SYSTEM_H

#include "Type.h"
#include "EntityComponentSystem.h"

#include <unordered_map>

class MaterialSystem {
public:
  MaterialSystem();
  ~MaterialSystem();
  
  yacs::Entity* create(const Type &type);
  yacs::Entity* get(const Type &type);
  
  void clear();
private:
  std::unordered_map<Type, yacs::Entity*> materials;
};

#endif
