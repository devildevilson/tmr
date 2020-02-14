#ifndef INVENTORY_COMPONENT_H
#define INVENTORY_COMPONENT_H

#include "id.h"

namespace devils_engine {
  namespace components {
    class inventory {
    public:
      struct container {
        utils::id type;
        size_t quantity;
      };

      size_t add(const utils::id &type, const size_t &count);
      size_t remove(const utils::id &type, const size_t &count);
      size_t has(const utils::id &type) const;
      
      size_t size() const;
      const container* at(const size_t &index) const;
    private:
      std::vector<container> items;
      
      size_t find(const utils::id &id) const;
      void sort();
    };
  }
  
  namespace properties {
    struct pickup {
      utils::id type;
      size_t count;
    };
  }
}

#endif
