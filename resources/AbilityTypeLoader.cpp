#include "AbilityTypeLoader.h"

#include "AttributesLoader.h"
#include "EffectsLoader.h"

#include <fstream>
#include <iostream>

bool checkJsonAbilityValidity(const std::string &path, const nlohmann::json &json, const size_t &mark, AbilityTypeLoader::LoadData::CreateInfo &info, std::vector<ErrorDesc> &errors, std::vector<WarningDesc> &warnings) {
  bool hasId = false, hasEntityData = false, hasWeaponType = false;
  
  const size_t errorsCount = errors.size();
  
  for (auto itr = json.begin(); itr != json.end(); ++itr) {
    if (itr.value().is_string() && itr.key() == "id") {
      hasId = true;
      info.m_id = Type::get(itr.value().get<std::string>());
      info.resInfo.resId = ResourceID::get(itr.value().get<std::string>());
    }
    
    if (itr.value().is_string() && itr.key() == "name") {
      info.m_name = itr.value().get<std::string>();
    }
    
    if (itr.value().is_string() && itr.key() == "description") {
      info.m_description = itr.value().get<std::string>();
    }
    
    if (itr.value().is_number_unsigned() && itr.key() == "cast_time") {
      info.m_castTime = itr.value().get<size_t>();
    }
    
    if (itr.value().is_number_unsigned() && itr.key() == "cooldown") {
      info.m_cooldown = itr.value().get<size_t>();
    }
    
    if (itr.value().is_object() && itr.key() == "cost") {
      size_t count = 0;
      for (auto costItr = itr.value().begin(); costItr != itr.value().end(); ++costItr) {
        if (!costItr.value().is_number()) {
          ErrorDesc desc(mark, AbilityTypeLoader::ERROR_BAD_ABILITY_COST_VALUE, "Bad ability cost value");
          std::cout << desc.description << "\n";
          errors.push_back(desc);
        } else {
          info.m_costs[count] = AbilityTypeLoader::LoadData::Cost{ResourceID::get(costItr.key()), costItr.value().get<double>()};
        }
        
        ++count;
        if (count == MAX_ATTRIBUTES_COST_COUNT) break;
      }
    }
    
    if (itr.value().is_array() && itr.key() == "effects") {
      for (size_t i = 0; i < itr.value().size(); ++i) {
        info.m_effectsType.push_back(ResourceID::get(itr.value()[i].get<std::string>()));
        // нам бы где нибудь проверить наличие таких ресурсов 
      }
    }
    
    if (itr.value().is_string() && itr.key() == "weapon") {
      hasWeaponType = true;
      info.m_weaponType = ResourceID::get(itr.value().get<std::string>());
    }
    
    if (itr.value().is_object() && itr.key() == "entity_create_data") {
      hasEntityData = true;
      
      bool hasType = false;
      for (auto entItr = itr.value().begin(); entItr != itr.value().end(); ++entItr) {
        if (entItr.value().is_string() && entItr.key() == "type") {
          hasType = true;
          info.m_createData.entityType = ResourceID::get(entItr.value().get<std::string>());
        }
        
        if (entItr.value().is_string() && entItr.key() == "impact_event") {
          info.m_createData.impactEvent = Type::get(entItr.value().get<std::string>());
        }
        
        if (entItr.value().is_number_unsigned() && entItr.key() == "interaction_delay_time") {
          info.m_createData.delayTime = entItr.value().get<size_t>();
        }
        
        if (entItr.value().is_string() && entItr.key() == "transform_compute_func") {
          info.m_createData.transformComputeFunction = entItr.value().get<std::string>();
        }
        
        if (entItr.value().is_string() && entItr.key() == "attributes_compute_func") {
          info.m_createData.attributesComputeFunction = entItr.value().get<std::string>();
        }
        
        if (entItr.value().is_object() && entItr.key() == "attributes_list") {
          for (auto attribListItr = entItr.value().begin(); attribListItr != entItr.value().end(); ++attribListItr) {
            info.m_createData.attributesList.push_back(AbilityTypeLoader::LoadData::AttributeListElement{ResourceID::get(attribListItr.key()), attribListItr.value().get<double>()});
          }
        }
        
        if (entItr.value().is_array() && entItr.key() == "impact_effects") {
          for (size_t i = 0; i < entItr.value().size(); ++i) {
            info.m_createData.impactEffects.push_back(ResourceID::get(entItr.value()[i].get<std::string>()));
          }
        }
        
        if (entItr.value().is_boolean() && entItr.key() == "inherit_transform") {
          info.m_createData.inheritTransform = entItr.value().get<bool>();
        }
        
        if (entItr.value().is_boolean() && entItr.key() == "inherit_attributes") {
          info.m_createData.inheritAttributes = entItr.value().get<bool>();
        }
        
        if (entItr.value().is_boolean() && entItr.key() == "inherit_effects") {
          info.m_createData.inheritEffects = entItr.value().get<bool>();
        }
      }
      
      if (!hasType && !hasWeaponType) {
        ErrorDesc desc(mark, AbilityTypeLoader::ERROR_ENTITY_CREATE_TYPE_NOT_FOUND, "Entity create type must be specified");
        std::cout << "Error: " << desc.description << "\n";
        errors.push_back(desc);
        //return false;
      }
    }
  }
  
  if (!hasId) {
    ErrorDesc error(mark, AbilityTypeLoader::ERROR_ABILITY_MUST_HAVE_AN_ID, "Ability type must have an id");
    std::cout << "Error: " << error.description << "\n";
    errors.push_back(error);
    //return false;
  }
  
  if (!hasEntityData && !hasWeaponType) {
    ErrorDesc error(mark, AbilityTypeLoader::ERROR_ABILITY_TYPE_LOADER_NOT_ENOUGH_DATA, "Either weapon type or entity create data must be specified");
    std::cout << "Error: " << error.description << "\n";
    errors.push_back(error);
  }
  
  if (hasEntityData && hasWeaponType) {
    WarningDesc warning(mark, AbilityTypeLoader::WARNING_SPECIFIED_WEAPON_TYPE_AND_ENTITY_CREATE_DATA, "Specified weapon type and entity create data. Entity data wil be ignored");
    std::cout << "Warning: " << warning.description << "\n";
    warnings.push_back(warning);
  }
  
  return errorsCount == errors.size();
}

