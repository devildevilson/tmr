#include "EntityLoader.h"

#include "EntityCreators.h"
#include "AbilityTypeLoader.h"
#include "ItemLoader.h"
#include "AttributesLoader.h"
#include "EffectsLoader.h"
#include "ImageLoader.h"

EntityLoader::LoadData::LoadData(const CreateInfo &info) : Resource(info.resInfo), m_id(info.m_id), m_physData(info.m_physData), m_graphicsData(info.m_graphicsData), m_attributesData(info.m_attributesData), m_effectsData(info.m_effectsData), m_inventoryData(info.m_inventoryData), m_weaponsData(info.m_weaponsData), m_abilitiesData(info.m_abilitiesData), m_intelligence(info.m_intelligence), m_statesData(info.m_statesData), m_defaultTexture(info.m_defaultTexture) {}
Type EntityLoader::LoadData::creatorId() const { return m_id; }
EntityLoader::LoadData::PhysData EntityLoader::LoadData::physData() const { return m_physData; }
EntityLoader::LoadData::GraphicsData EntityLoader::LoadData::graphicsData() const { return m_graphicsData; }
EntityLoader::LoadData::AttributesData EntityLoader::LoadData::attributesData() const { return m_attributesData; }
EntityLoader::LoadData::Effects EntityLoader::LoadData::effectsData() const { return m_effectsData; }
EntityLoader::LoadData::Inventory EntityLoader::LoadData::inventoryData() const { return m_inventoryData; }
EntityLoader::LoadData::Weapons EntityLoader::LoadData::weaponsData() const { return m_weaponsData; }
EntityLoader::LoadData::Abilities EntityLoader::LoadData::abilitiesData() const { return m_abilitiesData; }
EntityLoader::LoadData::Intelligence EntityLoader::LoadData::intelligence() const { return m_intelligence; }
EntityLoader::LoadData::States EntityLoader::LoadData::statesData() const { return m_statesData; }
EntityLoader::LoadData::Texture EntityLoader::LoadData::defaultTexture() const { return m_defaultTexture; }

