#ifndef ATTRIBUTES_LOADER_H
#define ATTRIBUTES_LOADER_H

#include "Type.h"
#include "Loader.h"
#include "ResourceParser.h"
#include "Resource.h"
#include "Attributes.h"

#include "MemoryPool.h"

#include <unordered_map>

class AttributesLoader : public Loader, public ResourceParser {
public:
  enum ErrorTypes {
    ERROR_FILE_NOT_FOUND = 0,
    ERROR_BAD_TYPE_VALUE,
    ERROR_ATTRIBUTE_ID_NOT_FOUND,
    ERROR_ATTRIBUTE_TYPE_IS_NOT_SPECIFIED
  };
  
  class LoadData : public Resource {
  public:
    struct CreateInfo {
      Resource::CreateInfo resInfo;
      
      bool floattype;
      Type m_id;
      std::string m_name;
      std::string m_description;
      std::string m_pathToComputeFunction;
    };
    LoadData(const CreateInfo &info);
    
    bool float_type() const;
    Type attribId() const;
    std::string name() const;
    std::string description() const;
    std::string pathToComputeFunction() const;
  private:
    bool floattype;
    Type m_id;
    std::string m_name;
    std::string m_description;
    std::string m_pathToComputeFunction;
  };
  
  struct CreateInfo {
    std::unordered_map<std::string, AttributeType<FLOAT_ATTRIBUTE_TYPE>::FuncType> floatComputeFuncs;
    std::unordered_map<std::string, AttributeType<INT_ATTRIBUTE_TYPE>::FuncType> intComputeFuncs;
  };
  AttributesLoader(const CreateInfo &info);
  ~AttributesLoader();
  
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
  
  const AttributeType<FLOAT_ATTRIBUTE_TYPE>* getFloatType(const Type &id) const;
  const AttributeType<INT_ATTRIBUTE_TYPE>* getIntType(const Type &id) const;
private:
  struct TempData {
    ~TempData();
    
    MemoryPool<LoadData, sizeof(LoadData)*30> tempPool;
    std::vector<LoadData*> tempPtr;
  };
  
  TempData* tempData;
  
  MemoryPool<AttributeType<FLOAT_ATTRIBUTE_TYPE>, sizeof(AttributeType<FLOAT_ATTRIBUTE_TYPE>)*50> floatAttribTypePool;
  MemoryPool<AttributeType<INT_ATTRIBUTE_TYPE>, sizeof(AttributeType<INT_ATTRIBUTE_TYPE>)*50> intAttribTypePool;
  
  std::vector<AttributeType<FLOAT_ATTRIBUTE_TYPE>*> floatAttribsTypesPtr;
  std::vector<AttributeType<INT_ATTRIBUTE_TYPE>*> intAttribsTypesPtr;
  
  std::unordered_map<Type, const AttributeType<FLOAT_ATTRIBUTE_TYPE>*> floatAttribsTypes;
  std::unordered_map<Type, const AttributeType<INT_ATTRIBUTE_TYPE>*> intAttribsTypes;
  
  // какие-то функции останутся даже тогда когда я сделаю lua
  std::unordered_map<std::string, AttributeType<FLOAT_ATTRIBUTE_TYPE>::FuncType> floatComputeFuncs;
  std::unordered_map<std::string, AttributeType<INT_ATTRIBUTE_TYPE>::FuncType> intComputeFuncs;
};

#endif
