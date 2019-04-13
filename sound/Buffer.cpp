#include "Buffer.h"

#include "AL/al.h"
#include "AL/alc.h"
// #include "AL/alut.h"
#include "AL/alext.h"

#include "alHelpers.h"

#ifdef _DEBUG
#include <cassert>
#define ASSERT(expr) assert(expr)
#else
#define ASSERT(expr)
#endif

static PFNALBUFFERDATASTATICPROC alBufferDataStatic = nullptr;

void Buffer::setBufferDataStatic(const bool &data) {
  bufferDataStaticEnabled = data;
  
  if (bufferDataStaticEnabled && alBufferDataStatic == nullptr) {
    alBufferDataStatic = reinterpret_cast<PFNALBUFFERDATASTATICPROC>(alGetProcAddress("alBufferDataStatic"));
  }
}

bool Buffer::isBufferDataStaticPresent() {
  return bufferDataStaticEnabled && alBufferDataStatic != nullptr;
}
  
Buffer::Buffer() : alId(UINT32_MAX) {}
Buffer::Buffer(const uint32_t &id) : alId(id) {
//   if (alId == UINT32_MAX) {
//     alGenBuffers(1, &alId);
//     openalError("Could not create buffer");
//   }
}

// Buffer::Buffer(Buffer &&other) {
//   alId = other.alId;
//   other.alId = UINT32_MAX;
// }

Buffer::~Buffer() {
//   if (alId != UINT32_MAX) {
//     alDeleteBuffers(1, &alId);
//     openalError("Could not delete buffer");
//     alId = UINT32_MAX;
//   }
}

bool Buffer::isValid() const {
  return alId != UINT32_MAX;
}

void Buffer::bufferData(const int32_t &format, void* data, const int32_t &size, const int32_t &freq) {
  alBufferData(alId, format, data, size, freq);
  openalError("Could not buffer data");
}

void Buffer::bufferDataStatic(const int32_t &format, void* data, const int32_t &size, const int32_t &freq) {
  if (bufferDataStaticEnabled) {
    alBufferDataStatic(alId, format, data, size, freq);
    openalError("Could not buffer static data");
    return;
  }
  
  bufferData(format, data, size, freq);
}

int32_t Buffer::frequency() const {
  int32_t data;
  alGetBufferi(alId, AL_FREQUENCY, &data);
  openalError("Could not get buffer frequency");
  
  return data;
}

int32_t Buffer::bits() const {
  int32_t data;
  alGetBufferi(alId, AL_BITS, &data);
  openalError("Could not get buffer bits");
  
  return data;
}

int32_t Buffer::channels() const {
  int32_t data;
  alGetBufferi(alId, AL_CHANNELS, &data);
  openalError("Could not get buffer channels");
  
  return data;
}

int32_t Buffer::size() const {
  int32_t data;
  alGetBufferi(alId, AL_SIZE, &data);
  openalError("Could not get buffer size");
  
  return data;
}

uint32_t Buffer::id() const {
  return alId;
}

Buffer & Buffer::operator=(const Buffer &other) {
  alId = other.alId;
  
  return *this;
}

bool Buffer::operator==(const Buffer &other) {
  return alId == other.alId;
}

bool Buffer::operator!=(const Buffer &other) {
  return alId != other.alId;
}

bool Buffer::bufferDataStaticEnabled = false;
