#ifndef TYPELESS_CONTAINER_H
#define TYPELESS_CONTAINER_H

#ifdef _DEBUG
  #include <cassert>
  #define ASSERT(expr) assert(expr)
#else
  #define ASSERT(expr)
#endif

#include "stack_allocator.h"

namespace devils_engine {
  namespace utils {
    class typeless_container {
    public:
      typeless_container(const size_t &size) : allocator(size) {}
      typeless_container(typeless_container &&cont) : allocator(std::move(cont.allocator)) {}
      
      template <typename T, typename... Args>
      T* create(Args&&... args) {
        void* mem = allocator.alloc(sizeof(T));
        ASSERT(mem != nullptr && "Not enough allocator size");
        T* ptr = new (mem) T(std::forward<Args>(args)...);
        return ptr;
      }
      
      // аллокатор возвращающий память это new и delete, сомневаюсь что мне нужно заморачиваться с этим
      template <typename T>
      void destroy(T* ptr) {
        ptr->~T();
      }
      
      void clear() { allocator.clear(); }
      
      size_t size() const { return allocator.size(); }
      size_t allocated_size() const { return allocator.allocated_size(); } 
    private:
      stack_allocator allocator;
    };
  }
}

#endif
