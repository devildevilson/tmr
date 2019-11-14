#include "EffectsLoader.h"

#include <fstream>
#include <iostream>

#include "AttributesLoader.h"

EffectsLoader::LoadData::LoadData(const CreateInfo &info) 
  : Resource(info.resInfo), 
    m_id(info.m_id), 
    m_name(info.m_name), 
    m_description(info.m_description), 
    m_baseEffectTime(info.m_baseEffectTime), 
    m_periodTime(info.m_periodTime), 
    m_bonuses(info.m_bonuses), 
    m_mods(info.m_mods), 
    m_type(info.m_type),
    m_computeFuncPath(info.m_computeFuncPath), 
    m_resistFuncPath(info.m_resistFuncPath) {}
    
Type EffectsLoader::LoadData::effectId() const { return m_id; }
std::string EffectsLoader::LoadData::name() const { return m_name; }
std::string EffectsLoader::LoadData::description() const { return m_description; }
size_t EffectsLoader::LoadData::baseEffectTime() const { return m_baseEffectTime; }
size_t EffectsLoader::LoadData::periodTime() const { return m_periodTime; }
const std::vector<EffectsLoader::LoadData::BonusType> & EffectsLoader::LoadData::bonuses() const { return m_bonuses; }
const std::vector<EffectsLoader::LoadData::Modificator> & EffectsLoader::LoadData::mods() const { return m_mods; }
EffectType EffectsLoader::LoadData::type() const { return m_type; }
std::string EffectsLoader::LoadData::computeFuncPath() const { return m_computeFuncPath; }
std::string EffectsLoader::LoadData::resistFuncPath() const { return m_resistFuncPath; }

enum ErrorTypes {
  ERROR_WRONG_DATA_TYPE = 0,
  ERROR_MISSED_EFFECT_ID,
  ERROR_MISSED_EFFECT_TIME
};

enum WarningTypes {
  WARNING_MISSED_EFFECT_NAME = 0,
  WARNING_MISSED_EFFECT_DESCRIPTION,
};

