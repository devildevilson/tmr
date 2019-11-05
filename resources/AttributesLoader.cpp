#include "AttributesLoader.h"

#include <iostream>
#include <fstream>

AttributesLoader::LoadData::LoadData(const CreateInfo &info) : Resource(info.resInfo), floattype(info.floattype), m_id(info.m_id), m_name(info.m_name), m_description(info.m_description), m_pathToComputeFunction(info.m_pathToComputeFunction) {}
bool AttributesLoader::LoadData::float_type() const { return floattype; }
Type AttributesLoader::LoadData::attribId() const { return m_id; }
std::string AttributesLoader::LoadData::name() const { return m_name; }
std::string AttributesLoader::LoadData::description() const { return m_description; }
std::string AttributesLoader::LoadData::pathToComputeFunction() const { return m_pathToComputeFunction; }

bool checkAttributesJsonValidity(const std::string &path, const nlohmann::json &json, const size_t &mark, AttributesLoader::LoadData::CreateInfo &info, std::vector<ErrorDesc> &errors, std::vector<WarningDesc> &warnings) {
  bool hasId = false, hasType = false;
  
  for (auto itr = json.begin(); itr != json.end(); ++itr) {
    if (itr.value().is_string() && itr.key() == "id") {
      hasId = true;
      info.m_id = Type::get(itr.value().get<std::string>());
    }
    
    if (itr.value().is_string() && itr.key() == "type") {
      hasType = true;
      const bool rightFloat = itr.value().get<std::string>() == "fractional";
      const bool rightInt = itr.value().get<std::string>() == "integer";
      if (!rightFloat && !rightInt) {
        ErrorDesc desc(mark, AttributesLoader::ERROR_BAD_TYPE_VALUE, "Variable with key \"type\" must be equal \"fractional\" or \"integer\"");
        std::cout << "Error: " << desc.description << "\n";
        continue;
      }
      
      info.floattype = rightFloat;
    }
    
    if (itr.value().is_string() && itr.key() == "name") {
      info.m_name = itr.value().get<std::string>();
    }
    
    if (itr.value().is_string() && itr.key() == "description") {
      info.m_description = itr.value().get<std::string>();
    }
    
    if (itr.value().is_string() && itr.key() == "compute_function") {
      info.m_pathToComputeFunction = itr.value().get<std::string>();
    }
  }
  
  if (!hasId) {
    ErrorDesc desc(mark, AttributesLoader::ERROR_ATTRIBUTE_ID_NOT_FOUND, "Attribute description must have an id");
    std::cout << "Error: " << desc.description << "\n";
    return false;
  }
  
  if (!hasType) {
    ErrorDesc desc(mark, AttributesLoader::ERROR_ATTRIBUTE_TYPE_IS_NOT_SPECIFIED, "Attribute description must have a type");
    std::cout << "Error: " << desc.description << "\n";
    return false;
  }
}

AttributesLoader::AttributesLoader(const CreateInfo &info) : tempData(nullptr), floatComputeFuncs(info.floatComputeFuncs), intComputeFuncs(info.intComputeFuncs) {}
AttributesLoader::~AttributesLoader() {
  clear();
  
  for (auto ptr : floatAttribsTypesPtr) {
    floatAttribTypePool.deleteElement(ptr);
  }
  
  for (auto ptr : intAttribsTypesPtr) {
    intAttribTypePool.deleteElement(ptr);
  }
}

bool AttributesLoader::canParse(const std::string &key) const {
  return key == "attributes";
}

