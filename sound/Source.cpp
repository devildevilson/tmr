#include "Source.h"

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

Source::Source() : alId(UINT32_MAX) {}
//Source::Source(const Source &source) : alId(source.id()) {}
Source::Source(const uint32_t &id) : alId(id) {
//   if (alId == UINT32_MAX) {
//     alGenSources(1, &alId);
//     openalError("Could not create source");
//   }
}

// Source::Source(const CreateInfo &info) : alId(UINT32_MAX) {
//   if (alId == UINT32_MAX) {
//     alGenSources(1, &alId);
//     openalError("Could not create source");
//   }
//   
//   setPitch(info.pitch);
//   setGain(info.gain);
//   setMaxDist(info.maxDist);
//   setRolloff(info.rolloff);
//   setRefDist(info.refDist);
//   setMinGain(info.minGain);
//   setMaxGain(info.maxGain);
//   relative(info.relative);
//   looping(info.looping);
// }
// 
// Source::Source(Source &&source) {
//   alId = source.alId;
//   source.alId = UINT32_MAX;
// }

Source::~Source() {
//   if (alId != UINT32_MAX) {
//     alDeleteSources(1, &alId);
//     openalError("Could not delete source");
//     alId = UINT32_MAX;
//   }
}

bool Source::isValid() const {
  return alId != UINT32_MAX;
}

void Source::play() {
  alSourcePlay(alId);
  openalError("Could not play source");
}

void Source::pause() {
  alSourcePause(alId);
  openalError("Could not pause source");
}

void Source::stop() {
  alSourceStop(alId);
  openalError("Could not stop source");
}

void Source::rewind() {
  alSourceRewind(alId);
  openalError("Could not rewind source");
}

void Source::queueBuffers(const int32_t &size, uint32_t* buffers) {
  alSourceQueueBuffers(alId, size, buffers);
  openalError("Could not queue buffers to source");
}

void Source::queueBuffers(const int32_t &size, Buffer* buffers) {
  alSourceQueueBuffers(alId, size, reinterpret_cast<uint32_t*>(buffers));
  openalError("Could not queue buffers to source");
}

void Source::unqueueBuffers(const int32_t &size, uint32_t* buffers) {
  alSourceUnqueueBuffers(alId, size, buffers);
  openalError("Could not unqueue buffers to source");
}

void Source::unqueueBuffers(const int32_t &size, Buffer* buffers) {
  alSourceUnqueueBuffers(alId, size, reinterpret_cast<uint32_t*>(buffers));
  openalError("Could not unqueue buffers to source");
}

void Source::setPitch(const float &data) {
  alSourcef(alId, AL_PITCH, data);
  openalError("Could not set source pitch");
}

void Source::setGain(const float &data) {
  alSourcef(alId, AL_GAIN, data);
  openalError("Could not set source gain");
}

void Source::setMaxDist(const float &data) {
  alSourcef(alId, AL_MAX_DISTANCE, data);
  openalError("Could not set source max distance");
}

void Source::setRolloff(const float &data) {
  alSourcef(alId, AL_ROLLOFF_FACTOR, data);
  openalError("Could not set source rolloff factor");
}

void Source::setRefDist(const float &data) {
  alSourcef(alId, AL_REFERENCE_DISTANCE, data);
  openalError("Could not set source reference dist");
}

void Source::setMinGain(const float &data) {
  alSourcef(alId, AL_MIN_GAIN, data);
  openalError("Could not set source min gain");
}

void Source::setMaxGain(const float &data) {
  alSourcef(alId, AL_MAX_GAIN, data);
  openalError("Could not set source max gain");
}

void Source::setConeOuterGain(const float &data) {
  alSourcef(alId, AL_CONE_OUTER_GAIN, data);
  openalError("Could not set source cone outer gain");
}

void Source::setConeInnerAngle(const float &data) {
  alSourcef(alId, AL_CONE_INNER_ANGLE, data);
  openalError("Could not set source cone inner angle");
}

