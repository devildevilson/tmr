#include "double_ended_stack.h"

namespace devils_engine {
  namespace utils {
    double_ended_stack::double_ended_stack(const size_t &size) : m_memory(new char[size]), m_size(size), m_front(0), m_back(m_size) {}
    double_ended_stack::~double_ended_stack() { delete [] m_memory; }
    void* double_ended_stack::alloc_front(const size_t &size) noexcept {
      if (size == 0) return nullptr;
      if (m_front + size >= m_back) return nullptr;
      
      void* ptr = &m_memory[m_front];
      m_front += size;
      return ptr;
    }
    
    void* double_ended_stack::alloc_back(const size_t &size) noexcept {
      if (size == 0) return nullptr;
      if (size > m_back) return nullptr;
      if (m_back - size < m_front) return nullptr;
      
      m_back -= size;
      return &m_memory[m_back];
    }
    
    void double_ended_stack::clear() noexcept {
      m_front = 0;
      m_back = m_size;
    }
  }
}
