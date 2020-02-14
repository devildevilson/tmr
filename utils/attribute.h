#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include <cstdint>
#include <string>
#include <functional>
#include "id.h"

namespace yacs {
  class entity;
}

namespace devils_engine {
  namespace game {
    struct bonus_t {
      float add;
      float mul;
    };
    
    struct attrib_bonus {
      utils::id attrib;
      bonus_t bonus;
    };
  }
  
  namespace core {
    using float_type = float;
    using int_type = int64_t;
    
    template <typename T>
    class attribute_t {
    public:
      struct type {
        using func_type = std::function<T(yacs::entity*, const type*, const T&, const T&, const float&, const T&, const float&)>;
        
        utils::id id;
        std::string name;
        std::string description;
        // удобно делать разные действия при вычислении аттрибута, например если мы вычислили что хп = 0, то нужно умереть
        // для этого нужно передать не конст, но с другой стороны больше конст = меньше ошибок
        func_type compute;
        // я так полагаю что тут могут появится некие булевы значения, например дамп raw и final значений
        // хорошо бы сразу понять какие
        bool dumping;
      };
      
      class finder {
      public:
        finder(const size_t &size, attribute_t<T>* ptr) : size(size), ptr(ptr) {}
        
        const attribute_t* find(const utils::id &id) const {
          for (size_t i = 0; i < size; ++i) {
            if (ptr[i].type()->id == id) return &ptr[i];
          }
          return nullptr;
        }
        
        const attribute_t* find(const struct type* type) const {
          for (size_t i = 0; i < size; ++i) {
            if (ptr[i].type() == type) return &ptr[i];
          }
          return nullptr;
        }
        
        attribute_t* find(const utils::id &id) {
          for (size_t i = 0; i < size; ++i) {
            if (ptr[i].type()->id == id) return &ptr[i];
          }
          return nullptr;
        }
        
        attribute_t* find(const struct type* type) {
          for (size_t i = 0; i < size; ++i) {
            if (ptr[i].type() == type) return &ptr[i];
          }
          
          return nullptr;
        }
      private:
        size_t size;
        attribute_t<T>* ptr;
      };
      
      class const_finder {
      public:
        const_finder(const size_t &size, const attribute_t<T>* ptr) : size(size), ptr(ptr) {}
        
        const attribute_t* find(const utils::id &id) const {
          for (size_t i = 0; i < size; ++i) {
            if (ptr[i].type()->id == id) return &ptr[i];
          }
          return nullptr;
        }
        
        const attribute_t* find(const struct type* type) const {
          for (size_t i = 0; i < size; ++i) {
            if (ptr[i].type() == type) return &ptr[i];
          }
          return nullptr;
        }
      private:
        size_t size;
        const attribute_t<T>* ptr;
      };
      
      attribute_t(const T &base, const struct type* type) : m_base(base), m_current(m_base), m_raw_add(T(0)), m_final_add(T(0)), m_raw_mul(1.0f), m_final_mul(1.0f), m_type(type) {}
      
      T base() const { return m_base; }
      T current() const { return m_current; }
      
      void raw_add(const game::bonus_t &bonus) { m_raw_add += bonus.add; m_raw_mul += bonus.mul; }
      void final_add(const game::bonus_t &bonus) { m_final_add += bonus.add; m_final_mul += bonus.mul; }
      
      void raw_remove(const game::bonus_t &bonus) { m_raw_add -= bonus.add; m_raw_mul -= bonus.mul; }
      void final_remove(const game::bonus_t &bonus) { m_final_add -= bonus.add; m_final_mul -= bonus.mul; }
      
      const struct type* type() const { return m_type; }
      void compute(yacs::entity* ent) {
        if (m_type->compute) {
          m_current = m_type->compute(ent, m_type, m_base, m_raw_add, m_raw_mul, m_final_add, m_final_mul);
          if (m_type->dumping) {
            m_base = m_current;
            m_raw_add = T(0); 
            m_raw_mul = 1.0f;
            m_final_add = T(0); 
            m_final_mul = 1.0f;
          }
          return;
        }
        
        // дефолтная?
      }
    private:
      T m_base;
      T m_current;
      
      T m_raw_add;
      T m_final_add;
      
      float m_raw_mul;
      float m_final_mul;
      
      const struct type* m_type;
    };
  }
}

#endif
