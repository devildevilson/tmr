#ifndef BUFFER_H
#define BUFFER_H

#include <cstdint>

class Buffer {
public:
  static void setBufferDataStatic(const bool &data);
  static bool isBufferDataStaticPresent();
  
  Buffer();
  Buffer(const Buffer &buffer);
  Buffer(const uint32_t &id);
//   Buffer(Buffer &&other);
  ~Buffer();
  
  bool isValid() const;
  
  void bufferData(const int32_t &format, void* data, const int32_t &size, const int32_t &freq);
  void bufferDataStatic(const int32_t &format, void* data, const int32_t &size, const int32_t &freq);
  
  int32_t frequency() const;
  int32_t bits() const;
  int32_t channels() const;
  int32_t size() const;
  
  uint32_t id() const;
  
  //Buffer & operator=(Buffer &&other);
  Buffer & operator=(const Buffer &other);
  bool operator==(const Buffer &other);
  bool operator!=(const Buffer &other);
private:
  uint32_t alId;
  
  // судя по спекам данные в буфер КОПИРУЮТСЯ
  // с alBufferDataStatic данные не копируются, а значит нужно использовать свое хранилище
  static bool bufferDataStaticEnabled;
  // походу нет у меня этого расширения
};

#endif