AbilityTypeLoader::LoadData::LoadData(const CreateInfo &info) : Resource(info.resInfo), m_id(info.m_id), m_name(info.m_name), m_description(info.m_description), m_castTime(info.m_castTime), m_cooldown(info.m_cooldown), m_costs{info.m_costs[0], info.m_costs[1], info.m_costs[2]}, m_effectsType(info.m_effectsType), m_weaponType(info.m_weaponType), m_createData(info.m_createData) {}
Type AbilityTypeLoader::LoadData::abilityId() const { return m_id; }
std::string AbilityTypeLoader::LoadData::name() const { return m_name; }
std::string AbilityTypeLoader::LoadData::description() const { return m_description; }
size_t AbilityTypeLoader::LoadData::castTime() const { return m_castTime; }
size_t AbilityTypeLoader::LoadData::cooldown() const { return m_cooldown; }
AbilityTypeLoader::LoadData::Cost AbilityTypeLoader::LoadData::costs(const size_t &index) const {
  ASSERT(index < MAX_ATTRIBUTES_COST_COUNT);
  return m_costs[index];
}
const std::vector<ResourceID> & AbilityTypeLoader::LoadData::effects() const { return m_effectsType; }
ResourceID AbilityTypeLoader::LoadData::weapon() const { return m_weaponType; }
const AbilityTypeLoader::LoadData::EntityCreateData & AbilityTypeLoader::LoadData::entityCreateData() const { return m_createData; }

AbilityTypeLoader::AbilityTypeLoader(const CreateInfo &info) : attribLoader(info.attribLoader), effectsLoader(info.effectsLoader), tempData(nullptr), transFuncs(info.transFuncs), attribsFuncs(info.attribsFuncs) {}
AbilityTypeLoader::~AbilityTypeLoader() {
  clear();
  
  for (auto ptr : abilityTypePtr) {
    abilityTypePool.deleteElement(ptr);
  }
}

bool AbilityTypeLoader::canParse(const std::string &key) const {
  return key == "abilities" || key == "abilityTypes";
}

