#include "memory_component.h"

namespace devils_engine {
  namespace components {
    memory::memory(const size_t &size) : size(size), pieces(new piece[size]) {}
    memory::~memory() { delete [] pieces; }
    size_t memory::get(const utils::id &id) const {
      const auto index = find(id);
      if (index == SIZE_MAX) return SIZE_MAX;
      return pieces[index].mem;
    }
    
    size_t memory::set(const utils::id &id, const size_t &value) {
      const auto index = find(id);
      if (index == SIZE_MAX) return SIZE_MAX;
      return pieces[index].mem.exchange(value);
    }
    
    size_t memory::inc(const utils::id &id, const size_t &value) {
      const auto index = find(id);
      if (index == SIZE_MAX) return SIZE_MAX;
      size_t tmp = SIZE_MAX;
      const size_t ret = pieces[index].mem.fetch_add(value);
      pieces[index].mem.compare_exchange_strong(tmp, 0);
      return ret;
    }
    
    size_t memory::dec(const utils::id &id, const size_t &value) {
      const auto index = find(id);
      if (index == SIZE_MAX) return SIZE_MAX;
      size_t tmp = SIZE_MAX;
      const size_t ret = pieces[index].mem.fetch_sub(value);
      pieces[index].mem.compare_exchange_strong(tmp, 0);
      return ret;
    }
    
    size_t memory::find(const utils::id &id) const {
      for (size_t i = 0; i < size; ++i) {
        if (pieces[i].id == id) return i;
      }
      
      return SIZE_MAX;
    }
  }
}
