#ifndef INVENTORY_COMPONENT_H
#define INVENTORY_COMPONENT_H

#include <cstdint>
#include <cstddef>
#include <vector>

#include "Type.h"
#include "Attributes.h"
#include "MemoryPool.h"
#include "Interaction.h"
#include "ItemType.h"
#include "Attributes.h"

#include "EventFunctor.h"

// для того чтобы смоделировать какое то поведение
// можно использовать дерево поведения
// нужно сделать возможность создать дерево поведения с единственным нодом
// это будет просто функция, которая будет делать примерно тоже самое что и актион нод
// в дереве но без дерева, нужен механизм пробуждения, 
// так я могу сделать простенькие действия для стен и дверей например
// что еще я не учел? как эти дела делать по нажатию кнопки? пробуждать объект по нажатию
// как возвращать объект в исходное положение? должно пройти какое то время и опять объект пробудится
// время? механизм какой нибудь должен откладывать пробуждение объекта, добавлять пробуждение как работу?
// в принципе почему нет, че с кнопками? нужно где то хранить указатели на объекты пробуждения
// это сложнее, мы можем использовать для этого блекбоард (в принципе зачем он еще нужен), 
// но там нужен тип, впринципе с этим можно работать, как передавать какие то данные при соприкосновении?
// опять же у нас есть функция, проверяем с кем мы соприкоснулись и передаем им какие то вещи
// так а как передаем? у нас будет функция pickup, нам еще что нибудь может потребоваться?

// мы можем проверить берем ли мы айтем в дереве поведения
// нужен метод pickup который должен проверять является ли энтити предметом,
// брать его (добавлять в инвентарь либо применять эффект), а потом удалять если это предмет

namespace yacs{
  class entity;
}

class AttributesComponent;
class EffectComponent;
class InteractionComponent;
class Effect;
class StateControllerType;
class InventoryComponent;

class PickupItem {
public:
  using PickupFunc = std::function<bool(const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>>&, const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>>&, const EffectComponent*, const InventoryComponent*, const PickupItem*)>;
  
  struct CreateInfo {
    size_t quantity;
    const ItemType* type;
    const Effect* effect;
    PickupFunc pickup_func;
  };
  PickupItem(const CreateInfo &info);

  size_t quantity() const;
  const ItemType* type() const;
  const Effect* effect() const;
  bool check_pickup(const AttributeFinder<Attribute<FLOAT_ATTRIBUTE_TYPE>> &float_finder, const AttributeFinder<Attribute<INT_ATTRIBUTE_TYPE>> &int_finder, const EffectComponent* effects, const InventoryComponent* inventory) const;
protected:
  size_t m_quantity;
  const ItemType* m_type;
  const Effect* m_effect;
  
  PickupFunc pickup_func;
};

class PickupWeapon {
public:
  struct CreateInfo {
    const ItemType* m_type;
  };
  PickupWeapon(const CreateInfo &info);
  
  const ItemType* type() const;
private:
  const ItemType* m_type;
};

class InventoryComponent {
public:
  struct ItemStack {
    const ItemType* type;
    size_t count;
  };
  
  struct CreateInfo {
    std::vector<ItemStack> predefinedItems;
  };
  InventoryComponent(const CreateInfo &info);

  size_t add(const ItemType* item, const size_t &count); // возвращает сколько айтемов этого типа после добавления
  size_t remove(const ItemType* item, const size_t &count); // возвращает сколько удалось удалить
  size_t size() const;
  ItemStack get(const size_t &index) const;

  // условие? по идее сортировать мы должны по итем группе
  void sort();
private:
  std::vector<ItemStack> items;

  size_t findIndex(const size_t &start, const ItemType* type) const;
};

#define MAX_WEAPON_COUNT (sizeof(size_t)*CHAR_WIDTH)

struct ArrayBool {
  size_t container;
  
  ArrayBool();
  ArrayBool(const ArrayBool &array);
  
  void set(const size_t &index, const bool value);
  bool get(const size_t &index) const;
};

class WeaponsComponent {
public:
  struct CreateInfo {
    size_t weaponCount;
    const ItemType** types;
    ArrayBool predefinedWeapons;
  };
  WeaponsComponent(const CreateInfo &info);
  
  bool take(const ItemType* type);
  bool remove(const ItemType* type);
  bool remove(const size_t &index);
  
  bool hasWeapon(const size_t &index) const;
  const ItemType* get(const size_t &index) const;
private:
  const size_t weaponCount;
  ArrayBool hasWeapons;
  const ItemType** types; // тут надо бы только указатель откуда то брать
};

class UserInventoryComponent {
public:

private:

};

#endif //INVENTORY_COMPONENT_H