bool AbilityTypeLoader::parse(const Modification* mod,
                              const std::string &pathPrefix,
                              const nlohmann::json &data,
                              std::vector<Resource*> &resource,
                              std::vector<ErrorDesc> &errors,
                              std::vector<WarningDesc> &warnings) {
  if (tempData == nullptr) tempData = new LoadingTemporaryData<LoadData, 30>;
  
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
    bool ret = true;
    for (size_t i = 0; i < data.size(); ++i) {
      ret = ret && parse(mod, pathPrefix, data[i], resource, errors, warnings);
    }
    return ret;
  } else if (data.is_object()) {
    LoadData::CreateInfo info = {};
    info.resInfo.mod = mod;
    info.resInfo.parsedBy = this;
    info.resInfo.resGPUSize = 0;
    info.resInfo.pathStr = "";
    info.resInfo.resSize = sizeof(AbilityType);
    info.m_castTime = 0;
    info.m_cooldown = 0;
    info.m_createData.delayTime = 0;
    
    bool ret = checkJsonAbilityValidity(pathPrefix, data, 12512, info, errors, warnings);
    if (!ret) return false;
    info.resInfo.resId = ResourceID::get(info.m_id.name());
    
    auto ptr = tempData->container.create(info.resInfo.resId, info);
    resource.push_back(ptr);
    return true;
  }
  
  return false;
}

bool AbilityTypeLoader::forget(const ResourceID &name) {
  if (tempData == nullptr) throw std::runtime_error("Not in loading state");
  
//   const size_t index = findTempData(name);
//   if (index == SIZE_MAX) return false;
//   
//   tempData->dataPool.deleteElement(tempData->dataPtr[index]);
//   std::swap(tempData->dataPtr[index], tempData->dataPtr.back());
//   tempData->dataPtr.pop_back();
  
  return tempData->container.destroy(name);
}

Resource* AbilityTypeLoader::getParsedResource(const ResourceID &id) {
  if (tempData == nullptr) throw std::runtime_error("Not in loading state");
  
  return tempData->container.get(id);
}

const Resource* AbilityTypeLoader::getParsedResource(const ResourceID &id) const {
  if (tempData == nullptr) throw std::runtime_error("Not in loading state");
  
  return tempData->container.get(id);
}

