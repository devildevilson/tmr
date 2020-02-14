#ifndef ABILITIES_COMPONENT_H
#define ABILITIES_COMPONENT_H

#include "id.h"

namespace devils_engine {
  namespace game {
    struct weapon_t;
  }
  
  namespace components {
    // это для монстров, все что нужно это сохранять текущую абилку
    // зачем? еще же хрен проверишь по стейту к какой абилке он относится
    // 
    class abilities {
    public:
      
    private:
      
    };
    
    // во первых нам тут нужно проверить есть ли у нас это оружее
    // а для этого нужно откуда то получить указатель на список оружия
    class weapons {
    public:
      static const size_t max_count = SIZE_WIDTH;
      
      struct availability {
        size_t container;
        
        availability();
        bool has(const size_t &index) const;
        void set(const size_t &index, const bool value);
      };
      
      struct create_info {
        size_t count;
        const game::weapon_t** weapons;
      };
      weapons(const create_info &info);
      
      const game::weapon_t* get(const size_t &index) const;
      const game::weapon_t* get(const utils::id &id) const;
      
      void pick(const game::weapon_t* weapon);
      
      bool has(const size_t &index) const;
      bool has(const utils::id &id) const;
      
      void change();
      void set_pending(const size_t &index);
      void set_pending(const utils::id &id);
      
      size_t find(const utils::id &id) const;
      
      const game::weapon_t* current() const;
      const game::weapon_t* pending() const;
    private:
      const game::weapon_t* m_current;
      const game::weapon_t* m_pending;
      
      availability container;
      size_t count;
      const game::weapon_t** type_weapons;
    };
  }
}

// обработка поднятий предметов может быть и в одном потоке
// это будет происходить не слишком часто
// обработка урона? нужно вызвать функцию "дамаг" (сорс, обдж, эффект)
// урон может быть многопоточным

#endif
