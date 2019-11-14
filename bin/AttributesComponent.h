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
class Effect;

struct AttribChangeData {
//  AttribChangeType type;
  AttribChanging type;
  Bonus b;
  TypelessAttributeType attribType;
  const Effect* effect;
  const EntityAI* entity;
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
};
// тип изменения? это полезно, например, для того чтобы понять как умирать
// реакции теперь должны выглядеть по другому
// что такое реакция? по идее это использование абилки при каком то значении аттрибута
// нетолько, мы еще должны сменить состояние
// вообще все эти вещи можно сделать с помощью дерева поведения:
// проверяем все аттрибуты и на основе этого делаем действия, например умираем
// вообще в принципе есть что-то что мы НЕ можем сделать с помощью деревьев?
// неплохо было бы захардкодить наиболее оптимально несколько действий в дереве
// реакции нужно будет убрать

class PhysicsComponent;
class EventComponent;

// у нас помимо AttributeType должен быть еще какой то MonsterType
class AttributeComponent {
public:
  static void setContainer(Container<ExternalData>* cont);

  template <typename T>
  struct InitInfo {
    const AttributeType<T>* type;
    T baseValue;
  };
  
  struct CreateInfo {
//    AttributeSystem* system;
    PhysicsComponent* phys;
    EventComponent* events;
    // единственный смысл ставить здесь не SIZE_MAX это если аттрибут связан с текущей скоростью
    size_t updateTime;
    std::vector<InitInfo<FLOAT_ATTRIBUTE_TYPE>> float_attribs;
    std::vector<InitInfo<INT_ATTRIBUTE_TYPE>> int_attribs;
  };
  AttributeComponent(const CreateInfo &info);
  ~AttributeComponent();
  
  void update();
//  void init(void* userData);
  
  template <typename Type>
  const Attribute<Type>* get(const AttributeType<Type>* type) const;
  
  template <typename T>
  const Attribute<T>* get(const Type &type) const;
  
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
//   std::vector<AttributeReaction> reactions;
  
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
