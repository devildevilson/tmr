#ifndef GPU_ARRRAY_H
#define GPU_ARRRAY_H

#include "array_interface.h"
#include "yavf.h"

namespace devils_engine {
  namespace utils {
    template <typename T>
    class gpu_array : public array_interface<T> {
    public:
      gpu_array(yavf::Device* device) : m_array(device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) { update(); }
      gpu_array(yavf::Device* device, const VkBufferUsageFlags &flags) : m_array(device, flags) { update(); }
      gpu_array(yavf::Device* device, const VkBufferUsageFlags &flags, const size_t &size) : m_array(device, flags, size) { update(); }
      void push_back(const T &value) override { m_array.push_back(value); update(); }
      void clear() override { m_array.clear(); update(); }
      void resize(const uint32_t &size) override { m_array.resize(size); update(); }
      void update() { array_interface<T>::data_ptr = m_array.data(); array_interface<T>::data_size = m_array.size(); }
      yavf::vector<T> & array() { return m_array; }
      const yavf::vector<T> & array() const { return m_array; }
    private:
      yavf::vector<T> m_array;
    };
    
    template <typename T>
    class gpu_container : public container<T> {
      union element {
        T obj;
        uint32_t next_index;
      };
    public:
      gpu_container(yavf::Device* device) : free_index(UINT32_MAX), m_array(device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) { update(); }
      gpu_container(yavf::Device* device, const VkBufferUsageFlags &flags) : free_index(UINT32_MAX), m_array(device, flags) { update(); }
      gpu_container(yavf::Device* device, const VkBufferUsageFlags &flags, const size_t &size) : free_index(UINT32_MAX), m_array(device, flags, size) { update(); }
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
      yavf::vector<T> & array() { return m_array; }
      const yavf::vector<T> & array() const { return m_array; }
    private:
      uint32_t free_index;
      yavf::vector<T> m_array;
    };
  }
}

#endif