void Source::setConeOuterAngle(const float &data) {
  alSourcef(alId, AL_CONE_OUTER_ANGLE, data);
  openalError("Could not set source cone outer angle");
}

void Source::setPos(const glm::vec3 &data) {
  alSource3f(alId, AL_POSITION, data.x, data.y, data.z);
  openalError("Could not set source position");
}

void Source::setVel(const glm::vec3 &data) {
  alSource3f(alId, AL_VELOCITY, data.x, data.y, data.z);
  openalError("Could not set source velocity");
}

void Source::setDir(const glm::vec3 &data) {
  alSource3f(alId, AL_DIRECTION, data.x, data.y, data.z);
  openalError("Could not set source direction");
}

void Source::setPos(const float* data) {
  alSourcefv(alId, AL_POSITION, data);
  openalError("Could not set source position");
}

void Source::setVel(const float* data) {
  alSourcefv(alId, AL_VELOCITY, data);
  openalError("Could not set source velocity");
}

void Source::setDir(const float* data) {
  alSourcefv(alId, AL_DIRECTION, data);
  openalError("Could not set source direction");
}

void Source::setSecOffset(const float &data) {
  alSourcef(alId, AL_SEC_OFFSET, data);
  openalError("Could not set source sec offset");
}

void Source::setSampleOffset(const float &data) {
  alSourcef(alId, AL_SAMPLE_OFFSET, data);
  openalError("Could not set source sample offset");
}

void Source::setByteOffset(const float &data) {
  alSourcef(alId, AL_BYTE_OFFSET, data);
  openalError("Could not set source byte offset");
}

void Source::setByteOffset(const int32_t &data) {
  alSourcei(alId, AL_BYTE_OFFSET, data);
  openalError("Could not set source byte offset");
}


void Source::buffer(const uint32_t &data) {
  alSourcei(alId, AL_BUFFER, data);
  openalError("Could not set source buffer");
}

void Source::relative(const bool &data) {
  alSourcei(alId, AL_SOURCE_RELATIVE, data);
  openalError("Could not set source relative to listener");
}

void Source::looping(const bool &data) {
  alSourcei(alId, AL_LOOPING, data);
  openalError("Could not set source looping");
}

float Source::getPitch() const {
  float data;
  alGetSourcef(alId, AL_PITCH, &data);
  openalError("Could not get source pitch");
  
  return data;
}

float Source::getGain() const {
  float data;
  alGetSourcef(alId, AL_GAIN, &data);
  openalError("Could not get source gain");
  
  return data;
}

float Source::getMaxDist() const {
  float data;
  alGetSourcef(alId, AL_MAX_DISTANCE, &data);
  openalError("Could not get source max distance");
  
  return data;
}

float Source::getRolloff() const {
  float data;
  alGetSourcef(alId, AL_ROLLOFF_FACTOR, &data);
  openalError("Could not get source rolloff factor");
  
  return data;
}

float Source::getRefDist() const {
  float data;
  alGetSourcef(alId, AL_REFERENCE_DISTANCE, &data);
  openalError("Could not get source reference distance");
  
  return data;
}

float Source::getMinGain() const {
  float data;
  alGetSourcef(alId, AL_MIN_GAIN, &data);
  openalError("Could not get source min gain");
  
  return data;
}

float Source::getMaxGain() const {
  float data;
  alGetSourcef(alId, AL_MAX_GAIN, &data);
  openalError("Could not get source max gain");
  
  return data;
}

float Source::getConeOuterGain() const {
  float data;
  alGetSourcef(alId, AL_CONE_OUTER_GAIN, &data);
  openalError("Could not get source cone outer gain");
  
  return data;
}

float Source::getConeInnerAngle() const {
  float data;
  alGetSourcef(alId, AL_CONE_INNER_ANGLE, &data);
  openalError("Could not get source cone inner angle");
  
  return data;
}

float Source::getConeOuterAngle() const {
  float data;
  alGetSourcef(alId, AL_CONE_OUTER_ANGLE, &data);
  openalError("Could not get source cone outer angle");
  
  return data;
}

