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

#define FLOAT_ATTRIBUTE_TYPE float
//#define FLOAT_ATTRIBUTE_TYPE double
#define INT_ATTRIBUTE_TYPE int64_t

// class Bonus {
// public:
//   Bonus() : add(0.0f), mul(0.0f) {}
//   Bonus(const float &val, const float &mul) : add(val), mul(mul) {}
//   
//   float add;
//   float mul;
// };

struct Bonus {
  float add;
  float mul;
};

// min max значения тоже сделать аттрибутами?
// вообще это неплохая идея, тогда я смогу изи решать кое какие задачи
// можно ли сделать еще гибче? тип в массиве аттрибуты и разные отношени с ними
// мин макс зависимость и проч, другое дело что тогда вводить массив
// вообще задачи очень типовые, мин макс смысла не имеет делать несколько аттрибутов
// аттрибут от которого зависим... может ли их быть несколько?
// теоретически может, а на практике очень врядли

// скорее всего нужно создать для каждого энтити все атрибуты,
// тогда я смогу писать сложные взаимоотношения между аттрибутами
// (ну точнее тогда мне это будет легче и работать это будет быстрее)
// так же для сложных аттрибутов (типо скорости, которая у меня находится в другом месте)
// нужно написать отдельный апдейт

// с отдельной функцией для атрибутов, нужно будет обновлять атрибуты каждый кадр
// долго? в принципе чаще всего это просто счет, может быть компилятор даже заинлайнит функцию
// атрибутов вряд ли будет много + компонент легко параллелится

// с другой стороны атрибут как компонент? в этом случае атрибут может быть уникальным
// но нужны ли мне такие преимущества? как задавать тогда луа? как обновлять? как получать доступ из ии?
// 

// текущее значение хп все же наверное отдельный аттрибут
template <typename Type>
class Attribute;

template <typename T>
class AttributeFinder {
public:
  AttributeFinder(const size_t &size, T* attribs) : size(size), attribs(attribs) {}
  
  template <typename AT>
  T* find(const AT &type) {
    for (size_t i = 0; i < size; ++i) {
      if (attribs[i].type() == type) return &attribs[i];
    }
    
    return nullptr;
  }
  
  template <typename AT>
  const T* find(const AT &type) const {
    for (size_t i = 0; i < size; ++i) {
      if (attribs[i].type() == type) return &attribs[i];
    }
    
    return nullptr;
  }
private:
  size_t size;
  T* attribs;
};

// вот так и появляются хардкодед аттрибуты
// можно ли вообще сделать эти данные не захардкоженными?
enum DataType {
  ATTRIBUTE_DATA_TYPE_NONE,
  ATTRIBUTE_CURRENT_SPEED,
  ATTRIBUTE_UPDATE_EXTERNAL_DATA_MAX_SPEED,
  ATTRIBUTE_UPDATE_EXTERNAL_DATA_ACCELERATION,
  ATTRIBUTE_DATA_TYPE_COUNT
};

// может ли быть у одного типа аттрибута разные функции? это скорее всего мне все нахрен сломает, скорее нет чем да
// могут ли различаться базовые значения у аттрибута между энтити? возможно, но такой функционал лучше сделать просто добавив доп значения в бонус
// хотя наверное можно и менять базовое значение
// базовое значение может зависеть от другого аттрибута, какие тут проблемы? да собственно никаких, нужно просто не допускать петли
// некоторые аттрибуты полностью зависят от внешних переменных, например ускорение, макс скорость, эта система пока ничего этого не учитывает
// видимо придется делать виртуальные функции

// вообще далеко не всем вообще нужны все аттрибуты, еще можно разделить аттрибуты по группам
// например, хп/мп/щит, скорость/макс скорость/ускорение, пользовательские
// ну и да мне нужны аттрибуты интовые
// короче чет я подозреваю что возможно даже придется создавать не все аттрибуты сразу, то есть хранить их в мапе
// я это все городил для того чтобы вычислять эффективно, но как это бывает либо эффективно либо удобно и приятно работать
// лучше наверное все же вариант с мапой

// так че по зависимостям? в принципе, пока я не меняю аттрибуты параллельно, все равно
// может возникнуть ситуация когда аттрибуты бесконечно увеличиваются (например, аттрибуты в кольце и кадый кадр прибавляется один аттрибут к другому)
// вообще это скорее ошибка дизайна, нужно ли ее какнибудь исправить? думаю что нет
// нам нужны доп данные при вычислении аттрибутов: расстояние до объекта? (интересная механика была бы) в общем то я особ придумать не могу сейчас

