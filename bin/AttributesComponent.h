#ifndef ATTRIBUTES_COMPONENT_H
#define ATTRIBUTES_COMPONENT_H

#include <unordered_map>
#include <atomic>

#include "Globals.h"
#include "EntityComponentSystem.h"
#include "Utility.h"
#include "Engine.h"
#include "ThreadPool.h"
#include "PhysicsUtils.h"
#include "ArrayInterface.h"
#include "Attributes.h"

enum AttribChanging {
  ATTRIB_BONUS_TYPE_RAW_ADD,
  ATTRIB_BONUS_TYPE_RAW_REMOVE,
  ATTRIB_BONUS_TYPE_FINAL_ADD,
  ATTRIB_BONUS_TYPE_FINAL_REMOVE,
  ATTRIB_BONUS_TYPE_COUNT
};

//struct AttribChangeType {
//  uint32_t container;
//
//  AttribChangeType();
//  AttribChangeType(const bool raw, const bool add);
//  void make(const bool raw, const bool add);
//
//  bool bonus_type_raw() const;
//  bool bonus_math_add() const;
//};

class EntityAI;

struct AttribChangeData {
//  AttribChangeType type;
  AttribChanging type;
  Bonus b;
  TypelessAttributeType attribType;
  // в эффектах должен быть способ понять кто его наложил
  // но и неплохо было бы узнать кто сейчас непосредственно меняет статы
  // и + кто последний изменил какой то определенный стат
  EntityAI* entity;
};

// нам гораздо лучше будет если вот эти вещи будут упакованы в типы, так мы сильно сократим использование памяти
// таким же образом мы скорее всего сократим время на выделения памяти, но я не уверен пока что как это сделать
struct AttributeReaction {
  enum class comparison {
    less,
    more,
    equal,
    count
  };
  
  TypelessAttributeType attribType;
  float value;
  comparison comp;
  Type event;

  // тип изменения? это полезно, например, для того чтобы понять как умирать
};

class PhysicsComponent;
class EventComponent;

// у нас помимо AttributeType должен быть еще какой то MonsterType
class AttributeComponent {
public:
  static void setContainer(Container<ExternalData>* cont);
  
  struct float_InitInfo {
    AttributeType<FLOAT_ATTRIBUTE_TYPE> type;
    FLOAT_ATTRIBUTE_TYPE baseValue;
  };
  struct int_InitInfo {
    AttributeType<INT_ATTRIBUTE_TYPE> type;
    INT_ATTRIBUTE_TYPE baseValue;
  };
  
  struct CreateInfo {
//    AttributeSystem* system;
    PhysicsComponent* phys;
    EventComponent* events;
    size_t updateTime; // тут бессмысленно ставить что то не SIZE_MAX
    std::vector<float_InitInfo> float_attribs;
    std::vector<int_InitInfo> int_attribs;
  };
  AttributeComponent(const CreateInfo &info);
  ~AttributeComponent();
  
  void update();
//  void init(void* userData);
  
  template <typename Type>
  const Attribute<Type>* get(const AttributeType<Type> &type) const;
  
  template <typename Type>
  const Attribute<Type>* get(const TypelessAttributeType &type) const;
  
  template <typename Type>
  AttributeFinder<Attribute<Type>> get_finder() const;
  
  void change_attribute(const AttribChangeData &data);
  const AttribChangeData* get_attribute_change(const TypelessAttributeType &type, size_t &counter) const;
//   void clear_counter();
  
  void addReaction(const AttributeReaction &reaction);
  
//  size_t & internalIndex();
private:
//  size_t index;
  size_t fcount;
  size_t icount;
  Attribute<FLOAT_ATTRIBUTE_TYPE>* attribsf;
  Attribute<INT_ATTRIBUTE_TYPE>* attribsi;
  PhysicsComponent* phys;
  EventComponent* events;

  // можно сделать так чтобы аттрибуты не обновлялись каждый кадр
  // обновляться аттрибуты должны только когда datas не empty
  // + наверное еще следующий кадр, так мы сможем гарантировать воздействие на каждый аттрибут
  size_t updateTime;
  size_t currentTime;

  std::vector<AttribChangeData> datas;
  std::vector<AttributeReaction> reactions;
  
  static Container<ExternalData>* externalDatas;
};

class AttributeSystem : public Engine, public yacs::system {
public:
  struct CreateInfo {
    dt::thread_pool* pool;
  };
  AttributeSystem(const CreateInfo &info);
  ~AttributeSystem();
  
  void update(const size_t &time) override;
  
//  void add(AttributeComponent* comp);
//  void remove(AttributeComponent* comp);
private:
  dt::thread_pool* pool;
//  std::vector<AttributeComponent*> components;
};

#endif