float Source::secOffset() const {
  float value;
  alGetSourcef(alId, AL_SEC_OFFSET, &value);
  openalError("Could not get source sec offset");
  
  return value;
}

float Source::sampleOffset() const {
  float value;
  alGetSourcef(alId, AL_SAMPLE_OFFSET, &value);
  openalError("Could not get source sample offset");
  
  return value;
}

int32_t Source::sampleOffsetInt() const {
  int32_t value;
  alGetSourcei(alId, AL_SAMPLE_OFFSET, &value);
  openalError("Could not get source byte offset");
  
  return value;
}

float Source::byteOffset() const {
  float value;
  alGetSourcef(alId, AL_BYTE_OFFSET, &value);
  openalError("Could not get source byte offset");
  
  return value;
}

int32_t Source::byteOffsetInt() const {
  int32_t value;
  alGetSourcei(alId, AL_BYTE_OFFSET, &value);
  openalError("Could not get source byte offset");
  
  return value;
}

glm::vec3 Source::getPos() const {
  glm::vec3 data;
  alGetSource3f(alId, AL_POSITION, &data.x, &data.y, &data.z);
  openalError("Could not get source position");
  
  return data;
}

glm::vec3 Source::getVel() const {
  glm::vec3 data;
  alGetSource3f(alId, AL_VELOCITY, &data.x, &data.y, &data.z);
  openalError("Could not get source velocity");
  
  return data;
}

glm::vec3 Source::getDir() const {
  glm::vec3 data;
  alGetSource3f(alId, AL_DIRECTION, &data.x, &data.y, &data.z);
  openalError("Could not get source direction");
  
  return data;
}

uint32_t Source::getBuffer() const {
  int32_t data;
  alGetSourcei(alId, AL_BUFFER, &data);
  openalError("Could not get source buffer");
  
  return uint32_t(data);
}

int32_t Source::queuedBuffers() const {
  int32_t data;
  alGetSourcei(alId, AL_BUFFERS_QUEUED, &data);
  openalError("Could not get source buffers queued");
  
  return data;
}

int32_t Source::processedBuffers() const {
  int32_t data;
  alGetSourcei(alId, AL_BUFFERS_PROCESSED, &data);
  openalError("Could not get source buffers processed");
  
  return data;
}

Source::Type Source::type() const {
  int32_t data;
  alGetSourcei(alId, AL_SOURCE_TYPE, &data);
  openalError("Could not get source type");
  
  Type type = Type::undetermined;
  switch (data) {
    case AL_UNDETERMINED: { type = Type::undetermined; break; }
    case AL_STATIC:       { type = Type::statict;      break; }
    case AL_STREAMING:    { type = Type::streaming;    break; }
  }
  
  return type;
}

Source::State Source::state() const {
  int32_t data;
  alGetSourcei(alId, AL_SOURCE_STATE, &data);
  openalError("Could not get source state");
  
  State state = State::initial;
  switch (data) {
    case AL_STOPPED: { state = State::stopped; break; }
    case AL_PLAYING: { state = State::playing; break; }
    case AL_INITIAL: { state = State::initial; break; }
    case AL_PAUSED:  { state = State::paused;  break; }
  }
  
  return state;
}

bool Source::isRelative() const {
  int32_t data;
  alGetSourcei(alId, AL_SOURCE_RELATIVE, &data);
  openalError("Could not get source relative to listener");
  
  return bool(data);
}

bool Source::isLooping() const {
  int32_t data;
  alGetSourcei(alId, AL_LOOPING, &data);
  openalError("Could not get source looping");
  
  return bool(data);
}

uint32_t Source::id() const {
  return alId;
}

// Source & Source::operator=(Source &&other) {
//   alId = other.alId;
//   other.alId = UINT32_MAX;
//   
//   return *this;
// }

//DelayedWorkSystem

bool Source::operator==(const Source &other) {
  return alId == other.alId;
}

bool Source::operator!=(const Source &other) {
  return alId != other.alId;
}