bool checkJsonEntityValidity(const std::string &path, const nlohmann::json &data, const size_t &mark, EntityLoader::LoadData::CreateInfo &info, std::vector<ErrorDesc> &errors, std::vector<WarningDesc> &warnings) {
  bool hasId = false, hasStates = false, hasTexture = false, hasPhysicsData = false, hasImage = false;
  
  const size_t errorCount = errors.size();
  
  for (auto itr = data.begin(); itr != data.end(); ++itr) {
    if (itr.value().is_string() && itr.key() == "id") {
      hasId = true;
      info.resInfo.resId = ResourceID::get(itr.value().get<std::string>());
      info.m_id = Type::get(itr.value().get<std::string>());
      
      continue;
    }
    
    if (itr.value().is_object() && itr.key() == "attributes") {
      for (auto attrib = itr.value().begin(); attrib != itr.value().end(); ++attrib) {
        if (!attrib.value().is_number()) {
          ErrorDesc desc(4123, EntityLoader::ERROR_BAD_ATTRIBUTE_VALUE, "Bad attribute type");
          std::cout << "Error: " << desc.description << "\n";
          errors.push_back(desc);
          continue;
        }
        
        info.m_attributesData.attribs.push_back({Type::get(attrib.key()), attrib.value().get<double>()});
      }
      
      continue;
    }
    
    if (itr.value().is_object() && itr.key() == "effects") {
      for (auto effect = itr.value().begin(); effect != itr.value().end(); ++effect) {
        info.m_effectsData.eventEffects.push_back(std::make_pair(Type::get(effect.key()), Type::get(effect.value().get<std::string>())));
      }
      
      continue;
    }
    
    if (itr.value().is_object() && itr.key() == "weapons") {
      // ????
    }
    
    if (itr.value().is_object() && itr.key() == "inventory") {
      for (auto item = itr.value().begin(); item != itr.value().end(); ++item) {
        if (!item.value().is_number_unsigned()) {
          ErrorDesc desc(4123, EntityLoader::ERROR_BAD_ITEM_COUNT_VALUE, "Bad item count value");
          std::cout << "Error: " << desc.description << "\n";
          errors.push_back(desc);
          continue;
        }
        
        info.m_inventoryData.items.push_back({Type::get(item.key()), item.value().get<size_t>()});
      }
      
      continue;
    }
    
    if (itr.value().is_object() && itr.key() == "abilities") {
      for (auto slot = itr.value().begin(); slot != itr.value().end(); ++slot) {
        if (!slot.value().is_object()) {
          ErrorDesc desc(mark, EntityLoader::ERROR_ABILITY_SLOT_IS_NOT_VALID, "Could not parse ability slot "+slot.key());
          std::cout << "Error: " << desc.description << "\n";
          errors.push_back(desc);
          continue;
        }
        
        size_t index = parseSlot(slot.key());
        if (index == SIZE_MAX) {
          ErrorDesc desc(mark, EntityLoader::ERROR_SLOT_KEY_IS_NOT_VALID, "Could not parse slot index "+slot.key());
          std::cout << "Error: " << desc.description << "\n";
          errors.push_back(desc);
          continue;
        }
        
        bool hasAbilityType = false;
        EntityLoader::LoadData::Abilities::Slot slotInfo{
          index,
          Type(),
          Type()
        };
        for (auto abilitySlot = slot.value().begin(); slot != slot.value().end(); ++slot) {
          if (abilitySlot.value().is_string() && abilitySlot.key() == "type") {
            hasAbilityType = true;
            slotInfo.ability = Type::get(abilitySlot.value().get<std::string>());
            continue;
          }
          
          if (abilitySlot.value().is_string() && abilitySlot.key() == "state") {
            slotInfo.state = Type::get(abilitySlot.value().get<std::string>());
            continue;
          }
        }
        
        if (!hasAbilityType) {
          ErrorDesc desc(mark, EntityLoader::ERROR_BAD_SLOT_ABILITY_TYPE, "Could not find ability type in slot "+slot.key());
          std::cout << "Error: " << desc.description << "\n";
          errors.push_back(desc);
          continue;
        }
        
        info.m_abilitiesData.slots.push_back(slotInfo);
      }
      
      continue;
    }
    
    if (itr.value().is_object() && itr.key() == "intelligence") {
      bool hasBehaviourTree = false;
      
      for (auto intData = itr.value().begin(); intData != itr.value().end(); ++intData) {
        if (intData.value().is_string() && intData.key() == "behaviour") {
          hasBehaviourTree = true;
          info.m_intelligence.tree = Type::get(intData.value().get<std::string>());
          continue;
        }
      }
      
      if (!hasBehaviourTree) {
        ErrorDesc desc(mark, EntityLoader::ERROR_BAD_BEHAVIOUR_TREE_DATA, "Bad behaviour data");
        std::cout << "Error: " << desc.description << "\n";
        errors.push_back(desc);
      }
    }
    
    if (itr.value().is_object() && itr.key() == "states") {
      hasStates = true;
      for (auto state = itr.value().begin(); state != itr.value().end(); ++state) {
        if (!state.value().is_object()) {
          ErrorDesc desc(mark, EntityLoader::ERROR_BAD_STATE_DATA, "Bad state data");
          std::cout << "Error: " << desc.description << "\n";
          errors.push_back(desc);
          continue;
        }
        
        EntityLoader::LoadData::States::Data data = {
          Type::get(state.key()),
          0,
          Type(),
          Type(),
          false,
          false,
          false,
          ResourceID(),
          0,
          ResourceID(),
          0,
          false,
          true,
          0.0f
        };
        for (auto stateData = state.value().begin(); stateData != state.value().end(); ++stateData) {
          if (stateData.value().is_number_unsigned() && stateData.key() == "time") {
            data.time = stateData.value().get<size_t>();
            continue;
          }
          
          if (stateData.value().is_string() && stateData.key() == "next_state") {
            data.nextState = Type::get(stateData.value().get<std::string>());
            continue;
          }
          
          if (stateData.value().is_string() && stateData.key() == "speed_attribute") {
            data.speedAttribute = Type::get(stateData.value().get<std::string>());
            continue;
          }
          
          if (stateData.value().is_boolean() && stateData.key() == "loop") {
            data.loop = stateData.value().get<bool>();
            continue;
          }
          
          if (stateData.value().is_boolean() && stateData.key() == "blocked") {
            data.blocked = stateData.value().get<bool>();
            continue;
          }
          
          if (stateData.value().is_boolean() && stateData.key() == "blocked_movement") {
            data.blockedMovement = stateData.value().get<bool>();
            continue;
          }
          
          if (stateData.value().is_object() && stateData.key() == "animation") {
            for (auto animation = stateData.value().begin(); animation != stateData.value().end(); ++animation) {
              if (animation.value().is_string() && animation.key() == "id") {
                data.animationId = ResourceID::get(animation.value().get<std::string>());
                continue;
              }
              
              if (animation.value().is_number_unsigned() && animation.key() == "delay") {
                data.animationDelay = animation.value().get<size_t>();
                continue;
              }
            }
            
            continue;
          }
          
          if (stateData.value().is_object() && stateData.key() == "sound") {
            for (auto sound = stateData.value().begin(); sound != stateData.value().end(); ++sound) {
              if (sound.value().is_string() && sound.key() == "id") {
                data.soundId = ResourceID::get(sound.value().get<std::string>());
                continue;
              }
              
              if (sound.value().is_number_unsigned() && sound.key() == "delay") {
                data.soundDelay = sound.value().get<size_t>();
                continue;
              }
              
              if (sound.value().is_boolean() && sound.key() == "static") {
                data.staticSound = sound.value().get<bool>();
                continue;
              }
              
              if (sound.value().is_boolean() && sound.key() == "relative") {
                data.relative = sound.value().get<bool>();
                continue;
              }
              
              if (sound.value().is_number() && sound.key() == "scalar") {
                data.scalar = sound.value().get<float>();
                continue;
              }
            }
          }
        }
        
        if (data.time == 0) {
          ErrorDesc desc(mark, EntityLoader::ERROR_BAD_STATE_DATA, "Bad state data");
          std::cout << "Error: " << desc.description << "\n";
          errors.push_back(desc);
        } else {
          info.m_statesData.states.push_back(data);
        }
      }
      
      continue;
    }
    
    if (itr.value().is_string() && itr.key() == "default_state") {
      info.m_statesData.defaultState = Type::get(itr.value().get<std::string>());
      continue;
    }
    
    if (itr.value().is_object() && itr.key() == "physics") {
      hasPhysicsData = true;
      
      bool hasHeight = false, hasWidth = false, hasStairHeight = false;;
      for (auto physData = itr.value().begin(); physData != itr.value().end(); ++physData) {
        if (physData.value().is_number() && physData.key() == "height") {
          hasHeight = true;
          info.m_physData.height = physData.value().get<float>();
          continue;
        }
        
        if (physData.value().is_number() && physData.key() == "width") {
          hasWidth = true;
          info.m_physData.width = physData.value().get<float>();
          continue;
        }
        
        if (physData.value().is_number() && physData.key() == "stair_height") {
          hasStairHeight = true;
          info.m_physData.stairHeight = physData.value().get<float>();
          continue;
        }
      }
      
      if (!(hasHeight && hasWidth)) {
        ErrorDesc desc(mark, EntityLoader::ERROR_BAD_HEIGHT_WIDTH_PHYSICS_DATA, "Could not find height and width data");
        std::cout << "Error: " << desc.description << "\n";
        errors.push_back(desc);
      }
      
      if (!hasStairHeight) {
        info.m_physData.stairHeight = info.m_physData.height * 0.33f;
      }
      
      continue;
    }
    
    info.m_defaultTexture = {
      ResourceID(),
      0,
      false,
      false
    };
    if (itr.value().is_string() && itr.key() == "texture") {
      const auto &str = itr.value().get<std::string>();
      std::cout << "Proccessing texture" << "\n";
      parseTextureDataString(str, info.m_defaultTexture.id, info.m_defaultTexture.index, info.m_defaultTexture.flipU, info.m_defaultTexture.flipV);
      if (!info.m_defaultTexture.id.valid()) {
        throw std::runtime_error("Could not proccess texture "+str);
      }
      continue;
    }
  }
  
  if (!hasId) {
    ErrorDesc desc(mark, EntityLoader::ERROR_COULD_NOT_FIND_ENTITY_ID, "Could not find entity type id");
    std::cout << "Error: " << desc.description << "\n";
    errors.push_back(desc);
    return false;
  }
  
  if (!hasPhysicsData) {
    ErrorDesc desc(mark, EntityLoader::ERROR_COULD_NOT_FIND_PHYSICS_DATA, "Could not find physics data");
    std::cout << "Error: " << desc.description << "\n";
    errors.push_back(desc);
    return false;
  }
  
  if (!(hasStates || hasTexture || hasImage)) {
    
  }
  
  const size_t tmp = size_t(hasStates) + size_t(hasTexture) + size_t(hasImage);
  if (tmp > 1) {
    WarningDesc desc(mark, EntityLoader::WARNING_TOO_MUCH_APPEARENCE_DATA, "Too much appearence data. Priority: states > texture > image");
    std::cout << "Warning: " << desc.description << "\n";
    warnings.push_back(desc);
  }
  
  return errorCount == errors.size();
}

