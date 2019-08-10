#ifndef TYPELESS_CONTAINER_H
#define TYPELESS_CONTAINER_H

#include <cstddef>
#include <utility>
#include <iostream>

#ifdef _DEBUG
#include <cassert>
#define ASSERT(expr) assert(expr)
#else
#define ASSERT(expr)
#endif

class TypelessContainer {
public:
  TypelessContainer(const size_t &bufferSize) : bufferSize(bufferSize), dataSize(0), buffer(nullptr) {
    createBuffer();
  }
  
  ~TypelessContainer() {
    char* tmp = buffer;
    while (tmp != nullptr) {
      char** ptr = reinterpret_cast<char**>(tmp);
      char* nextBuffer = ptr[0];
      
      delete [] tmp;
      tmp = nextBuffer;
    }
  }
  
  // добавить данные для конструктора?
  template<typename T, typename... Args>
  T* create(Args&&... args) {
    ASSERT(sizeof(T) <= bufferSize && "Bad sizeof(T) or size for stage container");
//     std::cout << "dataSize + sizeof(T) " << dataSize + sizeof(T) << " buffer size " << bufferSize << "\n";
    ASSERT(dataSize + sizeof(T) <= bufferSize && "Need more buffer size");
    
    if (dataSize + sizeof(T) > bufferSize) createBuffer();
    
    T* ptr = reinterpret_cast<T*>(buffer + sizeof(char*) + dataSize);
    new (ptr) T(std::forward<Args>(args)...);
//     T* ptr = new T(std::forward<Args>(args)...);

    dataSize += sizeof(T);
    
    return ptr;
  }
  
//   template<typename T>
//   T* getStage() {
//     // нужно ли мне будет вообще эти сдейджи получать где-нибудь в коде после программы?
//     // по идее все что мне нужно будет сделать - это правильно их проинициализировать
//     // а это при создании рендера делается, а потом мне эти штуки ни к чему
//   }

  // мне потребуется деструктор для стейджей
  // но тут скорее всего просто вызов деструктора и ничего более
  template<typename T>
  void destroy(T* object) {
    object->~T();
//     delete stage;
  }
  
  constexpr size_t size() const {
    return bufferSize;
  }
private:
  const size_t bufferSize;
  size_t dataSize;
  char* buffer;
  
  void createBuffer() {
    // мы создаем буфер размера size+sizeof(char*)
    const size_t newBufferSize = bufferSize + sizeof(char*);
    char* newBuffer = new char[newBufferSize];

    dataSize = 0;
    
    // в первые sizeof(char*) байт кладем указатель
    char** tmp = reinterpret_cast<char**>(newBuffer);
    tmp[0] = buffer;
    buffer = newBuffer;
    
//     std::cout << "TypelessContainer createBuffer " << bufferSize << "\n";
  }
};

#endif
