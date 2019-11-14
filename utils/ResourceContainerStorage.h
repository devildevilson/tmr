#ifndef RESOURCE_CONTAINER_STORAGE_H
#define RESOURCE_CONTAINER_STORAGE_H

#include "TypelessContainer.h"
#include "ResourceContainer.h"

#define DECLARE_STORAGE_PTR_ARRAY(ObjectType, StorageSize) public: \
    ResourceContainerArray<Type, ObjectType, StorageSize>* get##ObjectType##Container() { return container##ObjectType ; } \
  private: \
    ResourceContainerArray<Type, ObjectType, StorageSize>* container##ObjectType ;
    
#define DECLARE_STORAGE_PTR_MAP(ObjectType, StorageSize) public: \
    ResourceContainerMap<Type, ObjectType, StorageSize>* get##ObjectType##Container() { return container##ObjectType ; } \
  private: \
    ResourceContainerMap<Type, ObjectType, StorageSize>* container##ObjectType ;

class ItemType;
class AbilityType;
template <typename T>
class AttributeType;
class Effect;
class EntityCreator; // но это не точно
// карта тоже может сюда подойти

typedef AttributeType<float> float_AttributeType;
typedef AttributeType<int64_t> int_AttributeType;
    
class ResourceContainerStorage {
public:
  ResourceContainerStorage();
  ~ResourceContainerStorage();
  
  DECLARE_STORAGE_PTR_ARRAY(ItemType, 50)
  DECLARE_STORAGE_PTR_ARRAY(AbilityType, 50)
  DECLARE_STORAGE_PTR_ARRAY(float_AttributeType, 50)
  DECLARE_STORAGE_PTR_ARRAY(int_AttributeType, 50)
  DECLARE_STORAGE_PTR_ARRAY(Effect, 50)
private:
  TypelessContainer container;
};

#endif