// еще один вопрос, нужно ли делать так чтобы можно было добавить аттрибут по ходу игры? вряд ли это полезная фича, 
// а значит можно засунуть все (вообще от каждого юнита флоат) аттрибуты в один массив и выдавать их при создании непосредственно юнита
// таким образом мы решим проблему с локальностью, указатели на аттрибуты придется дополнительно складывать в мапы для того чтобы к ним обращаться из вне
template <typename Type>
class AttributeType {
public:
  // что еще?
  struct CreateInfo {
    std::string name;
    DataType type;
    std::function<Type(const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>>&, const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>>&, const Type&, const Type&, const FLOAT_ATTRIBUTE_TYPE&, const Type&, const FLOAT_ATTRIBUTE_TYPE&)> func;
  };
  
  static void create_types(const std::vector<CreateInfo> &data) {
    funcs.resize(data.size());
    idToName.resize(data.size());
    
    for (size_t i = 0; i < funcs.size(); ++i) {
      funcs[i].type = data[i].type;
      funcs[i].func = data[i].func;
    }
    
    for (size_t i = 0; i < idToName.size(); ++i) {
      idToName[i] = data[i].name;
    }
    
    size_t id = 0;
    for (const auto &name : data) {
      names[name.name] = id;
      ++id;
    }
  }
  
  static AttributeType get(const std::string &name) {
    auto itr = names.find(name);
    if (itr != names.end()) return AttributeType(itr->second);
    
    return AttributeType();
  }
  
  static bool has(const std::string &name) {
    auto itr = names.find(name);
    return itr != names.end();
  }
  
  static size_t count() {
    return funcs.size();
  }
  
  static void setFunction(const AttributeType &type, const std::function<Type(const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>>&, const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>>&, const Type&, const Type&, const FLOAT_ATTRIBUTE_TYPE&, const Type&, const FLOAT_ATTRIBUTE_TYPE&)> &func) {
    funcs[type.id()].func = func;
  }
  
  static std::function<Type(const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>>&, const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>>&, const Type&, const Type&, const FLOAT_ATTRIBUTE_TYPE&, const Type&, const FLOAT_ATTRIBUTE_TYPE&)> attributeFunction(const AttributeType &type) {
    return funcs[type.id()].func;
  }
  
  AttributeType() : type(SIZE_MAX) {}
  AttributeType(const size_t &id) : type(id) {}
  AttributeType(const AttributeType &type) : type(type.id()) {}
  
  bool valid() const { return type != SIZE_MAX; }
  size_t id() const { return type; }
  std::string name() const { return idToName[type]; }
  
  void setFunction(const std::function<Type(const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>>&, const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>>&, const Type&, const Type&, const FLOAT_ATTRIBUTE_TYPE&, const Type&, const FLOAT_ATTRIBUTE_TYPE&)> &func) {
    funcs[type].func = func;
  }
  std::function<Type(const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>>&, const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>>&, const Type&, const Type&, const FLOAT_ATTRIBUTE_TYPE&, const Type&, const FLOAT_ATTRIBUTE_TYPE&)> attributeFunction() { 
    return funcs[type].func;
  }
  
  DataType data_type() const { return funcs[type].type; }
  
  AttributeType & operator=(const AttributeType &type) { this->type = type.type; }
  bool operator==(const AttributeType &type) const { this->type == type.type; }
  bool operator!=(const AttributeType &type) const { this->type != type.type; }
private:
  struct TypeData {
    DataType type;
    std::function<Type(const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>>&, const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>>&, const Type&, const Type&, const FLOAT_ATTRIBUTE_TYPE&, const Type&, const FLOAT_ATTRIBUTE_TYPE&)> func;
  };
  
  size_t type;
  
  static std::vector<TypeData> funcs;
  static std::unordered_map<std::string, size_t> names;
  static std::vector<std::string> idToName;
};

template <typename Type>
std::vector<typename AttributeType<Type>::TypeData> AttributeType<Type>::funcs;

template <typename Type>
std::unordered_map<std::string, size_t> AttributeType<Type>::names;

template <typename Type>
std::vector<std::string> AttributeType<Type>::idToName;

class TypelessAttributeType {
public:
  TypelessAttributeType(const TypelessAttributeType &type);
  TypelessAttributeType(const AttributeType<FLOAT_ATTRIBUTE_TYPE> &type);
  TypelessAttributeType(const AttributeType<INT_ATTRIBUTE_TYPE> &type);
  
  bool float_type() const;
  bool int_type() const;
  
  AttributeType<FLOAT_ATTRIBUTE_TYPE> get_float_type() const;
  AttributeType<INT_ATTRIBUTE_TYPE> get_int_type() const;
  
