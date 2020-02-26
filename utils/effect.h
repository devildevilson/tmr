#ifndef EFFECT_H
#define EFFECT_H

#include <string>
#include "shared_memory_constants.h"
#include "id.h"
#include "attribute.h"

// может быть сделать функцию эффект::апдейт, как тогда определить что я сдох?

namespace devils_engine {
  namespace game {
    struct effect_t {
      static const size_t max_bonuses = 16;
      
      struct container {
        size_t time;
        size_t period;
        //size_t size;
        attrib_bonus bonuses[max_bonuses];
        
        container();
        container(const container &c);
        struct container & operator=(const container &c);
      };
      
      struct type {
        uint32_t container;
        
        type();
        type(const type &t);
        type(const bool raw, const bool add, const bool remove, const bool timer_reset, const bool easy_stack, const bool unique, const bool update, const bool complete_remove, const bool periodic_add, const bool periodic_increase_stack, const bool timed_remove);
        void make(const bool raw, const bool add, const bool remove, const bool timer_reset, const bool easy_stack, const bool unique, const bool update, const bool complete_remove, const bool periodic_add, const bool periodic_increase_stack, const bool timed_remove);
        
        bool raw() const;
        bool add() const;
        bool remove() const; // нужно ли удалить по окончанию действия
        //bool periodically_apply() const; // если период = SIZE_MAX то его нет
        
        bool timer_reset() const; // если этот эффект существует, то просто резетим время
        bool stackable() const; // увеличиваем счетчик стаков + добавляем изменения аттрибутов
        bool unique() const; // если нет, то работаем в целом по эффект типу, добавляем каждый раз
        bool update() const; // если уникальный, то обновляем контейнер, если нет, то что? у всех обновляем контейнер? да
        bool complete_remove() const;
        bool periodic_add() const;
        bool periodic_increase_stack() const;
        bool timed_remove() const;
        type & operator=(const type &t);
      };
      
      utils::id id;
      std::string name; // как бы это убрать в какое то одно хранилище
      std::string description;
      struct type type;
      struct container container;
    };
  }
}

// вычислять где? я думал что можно вполне в пользовательских функциях вычислять

#endif
