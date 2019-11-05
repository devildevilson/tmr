#ifndef ABILITY_TYPE_LOADER_H
#define ABILITY_TYPE_LOADER_H

#include "Type.h"
#include "Loader.h"
#include "ResourceParser.h"
#include "Resource.h"

#include "AbilityType.h"
#include "MemoryPool.h"
#include "LoadingTemporaryData.h"

class AttributesLoader;
class EffectsLoader;

class AbilityTypeLoader : public ResourceParser, public Loader {
public:
  enum ErrorTypes {
    ERROR_FILE_NOT_FOUND = 0,
    ERROR_BAD_ABILITY_COST_VALUE,
    ERROR_ENTITY_CREATE_TYPE_NOT_FOUND,
    ERROR_ENTITY_IMPACT_EVENT_NOT_FOUND,
    ERROR_ABILITY_MUST_HAVE_AN_ID,
    ERROR_ABILITY_TYPE_LOADER_NOT_ENOUGH_DATA
  };
  
  enum WarningTypes {
    WARNING_SPECIFIED_WEAPON_TYPE_AND_ENTITY_CREATE_DATA
  };
  
  class LoadData : public Resource {
  public:
    struct Cost {
      ResourceID attribute;
      int64_t cost;
    };
    
    struct AttributeListElement {
      ResourceID attribute;
      double value;
    };
    
    struct EntityCreateData {
      ResourceID entityType;
      bool inheritTransform;
      bool inheritAttributes;
      bool inheritEffects;
      Type impactEvent;
      size_t delayTime;
      std::string transformComputeFunction;
      std::string attributesComputeFunction;
      std::vector<AttributeListElement> attributesList;
      std::vector<ResourceID> impactEffects;
    };
    
    struct CreateInfo {
      Resource::CreateInfo resInfo;
      
      Type m_id;
      std::string m_name;
      std::string m_description;
      size_t m_castTime;
      size_t m_cooldown;
      Cost m_costs[MAX_ATTRIBUTES_COST_COUNT];
      std::vector<ResourceID> m_effectsType; // это должен быть ResourceID
      ResourceID m_weaponType;
      EntityCreateData m_createData;
    };
    LoadData(const CreateInfo &info);
    
    Type abilityId() const;
    std::string name() const;
    std::string description() const;
    size_t castTime() const;
    size_t cooldown() const;
    Cost costs(const size_t &index) const;
    const std::vector<ResourceID> & effects() const;
    ResourceID weapon() const;
    const EntityCreateData & entityCreateData() const;
  private:
    Type m_id;
    std::string m_name;
    std::string m_description;
    size_t m_castTime;
    size_t m_cooldown;
    Cost m_costs[MAX_ATTRIBUTES_COST_COUNT];
    std::vector<ResourceID> m_effectsType;
    ResourceID m_weaponType;
    EntityCreateData m_createData;
  };
  
  struct CreateInfo {
    const AttributesLoader* attribLoader;
    const EffectsLoader* effectsLoader;
    std::unordered_map<std::string, AbilityType::ComputeTransformFunction> transFuncs;
    std::unordered_map<std::string, AbilityType::ComputeAttributesFunction> attribsFuncs;
  };
  AbilityTypeLoader(const CreateInfo &info);
  ~AbilityTypeLoader();
  
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
  
  const AbilityType* getAbilityType(const Type &id) const;
private:
  const AttributesLoader* attribLoader;
  const EffectsLoader* effectsLoader;
  
  LoadingTemporaryData<LoadData, 30>* tempData;
  
  MemoryPool<AbilityType, sizeof(AbilityType)*30> abilityTypePool;
  std::vector<AbilityType*> abilityTypePtr;
  std::unordered_map<Type, const AbilityType*> abilityTypes;
  
  std::unordered_map<std::string, AbilityType::ComputeTransformFunction> transFuncs;
  std::unordered_map<std::string, AbilityType::ComputeAttributesFunction> attribsFuncs;
  
  size_t findTempData(const ResourceID &id) const;
};

#endif
