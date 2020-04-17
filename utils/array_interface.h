#ifndef ARRAY_INTERFACE_H
#define ARRAY_INTERFACE_H

#include <cstddef>

#ifdef _DEBUG
  #include <cassert>
  #define ASSERT(expr) assert(expr)
#else
  #define ASSERT(expr)
#endif

namespace devils_engine {
  namespace utils {
    template <typename T>
    class array_interface {
    public:
      virtual ~array_interface() {}
      
      T* data() { return data_ptr; }
      const T* data() const { return data_ptr; }
      T& at(const size_t &index) { ASSERT(index < data_size); return data_ptr[index]; }
      const T& at(const size_t &index) const { ASSERT(index < data_size); return data_ptr[index]; }
      T& operator[](const size_t &index) { ASSERT(index < data_size); return data_ptr[index]; }
      const T& operator[](const size_t &index) const { ASSERT(index < data_size); return data_ptr[index]; }
      size_t size() const { return data_size; }
      virtual void push_back(const T& value) = 0;
      virtual void clear() = 0;
      virtual void resize(const uint32_t &size) = 0;
    protected:
      T* data_ptr;
      size_t data_size;
    };
    
    template <typename T>
    class container : public array_interface<T> {
    public:
      virtual uint32_t insert(const T &value) = 0;
      virtual void erase(const uint32_t &index) = 0;
    };
  }
}

#endif
