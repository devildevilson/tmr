#include "ResourceContainerStorage.h"

#include "ItemType.h"
#include "AbilityType.h"
#include "Attributes.h"
#include "Effect.h"

#define STORAGE_PTR_ARRAY(ObjectType, StorageSize) ResourceContainerArray<Type, ObjectType, StorageSize>
#define STORAGE_PTR_MAP(ObjectType, StorageSize) ResourceContainerMap<Type, ObjectType, StorageSize>
#define CREATE_STORAGE_PTR(ObjectType, StorageType) container##ObjectType = container.create<StorageType>();
#define DESTROY_STORAGE_PTR(ObjectType) container.destroy( container##ObjectType );

ResourceContainerStorage::ResourceContainerStorage()
: container(sizeof(STORAGE_PTR_ARRAY(ItemType, 50)) + 
            sizeof(STORAGE_PTR_ARRAY(AbilityType, 50)) + 
            sizeof(STORAGE_PTR_ARRAY(float_AttributeType, 50)) + 
            sizeof(STORAGE_PTR_ARRAY(int_AttributeType, 50)) + 
            sizeof(STORAGE_PTR_ARRAY(Effect, 50))) {
  CREATE_STORAGE_PTR(ItemType, STORAGE_PTR_ARRAY(ItemType, 50))
  CREATE_STORAGE_PTR(AbilityType, STORAGE_PTR_ARRAY(AbilityType, 50))
  CREATE_STORAGE_PTR(float_AttributeType, STORAGE_PTR_ARRAY(float_AttributeType, 50))
  CREATE_STORAGE_PTR(int_AttributeType, STORAGE_PTR_ARRAY(int_AttributeType, 50))
  CREATE_STORAGE_PTR(Effect, STORAGE_PTR_ARRAY(Effect, 50))
}
ResourceContainerStorage::~ResourceContainerStorage() {
  DESTROY_STORAGE_PTR(ItemType)
  DESTROY_STORAGE_PTR(AbilityType)
  DESTROY_STORAGE_PTR(float_AttributeType)
  DESTROY_STORAGE_PTR(int_AttributeType)
  DESTROY_STORAGE_PTR(Effect)
}