bool checkEffectJsonValidity(const std::string &pathPrefix, const std::string &path, const nlohmann::json &data, const size_t &mark, EffectsLoader::LoadData::CreateInfo &info, std::vector<ErrorDesc> &errors, std::vector<WarningDesc> &warnings) {
  bool hasId = false, hasName = false, hasDesc = false, hasTime = false;
  bool raw_add = false, effect_add = false, periodic = false, one_time_effect = false, resetable = false, stackable = false, easy_stack = false;
  
  for (auto concreteTIt = data.begin(); concreteTIt != data.end(); ++concreteTIt) {
    if (concreteTIt.value().is_string() && concreteTIt.key() == "id") {
      hasId = true;
      info.m_id = Type::get(concreteTIt.value().get<std::string>());
    }
    
    if (concreteTIt.value().is_string() && concreteTIt.key() == "name") {
      hasName = true;
      info.m_name = concreteTIt.value().get<std::string>();
    }
    
    if (concreteTIt.value().is_string() && concreteTIt.key() == "description") {
      hasDesc = true;
      info.m_description = concreteTIt.value().get<std::string>();
    }
    
    if (concreteTIt.value().is_number_unsigned() && concreteTIt.key() == "base_effect_time") {
      hasTime = true;
      info.m_baseEffectTime = concreteTIt.value().get<size_t>();
    }
    
    if (concreteTIt.value().is_boolean() && concreteTIt.key() == "raw_attrib_add") {
      raw_add = concreteTIt.value().get<bool>();
    }
    
    if (concreteTIt.value().is_boolean() && concreteTIt.key() == "add_bonuses") {
      effect_add = concreteTIt.value().get<bool>();
    }
    
    if (concreteTIt.value().is_array() && concreteTIt.key() == "effect_bonuses") {
      for (size_t i = 0; i < concreteTIt.value().size(); ++i) {
        EffectsLoader::LoadData::BonusType bonus;
        for (auto itr = concreteTIt.value()[i].begin(); itr != concreteTIt.value()[i].end(); ++itr) {
          if (itr.value().is_string() && itr.key() == "attribute_type") {
            bonus.attribId = ResourceID::get(itr.value().get<std::string>());
          }
          
          if (itr.value().is_number() && itr.key() == "add") {
            bonus.bonus.add = itr.value().get<float>();
          }
          
          if (itr.value().is_number() && itr.key() == "mul") {
            bonus.bonus.mul = itr.value().get<float>();
          }
        }
        
        info.m_bonuses.push_back(bonus);
      }
    }
    
    if (concreteTIt.value().is_boolean() && concreteTIt.key() == "periodic") {
      periodic = concreteTIt.value().get<bool>();
    }
    
    if (concreteTIt.value().is_boolean() && concreteTIt.key() == "one_time_effect") {
      one_time_effect = concreteTIt.value().get<bool>();
    }
    
    if (concreteTIt.value().is_boolean() && concreteTIt.key() == "resetable") {
      resetable = concreteTIt.value().get<bool>();
    }
    
    if (concreteTIt.value().is_boolean() && concreteTIt.key() == "stackable") {
      stackable = concreteTIt.value().get<bool>();
    }
    
    if (concreteTIt.value().is_boolean() && concreteTIt.key() == "easy_stack") {
      easy_stack = concreteTIt.value().get<bool>();
    }
    
    if (concreteTIt.value().is_number_unsigned() && concreteTIt.key() == "base_period_time") {
      info.m_periodTime = concreteTIt.value().get<size_t>();
    }
    
    if (concreteTIt.value().is_array() && concreteTIt.key() == "modificators") {
      for (size_t i = 0; i < concreteTIt.value().size(); ++i) {
        EffectsLoader::LoadData::Modificator mod;
        for (auto itr = concreteTIt.value()[i].begin(); itr != concreteTIt.value()[i].end(); ++itr) {
          if (itr.value().is_string() && itr.key() == "event") {
            mod.event = Type::get(itr.value().get<std::string>());
          }
          
          if (itr.value().is_number() && itr.key() == "effect") {
            mod.effectId = ResourceID::get(itr.value().get<std::string>());
          }
        }
        
        info.m_mods.push_back(mod);
      }
    }
    
    if (concreteTIt.value().is_string() && concreteTIt.key() == "compute_function") {
      info.m_computeFuncPath = concreteTIt.value().get<std::string>();
    }
    
    if (concreteTIt.value().is_string() && concreteTIt.key() == "resist_function") {
      info.m_resistFuncPath = concreteTIt.value().get<std::string>();
    }
  }
  
  size_t errorSize = errors.size();
  
  if (!hasId) {
    ErrorDesc desc(mark, ERROR_MISSED_EFFECT_ID, "Effect must have an id. File: "+path);
    std::cout << "Error: " << desc.description << "\n";
    errors.push_back(desc);
  }
  
  if (!hasName) {
    WarningDesc desc(mark, WARNING_MISSED_EFFECT_NAME, "Missing effect name. File: "+path);
    std::cout << "Warning: " << desc.description << "\n";
    warnings.push_back(desc);
  }
  
  if (!hasDesc) {
    WarningDesc desc(mark, WARNING_MISSED_EFFECT_DESCRIPTION, "Missing effect description. File: "+path);
    std::cout << "Warning: " << desc.description << "\n";
    warnings.push_back(desc);
  }
  
  if (!hasTime && !one_time_effect) {
    ErrorDesc desc(mark, ERROR_MISSED_EFFECT_ID, "Missing effect time. File: "+path);
    std::cout << "Error: " << desc.description << "\n";
    errors.push_back(desc);
  }
  
  if (errorSize != errors.size()) return false;
  
  info.m_type = EffectType(raw_add, effect_add, info.m_baseEffectTime != 0, periodic, !info.m_computeFuncPath.empty(), !info.m_resistFuncPath.empty(), one_time_effect, resetable, stackable, easy_stack);
  
  return true;
}

EffectsLoader::EffectsLoader(const CreateInfo &info) : tempData(nullptr), attributesLoader(info.attributesLoader), hardcodedComputeFunc(info.hardcodedComputeFunc), hardcodedResistFunc(info.hardcodedResistFunc) {}
EffectsLoader::~EffectsLoader() {}

bool EffectsLoader::canParse(const std::string &key) const {
  return key == "effects";
}

