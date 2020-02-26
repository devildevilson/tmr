#ifndef TYPE_INFO_COMPONENT_H
#define TYPE_INFO_COMPONENT_H

#include "id.h"
#include <atomic>

// тут должна быть просто информация о типе энтити
// например список состояний энтити, id типа и др
// тут же наверное можно пометить объект к удалению

namespace yacs {
  class entity;
}

namespace devils_engine {
  namespace core {
    struct state_t;
  }
  
  namespace game {
    struct ability_t;
  }
  
  namespace components {
    struct type_info {
      struct bit_data {
        static const size_t max_bit = SIZE_WIDTH;
        static const size_t reserved_user_bit = max_bit/2;
        
        std::atomic<size_t> container;
        
        inline bit_data() : container(0) {}
        inline bool is_dead() const {
          const size_t mask = 1 << 0;
          return (container & mask) == mask;
        }
        
        inline bool delete_next() const {
          const size_t mask = 1 << 1;
          return (container & mask) == mask;
        }
        
        inline bool is_player() const {
          const size_t mask = 1 << 2;
          return (container & mask) == mask;
        }
        
        inline bool has_collision_trigger() const {
          const size_t mask = 1 << 3;
          return (container & mask) == mask;
        }
        
        inline bool set_dead(const bool value) {
          const size_t mask = 1 << 0;
          const size_t old_val = value ? container.fetch_or(mask) : container.fetch_and(~(mask));
          return (old_val & mask) == mask;
        }
        
        inline bool set_delete(const bool value) {
          const size_t mask = 1 << 1;
          const size_t old_val = value ? container.fetch_or(mask) : container.fetch_and(~(mask));
          return (old_val & mask) == mask;
          //container = value ? container | (1 << 1) : container & ~(1 << 1);
        }
        
        inline bool set_player(const bool value) {
          const size_t mask = 1 << 2;
          const size_t old_val = value ? container.fetch_or(mask) : container.fetch_and(~(mask));
          return (old_val & mask) == mask;
        }
        
        inline bool set_collision_trigger(const bool value) {
          const size_t mask = 1 << 3;
          const size_t old_val = value ? container.fetch_or(mask) : container.fetch_and(~(mask));
          return (old_val & mask) == mask;
        }
        
        inline bool get_bit(const size_t &index) const {
          if (index >= reserved_user_bit) return false;
          const size_t mask = 1 << (reserved_user_bit + index);
          return (container & mask) == mask;
        }
        
        inline bool set_bit(const size_t &index, const bool value) {
          if (index >= reserved_user_bit) return false;
          const size_t mask = 1 << (reserved_user_bit + index);
          const size_t old_val = value ? container.fetch_or(mask) : container.fetch_and(~(mask));
          return (old_val & mask) == mask;
          //container = value ? container | mask : container & ~mask;
        }
      };
      
      struct int_data {
        static const size_t max_int = 16;
        std::atomic<int64_t> container[max_int];
        
        inline int_data() {
          for (size_t i = 0; i < max_int; ++i) {
            container[i] = 0;
          }
        }
        
        inline int64_t get(const size_t &index) const {
          if (index >= max_int) return INT64_MAX;
          return container[index];
        }
        
        inline int64_t set(const size_t &index, const int64_t &value) {
          if (index >= max_int) return INT64_MAX;
          return container[index].exchange(value);
        }
        
        inline int64_t inc(const size_t &index, const int64_t &value) {
          if (index >= max_int) return INT64_MAX;
          return container[index].fetch_add(value);
        }
        
        inline int64_t dec(const size_t &index, const int64_t &value) {
          if (index >= max_int) return INT64_MAX;
          return container[index].fetch_sub(value);
        }
        
        inline int64_t con(const size_t &index, const int64_t &value) {
          if (index >= max_int) return INT64_MAX;
          return container[index].fetch_and(value);
        }
        
        inline int64_t dis(const size_t &index, const int64_t &value) {
          if (index >= max_int) return INT64_MAX;
          return container[index].fetch_or(value);
        }
        
        inline int64_t ex_dis(const size_t &index, const int64_t &value) {
          if (index >= max_int) return INT64_MAX;
          return container[index].fetch_xor(value);
        }
        
        // ???
//         inline int64_t inv(const size_t &index) {
//           if (index >= max_int) return INT64_MAX;
//           const int64_t norm = container[index];
//           return container[index].exchange(~norm);
//         }
      };
      
      utils::id id;
      utils::id collision_property;
      size_t item_quantity;
      utils::id item_property;
      size_t states_count;
      const core::state_t* const* states;
      const game::ability_t* created_ability;
      const yacs::entity* parent;
      yacs::entity* ent;
      bit_data bit_container;
      int_data int_container;
      // еще добавлю
    };
  }
}

#endif
