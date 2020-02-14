#ifndef ATTRIBUTES_H
#define ATTRIBUTES_H

#include <cstddef>
#include <functional>
#include <vector>
#include <unordered_map>

#include "Type.h"

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
  T* find(const AT* type) noexcept {
    for (size_t i = 0; i < size; ++i) {
      if (attribs[i].type() == type) return &attribs[i];
    }

    return nullptr;
  }

  template <typename AT>
  const T* find(const AT* type) const noexcept {
    for (size_t i = 0; i < size; ++i) {
      if (attribs[i].type() == type) return &attribs[i];
    }

    return nullptr;
  }
  
  template <typename AT>
  T* find(const AT &type) noexcept {
    for (size_t i = 0; i < size; ++i) {
      if (attribs[i].type()->id() == type) return &attribs[i];
    }

    return nullptr;
  }

  template <typename AT>
  const T* find(const AT &type) const noexcept {
    for (size_t i = 0; i < size; ++i) {
      if (attribs[i].type()->id() == type) return &attribs[i];
    }

    return nullptr;
  }
private:
  size_t size;
  T* attribs;
};

// тип аттрибута нужно переделать конечно, сделать его константным указателем на память

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

template <typename ValueType>
class AttributeType {
public:
  using FuncType = std::function<ValueType(const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>>&, const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>>&, const ValueType&, const ValueType&, const FLOAT_ATTRIBUTE_TYPE&, const ValueType&, const FLOAT_ATTRIBUTE_TYPE&)>;
  
  struct CreateInfo {
    Type id;
    std::string name;
    std::string description;
    DataType type;
    FuncType func;
  };

  AttributeType(const CreateInfo &info) noexcept : m_id(info.id), m_name(info.name), m_description(info.description), typeOfType(info.type), typeFunction(info.func) {}

  Type id() const noexcept { return m_id; }
  std::string name() const noexcept { return m_name; }
  std::string description() const noexcept { return m_description; }
  DataType data_type() const noexcept { return typeOfType; }
  
  ValueType computeAttribute(const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>>& float_finder, const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>>& int_finder, const ValueType& current, const ValueType& rawAdd, const FLOAT_ATTRIBUTE_TYPE& rawMul, const ValueType& finalAdd, const FLOAT_ATTRIBUTE_TYPE& finalMul) const noexcept {
    return typeFunction(float_finder, int_finder, current, rawAdd, rawMul, finalAdd, finalMul);
  }
private:  
  Type m_id;
  std::string m_name;
  std::string m_description;
  DataType typeOfType;
  FuncType typeFunction;
  // иконка
};

class TypelessAttributeType {
public:
  TypelessAttributeType() noexcept;
  TypelessAttributeType(const AttributeType<FLOAT_ATTRIBUTE_TYPE>* type) noexcept;
  TypelessAttributeType(const AttributeType<INT_ATTRIBUTE_TYPE>* type) noexcept;

  bool valid() const;
  bool float_type() const noexcept;
  bool int_type() const noexcept;

  const AttributeType<FLOAT_ATTRIBUTE_TYPE>* get_float_type() const noexcept;
  const AttributeType<INT_ATTRIBUTE_TYPE>* get_int_type() const noexcept;

//  TypelessAttributeType & operator=(const TypelessAttributeType &type);
  TypelessAttributeType & operator=(const AttributeType<FLOAT_ATTRIBUTE_TYPE>* type) noexcept;
  TypelessAttributeType & operator=(const AttributeType<INT_ATTRIBUTE_TYPE>* type) noexcept;
  bool operator==(const TypelessAttributeType &type) const noexcept;
  bool operator!=(const TypelessAttributeType &type) const noexcept;
private:
  // если мы используем id, то нам нужен будет лоадер, причем мне нужен один бит для того чтобы определить к какому типу принадлежит
  bool type;
  const void* data;
};

// аттрибуты могут появиться в любых частях кода, причем и при отрисовке и при инпуте
// например, слепота - эффект который должен помимо прочего затенять экран игрока, это по идее модификатор гаммы
// для того чтобы избежать возможных конфликтов мы должны вынести аттрибут отсюда
// так, я пока еще не понимаю что делать с дрейн аттрибутом, то есть постоянным уменьшением аттрибута
// но с возможностью его восстановления, вообще если действовать по аналогии с хп
// у нас может быть max_strength и strength именно на второй идет воздейтсвие дрейн эффекта
// данный подход позволит решить многие проблемы, но увеличит расход памяти (не очень сильно на самом деле, но все же)

// мне не нравится что мы разделяем аттрибут на типы с помощью темплейтов
// с другой стороны тяжело наверное будет с функцией (не тяжело, но лишнее место будет занимать)
// что возвращать? короч идея такая себе

// union A {
//   FLOAT_ATTRIBUTE_TYPE float_value;
//   INT_ATTRIBUTE_TYPE int_value;
// };

template <typename Type>
class Attribute {
public:
  Attribute() noexcept : baseValue(Type(0)), current(Type(0)), rawBonusValue(Type(0)), finalBonusValue(Type(0)), rawBonusMul(1.0f), finalBonusMul(1.0f) {}
  ~Attribute() = default;

  void setType(const AttributeType<Type>* t) noexcept { this->t = t; }
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
    current = t->computeAttribute(float_finder, int_finder, baseValue, rawBonusValue, rawBonusMul, finalBonusValue, finalBonusMul);
  }

  void dumpCurrentValue() noexcept {
    baseValue = current;
    rawBonusValue = Type(0);
    finalBonusValue = Type(0);
    rawBonusMul = 1.0f;
    finalBonusMul = 1.0f;
  }

  const AttributeType<Type>* type() const noexcept { return t; }
  TypelessAttributeType typeless_type() const noexcept { return TypelessAttributeType(t); }
protected:
  Type baseValue;
  Type current;

  Type rawBonusValue;
  Type finalBonusValue;

  float rawBonusMul;     // + 1.0f
  float finalBonusMul;   // + 1.0f

  const AttributeType<Type>* t;
};

#endif //ATTRIBUTES_H