bool AbilityTypeLoader::load(const ModificationParser* modifications, const Resource* resource) {
  if (tempData == nullptr) throw std::runtime_error("Not in loading state");
  
  // скорее всего нужно грузить все же в end(), причем все лоадеры так должны делать без исключения
  // то есть здесь мы должны запомнить указатели которые к нам пришли
  // нет грузить мы должны по возможности в этом методе
  
  const Type id = Type::get(resource->id().name());
  auto itr = abilityTypes.find(id);
  if (itr != abilityTypes.end()) return true;
  
  for (auto data : tempData->dataToLoad) {
    if (data->id() == resource->id()) return true;
  }
  
  //const size_t index = findTempData(resource->id());
  //if (index == SIZE_MAX) return false;
  auto ptr = tempData->container.get(resource->id());
  if (ptr == nullptr) return false;
  //tempData->dataToLoad.push_back(tempData->dataPtr[index]);
  
  if (ptr->weapon().valid()) {
    // айтем тайп мы здесь не можем найти так как он выше по иерархии
    // нужно просто передать id
    
    
    
    return true;
  }
  
  auto transItr = transFuncs.find(ptr->entityCreateData().transformComputeFunction);
  auto attribsItr = attribsFuncs.find(ptr->entityCreateData().attributesComputeFunction);
  
  // к этому моменту мы должны уже загрузить все необходимые эффекты
  std::vector<const Effect*> effects(ptr->effects().size());
  for (size_t i = 0; i < ptr->effects().size(); ++i) {
    const bool ret = effectsLoader->load(nullptr, effectsLoader->getParsedResource(ptr->effects()[i]));
    if (!ret) throw std::runtime_error("Could not load effect "+ptr->effects()[i].name());
    
    const Type effectId = Type::get(ptr->effects()[i].name());
    effects[i] = effectsLoader->getEffect(effectId);
    if (effects[i] == nullptr) throw std::runtime_error("Could not find effect "+ptr->effects()[i].name());
  }
  
  // нужно ли с помощью ресурсов обращаться к аттрибутам, или с помощью типов? лучше типами
  std::vector<AbilityAttributeListElement<INT_ATTRIBUTE_TYPE>> intAttribs;
  std::vector<AbilityAttributeListElement<FLOAT_ATTRIBUTE_TYPE>> floatAttribs;
  for (const auto &attrib : ptr->entityCreateData().attributesList) {
    const bool ret = attribLoader->load(nullptr, attribLoader->getParsedResource(attrib.attribute));
    if (!ret) throw std::runtime_error("Could not load attribute "+attrib.attribute.name());
    
    const Type attribId = Type::get(attrib.attribute.name());
    auto floatAttrib = attribLoader->getFloatType(attribId);
    if (floatAttrib != nullptr) {
      floatAttribs.push_back({floatAttrib, FLOAT_ATTRIBUTE_TYPE(attrib.value)});
      continue;
    }
    
    auto intAttrib = attribLoader->getIntType(attribId);
    if (intAttrib != nullptr) {
      intAttribs.push_back({intAttrib, INT_ATTRIBUTE_TYPE(attrib.value)});
      continue;
    }
    
    throw std::runtime_error("Could not find attribute type "+attrib.attribute.name());
  }
  
  {
    const bool ret = attribLoader->load(nullptr, attribLoader->getParsedResource(ptr->costs(0).attribute));
    if (!ret) throw std::runtime_error("Could not load attribute "+ptr->costs(0).attribute.name());
  }
  
  {
    const bool ret = attribLoader->load(nullptr, attribLoader->getParsedResource(ptr->costs(0).attribute));
    if (!ret) throw std::runtime_error("Could not load attribute "+ptr->costs(0).attribute.name());
  }
  
  {
    const bool ret = attribLoader->load(nullptr, attribLoader->getParsedResource(ptr->costs(0).attribute));
    if (!ret) throw std::runtime_error("Could not load attribute "+ptr->costs(0).attribute.name());
  }
  
  const AbilityCost cost1{
    ptr->costs(0).attribute.valid() ? attribLoader->getIntType(Type::get(ptr->costs(0).attribute.name())) : nullptr,
    ptr->costs(0).cost
  };
  const AbilityCost cost2{
    ptr->costs(1).attribute.valid() ? attribLoader->getIntType(Type::get(ptr->costs(1).attribute.name())) : nullptr,
    ptr->costs(1).cost
  };
  const AbilityCost cost3{
    ptr->costs(2).attribute.valid() ? attribLoader->getIntType(Type::get(ptr->costs(2).attribute.name())) : nullptr,
    ptr->costs(2).cost
  };
  
  if (ptr->costs(0).attribute.valid() && cost1.type == nullptr) throw std::runtime_error("Could not find attribute type "+ptr->costs(0).attribute.name());
  if (ptr->costs(1).attribute.valid() && cost2.type == nullptr) throw std::runtime_error("Could not find attribute type "+ptr->costs(1).attribute.name());
  if (ptr->costs(2).attribute.valid() && cost3.type == nullptr) throw std::runtime_error("Could not find attribute type "+ptr->costs(2).attribute.name());
  
  const AbilityType::CreateInfo info{
    ptr->abilityId(),
    AbilityTypeT(ptr->entityCreateData().inheritTransform, ptr->entityCreateData().inheritEffects, ptr->entityCreateData().inheritAttributes, false),
    ptr->name(),
    ptr->description(),
    cost1,
    cost2,
    cost3,
    nullptr,
    ptr->entityCreateData().impactEvent,
    ptr->castTime(),
    ptr->cooldown(),
    ptr->entityCreateData().delayTime,
    // указатель на создание энтити, должен быть не указатель, а тип
    Type::get(ptr->entityCreateData().entityType.name()),
    transItr == transFuncs.end() ? nullptr : transItr->second,
    attribsItr == attribsFuncs.end() ? nullptr : attribsItr->second,
    effects,
    intAttribs,
    floatAttribs
  };
  auto ability = abilityTypePool.newElement(info);
  abilityTypePtr.push_back(ability);
  abilityTypes[info.abilityId] = ability;
  
  return true;
}

bool AbilityTypeLoader::unload(const ResourceID &id) {
  if (tempData == nullptr) throw std::runtime_error("Not in loading state");
  
  for (size_t i = 0; i < tempData->dataToLoad.size(); ++i) {
    if (tempData->dataToLoad[i]->id() == id) {
      std::swap(tempData->dataToLoad[i], tempData->dataToLoad.back());
      tempData->dataToLoad.pop_back();
      return true;
    }
  }
  
  return false;
}

