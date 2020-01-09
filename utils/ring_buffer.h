#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <cstddef>
#include <cstring>
#include <stdexcept>

// несколько мыслей по поводу кольцевого буфера
// нужно ли его делать с фиксированной памятью? 
// я так понимаю это наиболее лучшее решение для такого буфера (локальную память можно использовать)
// нефиксированный буфер требует нескольких правил 
// по которым будет изменяться размер буфера
// пусть пока будет фиксированный

namespace devils_engine {
  namespace utils {
    template <typename T, size_t N>
    class ring_buffer {
    public:
      using size_type = size_t;
      using reference = T&;
      //typedef T& reference;
      using const_reference = const T&;
      
      class iterator {
      public:
        iterator(T* start, const size_t &index) : start(start), index(index) {}
        iterator(const iterator &itr) : start(itr.start), index(itr.index) {}
        
        T* get() {
          return &start[index];
        }
        
        iterator & operator++() {
          index = (index+1)%N;
          return *this;
        }
        
        iterator & operator--() {
          index = (index-1)%N;
          return *this;
        }
        
        iterator operator++(int) {
          iterator ret(start, index);
          index = (index+1)%N;
          return ret;
        }
        
        iterator operator--(int) {
          iterator ret(start, index);
          index = (index-1)%N;
          return ret;
        }
        
        iterator operator+(const size_type &i) const {
          return iterator(start, (index+i)%N);
        }
        
        iterator operator-(const size_type &i) const {
          return iterator(start, (index-i)%N);
        }
        
        T* operator->() {
          return &start[index];
        }
        
        reference operator*() {
          return start[index];
        }
        
        bool operator==(const iterator &itr) const {
          return itr.start == start && itr.index == index;
        }
        
        bool operator!=(const iterator &itr) const {
          return itr.start == start && itr.index != index;
        }
        
        iterator & operator=(const iterator &itr) {
          start = itr.start;
          index = itr.index;
          return *this;
        }
      private:
        T* start;
        size_t index;
      };
      
      
      class const_iterator {
      public:
        const_iterator(const T* start, const size_t &index) : start(start), index(index) {}
        const_iterator(const const_iterator &itr) : start(itr.start), index(itr.index) {}
        const_iterator(const iterator &itr) : start(itr.start), index(itr.index) {}
        
        const T* get() const {
          return &start[index];
        }
        
        const_iterator operator+(const size_type &i) const {
          return const_iterator(start, (index+i)%N);
        }
        
        const_iterator operator-(const size_type &i) const {
          return const_iterator(start, (index-i)%N);
        }
        
        const T* operator->() const {
          return &start[index];
        }
        
        const_reference operator*() const {
          return start[index];
        }
        
        bool operator==(const const_iterator &itr) const {
          return itr.start == start && itr.index == index;
        }
        
        bool operator!=(const const_iterator &itr) const {
          return itr.start == start && itr.index != index;
        }
        
        const_iterator & operator=(const const_iterator &itr) {
          start = itr.start;
          index = itr.index;
          return *this;
        }
      private:
        const T* start;
        size_t index;
      };
      
      ring_buffer();
    //   ring_buffer(const size_type &capacity);
      
      const_iterator begin() const;
      const_iterator end() const;
      iterator begin();
      iterator end();
      reference operator[] (const size_type &index);
      const_reference operator[] (const size_type &index) const;
      reference at(const size_type &index);
      const_reference at(const size_type &index) const;
      reference front();
      reference back();
      const_reference front() const;
      const_reference back() const;
      size_type size() const;
      size_type capacity() const;
      bool empty() const;
      bool full() const;
      void resize(const size_type &size, const_reference item = T());
      void push_back(const_reference item);
      void push_front(const_reference item);
      void pop_back();
      void pop_front();
      template <typename... Args>
      void emplace_back(Args&&... args);
      
    //   void reserve(const size_type &capacity);
    //   iterator insert(iterator pos, const_reference item = T());
    //   void insert(iterator pos, size_type n, const_reference item);
    //   template <typename... Args>
    //   void emplace(iterator pos, Args&&... args);
    //   iterator erase(iterator pos);
    //   iterator erase(iterator first, iterator last);
    private:
      char m_memory[N*sizeof(T)];
      T* m_begin;
      T* m_end;
      
      T* startPtr();
      const T* startPtr() const;
      T* lastPtr();
      const T* lastPtr() const;
    };

    template <typename T, size_t N>
    ring_buffer<T, N>::ring_buffer() : m_begin(reinterpret_cast<T*>(m_memory)), m_end(m_begin+1) {
      memset(m_memory, 0, N*sizeof(T));
    }
    // ring_buffer(const size_type &capacity);

