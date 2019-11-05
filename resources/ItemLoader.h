#ifndef ITEM_LOADER_H
#define ITEM_LOADER_H

#include "Type.h"
#include "Loader.h"
#include "ResourceParser.h"
#include "Resource.h"

#include "ItemType.h"

#include <vector>
#include <unordered_map>
#include "MemoryPool.h"
#include "LoadingTemporaryData.h"

class AbilityTypeLoader;
class EffectsLoader;

struct Identificator {
  union {
    size_t num;
    struct {
      char str[sizeof(size_t)];
    };
  };
  
  Identificator(const char* name);
  Identificator(const Identificator &id);
  
  bool operator==(const Identificator &another) const;
  Identificator & operator=(const Identificator &id);
};

class ItemTypeLoader : public Loader, public ResourceParser {
public:
  enum Errors {
    ERROR_FILE_NOT_FOUND = 0,
    ERROR_ABILITY_SLOT_IS_NOT_VALID,
    ERROR_BAD_SLOT_ABILITY_TYPE,
    ERROR_SLOT_KEY_IS_NOT_VALID,
    ERROR_ITEM_ID_IS_NOT_SPECIFIED,
    ERROR_BAD_ITEM_TYPE_DATA
  };
  
  enum Warnings {
    WARNING_TOO_MUCH_DATA_WEAPON_PART_IS_IGNORED = 0,
    
  };
  
  class LoadData : public Resource {
  public:
    struct AbilitySlot {
      //Identificator slotIdentificator;
      size_t slot;
      Type ability;
      Type state;
    };
    
    struct CreateInfo {
      Type m_id;
      Type m_groupId;
      std::string m_name;
      std::string m_description;
      Type m_itemAbility;
      std::vector<Type> m_pickupEffects;
      std::vector<Type> m_weaponEffects;
      std::vector<AbilitySlot> m_abilities;
    };
    LoadData(const CreateInfo &info);
    
    Type itemId() const;
    Type groupId() const;
    std::string name() const;
    std::string description() const;
    Type itemAbility() const;
    const std::vector<Type> & pickupEffects() const;
    const std::vector<Type> & weaponEffects() const;
    const std::vector<AbilitySlot> & abilities() const;
  private:
    Type m_id;
    Type m_groupId;
    std::string m_name;
    std::string m_description;
    Type m_itemAbility;
    std::vector<Type> m_pickupEffects;
    std::vector<Type> m_weaponEffects;
    std::vector<AbilitySlot> m_abilities;
  };
  
  struct CreateInfo {
    const AbilityTypeLoader* abilityTypeLoader;
    const EffectsLoader* effectsLoader;
  };
  ItemTypeLoader(const CreateInfo &info);
  ~ItemTypeLoader();
  
  bool canParse(const std::string &key) const override;
  
  bool parse(const Modification* mod,
             const std::string &pathPrefix,
             const nlohmann::json &data,
             std::vector<Resource*> &resource,
             std::vector<ErrorDesc> &errors,
             std::vector<WarningDesc> &warnings) override;
  bool forget(const ResourceID &name) override;

  Resource* getParsedResource(const ResourceID &id) override;
  const Resource* getParsedResource(const ResourceID &id) const override;

  bool load(const ModificationParser* modifications, const Resource* resource) override;
  bool unload(const ResourceID &id) override;
  void end() override;
  
  void clear() override;
  
  size_t overallState() const override;
  size_t loadingState() const override;
  std::string hint() const override;
  
  const ItemType* getItemType(const Type &id) const;
private:
  const AbilityTypeLoader* abilityTypeLoader;
  const EffectsLoader* effectsLoader;
  LoadingTemporaryData<LoadData, 50>* tempData;
  
  MemoryPool<ItemType, sizeof(ItemType)*50> itemPool;
  std::vector<ItemType*> itemPointers;
  std::unordered_map<Type, const ItemType*> itemTypes;
  
  size_t findTempData(const ResourceID &id) const;
};

#endif
