#include "SoundSystem.h"

#include "AL/al.h"
#include "AL/alc.h"
// #include "AL/alut.h"
#include "AL/alext.h"

#define DR_MP3_IMPLEMENTATION
#include "dr_mp3.h"

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

#include <stdexcept>
#include <iostream>
#include <string>
#include <cstring>
#include <chrono>

#ifdef _DEBUG
#include <cassert>
#define ASSERT(expr) assert(expr)
#else
#define ASSERT(expr)
#endif

// #include "portaudio.h"

// с аудио библиотеками оказалась та еще запара
// я не знаю какую использовать
// ал софт обновляется постоянно, но при этом требуется тащить openal32.dll (по идее файл должен быть по умолчанию везде)
// портаудио не обновлялся по крайней мере уже 2 года и там какой то всратый мемори менеджмент
// и есть еще куча других библиотек разной степени упоротости
// либсаундио вроде неплохая библиотека, но не поддерживает android и iOS

// думаю, что лучше буду использовать openal 
// еще есть soloud, надстройка примерно как портаудио
// и есть mini_al, врочем портаудио не нужно скидывать со счетов

// нет все же наверное портаудио
// правд не очень понимаю как дела обстоят с каналами

/*******************************************************************/
// static void printSupportedStandardSampleRates(const PaStreamParameters *inputParameters, const PaStreamParameters *outputParameters) {
//   static const double standardSampleRates[] = {
//     8000.0, 9600.0, 11025.0, 12000.0, 16000.0, 22050.0, 24000.0, 32000.0,
//     44100.0, 48000.0, 88200.0, 96000.0, 192000.0, -1 /* negative terminated  list */
//   };
//   
//   int printCount;
//   PaError err;
//   
//   printCount = 0;
//   for (uint32_t i = 0; standardSampleRates[i] > 0; ++i) {
//     err = Pa_IsFormatSupported(inputParameters, outputParameters, standardSampleRates[i]);
//     
//     if (err == paFormatIsSupported) {
//       if (printCount == 0) {
//         std::cout << "\t" << standardSampleRates[i];
//         printCount = 1;
//       } else if (printCount == 4) {
//         std::cout << ",\n\t" << standardSampleRates[i];
//         printCount = 1;
//       } else {
//         std::cout << ", " << standardSampleRates[i];
//         ++printCount;
//       }
//     }
//   }
//   
//   if (!printCount) std::cout << "None\n";
//   else std::cout << "\n";
// }

