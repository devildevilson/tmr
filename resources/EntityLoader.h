#ifndef ENTITY_LOADER_H
#define ENTITY_LOADER_H

#include "Type.h"
#include "Loader.h"
#include "ResourceParser.h"
#include "EntityComponentSystem.h"
// #include "ComponentCreator.h"
#include "TypelessContainer.h"
#include "Resource.h"
#include "LoadingTemporaryData.h"

#include <unordered_map>

// может быть строит вернуть коцепцию модели как контейнер разных анимаций и звуков для разных ситуации
// думаю что нужно просто использовать набор состояний и переключать состояния при использовании той или иной абилки

// возможно стоит чуть облегчить суть создания энтити: не использовать энтити типы (то есть энтити с компонентами для создания)
// а просто переопределить функцию создания - и так и сяк мне нужно передать информацию в каком нибудь контейнере
// думаю что это будет предпочтительнее, зря делал компонент креаторы =(

#define CREATE_WALL_TYPE "internal_wall_creator"
#define CREATE_ABILITY_TYPE "internal_ability_creator"

class EntityCreator;
class AbilityTypeLoader;
class AttributesLoader;
class EffectsLoader;
class ItemTypeLoader;
class ImageLoader;

class EntityLoader : public Loader, public ResourceParser {
public:
  enum Errors {
    ERROR_FILE_NOT_FOUND = 0,
    ERROR_BAD_ATTRIBUTE_VALUE,
    ERROR_BAD_ITEM_COUNT_VALUE,
    ERROR_ABILITY_SLOT_IS_NOT_VALID,
    ERROR_SLOT_KEY_IS_NOT_VALID,
    ERROR_BAD_SLOT_ABILITY_TYPE,
    ERROR_BAD_BEHAVIOUR_TREE_DATA,
    ERROR_BAD_STATE_DATA,
    ERROR_BAD_HEIGHT_WIDTH_PHYSICS_DATA,
    ERROR_TEXTURE_IMAGE_IS_NOT_SPECIFIED,
    ERROR_COULD_NOT_FIND_ENTITY_ID,
    ERROR_COULD_NOT_FIND_PHYSICS_DATA,
    
  };
  
  enum Warnings {
    WARNING_TOO_MUCH_APPEARENCE_DATA = 0,
  };
  
  class LoadData : public Resource {
  public:
    struct PhysData {
      uint32_t collisionGroup;
      uint32_t collisionFilter;
      float stairHeight;
      float height;
      float width;
    };
    
    struct GraphicsData {
      std::string texture;
    };
    
    struct AttributesData {
      struct Attrib {
        Type attrib; // поди тут нужны сразу указатели на типы
        double value;
      };
      
      std::vector<Attrib> attribs;
    };
    
    struct Effects {
      std::vector<Type> initialEffects;
      std::vector<std::pair<Type, Type>> eventEffects;
    };
    
    struct Inventory {
      struct Item {
        Type type;
        size_t count;
      };
      
      std::vector<Item> items;
    };
    
    struct Weapons {
      // оружее может быть недоступно с самого начала, но при этом должны быть слоты под определенный тип оружия
      // такое поведение свойственно только для игрока, то есть думаю что нужно описать игрока в этом плане как то по особому
      std::vector<std::pair<size_t, Type>> weapons;
    };
    
    struct Abilities {
      struct Slot {
        size_t slot;
        Type ability;
        Type state;
      };
      
      std::vector<Slot> slots;
    };
    
    struct Intelligence {
      //Type behaviour; // тут можно получить сразу указатель
      Type tree;
      Type func;
    };
    
    struct States {
      struct Data {
        Type id;
        size_t time;
        // data
      };
      
      std::vector<Data> states;
    };
    
    struct CreateInfo {
      Resource::CreateInfo resInfo;
      
      Type m_id;
      PhysData m_physData;
      GraphicsData m_graphicsData;
      AttributesData m_attributesData;
      Effects m_effectsData;
      Inventory m_inventoryData;
      Weapons m_weaponsData;
      Abilities m_abilitiesData;
      Intelligence m_intelligence;
      States m_statesData;
    };
    LoadData(const CreateInfo &info);
    
    Type creatorId() const;
    PhysData physData() const;
    GraphicsData graphicsData() const;
    AttributesData attributesData() const;
    Effects effectsData() const;
    Inventory inventoryData() const;
    Weapons weaponsData() const;
    Abilities abilitiesData() const;
    Intelligence intelligence() const;
    States statesData() const;
  private:
    Type m_id;
    
    PhysData m_physData;
    GraphicsData m_graphicsData;
    AttributesData m_attributesData;
    Effects m_effectsData;
    Inventory m_inventoryData;
    Weapons m_weaponsData;
    Abilities m_abilitiesData;
    Intelligence m_intelligence;
    States m_statesData;
  };
  
  struct CreateInfo {
    const ImageLoader* imageLoader;
    const AbilityTypeLoader* abilityTypeLoader;
    const AttributesLoader* attributesLoader;
    const EffectsLoader* effectsLoader;
    const ItemTypeLoader* itemTypeLoader;
  };
  EntityLoader(const CreateInfo &info);
  ~EntityLoader();
  
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

  // ресурс будет означать подготовку entityType, который мы потом используем непосредственно для создания энтити
  bool load(const ModificationParser* modifications, const Resource* resource) override;
  bool unload(const ResourceID &id) override;
  void end() override;
  
  void clear() override;
  
  size_t overallState() const override;
  size_t loadingState() const override;
  std::string hint() const override;
  
//   yacs::entity* create(const Type &type, yacs::entity* parent, const UniversalDataContainer* container); // по идее это необходимо и достаточно для создания энтити
  yacs::entity* create(const Type &type, yacs::entity* parent, void* data) const;
private:
  LoadingTemporaryData<LoadData, 10>* tempData;
  
  yacs::world world;
  size_t containerSize;
  TypelessContainer* container; // размер получаем из лоада, сонтейнер коздаем в end(), там же создаем непосредственно загрузчики
  std::vector<EntityCreator*> creatorsPtr;
  std::unordered_map<Type, const EntityCreator*> entityCreators;
  
  const ImageLoader* imageLoader;
  const AbilityTypeLoader* abilityTypeLoader;
  const AttributesLoader* attributesLoader;
  const EffectsLoader* effectsLoader;
  const ItemTypeLoader* itemTypeLoader;
  
  size_t findTempData(const ResourceID &id) const;
};

#endif  
