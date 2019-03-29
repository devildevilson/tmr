#ifndef ATTRIBUTES_COMPONENT_H
#define ATTRIBUTES_COMPONENT_H

#include <unordered_map>

#include "Globals.h"
#include "EntityComponentSystem.h"
#include "Utility.h"

class Bonus {
public:
  Bonus(const float &val, const float &mul);
  
  float add = 0.0f;
  float mul = 0.0f;
};

class Attribute;

struct DependOnAttrib {
  DependOnAttrib() {}
  
  DependOnAttrib(Attribute* attrib, const float &add, const float &mul) {
    this->attrib = attrib;
    this->add = add;
    this->mul = mul;
  }
  
  Attribute* attrib = nullptr;
  float add = 0.0f;
  float mul = 0.0f;
};

class Attribute {
public:
  Attribute(const float &base = 0.0f, const float &current = 0.0f, const float &min = 0.0f) {
    baseVal = base;
    currentVal = current;
    minVal = min;
  }
  
  ~Attribute () {}

  void dependOn(Attribute* attrib, const float &add, const float &mul) {
    attribs.push_back({attrib, add, mul});
    calculated = false;
  }
//   void removeParentAttribute(const uint64_t &actorAttribute);
  
  void addBonus(const Bonus &bonus) {
    rawBonusValue += bonus.add;
    rawBonusMul += bonus.mul;
    
    calculated = false;
  }
  
  void addFinalBonus(const Bonus &bonus) {
    finalBonusValue += bonus.add;
    finalBonusMul += bonus.mul;
    
    calculated = false;
  }
  
  void removeBonus(const Bonus &bonus) {
    rawBonusValue -= bonus.add;
    rawBonusMul -= bonus.mul;
    
    calculated = false;
  }
  
  void removeFinalBonus(const Bonus &bonus) {
    finalBonusValue -= bonus.add;
    finalBonusMul -= bonus.mul;
    
    calculated = false;
  }
  
  float base() const {
    return baseVal;
  }
  
  float max() {
    return calculated ? maxVal : calculateValue();
  }
  
  float current() const {
    return currentVal;
  }
  
  float min() const {
    return minVal;
  }
  
  void add(const float &value) {
    if (currentVal >= maxVal) return;
    
    currentVal += value;
    if (currentVal > maxVal) currentVal = maxVal;
  }
  
  void forceAdd(const float &value) {
    currentVal += value;
  }
  
  void sub(const float &value) {
    currentVal -= value;
    if (currentVal < minVal) currentVal = minVal;
  }
  
  bool isCurrentEqMin() const {
    return fast_fabsf(currentVal - minVal) < EPSILON;
  }
  
  void operator=(const Attribute &other) {
    this->maxVal = other.maxVal;
    this->currentVal = other.currentVal;
    this->minVal = other.minVal;
    this->baseVal = other.baseVal;
    
    this->rawBonusValue = other.rawBonusValue;
    this->rawBonusMul = other.rawBonusMul;
    this->finalBonusValue = other.finalBonusValue;
    this->finalBonusMul = other.finalBonusMul;
    
    this->attribs = other.attribs;
    calculated = false;
  }
  
//   bool operator==(const Attribute &other) const {
//     return fast_fabsf(maxVal - maxVal) < EPSILON && fast_fabsf(maxVal - maxVal) < EPSILON && fast_fabsf(maxVal - maxVal) < EPSILON;
//   }
private:
  float calculateValue() {
    maxVal = baseVal;
    
    for (const auto &info : attribs) {
      maxVal += (info.attrib->max() * info.mul) + info.add;
    }
    
    maxVal += rawBonusValue;
    maxVal *= (rawBonusMul + 1.0f);
    
    maxVal += finalBonusValue;
    maxVal *= (finalBonusMul + 1.0f);
    
    if (attribs.empty()) calculated = true;
    
    return maxVal;
  }
  
  bool calculated = false;
  
  float maxVal = 0.0f;
  float currentVal = 0.0f;
  float minVal = 0.0f;
  float baseVal = 0.0f;
  
  float rawBonusValue   = 0.0f;
  float rawBonusMul     = 0.0f; // + 1.0f
  float finalBonusValue = 0.0f;
  float finalBonusMul   = 0.0f; // + 1.0f

  std::vector<DependOnAttrib> attribs;
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

class Attribute2 {
public:
  Attribute2(const float &base);
//   Attribute2(const float &base, const float &min);
//   Attribute2(const float &base, const float &min, const float &max);
  ~Attribute2();
  
  void setMax(Attribute2* attrib);
  void setMin(Attribute2* attrib);
  void setDependency(Attribute2* dep, const float &addDep, const float &mulDep);
  
  void addBonus(const Bonus &bonus);
  void addFinalBonus(const Bonus &bonus);
  void removeBonus(const Bonus &bonus);
  void removeFinalBonus(const Bonus &bonus);
  
  void add(const float &val);
  void add(const float &val, const float &min, const float &max);
//   void sub(const float &val);
//   void subBound(const float &val, const float &max);
  float max() const;
  float min() const;
  float value() const;
  bool calculated() const;
private:
  // указатель по идее можно тоже получать из мемори пула, но там к сожалению неприятно использовать с флоатами
  // нет указатель в принципе будет использовать неудобно
  //float* currentPtr;
  
//   float maxVal;
  float base;
//   float minVal;
  float current;
  
  float rawBonusValue;
  float rawBonusMul;     // + 1.0f
  float finalBonusValue;
  float finalBonusMul;   // + 1.0f
  
  Attribute2* minAttrib;
  Attribute2* maxAttrib;
  
  float addDep;
  float mulDep;
  Attribute2* dependency;
  
  Attribute2* attribs; // указатель на все атрибуты
  std::function<float(const Attribute2*, const float&, const float&, const float&, const float&, const float&)> attribFunc;
  
  // аттрибуты от которых зависит как сделать?
  
  // + нам еще нужно придумать как сделать инт атрибуты
  
  // мне еще пригодится 
  
  float calculateValue();
};

class Attributes : public yacs::Component {
public:
  CLASS_TYPE_DECLARE
  
  Attributes() {}
  ~Attributes() {}
  
  void update(const uint64_t &time = 0) override { (void)time; }
  void init(void* userData) override { (void)userData; }
  
  void add(const Type &type, const float &base = 0.0f, const float &current = 0.0f, const float &min = 0.0f) {
    attribs[type.getType()] = pool.newElement(base, current, min);
  }
  
//   Attribute* get(const Type &type) {
//     auto itr = attribs.find(type.getType());
//     if (itr == attribs.end()) return nullptr;
//     
//     return itr->second;  
//   }
  
  Attribute* get(const Type &type) const {
    auto itr = attribs.find(type.getType());
    if (itr == attribs.end()) return nullptr;
    
    return itr->second;
  }
private:
  MemoryPool<Attribute, sizeof(Attribute)*10> pool;
  std::unordered_map<size_t, Attribute*> attribs;
};

#endif