//   PaError err;
//   err = Pa_Initialize();
//   if (err != paNoError) {
//     Pa_Terminate();
//     std::cout << "Error number: " << err << "\n";
//     std::cout << "Error message: " << Pa_GetErrorText(err) << "\n";
//     throw std::runtime_error("Could not initialize sound system");
//   }
//   
//   std::cout << "PortAudio version: " << Pa_GetVersion() << "\n";
//   std::cout << "Version text: " << Pa_GetVersionInfo()->versionText << "\n";
//   
//   int32_t numDevices = Pa_GetDeviceCount();
//   if (numDevices < 0) {
//     Pa_Terminate();
//     err = numDevices;
//     std::cout << "Error number: " << err << "\n";
//     std::cout << "Error message: " << Pa_GetErrorText(err) << "\n";
//     throw std::runtime_error("Could not find sound devices");
//   }
//   
//   std::cout << "Number of devices = " << numDevices << "\n";
//   
//   PaStreamParameters inputParameters, outputParameters;
//   for (uint32_t i = 0; i < numDevices; ++i) {
//     const PaDeviceInfo *deviceInfo = Pa_GetDeviceInfo(i);
//     
//     std::cout << "--------------------------------------- device #" << i << "\n";
//     
//     /* Mark global and API specific default devices */
//     int32_t defaultDisplayed = 0;
//     if (i == Pa_GetDefaultInputDevice()) {
//       std::cout << "[ Default Input";
//       defaultDisplayed = 1;
//     } else if (i == Pa_GetHostApiInfo(deviceInfo->hostApi)->defaultInputDevice) {
//       const PaHostApiInfo *hostInfo = Pa_GetHostApiInfo(deviceInfo->hostApi);
//       std::cout << "[ Default " << hostInfo->name << " Input";
//       defaultDisplayed = 1;
//     }
//     
//     if (i == Pa_GetDefaultOutputDevice()) {
//       std::cout << (defaultDisplayed ? "," : "[");
//       std::cout << " Default Output";
//       defaultDisplayed = 1; 
//     } else if (i == Pa_GetHostApiInfo( deviceInfo->hostApi )->defaultOutputDevice) {
//       const PaHostApiInfo *hostInfo = Pa_GetHostApiInfo(deviceInfo->hostApi);
//       std::cout << (defaultDisplayed ? "," : "[");
//       std::cout << " Default " << hostInfo->name << " Output";
//       defaultDisplayed = 1;
//     }
//     
//     if (defaultDisplayed) std::cout << " ]\n";
//     
//     std::cout << "Name                        = " << deviceInfo->name << "\n";
//     
//     std::cout << "Host API                    = " << Pa_GetHostApiInfo( deviceInfo->hostApi )->name << "\n";
//     std::cout << "Max inputs = " << deviceInfo->maxInputChannels;
//     std::cout << ", Max outputs = " << deviceInfo->maxOutputChannels << "\n";
//     std::cout << "Default low input latency   = " << deviceInfo->defaultLowInputLatency << "\n";
//     std::cout << "Default low output latency  = " << deviceInfo->defaultLowOutputLatency << "\n";
//     std::cout << "Default high input latency  = " << deviceInfo->defaultHighInputLatency << "\n";
//     std::cout << "Default high output latency = " << deviceInfo->defaultHighOutputLatency << "\n";
//     
//     std::cout << "Default sample rate         = " << deviceInfo->defaultSampleRate << "\n";
//     /* poll for standard sample rates */
//     inputParameters.device = i;
//     inputParameters.channelCount = deviceInfo->maxInputChannels;
//     inputParameters.sampleFormat = paInt16;
//     inputParameters.suggestedLatency = 0; /* ignored by Pa_IsFormatSupported() */
//     inputParameters.hostApiSpecificStreamInfo = NULL;
//     
//     outputParameters.device = i;
//     outputParameters.channelCount = deviceInfo->maxOutputChannels;
//     outputParameters.sampleFormat = paInt16;
//     outputParameters.suggestedLatency = 0; /* ignored by Pa_IsFormatSupported() */
//     outputParameters.hostApiSpecificStreamInfo = NULL;
//     
//     if (inputParameters.channelCount > 0) {
//       std::cout << "Supported standard sample rates\n for half-duplex 16 bit " << inputParameters.channelCount << " channel input = " << "\n";
//       printSupportedStandardSampleRates(&inputParameters, nullptr);
//     }
//     
//     if (outputParameters.channelCount > 0) {
//       std::cout << "Supported standard sample rates\n for half-duplex 16 bit " << outputParameters.channelCount << " channel output = " << "\n";
//       printSupportedStandardSampleRates(nullptr, &outputParameters);
//     }
//     
//     if (inputParameters.channelCount > 0 && outputParameters.channelCount > 0) {
//       std::cout << "Supported standard sample rates\n for full-duplex 16 bit " << inputParameters.channelCount << " channel input, " << outputParameters.channelCount << " channel output = " << "\n";
//       printSupportedStandardSampleRates(&inputParameters, &outputParameters);
//     }
//   }
//   
//   Pa_Terminate();

// короче говоря в опенал'е только МОНО звуки могут затухать
// СТЕРЕО не затухают совсем
// с другой стороны, если выделить 2 источника с одним звуком разделеным на моно каналы?
// получилось! правда как странно проигрывается на мой взгляд, нужно больше почекать
// сранно проигрывалось походу из-за эффекта допплера
// без него все работает очень похоже
// следовательно можно разделять каналы для того чтобы получить требуемые эффекты
// с другой стороны это приводит к тому что теперь на кучу звуков будет использоваться по 2 и более каналов
// сколько их у меня всего? можно ли с легкостью тратить каналы на все звуки?

