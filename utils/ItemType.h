#ifndef ITEM_TYPE_H
#define ITEM_TYPE_H

#include "Type.h"

#include <vector>

class Effect;
class AbilityType;

class ItemType {
public:
  struct AbilitySlot {
    size_t slotIndex;
    const AbilityType* ability;
    Type state;
  };
  
  struct CreateInfo {
    Type m_id;
    Type m_group;
    const AbilityType* m_ability;
    std::string m_name;
    std::string m_description;
    std::vector<const Effect*> pickupEffects;
    std::vector<const Effect*> weaponEffects;
    std::vector<AbilitySlot> weaponAbilitySlots;
  };
  ItemType(const CreateInfo &info);
  
  Type id() const;
  Type group() const; // может быть тут нужен индекс, наверное будет лучше индексом (потом изменю)
  const AbilityType* ability() const;
  std::string name() const;
  std::string description() const;
  const std::vector<const Effect*> & onPickup() const;
  const std::vector<const Effect*> & onTakingWapon() const;
  const std::vector<AbilitySlot> & weaponAbilities() const;
  
  bool isWeapon() const;
private:
  Type m_id;
  Type m_group;
  const AbilityType* m_ability;
  std::string m_name;
  std::string m_description;
  std::vector<const Effect*> pickupEffects;
  std::vector<const Effect*> weaponEffects;
  std::vector<AbilitySlot> weaponAbilitySlots;
};

#endif