bool AttributesLoader::parse(const Modification* mod,
                             const std::string &pathPrefix,
                             const nlohmann::json &data,
                             std::vector<Resource*> &resource,
                             std::vector<ErrorDesc> &errors,
                             std::vector<WarningDesc> &warnings) {
  if (tempData == nullptr) tempData = new TempData;
  
  if (data.is_string()) {
    const std::string &path = data.get<std::string>();
    std::ifstream file(pathPrefix + path);
    if (!file) {
      ErrorDesc desc(4123, ERROR_FILE_NOT_FOUND, "Could not load file "+pathPrefix+path);
      std::cout << "Error: " << desc.description << "\n";
      errors.push_back(desc);
      return false;
    }
    
    nlohmann::json json;
    file >> json;
    
    return parse(mod, pathPrefix, json, resource, errors, warnings);
  } else if (data.is_array()) {
    for (size_t i = 0; i < data.size(); ++i) {
      // на самом деле тут наверное должен быть везде обжект
      if (data[i].is_object()) {
        AttributesLoader::LoadData::CreateInfo info = {};
        const bool ret = checkAttributesJsonValidity(pathPrefix, data[i], 4123, info, errors, warnings);
        if (!ret) continue;
        
        info.resInfo.mod = mod;
        info.resInfo.parsedBy = this;
        info.resInfo.pathStr = "";
        info.resInfo.resGPUSize = 0;
        info.resInfo.resId = ResourceID::get(info.m_id.name());
        info.resInfo.resSize = sizeof(AttributeType<FLOAT_ATTRIBUTE_TYPE>);
        
        auto ptr = tempData->tempPool.newElement(info);
        tempData->tempPtr.push_back(ptr);
        resource.push_back(ptr);
      } else if (data[i].is_string()) {
        const std::string &path = data.get<std::string>();
        std::ifstream file(pathPrefix + path);
        if (!file) {
          ErrorDesc desc(4123, ERROR_FILE_NOT_FOUND, "Could not load file "+pathPrefix+path);
          std::cout << "Error: " << desc.description << "\n";
          errors.push_back(desc);
          continue;
        }
        
        nlohmann::json json;
        json << file;
        
        parse(mod, pathPrefix, json, resource, errors, warnings);
      }
    }
  } else if (data.is_object()) {
    AttributesLoader::LoadData::CreateInfo info = {};
    const bool ret = checkAttributesJsonValidity(pathPrefix, data, 4123, info, errors, warnings);
    if (!ret) return false;
    
    info.resInfo.mod = mod;
    info.resInfo.parsedBy = this;
    info.resInfo.pathStr = "";
    info.resInfo.resGPUSize = 0;
    info.resInfo.resId = ResourceID::get(info.m_id.name());
    info.resInfo.resSize = sizeof(AttributeType<FLOAT_ATTRIBUTE_TYPE>);
    
    auto ptr = tempData->tempPool.newElement(info);
    tempData->tempPtr.push_back(ptr);
    resource.push_back(ptr);
  }
  
  return true;
}

bool AttributesLoader::forget(const ResourceID &name) {
  if (tempData == nullptr) throw std::runtime_error("Not in loading state");
  
  for (size_t i = 0; i < tempData->tempPtr.size(); ++i) {
    if (tempData->tempPtr[i]->id() == name) {
      tempData->tempPool.deleteElement(tempData->tempPtr[i]);
      std::swap(tempData->tempPtr[i], tempData->tempPtr.back());
      tempData->tempPtr.pop_back();
      return true;
    }
  }
  
  return false;
}

Resource* AttributesLoader::getParsedResource(const ResourceID &id) {
  if (tempData == nullptr) throw std::runtime_error("Not in loading state");
  
  for (size_t i = 0; i < tempData->tempPtr.size(); ++i) {
    if (tempData->tempPtr[i]->id() == id) return tempData->tempPtr[i];
  }
  
  return nullptr;
}

const Resource* AttributesLoader::getParsedResource(const ResourceID &id) const {
  if (tempData == nullptr) throw std::runtime_error("Not in loading state");
  
  for (size_t i = 0; i < tempData->tempPtr.size(); ++i) {
    if (tempData->tempPtr[i]->id() == id) return tempData->tempPtr[i];
  }
  
  return nullptr;
}