// таким образом у нас здесь будет по крайней мере 3 сущности
// непосредственно музыкальный файл (звук, саундтрек и прочее), сорс (источник звука) и буфер (храним в нем данные)
// + к этому добавится какой нибудь меморипул или что то вроде
// при возникновении необходимости в игре, звук должен получать количество неиспользуемых сорсов,
// брать какое-то их количество, добавлять в буферы куски данных, ставить в очередь сорса эти буферы 
// и в итоге все это дело запускать, и так пока не закончатся данные звука
// для этого для каждого сорса потребуется примерно 2-3 буфера как минимум (думаю что больше то особо не потребуется)
// и в каждом из них должно быть данных на 2-3 секунды (или максимум) для того чтобы я успевал их переключать

// 4 секунды (скорее даже около 5) это примерно 1.52 мб
// мне наверное потребуется секунды 3 (или макс) для каждого звука (по секунде на буфер)
// как определить эту секунду?

// так что у нас с мемори пулами? как я выяснил данные в буфер копируются
// следовательно свой мемори пул можно не делать (в смысле самому память выделять не нужно)
// а значит нужно держать большое количество опенал буферов, в которые мы эти данные будем добавлять
// вот что я нашел alBufferDataStatic
// это позволяет избежать копирования буферов, как раз то что нужно, надеюсь это везде работает

void openalError(const std::string &err) {
#ifdef _DEBUG
  ALenum error = alGetError();
  if (error == AL_NO_ERROR) return;
  
  std::cout << "Error: " << error << " desc: " << alGetString(error) << "\n";
  throw std::runtime_error(err);
#else
  (void)err;
#endif
}

void openalcError(ALCdevice* device, const std::string &err) {
#ifdef _DEBUG
  ALCenum error = alcGetError(device);
  if (error == ALC_NO_ERROR) return;
  
  std::cout << "Error: " << error << " desc: " << alcGetString(device, error) << "\n";
  throw std::runtime_error(err);
#else
  (void)err;
  (void)device;
#endif
}

void openalError(const ALenum &error, const std::string &err) {
#ifdef _DEBUG
  if (error == AL_NO_ERROR) return;

  std::cout << "Error: " << error << " desc: " << alGetString(error) << "\n";
  throw std::runtime_error(err);
#else
  (void)err;
  (void)error;
#endif
}

Source::Source() : alId(UINT32_MAX) {
  if (alId == UINT32_MAX) {
    alGenSources(1, &alId);
    openalError("Could not create source");
  }
}

Source::Source(const CreateInfo &info) : alId(UINT32_MAX) {
  if (alId == UINT32_MAX) {
    alGenSources(1, &alId);
    openalError("Could not create source");
  }
  
  setPitch(info.pitch);
  setGain(info.gain);
  setMaxDist(info.maxDist);
  setRolloff(info.rolloff);
  setRefDist(info.refDist);
  setMinGain(info.minGain);
  setMaxGain(info.maxGain);
  relative(info.relative);
  looping(info.looping);
}

