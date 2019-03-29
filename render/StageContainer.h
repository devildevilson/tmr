#ifndef STAGE_CONTAINER_H
#define STAGE_CONTAINER_H

#include <cstddef>
#include <utility>
#include <cassert>
#include <iostream>

class StageContainer {
public:
  StageContainer(const size_t &bufferSize) : bufferSize(bufferSize), dataSize(0), buffer(nullptr) {
    createBuffer();
  }
  
  ~StageContainer() {
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
  T* addStage(Args&&... args) {
    assert(sizeof(T) <= bufferSize && "Bad sizeof(T) or size for stage container");
    
    if (dataSize + sizeof(T) > bufferSize) createBuffer();
    
    T* ptr = reinterpret_cast<T*>(buffer + dataSize);
    new (ptr) T(std::forward<Args>(args)...);
    
    const size_t newSize = dataSize + sizeof(T);
    dataSize = newSize;
    
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
  void destroyStage(T* stage) {
    stage->~T();
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
    
    // в датаСайз кладем sizeof(char*) так как это указатель на следующий массив
    dataSize = sizeof(char*);
    //bufferSize = newBufferSize;
    
    // в первые sizeof(char*) байт кладем указатель
    char** tmp = reinterpret_cast<char**>(newBuffer);
    tmp[0] = buffer;
    buffer = newBuffer;
    
    std::cout << "StageContainer createBuffer " << bufferSize << "\n";
  }
};

#endif
