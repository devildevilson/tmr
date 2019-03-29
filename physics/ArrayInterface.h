#ifndef ARRAY_INTERFACE
#define ARRAY_INTERFACE

#include <cstdint>
#include <cstddef>
#include <iostream>

#ifdef _DEBUG
  #include <cassert>
  #define ASSERT(expr) assert(expr)
#else
  #define ASSERT(expr)
#endif

class Destructable {
public:
  virtual ~Destructable() {}
};

template <typename T>
class ArrayInterface : public Destructable {
public:
  ArrayInterface() {}
  //ArrayInterface(const uint32_t &size);
  virtual ~ArrayInterface() {}
  
  template <typename S>
  S* structure_from_begin() { /*std::cout << "ArrayInterface::structure_from_begin() " << sizeof(S) << '\n';*/ return reinterpret_cast<S*>(ptr); }
  template <typename S>
  const S* structure_from_begin() const { /*std::cout << "ArrayInterface::structure_from_begin() " << sizeof(S) << '\n';*/ return reinterpret_cast<S*>(ptr); }
  
  template <typename S>
  T* data_from() { 
//     std::cout << "ArrayInterface::data_from() " << sizeof(S) << '\n';
    
    S* tmp = reinterpret_cast<S*>(ptr);
    tmp++;
    return reinterpret_cast<T*>(tmp);
  }
  
  template <typename S>
  const T* data_from() const { 
//     std::cout << "ArrayInterface::data_from() " << sizeof(S) << '\n';
    
    //return reinterpret_cast<T*>(reinterpret_cast<S*>(ptr)++); 
    S* tmp = reinterpret_cast<S*>(ptr);
    tmp++;
    return reinterpret_cast<T*>(tmp);
  }
  
  T* data() { return ptr; }
  const T* data() const { return ptr; }
  
  uint32_t size() const { return sizeVar; }
  
  T& at(const uint32_t &index) { ASSERT(sizeVar > index); return ptr[index]; }
  const T& at(const uint32_t &index) const { ASSERT(sizeVar > index); return ptr[index]; }
  
  T& operator[] (const uint32_t &index) { ASSERT(sizeVar > index); return ptr[index]; }
  const T& operator[] (const uint32_t &index) const { ASSERT(sizeVar > index); return ptr[index]; }
  
  virtual void resize(const uint32_t &size) = 0;
  virtual void descriptorPtr(void* ptr) const = 0; // тут надо бы вернуть буфер вместе с дескриптором
  virtual void push_back(const T &value) = 0;
protected:
  uint32_t sizeVar;
  T* ptr;
};

template <typename S, typename T>
class StructureArrayInterface {
public:
  StructureArrayInterface() {}
  //ArrayInterface(const uint32_t &size);
  virtual ~StructureArrayInterface() {}
  
  S* structure() { return structPtr; }
  const S* structure() const { return structPtr; }
  
  T* data() { return ptr; }
  const T* data() const { return ptr; }
  
  uint32_t size() const { return sizeVar; }
  
  T& at(const uint32_t &index) { return ptr[index]; }
  const T& at(const uint32_t &index) const { return ptr[index]; }
  
  T& operator[] (const uint32_t &index) { return ptr[index]; }
  const T& operator[] (const uint32_t &index) const { return ptr[index]; }
  
  virtual void resize(const uint32_t &size) = 0;
  virtual void descriptorPtr(void* ptr) const = 0; // тут надо бы вернуть буфер вместе с дескриптором
  virtual void push_back(const T &value) = 0;
protected:
  uint32_t sizeVar;
  T* ptr;
  S* structPtr;
};

template <typename T>
union Slot {
  uint32_t nextIndex;
  T value;
};

template <typename T>
class Container : public ArrayInterface<T> {
public:
  Container() {}
  virtual ~Container() {}

  // вставляет в произвольное место и возвращает индекс куда вставили
  virtual uint32_t insert(const T &value) = 0;
  virtual void erase(const uint32_t &index) = 0;
protected:
  uint32_t freeIndex = UINT32_MAX;
};

#endif
