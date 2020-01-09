#include "ItemLoader.h"

#include "EffectsLoader.h"
#include "AbilityTypeLoader.h"

#include <iostream>
#include <ctype.h>

ItemTypeLoader::LoadData::LoadData(const CreateInfo &info) : Resource(info.resInfo), item(info.item), m_id(info.m_id), m_groupId(info.m_groupId), m_name(info.m_name), m_description(info.m_description), m_use(info.m_use), m_attacks(info.m_attacks) {}
Type ItemTypeLoader::LoadData::itemId() const { return m_id; }
Type ItemTypeLoader::LoadData::groupId() const { return m_groupId; }
std::string ItemTypeLoader::LoadData::name() const { return m_name; }
std::string ItemTypeLoader::LoadData::description() const { return m_description; }
bool ItemTypeLoader::LoadData::isItem() const { return item; }
ItemTypeLoader::LoadData::Ability ItemTypeLoader::LoadData::whenUse() const { return m_use; }
const std::vector<ItemTypeLoader::LoadData::Ability> & ItemTypeLoader::LoadData::whenAttack() const { return m_attacks; }

// Type ItemTypeLoader::LoadData::itemAbility() const { return m_itemAbility; }
// const std::vector<Type> & ItemTypeLoader::LoadData::pickupEffects() const { return m_pickupEffects; }
// const std::vector<Type> & ItemTypeLoader::LoadData::weaponEffects() const { return m_weaponEffects; }
// const std::vector<ItemTypeLoader::LoadData::AbilitySlot> & ItemTypeLoader::LoadData::abilities() const { return m_abilities; }

size_t parseSlot(const std::string &str) {
  if (str.length() < 5) return SIZE_MAX;
  
  auto n = str.find("slot");
  if (n == std::string::npos) return SIZE_MAX;
  
  if (!isdigit(str.c_str()[4])) return SIZE_MAX;
  
  size_t num = atol(&str.c_str()[4]);
  return num;
}

