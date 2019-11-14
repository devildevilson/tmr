#include "ItemType.h"

ItemType::ItemType(const CreateInfo &info) : m_id(info.m_id), m_group(info.m_group), m_ability(info.m_ability), m_name(info.m_name), m_description(info.m_description), pickupEffects(info.pickupEffects), weaponEffects(info.weaponEffects), weaponAbilitySlots(info.weaponAbilitySlots) {}
Type ItemType::id() const { return m_id; }
Type ItemType::group() const { return m_group; }
const AbilityType* ItemType::ability() const { return m_ability; }
std::string ItemType::name() const { return m_name; }
std::string ItemType::description() const { return m_description; }
const std::vector<const Effect*> & ItemType::onPickup() const { return pickupEffects; }
const std::vector<const Effect*> & ItemType::onTakingWapon() const { return weaponEffects; }
const std::vector<ItemType::AbilitySlot> & ItemType::weaponAbilities() const { return weaponAbilitySlots; }
bool ItemType::isWeapon() const { return m_ability == nullptr; }