#include "abilities_component.h"

#include "weapon.h"

namespace devils_engine {
  namespace components {
    weapons::availability::availability() : container(0) {}
    bool weapons::availability::has(const size_t &index) const {
      if (index >= max_count) return false;
      const size_t mask = 1 << index;
      return (container & mask) == mask;
    }
    
    void weapons::availability::set(const size_t &index, const bool value) {
      if (index >= max_count) return;
      container = value ? container | (1 << index) : container & ~(1 << index);
    }
    
    weapons::weapons(const create_info &info) : m_current(nullptr), m_pending(nullptr), count(info.count), type_weapons(info.weapons) {}
      
    const game::weapon_t* weapons::get(const size_t &index) const {
      if (container.has(index)) return type_weapons[index];
      return nullptr;
    }
    
    const game::weapon_t* weapons::get(const utils::id &id) const {
      const size_t index = find(id);
      if (container.has(index)) return type_weapons[index];
      return nullptr;
    }
    
    void weapons::pick(const game::weapon_t* weapon) {
      const size_t index = find(weapon->id);
      container.set(index, true);
    }
    
    bool weapons::has(const size_t &index) const {
      return container.has(index);
    }
    
    bool weapons::has(const utils::id &id) const {
      const size_t index = find(id);
      return container.has(index);
    }
    
    void weapons::change() {
      m_current = m_pending;
      m_pending = nullptr;
    }
    
    void weapons::set_pending(const size_t &index) {
      m_pending = get(index);
    }
    
    void weapons::set_pending(const utils::id &id) {
      m_pending = get(id);
    }
    
    size_t weapons::find(const utils::id &id) const {
      for (size_t i = 0; i < count; ++i) {
        if (type_weapons[i]->id == id) return i;
      }
      return SIZE_MAX;
    }
    
    const game::weapon_t* weapons::current() const { return m_current; }
    const game::weapon_t* weapons::pending() const { return m_pending; }
  }
}
