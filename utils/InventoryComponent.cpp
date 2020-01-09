#include "InventoryComponent.h"

#include <cstring>

PickupItem::PickupItem(const CreateInfo &info) : m_quantity(info.quantity), m_type(info.type), m_effect(info.effect) {}
size_t PickupItem::quantity() const { return m_quantity; }
const ItemType* PickupItem::type() const { return m_type; }
const Effect* PickupItem::effect() const { return m_effect; }

PickupWeapon::PickupWeapon(const CreateInfo &info) : m_type(info.m_type) {}
const ItemType* PickupWeapon::type() const { return m_type; }

InventoryComponent::InventoryComponent(const CreateInfo &info) : items(info.predefinedItems) {}
size_t InventoryComponent::add(const ItemType* item, const size_t &count) {
  if (count == 0) return 0;
  
  const size_t index = findIndex(0, item);
  if (index == SIZE_MAX) {
    items.push_back({item, count});
    return count;
  }
  
  items[index].count += count;
  return items[index].count;
}

size_t InventoryComponent::remove(const ItemType* item, const size_t &count) {
  if (count == 0) return 0;
  
  const size_t index = findIndex(0, item);
  if (index == SIZE_MAX) return 0;
  
  const size_t ret = std::min(items[index].count, count);
  items[index].count -= ret;
  if (items[index].count == 0) {
    std::swap(items[index], items.back());
    items.pop_back();
  }
  
  return ret;
}

size_t InventoryComponent::size() const {
  return items.size();
}

InventoryComponent::ItemStack InventoryComponent::get(const size_t &index) const {
  if (index >= items.size()) return {nullptr, 0};
  return items[index];
}

struct CompareItems {
  bool operator()(const InventoryComponent::ItemStack &first, const InventoryComponent::ItemStack &second) const {
    return first.type->group() < second.type->group();
  }
};

// условие? по идее сортировать мы должны по итем группе
void InventoryComponent::sort() {
  std::sort(items.begin(), items.end(), CompareItems());
}

size_t InventoryComponent::findIndex(const size_t &start, const ItemType* type) const {
  for (size_t i = start; i < items.size(); ++i) {
    if (items[i].type == type) return i;
  }

  return SIZE_MAX;
}

ArrayBool::ArrayBool() : container(0) {}
ArrayBool::ArrayBool(const ArrayBool &array) : container(array.container) {}

void ArrayBool::set(const size_t &index, const bool value) {
  if (index >= MAX_WEAPON_COUNT) return;
  const size_t mask = value ? 1 << index : ~(1 << index);
  container = value ? container | mask : container & mask;
}

bool ArrayBool::get(const size_t &index) const {
  if (index >= MAX_WEAPON_COUNT) return false;
  const size_t mask = 1 << index;
  return (container & mask) == mask;
}

WeaponsComponent::WeaponsComponent(const CreateInfo &info) : weaponCount(info.weaponCount), hasWeapons(info.predefinedWeapons), types(info.types) {}

bool WeaponsComponent::take(const ItemType* type) {
  for (size_t i = 0; i < weaponCount; ++i) {
    if (types[i] == type) {
      hasWeapons.set(i, true);
      return true;
    }
  }
  
  return false;
}

bool WeaponsComponent::remove(const ItemType* type) {
  for (size_t i = 0; i < weaponCount; ++i) {
    if (types[i] == type) {
      hasWeapons.set(i, false);
      return true;
    }
  }
  
  return false;
}

bool WeaponsComponent::remove(const size_t &index) {
  if (index >= weaponCount) return false;
  hasWeapons.set(index, false);
  return true;
}

bool WeaponsComponent::hasWeapon(const size_t &index) const {
  return hasWeapons.get(index);
}

const ItemType* WeaponsComponent::get(const size_t &index) const {
  if (!hasWeapon(index)) return nullptr;
  if (index >= weaponCount) return nullptr;
  return types[index];
}