bool EffectsLoader::parse(const Modification* mod,
                          const std::string &pathPrefix,
                          const nlohmann::json &data,
                          std::vector<Resource*> &resource,
                          std::vector<ErrorDesc> &errors,
                          std::vector<WarningDesc> &warnings) {
  if (tempData == nullptr) tempData = new TemporaryData;
  
  // у меня есть примерное понимание что тут нужно грузануть
  if (data.is_array()) {
    for (auto itr = data.begin(); itr != data.end(); ++itr) {
      if (itr.value().is_string()) {
        std::ifstream file(pathPrefix + itr.value().get<std::string>());
        nlohmann::json json;
        file >> json;
        parse(mod, pathPrefix, json, resource, errors, warnings);
      } else if (itr.value().is_object()) {
        // надо понять что мне будет удобнее всего передать в валидацию, чтоб отделить данные друг от друга
        //по идее нужно передать строку и путь до файла в котором эта строка
        EffectsLoader::LoadData::CreateInfo info = {};
        const bool valid = checkEffectJsonValidity(pathPrefix, "", itr.value(), 1234, info, errors, warnings);
        if (valid) {
          info.resInfo.resId = ResourceID(info.m_id.name());
          info.resInfo.pathStr = "";
          info.resInfo.resSize = sizeof(Effect);
          info.resInfo.resGPUSize = 0;
          info.resInfo.parsedBy = this;
          info.resInfo.mod = mod;
          info.m_baseEffectTime = info.m_baseEffectTime == 0 ? SIZE_MAX : info.m_baseEffectTime;
          
          auto loadData = tempData->dataPool.newElement(info);
          tempData->datasPtr.push_back(loadData);
        }
      }
    }
    
    return true;
  } else if (data.is_object()) {
    EffectsLoader::LoadData::CreateInfo info = {};
    const bool valid = checkEffectJsonValidity(pathPrefix, "", data, 1234, info, errors, warnings);
    if (valid) {
      info.resInfo.resId = ResourceID(info.m_id.name());
      info.resInfo.pathStr = "";
      info.resInfo.resSize = sizeof(Effect);
      info.resInfo.resGPUSize = 0;
      info.resInfo.parsedBy = this;
      info.resInfo.mod = mod;
      info.m_baseEffectTime = info.m_baseEffectTime == 0 ? SIZE_MAX : info.m_baseEffectTime;
      
      auto loadData = tempData->dataPool.newElement(info);
      tempData->datasPtr.push_back(loadData);
    }
    
    return true;
  }
  
  ErrorDesc desc(1234, ERROR_WRONG_DATA_TYPE, "Wrong effects data type");
  errors.push_back(desc);
  return false;
}

bool EffectsLoader::forget(const ResourceID &name) {
  if (tempData == nullptr) throw std::runtime_error("Not in loading state");
  
  for (size_t i = 0; i < tempData->datasPtr.size(); ++i) {
    if (tempData->datasPtr[i]->id() == name) {
      tempData->dataPool.deleteElement(tempData->datasPtr[i]);
      std::swap(tempData->datasPtr[i], tempData->datasPtr.back());
      tempData->datasPtr.pop_back();
      return true;
    }
  }
  
  return false;
}

Resource* EffectsLoader::getParsedResource(const ResourceID &id) {
  if (tempData == nullptr) throw std::runtime_error("Not in loading state");
  
  for (size_t i = 0; i < tempData->datasPtr.size(); ++i) {
    if (tempData->datasPtr[i]->id() == id) return tempData->datasPtr[i];
  }
  
  return nullptr;
}

const Resource* EffectsLoader::getParsedResource(const ResourceID &id) const {
  if (tempData == nullptr) throw std::runtime_error("Not in loading state");
  
  for (size_t i = 0; i < tempData->datasPtr.size(); ++i) {
    if (tempData->datasPtr[i]->id() == id) return tempData->datasPtr[i];
  }
  
  return nullptr;
}