    template <typename T, size_t N>
    typename ring_buffer<T, N>::const_iterator ring_buffer<T, N>::begin() const { return const_iterator(reinterpret_cast<const T*>(m_memory), m_begin-startPtr()); }
    template <typename T, size_t N>
    typename ring_buffer<T, N>::const_iterator ring_buffer<T, N>::end() const { return const_iterator(reinterpret_cast<const T*>(m_memory), m_end-startPtr()); }
    template <typename T, size_t N>
    typename ring_buffer<T, N>::iterator ring_buffer<T, N>::begin() { return iterator(reinterpret_cast<T*>(m_memory), m_begin-startPtr()); }
    template <typename T, size_t N>
    typename ring_buffer<T, N>::iterator ring_buffer<T, N>::end() { return iterator(reinterpret_cast<T*>(m_memory), m_end-startPtr()); }
    // как тут возвращать? поидее от старого к более молодому
    template <typename T, size_t N>
    typename ring_buffer<T, N>::reference ring_buffer<T, N>::operator[] (const size_type &index) {
      return *(begin()+index);
    }

    template <typename T, size_t N>
    typename ring_buffer<T, N>::const_reference ring_buffer<T, N>::operator[] (const size_type &index) const {
      return *(begin()+index);
    }

    template <typename T, size_t N>
    typename ring_buffer<T, N>::reference ring_buffer<T, N>::at(const size_type &index) {
      const size_type bufferSize = size();
      if (index > bufferSize) throw std::runtime_error("index > bufferSize");
      
      const auto diff = m_begin - startPtr();
      const auto finalIndex = (index + diff) % bufferSize;
      auto ptr = startPtr();
      return ptr[finalIndex];
    }

    template <typename T, size_t N>
    typename ring_buffer<T, N>::const_reference ring_buffer<T, N>::at(const size_type &index) const {
      const size_type bufferSize = size();
      if (index > bufferSize) throw std::runtime_error("index > bufferSize");
      
      const auto diff = m_begin - startPtr();
      const auto finalIndex = (index + diff) % bufferSize;
      auto ptr = startPtr();
      return ptr[finalIndex];
    }

    template <typename T, size_t N>
    typename ring_buffer<T, N>::reference ring_buffer<T, N>::front() { return *m_begin; }
    template <typename T, size_t N>
    typename ring_buffer<T, N>::reference ring_buffer<T, N>::back() { return m_end == m_memory ? *(lastPtr()-1) : *(m_end-1); }
    template <typename T, size_t N>
    typename ring_buffer<T, N>::const_reference ring_buffer<T, N>::front() const { return *m_begin; }
    template <typename T, size_t N>
    typename ring_buffer<T, N>::const_reference ring_buffer<T, N>::back() const { return m_end == m_memory ? *(lastPtr()-1) : *(m_end-1); }
    template <typename T, size_t N>
    typename ring_buffer<T, N>::size_type ring_buffer<T, N>::size() const { return full() ? capacity() : (lastPtr() - m_begin) + (m_end - startPtr()); }
    template <typename T, size_t N>
    typename ring_buffer<T, N>::size_type ring_buffer<T, N>::capacity() const { return N; }
    template <typename T, size_t N>
    bool ring_buffer<T, N>::empty() const { return begin()+1 == end(); }
    template <typename T, size_t N>
    bool ring_buffer<T, N>::full() const { return m_begin == m_end; }
    template <typename T, size_t N>
    void ring_buffer<T, N>::resize(const size_type &resize_size, const_reference item) {
      const size_type finalSize = std::min(resize_size, capacity());
      
      while (finalSize != size()) {
        push_back(item);
      }
    }

    template <typename T, size_t N>
    void ring_buffer<T, N>::push_back(const_reference item) {
      if (full()) pop_front();
      new (m_end) T(item);
      m_end = (end()+1).get();
    }

    template <typename T, size_t N>
    void ring_buffer<T, N>::push_front(const_reference item) {
      if (full()) pop_back();
      m_begin = (begin()-1).get();
      //*m_begin = item;
      new (m_begin) T(item);
    }

    template <typename T, size_t N>
    void ring_buffer<T, N>::pop_back() {
      if (empty()) return;
      m_end = (end()-1).get();
      m_end->~T();
    }

    template <typename T, size_t N>
    void ring_buffer<T, N>::pop_front() {
      if (empty()) return;
      m_begin->~T();
      m_begin = (begin()+1).get();
    }

    template <typename T, size_t N>
    template <typename... Args>
    void ring_buffer<T, N>::emplace_back(Args&&... args) {
      if (full()) pop_front();
      new (m_end) T(std::forward<Args>(args)...);
      m_end = (end()+1).get();
    }
    
    template <typename T, size_t N>
    T* ring_buffer<T, N>::startPtr() { return reinterpret_cast<T*>(m_memory); }
    template <typename T, size_t N>
    const T* ring_buffer<T, N>::startPtr() const { return reinterpret_cast<const T*>(m_memory); }

    template <typename T, size_t N>
    T* ring_buffer<T, N>::lastPtr() { return reinterpret_cast<T*>(&m_memory[N*sizeof(T)]); }
    
    template <typename T, size_t N>
    const T* ring_buffer<T, N>::lastPtr() const { return reinterpret_cast<const T*>(&m_memory[N*sizeof(T)]); }

    // template <typename T, size_t N>
    // void ring_buffer<T, N>::reserve(const size_type &capacity);
    // iterator insert(iterator pos, const_reference item = T());
    // void insert(iterator pos, size_type n, const_reference item);
    // template <typename... Args>
    // void emplace(iterator pos, Args&&... args);
    // iterator erase(iterator pos);
    // iterator erase(iterator first, iterator last);
  }
}

#endif