bool AttributesLoader::load(const ModificationParser* modifications, const Resource* resource) {
  // для чего мне здесь модификация?
  if (tempData == nullptr) throw std::runtime_error("Not in loading state");
  
  LoadData* res = nullptr;
  for (size_t i = 0; i < tempData->tempPtr.size(); ++i) {
    if (tempData->tempPtr[i] == resource) {
      res = tempData->tempPtr[i];
      break;
    }
  }
  
  if (res == nullptr) return false;
  
  if (res->float_type()) {
    if (floatAttribsTypes.find(res->attribId()) != floatAttribsTypes.end()) return true;
    
    auto itr = floatComputeFuncs.find(res->pathToComputeFunction());
    if (itr == floatComputeFuncs.end()) throw std::runtime_error("Could not find function for attribute "+res->attribId().name());
    
    const AttributeType<FLOAT_ATTRIBUTE_TYPE>::CreateInfo info{
      res->attribId(),
      res->name(),
      res->description(),
      ATTRIBUTE_DATA_TYPE_NONE,
      itr->second
    };
    auto ptr = floatAttribTypePool.newElement(info);
    floatAttribsTypesPtr.push_back(ptr);
    floatAttribsTypes[res->attribId()] = ptr;
  } else {
    if (intAttribsTypes.find(res->attribId()) != intAttribsTypes.end()) return true;
    
    auto itr = intComputeFuncs.find(res->pathToComputeFunction());
    if (itr == intComputeFuncs.end()) throw std::runtime_error("Could not find function for attribute "+res->attribId().name());
    
    const AttributeType<INT_ATTRIBUTE_TYPE>::CreateInfo info{
      res->attribId(),
      res->name(),
      res->description(),
      ATTRIBUTE_DATA_TYPE_NONE,
      itr->second
    };
    auto ptr = intAttribTypePool.newElement(info);
    intAttribsTypesPtr.push_back(ptr);
    intAttribsTypes[res->attribId()] = ptr;
  }
  
  return true;
}

bool AttributesLoader::unload(const ResourceID &id) {
  if (tempData == nullptr) throw std::runtime_error("Not in loading state");
  
  size_t res = SIZE_MAX;
  for (size_t i = 0; i < tempData->tempPtr.size(); ++i) {
    if (tempData->tempPtr[i]->id() == id) {
      res = i;
      break;
    }
  }
  
  if (res == SIZE_MAX) return false;
  
  if (tempData->tempPtr[res]->float_type()) {
    floatAttribsTypes.erase(tempData->tempPtr[res]->attribId());
    for (size_t i = 0; i < floatAttribsTypesPtr.size(); ++i) {
      if (floatAttribsTypesPtr[i]->id() == tempData->tempPtr[res]->attribId()) {
        floatAttribTypePool.deleteElement(floatAttribsTypesPtr[i]);
        std::swap(floatAttribsTypesPtr[i], floatAttribsTypesPtr.back());
        floatAttribsTypesPtr.pop_back();
        break;
      }
    }
  } else {
    intAttribsTypes.erase(tempData->tempPtr[res]->attribId());
    for (size_t i = 0; i < floatAttribsTypesPtr.size(); ++i) {
      if (intAttribsTypesPtr[i]->id() == tempData->tempPtr[res]->attribId()) {
        intAttribTypePool.deleteElement(intAttribsTypesPtr[i]);
        std::swap(intAttribsTypesPtr[i], intAttribsTypesPtr.back());
        intAttribsTypesPtr.pop_back();
        break;
      }
    }
  }
  
  tempData->tempPool.deleteElement(tempData->tempPtr[res]);
  std::swap(tempData->tempPtr[res], tempData->tempPtr.back());
  tempData->tempPtr.pop_back();
}

void AttributesLoader::end() {
  
}

void AttributesLoader::clear() {
  delete tempData;
  tempData = nullptr;
}

size_t AttributesLoader::overallState() const {
  
}

size_t AttributesLoader::loadingState() const {
  
}

std::string AttributesLoader::hint() const {
  
}

const AttributeType<FLOAT_ATTRIBUTE_TYPE>* AttributesLoader::getFloatType(const Type &id) const {
  auto itr = floatAttribsTypes.find(id);
  if (itr == floatAttribsTypes.end()) return nullptr;
  
  return itr->second;
}

const AttributeType<INT_ATTRIBUTE_TYPE>* AttributesLoader::getIntType(const Type &id) const {
  auto itr = intAttribsTypes.find(id);
  if (itr == intAttribsTypes.end()) return nullptr;
  
  return itr->second;
}
