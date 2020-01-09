#ifndef DOUBLE_ENDED_STACK_H
#define DOUBLE_ENDED_STACK_H

#include <cstddef>

namespace devils_engine {
  namespace utils {
    class double_ended_stack {
    public:
      double_ended_stack(const size_t &size);
      ~double_ended_stack();
      
      void* alloc_front(const size_t &size) noexcept;
      void* alloc_back(const size_t &size) noexcept;
      void clear() noexcept;
    private:
      char* m_memory;
      size_t m_size;
      size_t m_front; // тут поди уже не сделать атомарно
      size_t m_back;
    };
  }
}

#endif
