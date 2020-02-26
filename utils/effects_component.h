#ifndef EFFECTS_COMPONENT_H
#define EFFECTS_COMPONENT_H

#include <vector>
#include <mutex>
#include "effect.h"
#include "attribute.h"

namespace yacs {
  class entity;
}

// эффекты по идее могут стакаться
// это может означать либо просто дополнительную прибавку effect_t::container
// либо добавление дополнительного computed_effect

// порой мне кажется что просто дать возможность разработчику (то есть через функции) прямого доступа к аттрибутам
// и некоторым переменным лучше, чем городить класс для эффектов, потому что как не городи получается какая то фигня
// вот что я понял: одним из вариантов сделать "правильно" будет задать максимальное количество данных в effect'е
// и задать функцию в которой будет вычисляться и воздействие эффекта и вычисляться дополнительная информация (резист и проч)
// следовательно воздействовать на аттрибуты мы бедем напрямую 

// но тут я сталкиваюсь с проблемой что я не могу понять какие изменения сделаны
// должны быть 2 "атомарных" функции: добавить и удалить
// они должны принимать на вход флаги с возможными действиями
// или даже эти возможные действия должны быть заданы строго в эффекте
// (то есть если стакабл то добавляем и проч)
// при добавлении должны посчитаться аттрибуты тоже
// has()? можно оставить

// есть два типа аттрибутов: аттрибуты изменения которых необходимо применять сразу и которых можно отслеживать
// я имею ввиду что есть принципиальное различие между хп и макс хп
// то есть для хп если будем подбирать хилки, то в конце концов в raw_add или final_add аккумулируются очень большие числа
// и когда мы будем атаковать то отниматься прежде всего будет из raw_add или final_add, а значит нам нужно их
// дампить? нужен механизм чистки этих значений, я думаю что сойдет некая функция дамп, которая после вычислений 
// приведет в дефолтное значение эти числа, но только у тех аттрибутов у которых это явно задано

namespace devils_engine {
  namespace components {
    class attributes;
    
    class effects {
    public:
      enum effects_flags {
        force_add = (1 << 0),
        
      };
      
      struct computed_data {
        static const uint8_t max_stacks = 255;
        size_t container;
        
        computed_data();
        computed_data(const uint8_t &stacks);
        void set_stacks_count(const uint8_t &value);
        uint8_t get_stacks_count() const;
        void dec_stacks(const uint8_t &value = 1);
        void inc_stacks(const uint8_t &value = 1);
        void delete_next();
        bool deletion();
        void set_changed(const bool value);
        bool changed() const;
      };
      
      struct computed_effect {
        struct game::effect_t::container data;
        const game::effect_t* effect;
        yacs::entity* source;
        size_t current_time;
        //size_t stacks;
        computed_data stack_data;
      };
      
      struct effect_source {
        const game::effect_t* effect;
        yacs::entity* source;
      };
      
      struct change_effect {
        const game::effect_t* effect;
        yacs::entity* source;
        game::attrib_bonus attrib_bonus;
      };
      
      struct create_info {
        yacs::entity* ent;
      };
      effects(const create_info &info);
      
      void update(const size_t &time);
      
      size_t has(const utils::id &type) const;
      size_t has(const utils::id &type, const yacs::entity* source) const;
      
      bool reset_timer(const utils::id &id, size_t &mem);
      bool reset_timer(const utils::id &id, const yacs::entity* source, size_t &mem);
      bool increase_stack(const utils::id &id, const bool add, size_t &mem);
      bool increase_stack(const utils::id &id, const yacs::entity* source, const bool add, size_t &mem);
      
      bool add(const struct game::effect_t::container &data, const game::effect_t* effect, yacs::entity* source);
      bool remove(const utils::id &id, const bool complete_remove);
      bool remove(const utils::id &id, const yacs::entity* source, const bool complete_remove);
      
      // обход по эффектам
      // мне нужно определить есть ли среди атакующих игрок 
      // но для интерфейса мне все равно придется сделать какой то обход
      effect_source next(size_t &mem) const;
//       change_effect next(const utils::id &attrib_id, size_t &mem) const;
      change_effect next_change(const utils::id &attrib_id, size_t &mem) const;
    private:
      yacs::entity* ent;
      std::vector<computed_effect> datas;
      mutable std::mutex mutex;
//       std::vector<change_effect> changes;
      
      size_t find(const size_t &start_index, const utils::id &type) const;
      size_t find(const size_t &start_index, const utils::id &type, const yacs::entity* source) const;
      void compute_add(attributes* attribs, const computed_effect &effect, const bool is_needed) const;
      void compute_remove(attributes* attribs, const computed_effect &effect, const bool is_needed) const;
//       void compute_add(const computed_effect &effect, const bool is_needed);
//       void compute_remove(const computed_effect &effect, const bool is_needed);
    };
  }
  
//   namespace systems {
//     class effects {
//       
//     };
//   }
}

#endif
