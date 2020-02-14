#ifndef ATTRIBUTES_COMPONENT_H
#define ATTRIBUTES_COMPONENT_H

#include <vector>
#include "attribute.h"
#include "ring_buffer.h"
#include "effect.h"

namespace devils_engine {
  namespace components {
    class attributes {
    public:
      static const size_t max_float_attribs = 16;
      static const size_t max_int_attribs = 16;
      static const size_t ring_buffer_size = 20;
      
//       struct changes {
//         const yacs::entity* source;
//         const game::effect_t* effect;
//         struct game::effect_t::container container;
//         //size_t size;
//         //const game::attrib_bonus* bonuses; // это может удалиться
//       };
      
      struct create_info {
        template <typename T>
        struct init {
          T base;
          const struct core::attribute_t<T>::type* t;
        };
        
        yacs::entity* ent;
        std::vector<init<core::float_type>> float_init;
        std::vector<init<core::int_type>> int_init;
      };
      attributes(const create_info &info);
      
      void update(const size_t &time);
      
      template <typename T>
      const core::attribute_t<T>* find(const utils::id &type) const;
      
      template <typename T>
      core::attribute_t<T>* find(const utils::id &type);
      
//       core::attribute_t<core::float_type>::finder get();
//       core::attribute_t<core::int_type>::finder get();
    private:
      yacs::entity* ent;
      size_t float_size;
      size_t int_size;
      char float_attribs[max_float_attribs*sizeof(core::attribute_t<core::float_type>)];
      char int_attribs[max_int_attribs*sizeof(core::attribute_t<core::int_type>)];
      
      // у нас все изменения описаны в эффектах, зачем нам их копировать сюда?
      //utils::ring_buffer<struct changes, ring_buffer_size> changes;
      
      core::attribute_t<core::float_type>* getf();
      core::attribute_t<core::int_type>* geti();
      const core::attribute_t<core::float_type>* getf() const;
      const core::attribute_t<core::int_type>* geti() const;
    };
  }
}

#endif
