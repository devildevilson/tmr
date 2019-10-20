#include "InventoryComponent.h"

#include <cstring>

ItemType::ItemType(const CreateInfo &info)
  : typeId(info.id),
    itemGroup(info.itemGroup),
    typeName(info.name),
    typeDescription(info.description),
    effect(info.itemEffect),
    attackEvent(info.itemEvent),
    attackType(info.itemType),
    interactionData(info.interactionData),
    weaponData(info.weaponData) {}

Type ItemType::id() const {
  return typeId;
}

Type ItemType::group() const {
  return itemGroup;
}

std::string ItemType::name() const {
  return typeName;
}

std::string ItemType::description() const {
  return typeDescription;
}

const StateControllerType* ItemType::playerStates() const {
  return weaponData.player;
}

const StateControllerType* ItemType::objectStates() const {
  return weaponData.other;
}

bool ItemType::isWeapon() const {
  return weaponData.ammoType.valid() && weaponData.weaponEffect != nullptr && weaponData.player != nullptr && weaponData.other != nullptr;
}

const Effect* ItemType::itemEffect() const {
  return effect;
}

Type ItemType::useEvent() const {
  return attackEvent;
}

enum Interaction::type ItemType::useType() const {
  return attackType;
}

const InteractionData & ItemType::itemInteractionData() const {
  return interactionData;
}

const WeaponItemData & ItemType::weaponItemData() const {
  return weaponData;
}

Item::Item(const CreateInfo &info) : m_quantity(info.quantity), m_type(info.type) {}

size_t Item::quantity() const {
  return m_quantity;
}

const ItemType* Item::type() const {
  return m_type;
}

InventoryComponent::InventoryComponent(const CreateInfo &info) : currentItemIndex(0), items(info.predefinedItems), weaponCountVar(info.weaponCount), currentWeaponIndex(0), weapons(nullptr) {
  if (weaponCountVar != 0) {
    weapons = new const ItemType*[weaponCountVar];
    memset(weapons, 0, weaponCountVar*sizeof(const ItemType*));
  }
}

InventoryComponent::~InventoryComponent() {
  delete [] weapons;
}

size_t InventoryComponent::add(const ItemType* item, const size_t &count) {
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

  const size_t delCount = std::min(items[index].count, count);
  items[index].count -= delCount;
  if (items[index].count == 0) {
    std::swap(items[index], items.back());
    items.pop_back();
  }

  return delCount;
}

size_t InventoryComponent::nextItem() {
  currentItemIndex = (currentItemIndex+1) % items.size();
}

size_t InventoryComponent::count(const ItemType* type) const {
  const size_t index = findIndex(0, type);
  return index != SIZE_MAX ? items[index].count : 0;
}

size_t InventoryComponent::index(const ItemType* type) const {
  return findIndex(0, type);
}

const ItemType* InventoryComponent::currentItem() const {
  if (items.empty()) return nullptr;
  return items[currentItemIndex].type;
}

size_t InventoryComponent::itemCount() const {
  return items.size();
}

void InventoryComponent::addWeapon(const ItemType* weapon, const size_t &index) {
  ASSERT(weaponCountVar > index);
  weapons[index] = weapon;
}

void InventoryComponent::removeWeapon(const ItemType* weapon) {
  for (size_t i = 0; i < weaponCountVar; ++i) {
    if (weapons[i] == weapon) {
      weapons[i] = nullptr;
      break;
    }
  }
}

const ItemType* InventoryComponent::nextWeapon() {
  // тут наверное нужно запустить эвент на смену оружия
  // то есть взять стейт контроллер и заставить его сменить анимацию и звук
  // как ограничить смену оружия например при атаке? скорее всего нужно сделать ограничения сверху
  currentWeaponIndex = (currentWeaponIndex+1) % weaponCountVar;
}

const ItemType* InventoryComponent::weapon(const size_t &index) const {
  ASSERT(weaponCountVar > index);
  return weapons[index];
}

bool InventoryComponent::hasWeapon(const ItemType* weapon) const {
  for (size_t i = 0; i < weaponCountVar; ++i) {
    if (weapons[i] == weapon) return true;
  }

  return false;
}

bool InventoryComponent::hasWeapon(const Type &type) const {
  for (size_t i = 0; i < weaponCountVar; ++i) {
    if (weapons[i]->id() == type) return true;
  }

  return false;
}

const ItemType* InventoryComponent::currentWeapon() const {
  return weapons[currentWeaponIndex];
}

size_t InventoryComponent::weaponCount() const {
  return weaponCountVar;
}

struct CompareItems {
  bool operator()(const ItemStack &first, const ItemStack &second) const {
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
