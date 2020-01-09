#include "double_stack_allocator.h"

namespace devils_engine {
  namespace utils {
    double_stack_allocator::double_stack_allocator(const size_t &size) : m_current(0), m_stack1(size), m_stack2(size) {}
    void double_stack_allocator::swap_stack() {
      m_current = (m_current+1)%2;
    }
    
    void double_stack_allocator::clear_current() {
      stack_allocator* ptr = &m_stack1;
      ptr[m_current].clear();
    }
    
    void* double_stack_allocator::alloc(const size_t &size) {
      stack_allocator* ptr = &m_stack1;
      return ptr[m_current].alloc(size);
    }
  }
}
