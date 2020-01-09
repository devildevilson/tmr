#ifndef ARRAY_H
#define ARRAY_H

#include <cstring>

namespace devils_engine {
  namespace utils {
    template <typename T, size_t N>
    class array {
    public:
      using value_type = T;
      using size_type = size_t;
      using iterator = T*;
      using const_iterator = const T*;
      using reference = T&;
      using const_reference = const T&;
      using pointer = T*;
      using const_pointer = const T*;
      
      const_reference back() const;
      reference back();
      const_reference front() const;
      reference front();
      const_iterator begin() const;
      iterator begin();
      const_iterator end() const;
      iterator end();
      const_pointer data() const;
      pointer data();
      
      bool push_back(const_reference value);
      bool pop_back();
      bool erase(const size_type &index);
      bool erase(iterator itr);
      
      size_type size() const;
      size_type capacity() const;
      
      const_reference operator[](const size_type &index) const;
      reference operator[](const size_type &index);
    private:
      value_type m_arr[N];
      size_type m_size;
    };
    
    template <typename T, size_t N>
    const_reference array<T,N>::back() const { return m_arr[m_size == 0 ? 0 : m_size-1]; }
    template <typename T, size_t N>
    reference array<T,N>::back() { return m_arr[m_size == 0 ? 0 : m_size-1]; }
    template <typename T, size_t N>
    const_reference array<T,N>::front() const { return m_arr[0]; }
    template <typename T, size_t N>
    reference array<T,N>::front() { return m_arr[0]; }
    template <typename T, size_t N>
    const_iterator array<T,N>::begin() const { return const_iterator(&m_arr[0]); }
    template <typename T, size_t N>
    iterator array<T,N>::begin() { return iterator(&m_arr[0]); }
    template <typename T, size_t N>
    const_iterator array<T,N>::end() const { return const_iterator(&m_arr[m_size == 0 ? 0 : m_size-1]); }
    template <typename T, size_t N>
    iterator array<T,N>::end() { return iterator(&m_arr[m_size == 0 ? 0 : m_size-1]); }
    template <typename T, size_t N>
    const_pointer array<T,N>::data() const { return const_pointer(m_arr); }
    template <typename T, size_t N>
    pointer array<T,N>::data() { return pointer(m_arr); }
    
    template <typename T, size_t N>
    bool array<T,N>::push_back(const_reference value) {
      if (m_size >= N) return false;
      m_arr[m_size] = value;
      ++m_size;
      return true;
    }
    
    template <typename T, size_t N>
    bool array<T,N>::pop_back() {
      if (m_size == 0) return false;
      --m_size;
      m_arr[m_size].~T();
      return true;
    }
    
    template <typename T, size_t N>
    bool array<T,N>::erase(const size_type &index) {
      if (index >= m_size) return false;
      if (index == m_size-1) return pop_back();
      const size_type move_size = m_size - 1 - index;
      memmove(&m_arr[index], &m_arr[index+1], move_size*sizeof(T));
      --m_size;
      return true;
    }
    
    template <typename T, size_t N>
    bool array<T,N>::erase(iterator itr) {
      const auto diff = itr - begin();
      if (diff < 0) return false;
      if (diff >= m_size) return false;
      if (diff == m_size-1) return pop_back();
      const size_type move_size = m_size - 1 - diff;
      memmove(&m_arr[diff], &m_arr[diff+1], move_size*sizeof(T));
      --m_size;
      return true;
    }
    
    template <typename T, size_t N>
    size_type array<T,N>::size() const { return m_size; }
    template <typename T, size_t N>
    size_type array<T,N>::capacity() const { return N; }
    
    template <typename T, size_t N>
    const_reference array<T,N>::operator[](const size_type &index) const { return m_arr[index]; }
    template <typename T, size_t N>
    reference array<T,N>::operator[](const size_type &index) { return m_arr[index]; }
  }
}

#endif
