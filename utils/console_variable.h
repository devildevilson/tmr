#ifndef CONSOLE_VARIABLE_H
#define CONSOLE_VARIABLE_H

#include "id.h"

namespace devils_engine {
  namespace utils {
    struct cvar_container {
      static std::vector<cvar_container> variables;
      
      struct flags {
        uint32_t container;
      };
      
      utils::id id;
      double value;
      double default_value;
      
      inline void reset() { value = default_value; }
      inline bool to_bool() { return value != 0.0; }
      inline void zero() { value = 0.0; }
    };
    
    struct cvar {
      static void create(const utils::id &id, const double &default_value);
      static cvar get(const utils::id &id);
      static void get_f();
      static void set_f();
      
      size_t index;
      
      inline utils::id id() const { return cvar_container::variables[index].id; }
      inline double default_value() const { return cvar_container::variables[index].default_value; }
      inline double & value()       { return cvar_container::variables[index].value; }
      inline double   value() const { return cvar_container::variables[index].value; }
      inline void reset() { cvar_container::variables[index].reset(); }
      inline bool to_bool() { return valid() && cvar_container::variables[index].to_bool(); }
      inline void zero() { cvar_container::variables[index].zero(); }
      inline bool valid() const { return index != SIZE_MAX; }
    };
  }
}

#endif
