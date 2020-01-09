#ifndef ENTITY_CREATORS_H
#define ENTITY_CREATORS_H

#include "EntityCreator.h"

#include "Type.h"
#include "RenderStructures.h"

#include "AttributesComponent.h"
#include "shared_collision_constants.h"

#include <vector>

class vertex_t;

class AbilityType;
class ItemType;
class Effect;
template <typename T>
class AttributeType;
class StateControllerType;
namespace tb {
  class BehaviorTree;
}

// должно быть преобразовано из воид в это
struct WallData {
  std::string name;
  Type shapeType;
  
  size_t indexOffset;
  size_t faceVertices;
  size_t faceIndex;
  
  Texture wallTexture; // возможно строка с наименованием изображения
  float radius;
  vertex_t* vertex;
  
  uint32_t flags;
  uint32_t tag;
  
  // также необходимо задать связи между разными объектами на карте
  // это не укладывается в описание типа, так как одна кнопка может делать 
  // разные действия с любым другим объектом на карте
  // я полагаю что необходимо описать эти действия в json, а в описании карты 
  // задавать непосредственно само действие + цель (наиболее адекватное решение в моем случае)
  // как описать действие? в 99% случаях действие будет связано с перемещением объекта из точки А в точку Б
  // возможно мне потребуется изменить ротацию у объекта + некий триггер который может например телепортировать монстров из одного места в другое
  // нужен какой то легкий способ изменять ротацию и положение объекта + возможность делать сложные вещи с триггерами и прочим
};

class WallCreator : public EntityCreator {
public:
  yacs::entity* create(yacs::entity* parent, const void* data) const override;
};

// указатель воид должен быть преобразован видимо в AbilityType
// проблема в том что AbilityType должен пользоваться тем же создателем что и обычный объект
// другое дело что мы должны определить когда мы создаем по абилке объект, 
// а когда с помощью данных (например данных карты)
// class AbilityCreator : public EntityCreator {
// public:
//   yacs::entity* create(yacs::entity* parent, void* data) const override;
// };

// что такое действие?

// я так понимаю, мне нужно пометить с помощью тега энтити, которое проснется при взаимодействии с игроком
// нужно задать список действий которые могут вообще произойти с объектом или предметом

enum {
  ACTION_OBJECT_COLLIDE  = 1 << 0,
  ACTION_OBJECT_USE      = 1 << 1,
  ACTION_OBJECT_ABILITY  = 1 << 2,
  ACTION_OBJECT_SOUND    = 1 << 3,
  ACTION_OBJECT_PICK_UP  = 1 << 4,
  // ???
};

struct ObjectData {
  // максимально необходимые данные для объекта какие? позиция, направление, ???
  // выписал данные используемые в думе на карте для энтити
  // актион используется у стен для того чтобы обеспечить взаимодействие между объектами
  // сомневаюсь что оно используется у энтити как то
  // флаги и тэги используются например для того чтобы отделить монстров на легком уровне сложности и на других
  float pos[4];
  float dir[4];
  
  uint32_t flags; // тут например амбуш переменная, монстры не реагируют на звуки пока не увидят цель (точнее они только видят но не слышат)
  uint32_t tag;   // тэг я так понял используется для того чтобы пометить к какому объекту применяется действие - полезно
  uint32_t defaultAction;
  
  const AbilityType* ability;
  size_t entityIndex;
};

class ObjectCreator : public EntityCreator {
public:
  struct PhysData {
    bool dynamic;
    uint32_t collisionGroup;
    uint32_t collisionFilter;
    float stairHeight;
    float height;
    float width;
    float gravCoef;
  };
  
  struct GraphicsData {
    Texture t;
  };
  
  struct AttributesData {
    template <typename T>
    struct Attrib {
      const AttributeType<T>* attrib; // поди тут нужны сразу указатели на типы
      T value;
    };
    
    std::vector<AttributeComponent::InitInfo<FLOAT_ATTRIBUTE_TYPE>> float_attribs;
    std::vector<AttributeComponent::InitInfo<INT_ATTRIBUTE_TYPE>> int_attribs;
  };
  
  struct Effects {
    std::vector<const Effect*> initialEffects;
    std::vector<std::pair<Type, const Effect*>> eventEffects;
  };
  
  struct Inventory {
    struct Item {
      const ItemType* type;
      size_t count;
    };
    
    std::vector<Item> items;
  };
  
  struct Weapons {
    // оружее может быть недоступно с самого начала, но при этом должны быть слоты под определенный тип оружия
    // такое поведение свойственно только для игрока, то есть думаю что нужно описать игрока в этом плане как то по особому
    std::vector<std::pair<size_t, const ItemType*>> weapons;
  };
  
  struct Abilities {
    struct Slot {
      size_t slot;
      const AbilityType* ability;
      Type state;
    };
    
    std::vector<Slot> slots;
  };
  
  struct Intelligence {
    //Type behaviour; // тут можно получить сразу указатель
    tb::BehaviorTree* tree;
    Type func;
  };
  
  struct States {
//     struct Data {
//       Type id;
//       size_t time;
//       // data
//     };
//     
//     std::vector<Data> states;
    const StateControllerType* type;
  };
  
  struct CreateInfo {
    PhysData physData;
    GraphicsData graphicsData;
    AttributesData attributesData;
    Effects effectsData;
    Inventory inventoryData;
    Weapons weaponsData;
    Abilities abilitiesData;
    Intelligence intelligence;
    States statesData;
  };
  ObjectCreator(const CreateInfo &info);
  
  yacs::entity* create(yacs::entity* parent, const void* data) const override;
private:
  PhysData physData;
  GraphicsData graphicsData;
  AttributesData attributesData;
  Effects effectsData;
  Inventory inventoryData;
  Weapons weaponsData;
  Abilities abilitiesData;
  Intelligence intelligence;
  States statesData;
};

class PlayerCreator : public EntityCreator {
public:
  struct CreateInfo {
//     PhysData physData;
  };
  PlayerCreator(const CreateInfo &info);
  
  yacs::entity* create(yacs::entity* parent, const void* data) const override;
private:
//   PhysData physData;
  // attrib
  // abilities
  // states
};

struct AbilityData {
  const AbilityType* ability;
  // ???
  // может быть нужно добавить указатель на айтем? 
  // если мы все же укажем в информации об энтити как создать интеракцию
  // то нам потребуются скорее всего только данные абилки
  // лучше не разделять
};

#endif