Source::~Source() {
  if (alId != UINT32_MAX) {
    alDeleteSources(1, &alId);
    openalError("Could not delete source");
    alId = UINT32_MAX;
  }
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

void Source::unqueueBuffers(const int32_t &size, uint32_t* buffers) {
  alSourceUnqueueBuffers(alId, size, buffers);
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

int32_t Source::queueBuffers() const {
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
  
  Type type;
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
  
  State state;
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
  
Buffer::Buffer() : alId(UINT32_MAX) {
  if (alId == UINT32_MAX) {
    alGenBuffers(1, &alId);
    openalError("Could not create buffer");
  }
}

Buffer::~Buffer() {
  if (alId != UINT32_MAX) {
    alDeleteBuffers(1, &alId);
    openalError("Could not delete buffer");
    alId = UINT32_MAX;
  }
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

bool Buffer::bufferDataStaticEnabled = false;

void listAudioDevices(const ALCchar *devices) {
  const ALCchar *device = devices, *next = devices + 1;
  size_t len = 0;

  std::cout << "Devices list:\n";
  std::cout << "----------\n";
  
  while (device && *device != '\0' && next && *next != '\0') {
    std::cout << device << "\n";
    len = strlen(device);
    device += (len + 1);
    next += (len + 2);
  }
  
  std::cout << "----------\n";
}

void listExtensions(const ALCchar *devices) {
  const ALCchar *device = devices, *next = devices + 1;
  size_t len = 0;

  std::cout << "Extensions list:\n";
  std::cout << "----------\n";
  
  while (device && *device != '\0' && next && *next != '\0') {
    std::cout << device << "\n";
    len = strlen(device);
    device += (len + 1);
    next += (len + 2);
  }
  
  std::cout << "----------\n";
}

ALenum to_al_format(short channels, short samples) {
  bool stereo = (channels > 1);
  
  switch (samples) {
  case 16:
    if (stereo)
      return AL_FORMAT_STEREO16;
    else
      return AL_FORMAT_MONO16;
  case 8:
    if (stereo)
      return AL_FORMAT_STEREO8;
    else
      return AL_FORMAT_MONO8;
  default:
    return -1;
  }
}

// struct vec3 {
//   union {
//     struct {
//       float x, y, z;
//     };
//     float data[3];
//   };
// };

namespace Listener {
  void setGain(const float &data) {
    alListenerf(AL_GAIN, data);
    openalError("Could not set listener gain");
  }
  
  void setPos(const glm::vec3 &data) {
    alListener3f(AL_POSITION, data.x, data.y, data.z);
    openalError("Could not set listener position");
  }
  
  void setVel(const glm::vec3 &data) {
    alListener3f(AL_VELOCITY, data.x, data.y, data.z);
    openalError("Could not set listener velocity");
  }
  
  void setOrientation(const glm::vec3 &forward, const glm::vec3 &up) {
    float orient[6] = {forward.x, forward.y, forward.z, up.x, up.y, up.z};
    alListenerfv(AL_ORIENTATION, orient);
    openalError("Could not set listener orientation");
  }
  
  float gatGain() {
    float data;
    alGetListenerf(AL_GAIN, &data);
    openalError("Could not gat listener gain");
    
    return data;
  }
  
  glm::vec3 getPos() {
    glm::vec3 data;
    alGetListener3f(AL_POSITION, &data.x, &data.y, &data.z);
    openalError("Could not get listener position");
    
    return data;
  }
  
  glm::vec3 getVel() {
    glm::vec3 data;
    alGetListener3f(AL_VELOCITY, &data.x, &data.y, &data.z);
    openalError("Could not get listener velocity");
    
    return data;
  }
  
  void getOrientation(glm::vec3 &forward, glm::vec3 &up) {
    float orient[6];
    alGetListenerfv(AL_ORIENTATION, orient);
    openalError("Could not get listener orientation");
    
    forward = glm::vec3(orient[0], orient[1], orient[2]);
    up = glm::vec3(orient[3], orient[4], orient[5]);
  }
}

SoundSystem::SoundSystem() {
  ALCdevice* device;
  ALCcontext* ctx;
  ALCenum error;
  ALboolean ret;
  
//   openalError("Initial check");
  
  ret = alcIsExtensionPresent(nullptr, "ALC_ENUMERATION_EXT");
  if (ret) {
    listAudioDevices(alcGetString(nullptr, ALC_ALL_DEVICES_SPECIFIER));
  }
  
  ret = alcIsExtensionPresent(nullptr, "ALC_ENUMERATE_ALL_EXT");
  if (ret) {
    listExtensions(alcGetString(nullptr, ALC_EXTENSIONS));
  }
  
  listExtensions(alGetString(AL_EXTENSIONS));
  
  //ret = alcIsExtensionPresent(nullptr, "AL_EXT_static_buffer");
  //ret = alcIsExtensionPresent(nullptr, "AL_EXT_STATIC_BUFFER");
  //ret = alIsExtensionPresent("AL_EXT_STATIC_BUFFER");
  ret = alIsExtensionPresent("AL_EXT_static_buffer");
  if (ret) {
    Buffer::setBufferDataStatic(true);
    std::cout << "AL_EXT_static_buffer is present" << "\n";
  }
  
//   openalError("alcIsExtensionPresent");
  
  device = alcOpenDevice(NULL);
  if (device == nullptr) {
    openalcError(device, "Could not create OpenAL device");
  }
  
  ctx = alcCreateContext(device, NULL);
  openalcError(device, "Could not create OpenAL context");
  
  if (!alcMakeContextCurrent(ctx)) {
    openalError("Could not make current context");
  }
  
  alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED);
  openalError("Could not set distance model");
  
  const glm::vec3 front = glm::vec3(0.0f, 0.0f, 1.0f);
  const glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

  Listener::setPos(glm::vec3(0.0f, 0.0f, 0.0f));
  Listener::setVel(glm::vec3(0.0f, 0.0f, 0.0f));
  Listener::setOrientation(front, up);
  Listener::setGain(1.0f);
  
  Source source;
  source.setPitch(1.0f);
  source.setGain(0.1f);
  source.setMaxDist(100.0f);
  source.setRefDist(1.0f);
  source.setRolloff(1.0f);
  source.setMinGain(0.0f);
  source.setMaxGain(1.0f);
  source.setPos(glm::vec3(0.0f, 0.0f, 25.0f));
  source.setVel(glm::vec3(0.0f, 0.0f, 0.0f));
  source.looping(false);
  
  float var = 50.0f;
  Source source2;
  source2.setPitch(1.0f);
  source2.setGain(0.1f);
  source2.setMaxDist(100.0f);
  source2.setRefDist(1.0f);
  source2.setRolloff(1.0f);
  source2.setMinGain(0.0f);
  source2.setMaxGain(1.0f);
  source2.setPos(glm::vec3(var, 0.0f, 0.0f));
  source2.setVel(glm::vec3(0.0f, 0.0f, 0.0f));
  source2.looping(false);
  
  Buffer buffer;
  Buffer buffer2;
  
  drmp3 mp3;
  if (!drmp3_init_file(&mp3, "/home/mikuhatsune/Music/15.02.2019/Ephixa & Bossfight - Subside.mp3", NULL)) {
    throw std::runtime_error("Failed to open file");
  }
                               
  std::cout << "channels " << mp3.channels << " sampleRate " << mp3.sampleRate << "\n";
  
  uint64_t pcmFramesCount = drmp3_get_pcm_frame_count(&mp3);
  uint64_t mp3FramesCount = drmp3_get_mp3_frame_count(&mp3);
  
//   openalError("Could not buffer data");
  
  std::cout << "pcm frames count " << pcmFramesCount << " mp3 frames count " << mp3FramesCount << "\n";
  
  uint64_t framesToRead = 200000;
  float* pFrames = new float[mp3.channels*framesToRead];
  size_t size = mp3.channels*framesToRead*sizeof(float);
  
  size_t size1 = (mp3.channels*framesToRead/2+1)*sizeof(float);
  float* pFrames1 = new float[mp3.channels*framesToRead/2+1];
  
  size_t size2 = (mp3.channels*framesToRead/2+1)*sizeof(float);
  float* pFrames2 = new float[mp3.channels*framesToRead/2+1];
  
  uint64_t framesRead = drmp3_read_pcm_frames_f32(&mp3, framesToRead, pFrames);
  if (framesRead == 0) {
    throw std::runtime_error("Failed to read file");
  }
  
  for (size_t i = 0; i < mp3.channels*framesToRead; ++i) {
    if (i % 2 == 0) {
      pFrames1[i/2] = pFrames[i];
    } else {
      pFrames2[i/2+1] = pFrames[i];
    }
  }
  
  ALenum format = to_al_format(mp3.channels, 16);
//   const char* formatStr = alGetString(format);
  
//   openalError("Could not buffer data");
  
  std::cout << " ptr: " << pFrames << " size: " << size << " freq: " << mp3.sampleRate << " readed " << framesRead << " frames" << "\n";
  //alBufferData(buffer, AL_FORMAT_STEREO_FLOAT32, pFrames, size, mp3.sampleRate); //AL_FORMAT_MONO_FLOAT32
  //openalError("Could not buffer data");
  
  buffer.bufferDataStatic(AL_FORMAT_MONO_FLOAT32, pFrames1, size1, mp3.sampleRate);
  buffer2.bufferDataStatic(AL_FORMAT_MONO_FLOAT32, pFrames2, size2, mp3.sampleRate);
  
//   drwav wav;
//   if (!drwav_init_file(&wav, "/home/mikuhatsune/Downloads/SBDEAD.WAV")) {
//     throw std::runtime_error("Failed to open file");
//   }
//   
//   std::cout << "channels " << wav.channels << " sampleRate " << wav.sampleRate << " bits per sample " << wav.bitsPerSample << "\n";
//   
//   uint64_t pcmFramesCount = wav.totalPCMFrameCount;
//   std::cout << "pcm frames count " << pcmFramesCount << "\n";
//   
//   uint64_t framesToRead = pcmFramesCount;
//   short* pFrames = new short[wav.channels*framesToRead];
//   size_t size = wav.channels*framesToRead*sizeof(short);
//   
//   //uint64_t framesRead = drwav_read_pcm_frames_f32(&wav, framesToRead, pFrames);
//   uint64_t framesRead = drwav_read_pcm_frames_s16(&wav, framesToRead, pFrames);
//   //uint64_t framesRead = drwav_read_raw(&wav, framesToRead, pFrames);
//   if (framesRead == 0) {
//     throw std::runtime_error("Failed to read file");
//   }
//   
//   ALenum format = to_al_format(wav.channels, wav.bitsPerSample);
// //   const char* formatStr = alGetString(format);
//   
// //   openalError("Could not buffer data");
//   
//   std::cout << " ptr: " << pFrames << " size: " << size << " freq: " << wav.sampleRate << " readed " << framesRead << " frames" << "\n";
//   alBufferData(buffer, format, pFrames, size, wav.sampleRate);
//   openalError("Could not buffer data");
  
  source.buffer(buffer.id());
  source2.buffer(buffer2.id());
  
  const ALuint sources[] = {source.id(), source2.id()};
  
//   alSourcePlay(source);
//   openalError("Could not play sound");
//   
//   
//   
//   alSourcePlay(source2);
//   openalError("Could not play sound");
  
  alSourcePlayv(2, sources);
  openalError("Could not play sound");
  
  Source::State source_state = source.state();
  Source::State source_state2 = source2.state();
  
  auto start = std::chrono::steady_clock::now();
  
  while (source_state == Source::State::playing || source_state2 == Source::State::playing) {
    source_state = source.state();
    source_state2 = source2.state();
  }
  
  auto end = std::chrono::steady_clock::now() - start;
  auto sec = std::chrono::duration_cast<std::chrono::seconds>(end).count();
  std::cout << "Seconds: " << sec << "\n";
  
  drmp3_uninit(&mp3);
//   drwav_uninit(&wav);
  
  source.~Source();
  source2.~Source();
  
  buffer.~Buffer();
  buffer2.~Buffer();
  
  delete [] pFrames;
  delete [] pFrames1;
  delete [] pFrames2;
  
  device = alcGetContextsDevice(ctx);
  alcMakeContextCurrent(NULL);
  alcDestroyContext(ctx);
  alcCloseDevice(device);
}

SoundSystem::~SoundSystem() {
  
}

void SoundSystem::update(const uint64_t &time) {
  
}