bool checkJsonItemValidity(const std::string &path, const nlohmann::json &json, const size_t &mark, ItemTypeLoader::LoadData::CreateInfo &info, std::vector<ErrorDesc> &errors, std::vector<WarningDesc> &warnings) {
  bool hasId = false, hasItemAbility = false, hasWeaponAbilities = false;
  info.item = false;
  
  for (auto itr = json.begin(); itr != json.end(); ++itr) {
    if (itr.value().is_string() && itr.key() == "id") {
      hasId = true;
      info.m_id = Type::get(itr.value().get<std::string>());
      continue;
    }
    
    if (itr.value().is_string() && itr.key() == "name") {
      info.m_name = itr.value().get<std::string>();
      continue;
    }
    
    if (itr.value().is_string() && itr.key() == "description") {
      info.m_description = itr.value().get<std::string>();
      continue;
    }
    
    if (itr.value().is_string() && itr.key() == "item_group") {
      info.m_groupId = Type::get(itr.value().get<std::string>());
      continue;
    }
    
    if (itr.value().is_boolean() && itr.key() == "is_item") {
      info.item = itr.value().get<bool>();
      continue;
    }
    
    if (itr.value().is_object() && itr.key() == "on_use") {
      hasItemAbility = true;
      
      for (auto itemData = itr.value().begin(); itemData != itr.value().end(); ++itemData) {
        if (itemData.value().is_string() && itemData.key() == "ability") {
          info.m_use.type = ResourceID::get(itemData.value().get<std::string>());
          continue;
        }
        
        if (itemData.value().is_string() && itemData.key() == "state") {
          info.m_use.state = Type::get(itemData.value().get<std::string>());
          continue;
        }
      }
      
      if (!info.m_use.type.valid() && !info.m_use.state.valid()) {
        ErrorDesc desc(mark, ItemTypeLoader::ERROR_BAD_ON_USE_DATA, "Item on use data must contain ability type or state id");
        std::cout << "Error: " << desc.description << "\n";
        errors.push_back(desc);
      }
      
      continue;
    }
    
    if (itr.value().is_object() && itr.key() == "on_attack") {
      hasWeaponAbilities = true;
      
      for (auto attackData = itr.value().begin(); attackData != itr.value().end(); ++attackData) {
        info.m_attacks.push_back({ResourceID::get(attackData.value().get<std::string>()), Type::get(attackData.key())});
      }
      continue;
    }
    
//     if (itr.value().is_array() && itr.key() == "pickup_effects") {
//       for (size_t i = 0; i < itr.value().size(); ++i) {
//         info.m_pickupEffects.push_back(Type::get(itr.value()[i].get<std::string>()));
//       }
//     }
//     
//     if (itr.value().is_array() && itr.key() == "weapon_effects") {
//       for (size_t i = 0; i < itr.value().size(); ++i) {
//         info.m_weaponEffects.push_back(Type::get(itr.value()[i].get<std::string>()));
//       }
//     }
//     
//     if (itr.value().is_object() && itr.key() == "abilities") {
//       for (auto abilityItr = itr.value().begin(); abilityItr != itr.value().end(); ++abilityItr) {
//         if (!abilityItr.value().is_object()) {
//           ErrorDesc desc(mark, ItemTypeLoader::ERROR_ABILITY_SLOT_IS_NOT_VALID, "Could not parse ability slot "+abilityItr.key());
//           std::cout << "Error: " << desc.description << "\n";
//           errors.push_back(desc);
//           continue;
//         }
//         
//         size_t index = parseSlot(abilityItr.key());
//         if (index == SIZE_MAX) {
//           ErrorDesc desc(mark, ItemTypeLoader::ERROR_SLOT_KEY_IS_NOT_VALID, "Could not parse slot index "+abilityItr.key());
//           std::cout << "Error: " << desc.description << "\n";
//           errors.push_back(desc);
//           continue;
//         }
//         
//         bool hasAbilityType = false;
//         ItemTypeLoader::LoadData::AbilitySlot slotInfo{
//           index,
//           Type(),
//           Type()
//         };
//         for (auto slot = abilityItr.value().begin(); slot != abilityItr.value().end(); ++slot) {
//           if (slot.value().is_string() && slot.key() == "type") {
//             hasAbilityType = true;
//             slotInfo.ability = Type::get(slot.value().get<std::string>());
//           }
//           
//           if (slot.value().is_string() && slot.key() == "state") {
//             slotInfo.state = Type::get(slot.value().get<std::string>());
//           }
//         }
//         
//         if (!hasAbilityType) {
//           ErrorDesc desc(mark, ItemTypeLoader::ERROR_BAD_SLOT_ABILITY_TYPE, "Could not find ability type in slot "+abilityItr.key());
//           std::cout << "Error: " << desc.description << "\n";
//           errors.push_back(desc);
//           continue;
//         }
//         
//         info.m_abilities.push_back(slotInfo);
//       }
//     }
  }
  
  if (!hasId) {
    ErrorDesc desc(mark, ItemTypeLoader::ERROR_ITEM_ID_IS_NOT_SPECIFIED, "Item id must be specified");
    std::cout << "Error: " << desc.description << "\n";
    errors.push_back(desc);
    return false;
  }
  
  if (!info.item && !hasItemAbility && !hasWeaponAbilities) {
    ErrorDesc desc(mark, ItemTypeLoader::ERROR_BAD_ITEM_TYPE_DATA, "Item on use or on attack data must be specified");
    std::cout << "Error: " << desc.description << "\n";
    errors.push_back(desc);
    return false;
  }
  
  if (info.item && !hasItemAbility) {
    ErrorDesc desc(mark, ItemTypeLoader::ERROR_BAD_ITEM_TYPE_DATA, "Item on use data must be specified");
    std::cout << "Error: " << desc.description << "\n";
    errors.push_back(desc);
    return false;
  }
  
  if (info.item && !info.m_attacks.empty()) {
    WarningDesc desc(mark, ItemTypeLoader::WARNING_ATTACKS_DATA_IS_IGNORED, "On attack data is ignored for item");
    std::cout << "Warning: " << desc.description << "\n";
    warnings.push_back(desc);
  }
  
//   if (hasItemAbility && hasWeaponAbilities) {
//     WarningDesc desc(mark, ItemTypeLoader::WARNING_TOO_MUCH_DATA_WEAPON_PART_IS_IGNORED, "Item data contain item_ability and weapon abilities. Weapon part would be ignored");
//     std::cout << "Warning: " << desc.description << "\n";
//     warnings.push_back(desc);
//   }
  
  return true;
}

ItemTypeLoader::ItemTypeLoader(const CreateInfo &info) : abilityTypeLoader(info.abilityTypeLoader), effectsLoader(info.effectsLoader), tempData(nullptr) {}
ItemTypeLoader::~ItemTypeLoader() {
  clear();
  
  for (auto ptr : itemPointers) {
    itemPool.deleteElement(ptr);
  }
}

bool ItemTypeLoader::canParse(const std::string &key) const {
  return key == "items";
}

