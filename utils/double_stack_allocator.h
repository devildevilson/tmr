#ifndef DOUBLE_STACK_ALLOCATOR_H
#define DOUBLE_STACK_ALLOCATOR_H

#include "stack_allocator.h"

namespace devils_engine {
  namespace utils {
    class double_stack_allocator {
    public:
      double_stack_allocator(const size_t &size);
      
      void swap_stack();
      void clear_current();
      void* alloc(const size_t &size);
    private:
      size_t m_current;
      stack_allocator m_stack1;
      stack_allocator m_stack2;
    };
  }
}

#endif