EntityLoader::EntityLoader(const CreateInfo &info) : imageLoader(info.imageLoader), abilityTypeLoader(info.abilityTypeLoader), attributesLoader(info.attributesLoader), effectsLoader(info.effectsLoader), itemTypeLoader(info.itemTypeLoader), animationLoader(info.animationLoader), soundLoader(info.soundLoader) {
  Global g;
  g.setWorld(&world);
}

EntityLoader::~EntityLoader() {
  clear();
  
  for (auto ptr : creatorsPtr) {
    objectCreatorPool.deleteElement(ptr);
  }
}

bool EntityLoader::canParse(const std::string &key) const {
  return key == "entities" || key == "entity_types";
}

bool EntityLoader::parse(const Modification* mod,
                         const std::string &pathPrefix,
                         const nlohmann::json &data,
                         std::vector<Resource*> &resource,
                         std::vector<ErrorDesc> &errors,
                         std::vector<WarningDesc> &warnings) {
  if (tempData == nullptr) tempData = new LoadingTemporaryData<LoadData, 10>;
  
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
    
    bool ret = checkJsonEntityValidity(pathPrefix, data, 12512, info, errors, warnings);
    if (!ret) return false;
    
    info.resInfo.mod = mod;
    info.resInfo.parsedBy = this;
    info.resInfo.resGPUSize = 0;
    info.resInfo.resSize = sizeof(ObjectCreator);
    info.resInfo.pathStr = "";
    
    auto ptr = tempData->container.create(info.resInfo.resId, info);
    resource.push_back(ptr);
    
    std::cout << "Parsed " << ptr->id().name() << "\n";
    
    return true;
  }
  
  return false;
}