bool ItemTypeLoader::parse(const Modification* mod,
                           const std::string &pathPrefix,
                           const nlohmann::json &data,
                           std::vector<Resource*> &resource,
                           std::vector<ErrorDesc> &errors,
                           std::vector<WarningDesc> &warnings) {
  if (tempData == nullptr) tempData = new LoadingTemporaryData<LoadData, 50>;
  
  if (data.is_string()) {
    const std::string &path = data.get<std::string>();
    std::ifstream file(pathPrefix + path);
    if (!file) {
      ErrorDesc desc(1251124, ERROR_FILE_NOT_FOUND, "Could not load file "+pathPrefix+path);
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
    info.resInfo.resSize = sizeof(ItemType);
    
    bool ret = checkJsonItemValidity(pathPrefix, data, 1251124, info, errors, warnings);
    if (!ret) return false;
    
    info.resInfo.resId = ResourceID::get(info.m_id.name());
    auto ptr = tempData->container.create(info.resInfo.resId, info);
    resource.push_back(ptr);
    return true;
  }
  
  return false;
}

bool ItemTypeLoader::forget(const ResourceID &name) {
  if (tempData == nullptr) throw std::runtime_error("Not in loading state");
  
//   const size_t index = findTempData(name);
//   if (index == SIZE_MAX) return false;
//   
//   tempData->dataPool.deleteElement(tempData->dataPtr[index]);
//   std::swap(tempData->dataPtr[index], tempData->dataPtr.back());
//   tempData->dataPtr.pop_back();
//   
//   return true;
  
  return tempData->container.destroy(name);
}

Resource* ItemTypeLoader::getParsedResource(const ResourceID &id) {
  if (tempData == nullptr) throw std::runtime_error("Not in loading state");
  
//   const size_t index = findTempData(id);
//   return index == SIZE_MAX ? nullptr : tempData->dataPtr[index];
  return tempData->container.get(id);
}

const Resource* ItemTypeLoader::getParsedResource(const ResourceID &id) const {
  if (tempData == nullptr) throw std::runtime_error("Not in loading state");
  
//   const size_t index = findTempData(id);
//   return index == SIZE_MAX ? nullptr : tempData->dataPtr[index];
  return tempData->container.get(id);
}

bool ItemTypeLoader::load(const ModificationParser* modifications, const Resource* resource) {
  if (tempData == nullptr) throw std::runtime_error("Not in loading state");
  
  const Type id = Type::get(resource->id().name());
  auto item = getItemType(id);
  if (item != nullptr) return true;
  
//   const size_t index = findTempData(resource->id());
//   if (index == SIZE_MAX) return false;
  
  //tempData->dataToLoad.push_back(tempData->dataPtr[index]);
  
  auto loadData = tempData->container.get(resource->id());
  if (loadData == nullptr) return false;
  
  const AbilityType* onUseAbility = nullptr;
  if (loadData->whenUse().type.valid()) {
    const bool ret = abilityTypeLoader->load(nullptr, abilityTypeLoader->getParsedResource(loadData->whenUse().type));
    if (!ret) throw std::runtime_error("Could not load ability "+loadData->whenUse().type.name());
    
    const Type id = Type::get(loadData->whenUse().type.name());
    onUseAbility = abilityTypeLoader->getAbilityType(id);
    if (onUseAbility == nullptr) throw std::runtime_error("Could not find ability "+loadData->whenUse().type.name());
  }
  
  std::vector<ItemType::Ability> abilities(loadData->whenAttack().size());
  for (size_t i = 0; i < loadData->whenAttack().size(); ++i) {
    const bool ret = abilityTypeLoader->load(nullptr, abilityTypeLoader->getParsedResource(loadData->whenAttack()[i].type));
    if (!ret) throw std::runtime_error("Could not load ability "+loadData->whenAttack()[i].type.name());
    
    const Type id = Type::get(loadData->whenAttack()[i].type.name());
    abilities[i].ability = abilityTypeLoader->getAbilityType(id);
    if (abilities[i].ability == nullptr) throw std::runtime_error("Could not find ability "+loadData->whenAttack()[i].type.name());
    abilities[i].state = loadData->whenAttack()[i].state;
  }
  
//   if (loadData->itemAbility().valid()) {
//     const bool ret = abilityTypeLoader->load(nullptr, abilityTypeLoader->getParsedResource(ResourceID::get(loadData->itemAbility().name())));
//     if (!ret) throw std::runtime_error("Could not load ability "+loadData->itemAbility().name());
//     
//     auto ability = abilityTypeLoader->getAbilityType(loadData->itemAbility());
//     if (ability == nullptr) throw std::runtime_error("Could not find ability type "+loadData->itemAbility().name());
//     
//     std::vector<const Effect*> effects(loadData->pickupEffects().size());
//     for (size_t i = 0; i < loadData->pickupEffects().size(); ++i) {
//       const bool ret = effectsLoader->load(nullptr, effectsLoader->getParsedResource(ResourceID::get(loadData->pickupEffects()[i].name())));
//       if (!ret) throw std::runtime_error("Could not load effect "+loadData->pickupEffects()[i].name());
//       
//       effects[i] = effectsLoader->getEffect(loadData->pickupEffects()[i]);
//       if (effects[i] == nullptr) throw std::runtime_error("Could not find effect "+loadData->pickupEffects()[i].name());
//     }
//     
//     const ItemType::CreateInfo info{
//       loadData->itemId(),
//       loadData->groupId(),
//       ability,
//       loadData->name(),
//       loadData->description(),
//       effects,
//       {},
//       {}
//     };
//     auto ptr = itemPool.newElement(info);
//     itemPointers.push_back(ptr);
//     itemTypes[loadData->itemId()] = ptr;
//     return true;
//   }
  
//   std::vector<ItemType::AbilitySlot> slots(loadData->abilities().size());
//   for (size_t i = 0; i < loadData->abilities().size(); ++i) {
//     const bool ret = abilityTypeLoader->load(nullptr, abilityTypeLoader->getParsedResource(ResourceID::get(loadData->abilities()[i].ability.name())));
//     if (!ret) throw std::runtime_error("Could not load ability "+loadData->abilities()[i].ability.name());
//     
//     auto ability = abilityTypeLoader->getAbilityType(loadData->abilities()[i].ability);
//     if (ability == nullptr) throw std::runtime_error("Could not find ability type "+loadData->abilities()[i].ability.name());
//     slots[i] = {
//       loadData->abilities()[i].slot,
//       ability,
//       loadData->abilities()[i].state
//     };
//   }
//   
//   std::vector<const Effect*> pickupEffects(loadData->pickupEffects().size());
//   for (size_t i = 0; i < loadData->pickupEffects().size(); ++i) {
//     const bool ret = effectsLoader->load(nullptr, effectsLoader->getParsedResource(ResourceID::get(loadData->pickupEffects()[i].name())));
//     if (!ret) throw std::runtime_error("Could not load effect "+loadData->pickupEffects()[i].name());
//     
//     pickupEffects[i] = effectsLoader->getEffect(loadData->pickupEffects()[i]);
//     if (pickupEffects[i] == nullptr) throw std::runtime_error("Could not find effect "+loadData->pickupEffects()[i].name());
//   }
//   
//   std::vector<const Effect*> weaponEffects(loadData->weaponEffects().size());
//   for (size_t i = 0; i < loadData->weaponEffects().size(); ++i) {
//     const bool ret = effectsLoader->load(nullptr, effectsLoader->getParsedResource(ResourceID::get(loadData->weaponEffects()[i].name())));
//     if (!ret) throw std::runtime_error("Could not load effect "+loadData->weaponEffects()[i].name());
//     
//     weaponEffects[i] = effectsLoader->getEffect(loadData->weaponEffects()[i]);
//     if (weaponEffects[i] == nullptr) throw std::runtime_error("Could not find effect "+loadData->weaponEffects()[i].name());
//   }
  
  const ItemType::CreateInfo info{
    loadData->itemId(),
    loadData->groupId(),
    loadData->name(),
    loadData->description(),
    loadData->isItem(),
    {onUseAbility, loadData->whenUse().state},
    abilities
  };
  auto ptr = itemPool.newElement(info);
  itemPointers.push_back(ptr);
  itemTypes[loadData->itemId()] = ptr;
  
  return true;
}

bool ItemTypeLoader::unload(const ResourceID &id) {
  //if (tempData == nullptr) throw std::runtime_error("Not in loading state");
  
  const Type itemId = Type::get(id.name());
  for (size_t i = 0; i < itemPointers.size(); ++i) {
    if (itemPointers[i]->id() == itemId) {
      itemPool.deleteElement(itemPointers[i]);
      itemTypes.erase(itemId);
      std::swap(itemPointers[i], itemPointers.back());
      itemPointers.pop_back();
      return true;
    }
  }
  
//   for (size_t i = 0; i < tempData->dataToLoad.size(); ++i) {
//     if (tempData->dataToLoad[i]->id() == id) {
//       std::swap(tempData->dataToLoad[i], tempData->dataToLoad.back());
//       tempData->dataToLoad.pop_back();
//       return true;
//     }
//   }
  
  return false;
}

void ItemTypeLoader::end() {
  if (tempData == nullptr) throw std::runtime_error("Not in loading state");

}

void ItemTypeLoader::clear() {
  delete tempData;
  tempData = nullptr;
}

size_t ItemTypeLoader::overallState() const {
  
}

size_t ItemTypeLoader::loadingState() const {
  
}

std::string ItemTypeLoader::hint() const {
  
}

const ItemType* ItemTypeLoader::getItemType(const Type &id) const {
  auto itr = itemTypes.find(id);
  if (itr == itemTypes.end()) return nullptr;
  
  return itr->second;
}

size_t ItemTypeLoader::findTempData(const ResourceID &id) const {
//   for (size_t i = 0; i < tempData->dataPtr.size(); ++i) {
//     if (tempData->dataPtr[i]->id() == id) return i;
//   }
//   
//   return SIZE_MAX;
}
