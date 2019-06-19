#ifndef SOURCE_H
#define SOURCE_H

#include <cstdint>

#include "Utility.h"

#include "Buffer.h"

class Source {
public:
  enum class State {
    playing,
    paused,
    stopped,
    initial
  };

  enum class Type {
    undetermined,
    statict,
    streaming
  };
  
//   struct CreateInfo {
//     float pitch;
//     float gain;
//     float maxDist;
//     float rolloff;
//     float refDist;
//     float minGain;
//     float maxGain;
//     bool looping;
//     bool relative;
//   };
  
  Source();
  Source(const Source &source);
  Source(const uint32_t &id);
//   Source(const CreateInfo &info);
//   Source(Source &&source);
  ~Source();
  
  bool isValid() const;
  
  void play();
  void pause();
  void stop();
  void rewind();
  // буду ли я хранить указатели на буферы в сорсе?
  void queueBuffers(const int32_t &size, uint32_t* buffers);
  void queueBuffers(const int32_t &size, Buffer* buffers);
  void unqueueBuffers(const int32_t &size, uint32_t* buffers);
  void unqueueBuffers(const int32_t &size, Buffer* buffers);
  
  void setPitch(const float &data);
  void setGain(const float &data);
  void setMaxDist(const float &data);
  void setRolloff(const float &data);
  void setRefDist(const float &data);
  void setMinGain(const float &data);
  void setMaxGain(const float &data);
  void setConeOuterGain(const float &data);
  void setConeInnerAngle(const float &data);
  void setConeOuterAngle(const float &data);
  
  void setPos(const glm::vec3 &data);
  void setVel(const glm::vec3 &data);
  void setDir(const glm::vec3 &data);
  
  void setPos(const float* data);
  void setVel(const float* data);
  void setDir(const float* data);
  
  // в спеках чет нигде это не указано
  // в спеках не указано, но это переменные для того чтобы регулировать положение проигрования
  // то есть при установки такой переменной, мы прыгаем на определенную секунду, сэмпл или байт звука
  void setSecOffset(const float &data);
  void setSampleOffset(const float &data);
  void setByteOffset(const float &data);
  void setByteOffset(const int32_t &data);
  
  void buffer(const uint32_t &data);
  void buffer(const Buffer &data);
  void relative(const bool &data);
  void looping(const bool &data);
  
  float getPitch() const;
  float getGain() const;
  float getMaxDist() const;
  float getRolloff() const;
  float getRefDist() const;
  float getMinGain() const;
  float getMaxGain() const;
  float getConeOuterGain() const;
  float getConeInnerAngle() const;
  float getConeOuterAngle() const;
  
  float secOffset() const;
  float sampleOffset() const;
  int32_t sampleOffsetInt() const;
  float byteOffset() const;
  int32_t byteOffsetInt() const;
  
  glm::vec3 getPos() const;
  glm::vec3 getVel() const;
  glm::vec3 getDir() const;
  
  uint32_t getBuffer() const;
  
  int32_t queuedBuffers() const;
  int32_t processedBuffers() const;
  
  Type type() const;
  State state() const;
  
  bool isRelative() const;
  bool isLooping() const;
  
//   void setSecOffset(const float &data);
//   void setSampleOffset(const float &data);
//   void setByteOffset(const float &data);
  
  uint32_t id() const;
  
  //Source & operator=(Source &&other);
  Source & operator=(const Source &other);
  bool operator==(const Source &other);
  bool operator!=(const Source &other);
private:
  uint32_t alId;
  
  // указатели на буферы (нужны ли?)
  // указатель на то что сейчас играет
};

#endif