void AbilityTypeLoader::end() {
  if (tempData == nullptr) throw std::runtime_error("Not in loading state");
  
//   for (auto ptr : tempData->dataToLoad) {
//     if (ptr->weapon().valid()) {
//       // мы должны найти айтем тайп, данные о энтити можно проигнорировать
//       
//       continue;
//     }
//     
//     auto transItr = transFuncs.find(ptr->entityCreateData().transformComputeFunction);
//     auto attribsItr = attribsFuncs.find(ptr->entityCreateData().attributesComputeFunction);
//     
//     // к этому моменту мы должны уже загрузить все необходимые эффекты
//     std::vector<const Effect*> effects(ptr->effects().size());
//     for (size_t i = 0; i < ptr->effects().size(); ++i) {
//       effects[i] = effectsLoader->getEffect(Type::get(ptr->effects()[i].name()));
//       if (effects[i] == nullptr) throw std::runtime_error("Could not find effect "+ptr->effects()[i].name());
//     }
//     
//     // нужно ли с помощью ресурсов обращаться к аттрибутам, или с помощью типов? лучше типами
//     std::vector<AbilityAttributeListElement<INT_ATTRIBUTE_TYPE>> intAttribs;
//     std::vector<AbilityAttributeListElement<FLOAT_ATTRIBUTE_TYPE>> floatAttribs;
//     for (const auto &attrib : ptr->entityCreateData().attributesList) {
//       auto floatAttrib = attribLoader->getFloatType(Type::get(attrib.attribute.name()));
//       if (floatAttrib != nullptr) {
//         floatAttribs.push_back({floatAttrib, FLOAT_ATTRIBUTE_TYPE(attrib.value)});
//         continue;
//       }
//       
//       auto intAttrib = attribLoader->getIntType(Type::get(attrib.attribute.name()));
//       if (intAttrib != nullptr) {
//         intAttribs.push_back({intAttrib, INT_ATTRIBUTE_TYPE(attrib.value)});
//         continue;
//       }
//       
//       throw std::runtime_error("Could not find attribute type "+attrib.attribute.name());
//     }
//     
//     const AbilityCost cost1{
//       ptr->costs(0).attribute.valid() ? attribLoader->getIntType(Type::get(ptr->costs(0).attribute.name())) : nullptr,
//       ptr->costs(0).cost
//     };
//     const AbilityCost cost2{
//       ptr->costs(1).attribute.valid() ? attribLoader->getIntType(Type::get(ptr->costs(1).attribute.name())) : nullptr,
//       ptr->costs(1).cost
//     };
//     const AbilityCost cost3{
//       ptr->costs(2).attribute.valid() ? attribLoader->getIntType(Type::get(ptr->costs(2).attribute.name())) : nullptr,
//       ptr->costs(2).cost
//     };
//     
//     if (ptr->costs(0).attribute.valid() && cost1.type == nullptr) throw std::runtime_error("Could not find attribute type "+ptr->costs(0).attribute.name());
//     if (ptr->costs(1).attribute.valid() && cost2.type == nullptr) throw std::runtime_error("Could not find attribute type "+ptr->costs(1).attribute.name());
//     if (ptr->costs(2).attribute.valid() && cost3.type == nullptr) throw std::runtime_error("Could not find attribute type "+ptr->costs(2).attribute.name());
//     
//     const AbilityType::CreateInfo info{
//       ptr->abilityId(),
//       AbilityTypeT(ptr->entityCreateData().inheritTransform, ptr->entityCreateData().inheritEffects, ptr->entityCreateData().inheritAttributes, false),
//       ptr->name(),
//       ptr->description(),
//       cost1,
//       cost2,
//       cost3,
//       nullptr,
//       ptr->entityCreateData().impactEvent,
//       ptr->castTime(),
//       ptr->cooldown(),
//       ptr->entityCreateData().delayTime,
//       // указатель на создание энтити, должен быть не указатель, а тип
//       Type::get(ptr->entityCreateData().entityType.name()),
//       transItr == transFuncs.end() ? nullptr : transItr->second,
//       attribsItr == attribsFuncs.end() ? nullptr : attribsItr->second,
//       effects,
//       intAttribs,
//       floatAttribs
//     };
//     auto ability = abilityTypePool.newElement(info);
//     abilityTypePtr.push_back(ability);
//     abilityTypes[info.abilityId] = ability;
//   }
  
  // основная проблема это конечно перекрестное использование ресурсов
  // наверное стоит держать тип энтити в виде типа
}

void AbilityTypeLoader::clear() {
  delete tempData;
  tempData = nullptr;
}

size_t AbilityTypeLoader::overallState() const {
  
}

size_t AbilityTypeLoader::loadingState() const {
  
}

std::string AbilityTypeLoader::hint() const {
  
}

const AbilityType* AbilityTypeLoader::getAbilityType(const Type &id) const {
  auto itr = abilityTypes.find(id);
  if (itr == abilityTypes.end()) return nullptr;
  
  return itr->second;
}

size_t AbilityTypeLoader::findTempData(const ResourceID &id) const {
//   for (size_t i = 0; i < tempData->dataPtr.size(); ++i) {
//     if (tempData->dataPtr[i]->id() == id) return i;
//   }
//   
//   return SIZE_MAX;
}