bool EntityLoader::forget(const ResourceID &name) {
  if (tempData == nullptr) throw std::runtime_error("Not in loading state");
  
  tempData->container.destroy(name);
  
//   const size_t index = findTempData(name);
//   if (index == SIZE_MAX) return false;
//   
//   tempData->dataPool.deleteElement(tempData->dataPtr[index]);
//   std::swap(tempData->dataPtr[index], tempData->dataPtr.back());
//   tempData->dataPtr.pop_back();
  
  return true;
}

Resource* EntityLoader::getParsedResource(const ResourceID &id) {
  if (tempData == nullptr) throw std::runtime_error("Not in loading state");
  
//   const size_t index = findTempData(id);
//   return index == SIZE_MAX ? nullptr : tempData->dataPtr[index];
  return tempData->container.get(id);
}

const Resource* EntityLoader::getParsedResource(const ResourceID &id) const {
  if (tempData == nullptr) throw std::runtime_error("Not in loading state");
  
  return tempData->container.get(id);
}

bool EntityLoader::load(const ModificationParser* modifications, const Resource* resource) {
  if (tempData == nullptr) throw std::runtime_error("Not in loading state");
  
  const Type id = Type::get(resource->id().name());
  auto itr = entityCreators.find(id);
  if (itr != entityCreators.end()) return true;
  
  auto loadData = tempData->container.get(resource->id());
  if (loadData == nullptr) return false;
  
  const ObjectCreator::PhysData physData{
    1,
    1,
    loadData->physData().stairHeight,
    loadData->physData().height,
    loadData->physData().width
  };
  
  auto parsedres = imageLoader->getParsedResource(loadData->defaultTexture().id);
  if (parsedres == nullptr) throw std::runtime_error("Could not load image "+loadData->defaultTexture().id.name());
  const bool ret = imageLoader->load(nullptr, parsedres);
  if (!ret) throw std::runtime_error("Could not load image "+loadData->defaultTexture().id.name());
  
  const auto img = imageLoader->image(loadData->defaultTexture().id, loadData->defaultTexture().index);
  if (img.index == UINT32_MAX || img.layer == UINT32_MAX) throw std::runtime_error("Could not find image "+loadData->defaultTexture().id.name()+" with index "+std::to_string(loadData->defaultTexture().index));
  
  const ObjectCreator::GraphicsData graphics{
    {
      img,
      0,
      loadData->defaultTexture().flipU ? -1.0f : 0.0f,
      loadData->defaultTexture().flipV ? -1.0f : 0.0f
    }
  };
  
  std::vector<AttributeComponent::InitInfo<FLOAT_ATTRIBUTE_TYPE>> float_attribs;
  std::vector<AttributeComponent::InitInfo<INT_ATTRIBUTE_TYPE>> int_attribs;
  for (size_t i = 0; i < loadData->attributesData().attribs.size(); ++i) {
    const ResourceID &id = ResourceID::get(loadData->attributesData().attribs[i].attrib.name());
    const bool ret = attributesLoader->load(nullptr, attributesLoader->getParsedResource(id));
    if (!ret) throw std::runtime_error("could not load attribute "+loadData->attributesData().attribs[i].attrib.name());
    
    auto attribf = attributesLoader->getFloatType(loadData->attributesData().attribs[i].attrib);
    if (attribf != nullptr) {
      float_attribs.push_back({attribf, static_cast<float>(loadData->attributesData().attribs[i].value)});
      continue;
    }
    
    auto attribi = attributesLoader->getIntType(loadData->attributesData().attribs[i].attrib);
    if (attribi != nullptr) {
      int_attribs.push_back({attribi, static_cast<int64_t>(loadData->attributesData().attribs[i].value)});
      continue;
    }
    
    throw std::runtime_error("Could not find attribute "+loadData->attributesData().attribs[i].attrib.name());
  }
  
  const ObjectCreator::AttributesData attribs{
    float_attribs,
    int_attribs
  };
  
  std::vector<const Effect*> initialEffects(loadData->effectsData().initialEffects.size());
  std::vector<std::pair<Type, const Effect*>> eventEffects(loadData->effectsData().eventEffects.size());
  for (size_t i = 0; i < loadData->effectsData().initialEffects.size(); ++i) {
    const ResourceID &id = ResourceID::get(loadData->effectsData().initialEffects[i].name());
    const bool ret = effectsLoader->load(nullptr, effectsLoader->getParsedResource(id));
    if (!ret) throw std::runtime_error("Could not load effect "+loadData->effectsData().initialEffects[i].name());
    
    initialEffects[i] = effectsLoader->getEffect(loadData->effectsData().initialEffects[i]);
    if (initialEffects[i] == nullptr) throw std::runtime_error("Could not find effect "+loadData->effectsData().initialEffects[i].name());
  }
  
  for (size_t i = 0; i < loadData->effectsData().eventEffects.size(); ++i) {
    const ResourceID &id = ResourceID::get(loadData->effectsData().eventEffects[i].second.name());
    const bool ret = effectsLoader->load(nullptr, effectsLoader->getParsedResource(id));
    if (!ret) throw std::runtime_error("Could not load effect "+loadData->effectsData().eventEffects[i].second.name());
    
    eventEffects[i] = std::make_pair(loadData->effectsData().eventEffects[i].first, effectsLoader->getEffect(loadData->effectsData().eventEffects[i].second));
    if (eventEffects[i].second == nullptr) throw std::runtime_error("Could not find effect "+loadData->effectsData().eventEffects[i].second.name());
  }
  
  const ObjectCreator::Effects effects{
    initialEffects,
    eventEffects
  };
  
  std::vector<ObjectCreator::Inventory::Item> items(loadData->inventoryData().items.size());
  for (size_t i = 0; i < loadData->inventoryData().items.size(); ++i) {
    const ResourceID &id = ResourceID::get(loadData->inventoryData().items[i].type.name());
    const bool ret = effectsLoader->load(nullptr, effectsLoader->getParsedResource(id));
    if (!ret) throw std::runtime_error("Could not load item type "+loadData->inventoryData().items[i].type.name());
    
    items[i] = {itemTypeLoader->getItemType(loadData->inventoryData().items[i].type), loadData->inventoryData().items[i].count};
    if (items[i].type == nullptr) throw std::runtime_error("Could not find item type "+loadData->inventoryData().items[i].type.name());
  }
  
  const ObjectCreator::Inventory inventory{
    items
  };
  
  std::vector<std::pair<size_t, const ItemType*>> weapons(loadData->weaponsData().weapons.size());
  for (size_t i = 0; i < loadData->weaponsData().weapons.size(); ++i) {
    const ResourceID &id = ResourceID::get(loadData->weaponsData().weapons[i].second.name());
    const bool ret = effectsLoader->load(nullptr, effectsLoader->getParsedResource(id));
    if (!ret) throw std::runtime_error("Could not load item type "+loadData->weaponsData().weapons[i].second.name());
    
    weapons[i] = std::make_pair(loadData->weaponsData().weapons[i].first, itemTypeLoader->getItemType(loadData->weaponsData().weapons[i].second));
    if (weapons[i].second == nullptr) throw std::runtime_error("Could not find item type "+loadData->weaponsData().weapons[i].second.name());
  }
  
  const ObjectCreator::Weapons weaponsData{
    weapons
  };
  
  std::vector<ObjectCreator::Abilities::Slot> slots(loadData->abilitiesData().slots.size());
  for (size_t i = 0; i < loadData->abilitiesData().slots.size(); ++i) {
    const ResourceID &id = ResourceID::get(loadData->abilitiesData().slots[i].ability.name());
    const bool ret = abilityTypeLoader->load(nullptr, abilityTypeLoader->getParsedResource(id));
    if (!ret) throw std::runtime_error("Could not load ability type "+loadData->abilitiesData().slots[i].ability.name());
    
    slots[i] = {loadData->abilitiesData().slots[i].slot, abilityTypeLoader->getAbilityType(loadData->abilitiesData().slots[i].ability), loadData->abilitiesData().slots[i].state};
    if (slots[i].ability == nullptr) throw std::runtime_error("Could not find ability type "+loadData->abilitiesData().slots[i].ability.name());
  }
  
  const ObjectCreator::Abilities abilitiesData{
    slots
  };
  
  const ObjectCreator::Intelligence intelligence{
    nullptr,
    Type()
  };
  
  StateControllerType* statesPtr = nullptr;
  if (!loadData->statesData().states.empty()) {
    std::vector<StateDataCreateInfo> statesInfo(loadData->statesData().states.size());
    for (size_t i = 0; i < loadData->statesData().states.size(); ++i) {
      const AttributeType<float>* attrib = nullptr;
      if (loadData->statesData().states[i].speedAttribute.valid()) {
        const ResourceID &id = ResourceID::get(loadData->statesData().states[i].speedAttribute.name());
        const bool ret = attributesLoader->load(nullptr, attributesLoader->getParsedResource(id));
        if (!ret) throw std::runtime_error("Could not load attribute "+loadData->statesData().states[i].speedAttribute.name());
        
        attrib = attributesLoader->getFloatType(loadData->statesData().states[i].speedAttribute);
        if (attrib == nullptr) throw std::runtime_error("Could not find attribute "+loadData->statesData().states[i].speedAttribute.name());
      }
      
      statesInfo[i] = {
        StateFlags(loadData->statesData().states[i].blocked, loadData->statesData().states[i].blockedMovement, loadData->statesData().states[i].loop, false, false),
        loadData->statesData().states[i].time,
        loadData->statesData().states[i].animationId,
        loadData->statesData().states[i].soundId,
        loadData->statesData().states[i].staticSound,
        loadData->statesData().states[i].relative,
        loadData->statesData().states[i].scalar,
        loadData->statesData().states[i].animationDelay,
        loadData->statesData().states[i].soundDelay,
        attrib,
        loadData->statesData().states[i].id,
        loadData->statesData().states[i].nextState
      };
    }
    
    statesPtr = stateControllerArray.create(loadData->creatorId(), StateControllerType::CreateInfo{statesInfo, loadData->statesData().defaultState, {}});
  }
  
  const ObjectCreator::States statesData{
    statesPtr
  };
  
  auto ptr = objectCreatorPool.newElement(ObjectCreator::CreateInfo{physData, graphics, attribs, effects, inventory, weaponsData, abilitiesData, intelligence, statesData});
  creatorsPtr.push_back(ptr);
  entityCreators[loadData->creatorId()] = ptr;
  
  return true;
}

