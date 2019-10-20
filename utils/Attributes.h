#ifndef ATTRIBUTES_H
#define ATTRIBUTES_H

#include <cstddef>
#include <functional>
#include <vector>
#include <unordered_map>

#define FLOAT_ATTRIBUTE_TYPE float
//#define FLOAT_ATTRIBUTE_TYPE double
#define INT_ATTRIBUTE_TYPE int64_t

struct Bonus {
  float add;
  float mul;
};

template <typename Type>
class Attribute;

template <typename T>
class AttributeFinder {
public:
  AttributeFinder(const size_t &size, T* attribs) noexcept : size(size), attribs(attribs) {}

  template <typename AT>
  T* find(const AT &type) noexcept {
    for (size_t i = 0; i < size; ++i) {
      if (attribs[i].type() == type) return &attribs[i];
    }

    return nullptr;
  }

  template <typename AT>
  const T* find(const AT &type) const noexcept {
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
// только если создать отдельную систему, которая будет хранить указатели, и обновлять все это дело
// то есть примерно тоже самое
enum DataType {
  ATTRIBUTE_DATA_TYPE_NONE,
  ATTRIBUTE_CURRENT_SPEED,
  ATTRIBUTE_UPDATE_EXTERNAL_DATA_MAX_SPEED,
  ATTRIBUTE_UPDATE_EXTERNAL_DATA_ACCELERATION,
  ATTRIBUTE_DATA_TYPE_COUNT
};

template <typename Type>
class AttributeType {
public:
  // что еще?
  struct CreateInfo {
    std::string name;
    DataType type;
    std::function<Type(const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>>&, const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>>&, const Type&, const Type&, const FLOAT_ATTRIBUTE_TYPE&, const Type&, const FLOAT_ATTRIBUTE_TYPE&)> func;
  };

  static void create_types(const std::vector<CreateInfo> &data) noexcept {
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

  static AttributeType get(const std::string &name) noexcept {
    auto itr = names.find(name);
    if (itr != names.end()) return AttributeType(itr->second);

    return AttributeType();
  }

  static bool has(const std::string &name) noexcept {
    auto itr = names.find(name);
    return itr != names.end();
  }

  static size_t count() noexcept {
    return funcs.size();
  }

  static void setFunction(const AttributeType &type, const std::function<Type(const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>>&, const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>>&, const Type&, const Type&, const FLOAT_ATTRIBUTE_TYPE&, const Type&, const FLOAT_ATTRIBUTE_TYPE&)> &func) noexcept {
    funcs[type.id()].func = func;
  }

  static std::function<Type(const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>>&, const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>>&, const Type&, const Type&, const FLOAT_ATTRIBUTE_TYPE&, const Type&, const FLOAT_ATTRIBUTE_TYPE&)> attributeFunction(const AttributeType &type) noexcept {
    return funcs[type.id()].func;
  }

  AttributeType() noexcept : type(SIZE_MAX) {}
  AttributeType(const size_t &id) noexcept : type(id) {}
//  AttributeType(const AttributeType &type) : type(type.id()) {}

  bool valid() const noexcept { return type != SIZE_MAX; }
  size_t id() const noexcept { return type; }
  std::string name() const noexcept { return idToName[type]; }

  void setFunction(const std::function<Type(const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>>&, const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>>&, const Type&, const Type&, const FLOAT_ATTRIBUTE_TYPE&, const Type&, const FLOAT_ATTRIBUTE_TYPE&)> &func) noexcept {
    funcs[type].func = func;
  }

  std::function<Type(const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>>&, const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>>&, const Type&, const Type&, const FLOAT_ATTRIBUTE_TYPE&, const Type&, const FLOAT_ATTRIBUTE_TYPE&)> attributeFunction() noexcept {
    return funcs[type].func;
  }

  DataType data_type() const noexcept { return funcs[type].type; }

//  AttributeType & operator=(const AttributeType &type) { this->type = type.type; }
  bool operator==(const AttributeType &type) const noexcept { return this->type == type.type; }
  bool operator!=(const AttributeType &type) const noexcept { return this->type != type.type; }
private:
  // тут будут еще дополнительный характеристики
  // например, описание, название (не техническое), иконка, ???
  struct TypeData {
    DataType type;
    std::function<Type(const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>>&, const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>>&, const Type&, const Type&, const FLOAT_ATTRIBUTE_TYPE&, const Type&, const FLOAT_ATTRIBUTE_TYPE&)> func;
  };

  size_t type;

  // хранить в статике? не думаю что какие то проблемы вообще будут
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
//  TypelessAttributeType(const TypelessAttributeType &type);
  TypelessAttributeType() noexcept;
  TypelessAttributeType(const AttributeType<FLOAT_ATTRIBUTE_TYPE> &type) noexcept;
  TypelessAttributeType(const AttributeType<INT_ATTRIBUTE_TYPE> &type) noexcept;

  bool valid() const;
  bool float_type() const noexcept;
  bool int_type() const noexcept;

  AttributeType<FLOAT_ATTRIBUTE_TYPE> get_float_type() const noexcept;
  AttributeType<INT_ATTRIBUTE_TYPE> get_int_type() const noexcept;

//  TypelessAttributeType & operator=(const TypelessAttributeType &type);
  TypelessAttributeType & operator=(const AttributeType<FLOAT_ATTRIBUTE_TYPE> &type) noexcept;
  TypelessAttributeType & operator=(const AttributeType<INT_ATTRIBUTE_TYPE> &type) noexcept;
  bool operator==(const TypelessAttributeType &type) const noexcept;
  bool operator!=(const TypelessAttributeType &type) const noexcept;
private:
  size_t data;
};

// аттрибуты могут появиться в любых частях кода, причем и при отрисовке и при инпуте
// например, слепота - эффект который должен помимо прочего затенять экран игрока, это по идее модификатор гаммы
// для того чтобы избежать возможных конфликтов мы должны вынести аттрибут отсюда
// так, я пока еще не понимаю что делать с дрейн аттрибутом, то есть постоянным уменьшением аттрибута
// но с возможностью его восстановления, вообще если действовать по аналогии с хп
// у нас может быть max_strength и strength именно на второй идет воздейтсвие дрейн эффекта
// данный подход позволит решить многие проблемы, но увеличит расход памяти (не очень сильно на самом деле, но все же)

template <typename Type>
class Attribute {
public:
  Attribute() noexcept : baseValue(Type(0)), current(Type(0)), rawBonusValue(Type(0)), finalBonusValue(Type(0)), rawBonusMul(0.0f), finalBonusMul(0.0f) {}
  ~Attribute() = default;

  void setType(const AttributeType<Type> &t) noexcept { this->t = t; }
  void setBase(const Type &base) noexcept { baseValue = base; }
  void setValue(const Type &value) noexcept { current = value; }

  void addBonus(const Bonus &bonus) noexcept { rawBonusValue += bonus.add; rawBonusMul += bonus.mul; }
  void addFinalBonus(const Bonus &bonus) noexcept { finalBonusValue += bonus.add; finalBonusMul += bonus.mul; }
  void removeBonus(const Bonus &bonus) noexcept { rawBonusValue -= bonus.add; rawBonusMul -= bonus.mul; }
  void removeFinalBonus(const Bonus &bonus) noexcept { finalBonusValue -= bonus.add; finalBonusMul -= bonus.mul; }

  void add(const float &val) noexcept { baseValue += val; }
  void add(const float &val, const float &min, const float &max) noexcept {
    baseValue += val;

    baseValue = std::max(min, std::min(max, baseValue));
  }

  Type base() const noexcept { return baseValue; }
  Type value() const noexcept { return current; }

  // в функцию было бы неплохо засунуть дополнительные данные
  // например какие? игрок/не игрок,
  void calculate(const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>>& float_finder, const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>>& int_finder) noexcept {
    current = t.attributeFunction()(float_finder, int_finder, baseValue, rawBonusValue, rawBonusMul, finalBonusValue, finalBonusMul);
  }

  void dumpCurrentValue() noexcept {
    baseValue = current;
    rawBonusValue = Type(0);
    finalBonusValue = Type(0);
    rawBonusMul = 0.0f;
    finalBonusMul = 0.0f;
  }

  AttributeType<Type> type() const noexcept { return t; }
  TypelessAttributeType typeless_type() const noexcept { return TypelessAttributeType(t); }
private:
  Type baseValue;
  Type current;

  Type rawBonusValue;
  Type finalBonusValue;

  float rawBonusMul;     // + 1.0f
  float finalBonusMul;   // + 1.0f

  AttributeType<Type> t;
};

#endif //ATTRIBUTES_H
