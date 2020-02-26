#include "console_variable.h"

#include <stdexcept>

namespace devils_engine {
  namespace utils {
    std::vector<cvar_container> cvar_container::variables;
    
    size_t find(const utils::id &id) {
      for (size_t i = 0; i < cvar_container::variables.size(); ++i) {
        if (id == cvar_container::variables[i].id) return i;
      }
      return SIZE_MAX;
    }
    
    void cvar::create(const utils::id &id, const double &default_value) {
      if (find(id) != SIZE_MAX) return;
      cvar_container::variables.push_back(cvar_container{id, default_value, default_value});
    }
    
    cvar cvar::get(const utils::id &id) {
      return cvar{find(id)};
    }
    
    void cvar::get_f() {
      throw std::runtime_error("not implemented yet");
    }
    
    void cvar::set_f() {
      throw std::runtime_error("not implemented yet");
    }
  }
}
