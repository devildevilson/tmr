#ifndef THREADSAFE_MEMORY_POOL
#define THREADSAFE_MEMORY_POOL

#include <cstddef>
#include <atomic>
#include <algorithm>
#include <thread>

namespace devils_engine {
  namespace utils {
    namespace threadsafe {
      template <typename T, size_t N = 4096>
      class memory_pool {
      public:
        using elem_ptr = T*;
        using const_elem_ptr = const T*;
        
        static_assert(sizeof(T) >= sizeof(T*), "Small element");
        
        memory_pool() : memory(nullptr), free_memory(nullptr), current_size(0), block_size(final_block_size()) {}
        memory_pool(const memory_pool &pool) = delete;
        memory_pool(memory_pool &&pool) : memory(pool.memory), free_memory(pool.free_memory), current_size(pool.current_size), block_size(pool.block_size) {
          pool.memory = nullptr;
          pool.block_size = 0;
          pool.current_size = 0;
          pool.free_memory = nullptr;
        }
        
        ~memory_pool() { clear(); }
        
        memory_pool & operator=(const memory_pool &pool) = delete;
        void operator=(memory_pool &&pool) {
          clear();
          
          memory = pool.memory;
          current_size = pool.current_size;
          block_size = pool.block_size;
          free_memory = pool.free_memory;
          pool.memory = nullptr;
          pool.current_size = 0;
          pool.block_size = 0;
          pool.free_memory = nullptr;
        }
        
        char* allocate() {
          char* tmp = nullptr;
          char* ptr = nullptr;
          if (!free_memory.compare_exchange_strong(tmp, nullptr)) {
            if (free_memory.compare_exchange_strong(tmp, reinterpret_cast<char**>(tmp)[0])) {
              ptr = reinterpret_cast<T*>(tmp);
              return ptr;
            }
          }
          
          char* mem = nullptr;
          do {
            std::this_thread::sleep_for(std::chrono::nanoseconds(1)); // с этой строкой работает стабильнее
            mem = memory.exchange(nullptr);
          } while (mem == nullptr);

          if (current_size+sizeof(T) > block_size) {
            mem = allocate_memory(mem);
            const size_t offset = N % alignof(T);
            current_size = ptr_align() + offset;
          }

          const size_t place = current_size;
          current_size += sizeof(T);
          ptr = reinterpret_cast<T*>(mem + place);

          memory = mem;
          
          return ptr;
        }
        
        template<typename T, typename... Args>
        elem_ptr create(Args&&... args) {
          auto ptr = allocate();
          auto p = new (ptr) T(std::forward<Args>(args)...);
          return p;
        }
        
        void destroy(elem_ptr ptr) {
          ptr->~T();
          auto ptr_mem = reinterpret_cast<char**>(ptr);
          ptr_mem[0] = free_memory.exchange(ptr);
        }
        
        size_t block_elem_count() const {
          return N / sizeof(T);
        }
        
        void clear() {
          char* old_mem = memory;
          while (old_mem != nullptr) {
            auto ptr_mem = reinterpret_cast<char**>(old_mem);
            char* tmp = ptr_mem[0];
            delete [] old_mem;
            old_mem = tmp;
          }
          
          memory = nullptr;
          block_size = 0;
          current_size = 0;
          free_memory = nullptr;
        }
      private:
        std::atomic<char*> memory;
        std::atomic<char*> free_memory;
        size_t block_size;
        size_t current_size;
        
        size_t ptr_align() const {
          return std::max(alignof(T), alignof(T*));
        }
        
        size_t final_block_size() const {
          const size_t count = N / sizeof(T);
          const size_t mem = count * sizeof(T);
          const size_t diff = N - mem;
          const size_t align_ptr = ptr_align();
          return diff < align_ptr ? N + align_ptr : N;
        }
        
        char* allocate_memory(char* mem) const {
          const size_t offset = N % alignof(T);
          char* new_memory = new char[block_size];
          auto ptr_mem = reinterpret_cast<char**>(new_memory + offset);
          ptr_mem[0] = mem;
          return new_memory;
        }
      };
    }
  }
}

#endif
