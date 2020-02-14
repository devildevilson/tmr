#include "inventory_component.h"

#include <algorithm>
#include <stdexcept>

namespace devils_engine {
  namespace components {
    struct compare {
      bool operator() (const inventory::container &first, const inventory::container &second) const {
        return first.type < second.type;
      }
    };
    
    size_t inventory::add(const utils::id &type, const size_t &count) {
      const size_t index = find(type);
      if (index == SIZE_MAX) {
        items.push_back({type, count});
        sort();
        return count;
      }
      
      items[index].quantity += count;
      return items[index].quantity;
    }
    
    size_t inventory::remove(const utils::id &type, const size_t &count) {
      const size_t index = find(type);
      if (index == SIZE_MAX) return 0;
      
      const size_t a = count > items[index].quantity ? count - items[index].quantity : count;
      items[index].quantity = items[index].quantity > count ? items[index].quantity - count : 0;
      if (items[index].quantity == 0) {
        std::swap(items[index], items.back());
        items.pop_back();
        sort();
      }
      return a;
    }
    
    size_t inventory::has(const utils::id &type) const {
      const size_t index = find(type);
      if (index == SIZE_MAX) return 0;
      return items[index].quantity;
    }
    
    size_t inventory::size() const {return items.size(); }
    const inventory::container* inventory::at(const size_t &index) const {
      if (index >= items.size())  return nullptr;
      return &items[index];
    }
    
    size_t inventory::find(const utils::id &id) const {
      for (size_t i = 0; i < items.size(); ++i) {
        if (items[i].type == id) return i;
      }
      return SIZE_MAX;
    }
    
    void inventory::sort() {
      std::sort(items.begin(), items.end(), compare());
    }
  }
}
