#ifndef EFFECTS_LOADER_H
#define EFFECTS_LOADER_H

#include "Type.h"
#include "Loader.h"
#include "ResourceParser.h"
#include "Resource.h"
#include "Effect.h"

#include "MemoryPool.h"

#include <unordered_map>
#include <vector>

class AttributesLoader;

class EffectsLoader : public Loader, public ResourceParser {
public:
  class LoadData : public Resource {
  public:
    struct BonusType {
      ResourceID attribId;
      Bonus bonus;
    };
    
    struct Modificator {
      Type event;
      ResourceID effectId;
    };
    
    struct CreateInfo {
      Resource::CreateInfo resInfo;
      
      Type m_id;
      std::string m_name;
      std::string m_description;
      size_t m_baseEffectTime;
      size_t m_periodTime;
      EffectType m_type;
      std::vector<BonusType> m_bonuses;
      std::vector<Modificator> m_mods;
      std::string m_computeFuncPath;
      std::string m_resistFuncPath;
    };
    LoadData(const CreateInfo &info);
    
    Type effectId() const;
    std::string name() const;
    std::string description() const;
    size_t baseEffectTime() const;
    size_t periodTime() const;
    const std::vector<BonusType> & bonuses() const;
    const std::vector<Modificator> & mods() const;
    EffectType type() const;
    std::string computeFuncPath() const;
    std::string resistFuncPath() const;
  private:
    // практически повторяет по содержанию Effect
    Type m_id;
    std::string m_name;
    std::string m_description;
    size_t m_baseEffectTime;
    size_t m_periodTime;
    std::vector<BonusType> m_bonuses;
    std::vector<Modificator> m_mods;
    EffectType m_type;
    std::string m_computeFuncPath;
    std::string m_resistFuncPath;
  };
  
  struct CreateInfo {
    AttributesLoader* attributesLoader;
    std::unordered_map<std::string, Effect::FuncType> hardcodedComputeFunc;
    std::unordered_map<std::string, Effect::FuncType> hardcodedResistFunc;
  };
  EffectsLoader(const CreateInfo &info);
  ~EffectsLoader();
  
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
  
  const Effect* getEffect(const Type &id) const;
private:
  struct TemporaryData {
    ~TemporaryData();
    
    MemoryPool<LoadData, sizeof(LoadData)*20> dataPool;
    std::vector<LoadData*> datasPtr;
  };
  
  TemporaryData* tempData;
  AttributesLoader* attributesLoader;
  
  MemoryPool<Effect, sizeof(Effect)*20> effectsPool;
  std::vector<Effect*> effectsPtr;
  std::unordered_map<Type, const Effect*> effects;
  
  std::unordered_map<std::string, Effect::FuncType> hardcodedComputeFunc;
  std::unordered_map<std::string, Effect::FuncType> hardcodedResistFunc;
  
  LoadData* getEffectResource(const Type &effectId) const;
};

#endif