bool EffectsLoader::load(const ModificationParser* modifications, const Resource* resource) {
  if (tempData == nullptr) throw std::runtime_error("Not in loading state");
  
  // проверяем загрузили ли мы уже этот ресурс
  // проверяем есть ли распарсенный эффект для модификатора
  
  if (resource->parser() != this) return false;
  
  Type id = Type::get(resource->id().name());
  auto e = getEffect(id);
  if (e != nullptr) return true;
  
  LoadData* res = nullptr;
  for (size_t i = 0; i < tempData->datasPtr.size(); ++i) {
    if (tempData->datasPtr[i] == resource) res = tempData->datasPtr[i];
  }
  
  if (res == nullptr) return false;
  
  // нужно переделать так чтобы это дело вычислялось в парсинге
  // это я должен видимо делать при валидации
  for (const auto &mod : res->mods()) {
    auto ptr = getParsedResource(mod.effectId);
    if (ptr == nullptr) throw std::runtime_error("Could not find effect modificator with id: "+mod.effectId.name());
  }
  
  std::vector<BonusType> bonuses(res->bonuses().size());
  for (size_t i = 0; i < res->bonuses().size(); ++i) {
    const bool ret = attributesLoader->load(nullptr, attributesLoader->getParsedResource(res->bonuses()[i].attribId));
    if (!ret) throw std::runtime_error("Could not load attribute "+res->bonuses()[i].attribId.name());
    
    Type attribId = Type::get(res->bonuses()[i].attribId.name());
    auto attribFloat = attributesLoader->getFloatType(attribId);
    if (attribFloat != nullptr) {
      bonuses[i].bonus = res->bonuses()[i].bonus;
      bonuses[i].type = TypelessAttributeType(attribFloat);
      continue;
    }
    
    auto attribInt = attributesLoader->getIntType(attribId);
    if (attribInt != nullptr) {
      bonuses[i].bonus = res->bonuses()[i].bonus;
      bonuses[i].type = TypelessAttributeType(attribInt);
      continue;
    }
    
    throw std::runtime_error("Could not find attribute "+attribId.name());
  }
  
  auto computeItr = hardcodedComputeFunc.find(res->computeFuncPath());
  auto resistItr = hardcodedResistFunc.find(res->resistFuncPath());
  
  const Effect::CreateInfo info{
    res->effectId(),
    res->type(),
    res->baseEffectTime(),
    res->periodTime(),
    res->name(),
    res->description(),
    bonuses,
    // для того чтобы заполнить модификаторы, нужно сначала создать эффекты
    // функции пока что будем хардкодить
    computeItr != hardcodedComputeFunc.end() ? computeItr->second : nullptr,
    resistItr != hardcodedResistFunc.end() ? resistItr->second : nullptr
  };
  auto effect = effectsPool.newElement(info);
  effectsPtr.push_back(effect);
  effects[effect->id()] = effect;
  
  return true;
}

bool EffectsLoader::unload(const ResourceID &id) {
  if (tempData == nullptr) throw std::runtime_error("Not in loading state");
  
  Type type = Type::get(id.name());
  for (size_t i = 0; i < effectsPtr.size(); ++i) {
    if (type == effectsPtr[i]->id()) {
      effectsPool.deleteElement(effectsPtr[i]);
      effects.erase(type);
      std::swap(effectsPtr[i], effectsPtr.back());
      effectsPtr.pop_back();
      return true;
    }
  }
  
//   for (size_t i = 0; i < tempData->datasPtr.size(); ++i) {
//     if (tempData->datasPtr[i]->id() == id) {
//       tempData->dataPool.deleteElement(tempData->datasPtr[i]);
//       std::swap(tempData->datasPtr[i], tempData->datasPtr.back());
//       tempData->datasPtr.pop_back();
//       return true;
//     }
//   }
  
  return false;
}

void EffectsLoader::end() {
  if (tempData == nullptr) throw std::runtime_error("Not in loading state");
  
//   for (auto effectRes : tempData->datasPtr) {
//     
//   }
  
  for (auto effectRes : tempData->datasPtr) {
    std::vector<Effect::EventModificator> mods(effectRes->mods().size());
    for (size_t i = 0; i < effectRes->mods().size(); ++i) {
      const bool ret = load(nullptr, getParsedResource(effectRes->mods()[i].effectId));
      if (!ret) throw std::runtime_error("Could not load effect "+effectRes->mods()[i].effectId.name());
      
      const Type id = Type::get(effectRes->mods()[i].effectId.name());
      auto itr = effects.find(id);
      if (itr == effects.end()) throw std::runtime_error("Could not load effect "+effectRes->mods()[i].effectId.name());
      mods[i] = {effectRes->mods()[i].event, itr->second};
    }
    
    // добавляем модификаторы в эффект
    for (auto effectPtr : effectsPtr) {
      if (effectPtr->id() == effectRes->effectId()) {
        effectPtr->setModificators(mods);
        break;
      }
    }
  }
}

void EffectsLoader::clear() {
  delete tempData;
}

size_t EffectsLoader::overallState() const {
  
}

size_t EffectsLoader::loadingState() const {
  
}

std::string EffectsLoader::hint() const {
  
}

const Effect* EffectsLoader::getEffect(const Type &id) const {
  auto itr = effects.find(id);
  if (itr == effects.end()) throw std::runtime_error("Could not find effect "+id.name());
  
  return itr->second;
}

EffectsLoader::LoadData* EffectsLoader::getEffectResource(const Type &effectId) const {
  if (tempData == nullptr) throw std::runtime_error("Not in loading state");
  
  for (size_t i = 0; i < tempData->datasPtr.size(); ++i) {
    if (tempData->datasPtr[i]->effectId() == effectId) return tempData->datasPtr[i];
  }
  
  return nullptr;
}

EffectsLoader::TemporaryData::~TemporaryData() {
  for (size_t i = 0; i < datasPtr.size(); ++i) {
    dataPool.deleteElement(datasPtr[i]);
  }
}
