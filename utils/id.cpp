#include "id.h"

namespace devils_engine {
  namespace utils {
    id id::get(const std::string &name) {
      for (size_t i = 0; i < names.size(); ++i) {
        if (names[i] == name) return id(i);
      }
      
      const size_t index = names.size();
      names.push_back(name);
      return id(index);
    }
    
    id::id() : m_id(SIZE_MAX) {}
    bool id::valid() const { return m_id != SIZE_MAX; }
    std::string id::name() const { return valid() ? names[m_id] : ""; }
    size_t id::num() const { return m_id; }
    bool id::operator==(const id &other) const { return this->m_id == other.m_id; }
    bool id::operator!=(const id &other) const { return this->m_id != other.m_id; }
    bool id::operator>(const id &other) const { return this->m_id > other.m_id; }
    bool id::operator<(const id &other) const { return this->m_id < other.m_id; }
    bool id::operator>=(const id &other) const { return this->m_id >= other.m_id; }
    bool id::operator<=(const id &other) const { return this->m_id <= other.m_id; }
    id::id(const size_t &id) : m_id(id) {}
    
    std::vector<std::string> id::names;
  }
}
