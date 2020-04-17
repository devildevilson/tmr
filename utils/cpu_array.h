#ifndef CPU_ARRAY_NEW_H
#define CPU_ARRAY_NEW_H

#include <vector>
#include "array_interface.h"

namespace devils_engine {
  namespace utils {
    template <typename T>
    class cpu_array : public array_interface<T> {
    public:
      cpu_array() {}
      cpu_array(const size_t &size) : m_array(size) { update(); }
      void push_back(const T &value) override { m_array.push_back(value); update(); }
      void clear() override { m_array.clear(); update(); }
      void resize(const uint32_t &size) override { m_array.resize(size); update(); }
      void update() { array_interface<T>::data_ptr = m_array.data(); array_interface<T>::data_size = m_array.size(); }
      std::vector<T> & array() { return m_array; }
      const std::vector<T> & array() const { return m_array; }
    private:
      std::vector<T> m_array;
    };
    
    template <typename T>
    class cpu_container : public container<T> {
      union element {
        T obj;
        uint32_t next_index;
      };
    public:
      cpu_container() : free_index(UINT32_MAX) {}
      cpu_container(const size_t &size) : free_index(UINT32_MAX), m_array(size) { update(); }
      void push_back(const T &value) override { m_array.push_back(value); update(); }
      void clear() override { m_array.clear(); update(); }
      void resize(const uint32_t &size) override { m_array.resize(size); update(); }
      uint32_t insert(const T &value) override {
        uint32_t index = UINT32_MAX;
        if (free_index != UINT32_MAX) {
          index = free_index;
          free_index = reinterpret_cast<element*>(&m_array[index])->next_index;
          m_array[index] = value;
        } else {
          index = m_array.size();
          m_array.push_back(value);
          update();
        }
        return index;
      }
      
      void erase(const uint32_t &index) override {
        ASSERT(index < m_array.size());
        m_array[index].~T();
        reinterpret_cast<element*>(&m_array[index])->next_index = free_index;
        free_index = index;
      }
      void update() { array_interface<T>::data_ptr = m_array.data(); array_interface<T>::data_size = m_array.size(); }
      std::vector<T> & array() { return m_array; }
      const std::vector<T> & array() const { return m_array; }
    private:
      uint32_t free_index;
      std::vector<T> m_array;
    };
  }
}

#endif