bool EntityLoader::unload(const ResourceID &id) {
  if (tempData == nullptr) throw std::runtime_error("Not in loading state");
  
  const Type type = Type::get(id.name());
  auto itr = entityCreators.find(type);
  for (size_t i = 0; i < creatorsPtr.size(); ++i) {
    if (creatorsPtr[i] == itr->second) {
      objectCreatorPool.deleteElement(creatorsPtr[i]);
      std::swap(creatorsPtr[i], creatorsPtr.back());
      creatorsPtr.pop_back();
      entityCreators.erase(itr);
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

void EntityLoader::end() {

}

void EntityLoader::clear() {
  delete tempData;
  tempData = nullptr;
}

size_t EntityLoader::overallState() const {
  
}

size_t EntityLoader::loadingState() const {
  
}

std::string EntityLoader::hint() const {
  
}

const Type wallType = Type::get("wall_creator_type");

yacs::entity* EntityLoader::create(const Type &type, yacs::entity* parent, void* data) const {
  //if (container == nullptr) throw std::runtime_error("Entity creators is not loaded");
  
  if (type == wallType) return wallCreator.create(parent, data);
  
  auto itr = entityCreators.find(type);
  if (itr == entityCreators.end()) throw std::runtime_error("Creator with id "+type.name()+" is not exist");
  
  return itr->second->create(parent, data);
}

// size_t EntityLoader::findTempData(const ResourceID &id) const {
//   for (size_t i = 0; i < tempData->dataPtr.size(); ++i) {
//     if (tempData->dataPtr[i]->id() == id) return i;
//   }
//   
//   return SIZE_MAX;
// }