  TypelessAttributeType & operator=(const TypelessAttributeType &type);
  TypelessAttributeType & operator=(const AttributeType<FLOAT_ATTRIBUTE_TYPE> &type);
  TypelessAttributeType & operator=(const AttributeType<INT_ATTRIBUTE_TYPE> &type);
  bool operator==(const TypelessAttributeType &type) const;
  bool operator!=(const TypelessAttributeType &type) const;
private:
  size_t data;
};

template <typename Type>
class Attribute {
public:
  Attribute() : baseValue(Type(0)), current(Type(0)), rawBonusValue(Type(0)), finalBonusValue(Type(0)), rawBonusMul(0.0f), finalBonusMul(0.0f) {}
  ~Attribute() {}
  
  void setType(const AttributeType<Type> &t) { this->t = t; }
  void setBase(const Type &base) { baseValue = base; }
  void setValue(const Type &value) { current = value; }
  
  void addBonus(const Bonus &bonus) { rawBonusValue += bonus.add; rawBonusMul += bonus.mul; }
  void addFinalBonus(const Bonus &bonus) { finalBonusValue += bonus.add; finalBonusMul += bonus.mul; }
  void removeBonus(const Bonus &bonus) { rawBonusValue -= bonus.add; rawBonusMul -= bonus.mul; }
  void removeFinalBonus(const Bonus &bonus) { finalBonusValue -= bonus.add; finalBonusMul -= bonus.mul; }
  
  void add(const float &val) { baseValue += val; }
  void add(const float &val, const float &min, const float &max) {
    baseValue += val;
    
    baseValue = std::max(min, std::min(max, baseValue));
  }
  
  Type base() const { return baseValue; }
  Type value() const { return current; }
  
  // в функцию было бы неплохо засунуть дополнительные данные
  // например какие? игрок/не игрок, 
  void calculate(const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>>& float_finder, const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>>& int_finder) {
    current = t.attributeFunction()(float_finder, int_finder, baseValue, rawBonusValue, rawBonusMul, finalBonusValue, finalBonusMul);
  }
  
  void dumpCurrentValue() {
    baseValue = current;
    rawBonusValue = Type(0);
    finalBonusValue = Type(0);
    rawBonusMul = 0.0f;
    finalBonusMul = 0.0f;
  }
  
  AttributeType<Type> type() const { return t; }
  TypelessAttributeType typeless_type() const { return TypelessAttributeType(t); }
private:
  Type baseValue;
  Type current;
  
  Type rawBonusValue;
  Type finalBonusValue;
  
  float rawBonusMul;     // + 1.0f
  float finalBonusMul;   // + 1.0f
  
  AttributeType<Type> t;
};

struct AttribChangeType {
  uint32_t container;
  
  AttribChangeType();
  AttribChangeType(const bool raw, const bool add);
  void make(const bool raw, const bool add);

  bool bonus_type_raw() const;
  bool bonus_math_add() const;
};

class EntityAI;

struct AttribChangeData {
  AttribChangeType type;
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
    equal
  };
  
  TypelessAttributeType attribType;
  float value;
  comparison comp;
  Type event;
};

class PhysicsComponent2;
class AttributeSystem;
class EventComponent;

// у нас помимо AttributeType должен быть еще какой то MonsterType
class AttributeComponent : public yacs::Component {
public:
  CLASS_TYPE_DECLARE
  
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
    AttributeSystem* system;
    std::vector<float_InitInfo> float_arrtibs;
    std::vector<int_InitInfo> int_arrtibs;
  };
  AttributeComponent(const CreateInfo &info);
  ~AttributeComponent();
  
  void update(const uint64_t &time = 0) override;
  void init(void* userData) override;
  
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
  
  size_t & internalIndex();
private:
  size_t index;
  size_t fcount;
  size_t icount;
  Attribute<FLOAT_ATTRIBUTE_TYPE>* attribsf;
  Attribute<INT_ATTRIBUTE_TYPE>* attribsi;
  PhysicsComponent2* phys;
  EventComponent* events;
  AttributeSystem* system;
  
  // массив изменений характеристик должен быть + многопоточность
//   std::atomic<size_t> count;
//   size_t counter;
  std::vector<AttribChangeData> datas;
  std::vector<AttributeReaction> reactions;
  
  static Container<ExternalData>* externalDatas;
};

class AttributeSystem : public Engine {
public:
  struct CreateInfo {
    dt::thread_pool* pool;
  };
  AttributeSystem(const CreateInfo &info);
  ~AttributeSystem();
  
  void update(const uint64_t &time) override;
  
  void add(AttributeComponent* comp);
  void remove(AttributeComponent* comp);
private:
  dt::thread_pool* pool;
  std::vector<AttributeComponent*> components;
}

#endif
