#include "EntityLoader.h"

#include "EntityCreators.h"
#include "AbilityTypeLoader.h"
#include "ItemLoader.h"
#include "AttributesLoader.h"
#include "EffectsLoader.h"
#include "ImageLoader.h"

EntityLoader::LoadData::LoadData(const CreateInfo &info) : Resource(info.resInfo), m_id(info.m_id), m_physData(info.m_physData), m_graphicsData(info.m_graphicsData), m_attributesData(info.m_attributesData), m_effectsData(info.m_effectsData), m_inventoryData(info.m_inventoryData), m_weaponsData(info.m_weaponsData), m_abilitiesData(info.m_abilitiesData), m_intelligence(info.m_intelligence), m_statesData(info.m_statesData) {}
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

size_t parseSlot(const std::string &str) {
  if (str.length() < 5) return SIZE_MAX;
  
  auto n = str.find("slot");
  if (n == std::string::npos) return SIZE_MAX;
  
  if (!isdigit(str.c_str()[4])) return SIZE_MAX;
  
  size_t num = atol(&str.c_str()[4]);
  return num;
}

bool checkJsonEntityValidity(const std::string &path, const nlohmann::json &data, const size_t &mark, EntityLoader::LoadData::CreateInfo &info, std::vector<ErrorDesc> &errors, std::vector<WarningDesc> &warnings) {
  bool hasId = false, hasStates = false, hasTexture = false, hasPhysicsData = false, hasImage = false;
  
  const size_t errorCount = errors.size();
  
  for (auto itr = data.begin(); itr != data.end(); ++itr) {
    if (itr.value().is_string() && itr.key() == "id") {
      hasId = true;
      info.resInfo.resId = ResourceID::get(itr.value().get<std::string>());
      info.m_id = Type::get(itr.value().get<std::string>());
    }
    
    if (itr.value().is_object() && itr.key() == "attributes") {
      for (auto attrib = itr.value().begin(); attrib != itr.value().end(); ++attrib) {
        if (!attrib.value().is_number()) {
          ErrorDesc desc(4123, EntityLoader::ERROR_BAD_ATTRIBUTE_VALUE, "Bad attribute type");
          std::cout << "Error: " << desc.description << "\n";
          errors.push_back(desc);
          continue;
        }
        
        info.m_attributesData.attribs.push_back({Type::get(itr.key()), itr.value().get<double>()});
      }
    }
    
    if (itr.value().is_object() && itr.key() == "effects") {
      for (auto effect = itr.value().begin(); effect != itr.value().end(); ++effect) {
        info.m_effectsData.eventEffects.push_back(std::make_pair(Type::get(effect.key()), Type::get(effect.value().get<std::string>())));
      }
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
          }
          
          if (abilitySlot.value().is_string() && abilitySlot.key() == "state") {
            slotInfo.state = Type::get(abilitySlot.value().get<std::string>());
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
    }
    
    if (itr.value().is_object() && itr.key() == "intelligence") {
      bool hasBehaviourTree = false;
      
      for (auto intData = itr.value().begin(); intData != itr.value().end(); ++intData) {
        if (intData.value().is_string() && intData.key() == "behaviour") {
          hasBehaviourTree = true;
          info.m_intelligence.tree = Type::get(intData.value().get<std::string>());
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
        
        EntityLoader::LoadData::States::Data data;
        data.id = Type::get(state.key());
        for (auto stateData = state.value().begin(); stateData != state.value().end(); ++stateData) {
          if (stateData.value().is_number_unsigned() && stateData.key() == "time") {
            data.time = stateData.value().get<size_t>();
          }
        }
        
        info.m_statesData.states.push_back(data);
      }
    }
    
    if (itr.value().is_object() && itr.key() == "physics") {
      hasPhysicsData = true;
      
      bool hasHeight = false, hasWidth = false;
      for (auto physData = itr.value().begin(); physData != itr.value().end(); ++physData) {
        if (physData.value().is_number() && physData.key() == "height") {
          hasHeight = true;
          info.m_physData.height = physData.value().get<float>();
        }
        
        if (physData.value().is_number() && physData.key() == "width") {
          hasWidth = true;
          info.m_physData.width = physData.value().get<float>();
        }
      }
      
      if (!(hasHeight && hasWidth)) {
        ErrorDesc desc(mark, EntityLoader::ERROR_BAD_HEIGHT_WIDTH_PHYSICS_DATA, "Could not find height and width data");
        std::cout << "Error: " << desc.description << "\n";
        errors.push_back(desc);
      }
    }
    
    if (itr.value().is_object() && itr.key() == "texture") {
      hasTexture = true;
      
      bool hasImage = false;
      for (auto textureData = itr.value().begin(); textureData != itr.value().end(); ++textureData) {
        if (textureData.value().is_string() && textureData.key() == "image") {
          hasImage = true;
          // с текстуркой пока не ясно
        }
      }
      
      if (!hasImage) {
        ErrorDesc desc(mark, EntityLoader::ERROR_TEXTURE_IMAGE_IS_NOT_SPECIFIED, "Could not find image data");
        std::cout << "Error: " << desc.description << "\n";
        errors.push_back(desc);
      }
    }
    
    if (itr.value().is_string() && itr.key() == "image") {
      hasImage = true;
      // грузим изображение
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

EntityLoader::EntityLoader(const CreateInfo &info) : containerSize(sizeof(WallCreator) + sizeof(AbilityCreator)), container(nullptr), imageLoader(info.imageLoader), abilityTypeLoader(info.abilityTypeLoader), attributesLoader(info.attributesLoader), effectsLoader(info.effectsLoader), itemTypeLoader(info.itemTypeLoader) {}
EntityLoader::~EntityLoader() {
  clear();
  
  for (auto ptr : creatorsPtr) {
    container->destroy(ptr);
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
    
    auto ptr = tempData->create(info);
    resource.push_back(ptr);
    return true;
  }
  
  return false;
}

bool EntityLoader::forget(const ResourceID &name) {
  if (tempData == nullptr) throw std::runtime_error("Not in loading state");
  
  const size_t index = findTempData(name);
  if (index == SIZE_MAX) return false;
  
  tempData->dataPool.deleteElement(tempData->dataPtr[index]);
  std::swap(tempData->dataPtr[index], tempData->dataPtr.back());
  tempData->dataPtr.pop_back();
  
  return true;
}

Resource* EntityLoader::getParsedResource(const ResourceID &id) {
  if (tempData == nullptr) throw std::runtime_error("Not in loading state");
  
  const size_t index = findTempData(id);
  return index == SIZE_MAX ? nullptr : tempData->dataPtr[index];
}

const Resource* EntityLoader::getParsedResource(const ResourceID &id) const {
  if (tempData == nullptr) throw std::runtime_error("Not in loading state");
  
  const size_t index = findTempData(id);
  return index == SIZE_MAX ? nullptr : tempData->dataPtr[index];
}

bool EntityLoader::load(const ModificationParser* modifications, const Resource* resource) {
  if (tempData == nullptr) throw std::runtime_error("Not in loading state");
  
  // скорее всего нужно грузить все же в end(), причем все лоадеры так должны делать без исключения
  // то есть здесь мы должны запомнить указатели которые к нам пришли
  
  const size_t index = findTempData(resource->id());
  if (index == SIZE_MAX) return false;
  
  tempData->dataToLoad.push_back(tempData->dataPtr[index]);
  
  return true;
}

bool EntityLoader::unload(const ResourceID &id) {
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

void EntityLoader::end() {
  container = new TypelessContainer(containerSize + tempData->dataToLoad.size() * sizeof(ObjectCreator));
  
  {
    auto ptr = container->create<WallCreator>();
    creatorsPtr.push_back(ptr);
    entityCreators[Type::get(CREATE_WALL_TYPE)] = ptr;
  }
  
  {
    auto ptr = container->create<AbilityCreator>();
    creatorsPtr.push_back(ptr);
    entityCreators[Type::get(CREATE_ABILITY_TYPE)] = ptr;
  }
  
  // создателя игрока поди нет необходимости создавать в контейнере
  
  for (auto loadData : tempData->dataToLoad) {
    const ObjectCreator::PhysData physData{
      1,
      1,
      0.5f,
      loadData->physData().height,
      loadData->physData().width
    };
    
    // находим необходимое изображение, я пока не решил как его хранить (либо строкой либо объектом, но строкой лучше)
    const ObjectCreator::GraphicsData graphics{
      
    };
    
    std::vector<ObjectCreator::AttributesData::Attrib<float>> float_attribs;
    std::vector<ObjectCreator::AttributesData::Attrib<int64_t>> int_attribs;
    for (size_t i = 0; i < loadData->attributesData().attribs.size(); ++i) {
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
      initialEffects[i] = effectsLoader->getEffect(loadData->effectsData().initialEffects[i]);
      if (initialEffects[i] == nullptr) throw std::runtime_error("Could not find effect "+loadData->effectsData().initialEffects[i].name());
    }
    
    for (size_t i = 0; i < loadData->effectsData().eventEffects.size(); ++i) {
      eventEffects[i] = std::make_pair(loadData->effectsData().eventEffects[i].first, effectsLoader->getEffect(loadData->effectsData().eventEffects[i].second));
      if (eventEffects[i].second == nullptr) throw std::runtime_error("Could not find effect "+loadData->effectsData().eventEffects[i].second.name());
    }
    
    const ObjectCreator::Effects effects{
      initialEffects,
      eventEffects
    };
    
    std::vector<ObjectCreator::Inventory::Item> items(loadData->inventoryData().items.size());
    for (size_t i = 0; i < loadData->inventoryData().items.size(); ++i) {
      items[i] = {itemTypeLoader->getItemType(loadData->inventoryData().items[i].type), loadData->inventoryData().items[i].count};
      if (items[i].type == nullptr) throw std::runtime_error("Could not find item type "+loadData->inventoryData().items[i].type.name());
    }
    
    const ObjectCreator::Inventory inventory{
      items
    };
    
    std::vector<std::pair<size_t, const ItemType*>> weapons(loadData->weaponsData().weapons.size());
    for (size_t i = 0; i < loadData->weaponsData().weapons.size(); ++i) {
      weapons[i] = std::make_pair(loadData->weaponsData().weapons[i].first, itemTypeLoader->getItemType(loadData->weaponsData().weapons[i].second));
      if (weapons[i].second == nullptr) throw std::runtime_error("Could not find item type "+loadData->weaponsData().weapons[i].second.name());
    }
    
    const ObjectCreator::Weapons weaponsData{
      weapons
    };
    
    std::vector<ObjectCreator::Abilities::Slot> slots(loadData->abilitiesData().slots.size());
    for (size_t i = 0; i < loadData->abilitiesData().slots.size(); ++i) {
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
    
    std::vector<ObjectCreator::States::Data> states(loadData->statesData().states.size());
    for (size_t i = 0; i < loadData->statesData().states.size(); ++i) {
      states[i] = {loadData->statesData().states[i].id, loadData->statesData().states[i].time};
    }
    
    const ObjectCreator::States statesData{
      states
    };
    
    auto ptr = container->create<ObjectCreator>(ObjectCreator::CreateInfo{physData, graphics, attribs, effects, inventory, weaponsData, abilitiesData, intelligence, statesData});
    creatorsPtr.push_back(ptr);
    entityCreators[loadData->creatorId()] = ptr;
  }
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

yacs::entity* EntityLoader::create(const Type &type, yacs::entity* parent, void* data) const {
  if (container == nullptr) throw std::runtime_error("Entity creators is not loaded");
  
  auto itr = entityCreators.find(type);
  if (itr == entityCreators.end()) throw std::runtime_error("Creator with id "+type.name()+" is not exist");
  
  return itr->second->create(parent, data);
}

size_t EntityLoader::findTempData(const ResourceID &id) const {
  for (size_t i = 0; i < tempData->dataPtr.size(); ++i) {
    if (tempData->dataPtr[i]->id() == id) return i;
  }
  
  return SIZE_MAX;
}
