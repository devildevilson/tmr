#include "SoundSystem.h"

#include "AL/al.h"
#include "AL/alc.h"
// #include "AL/alut.h"
#include "AL/alext.h"

#include "alHelpers.h"

#include "SoundComponent.h"

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

// мне нужно добавить поддержку ogg обязательно

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
  
  void setPos(const float* data) {
    alListenerfv(AL_POSITION, data);
    openalError("Could not set listener position");
  }
  
  void setVel(const glm::vec3 &data) {
    alListener3f(AL_VELOCITY, data.x, data.y, data.z);
    openalError("Could not set listener velocity");
  }
  
  void setVel(const float* data) {
    alListenerfv(AL_VELOCITY, data);
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

enum QueueSoundTypeEnum : uint32_t {
  QUEUED_SOUND_RELATIVE = (1<<0),
  QUEUED_SOUND_DELETE_AFTER_PLAY = (1<<1),
  QUEUED_SOUND_PLAY_ANYWAY = (1<<2),
};

QueueSoundType::QueueSoundType() : container(0) {}
QueueSoundType::QueueSoundType(const bool relative, const bool deleteAfterPlayed, const bool playAnyway) : container(0) {
  makeType(relative, deleteAfterPlayed, playAnyway);
}

void QueueSoundType::makeType(const bool relative, const bool deleteAfterPlayed, const bool playAnyway) {
  container = container | (uint32_t(relative) * QUEUED_SOUND_RELATIVE)
                        | (uint32_t(deleteAfterPlayed) * QUEUED_SOUND_DELETE_AFTER_PLAY)
                        | (uint32_t(playAnyway) * QUEUED_SOUND_PLAY_ANYWAY);
}

bool QueueSoundType::isRelative() const {
  return bool(container & QUEUED_SOUND_RELATIVE);
}

bool QueueSoundType::deleteAfterPlayed() const {
  return bool(container & QUEUED_SOUND_DELETE_AFTER_PLAY);
}

bool QueueSoundType::playAnyway() const {
  return bool(container & QUEUED_SOUND_PLAY_ANYWAY);
}

void QueueSoundType::setRelative(const bool enable) {
  container = enable ? container | QUEUED_SOUND_RELATIVE : container & (~QUEUED_SOUND_RELATIVE);
}

void QueueSoundType::setDeleteAfterPlayed(const bool enable) {
  container = enable ? container | QUEUED_SOUND_DELETE_AFTER_PLAY : container & (~QUEUED_SOUND_DELETE_AFTER_PLAY);
}

void QueueSoundType::setPlayAnyway(const bool enable) {
  container = enable ? container | QUEUED_SOUND_PLAY_ANYWAY : container & (~QUEUED_SOUND_PLAY_ANYWAY);
}

SoundSystem::SoundSystem() : device(nullptr), ctx(nullptr) {
//   ALCdevice* device;
//   ALCcontext* ctx;
  ALCenum error;
  ALboolean ret;
  
//   openalError("Initial check");
  
  // все эти листинги мне по идее не нужны
  ret = alcIsExtensionPresent(nullptr, "ALC_ENUMERATION_EXT");
  if (ret) {
    listAudioDevices(alcGetString(nullptr, ALC_ALL_DEVICES_SPECIFIER));
  }
  
  ret = alcIsExtensionPresent(nullptr, "ALC_ENUMERATE_ALL_EXT");
  if (ret) {
    listExtensions(alcGetString(nullptr, ALC_EXTENSIONS));
  }
  
  listExtensions(alGetString(AL_EXTENSIONS));
//   listExtensions(alcGetString(nullptr, AL_EXTENSIONS));
  
  //ret = alcIsExtensionPresent(nullptr, "AL_EXT_static_buffer");
  //ret = alcIsExtensionPresent(nullptr, "AL_EXT_STATIC_BUFFER");
  //ret = alIsExtensionPresent("AL_EXT_STATIC_BUFFER");
  // пока не знаю что с этим делать
//   ret = alIsExtensionPresent("AL_EXT_static_buffer");
//   if (ret) {
//     Buffer::setBufferDataStatic(true);
//     std::cout << "AL_EXT_static_buffer is present" << "\n";
//   }
  
//   openalError("alcIsExtensionPresent");
  
  device = alcOpenDevice(NULL);
  if (device == nullptr) {
    error = alcGetError(device);
    openalcError(device, error, "Could not create OpenAL device");
  }
  
  ctx = alcCreateContext(device, NULL);
  error = alcGetError(device);
  openalcError(device, error, "Could not create OpenAL context");
  
  if (!alcMakeContextCurrent(ctx)) {
    error = alGetError();
    openalcError(device, error, "Could not make current context");
  }
  
  alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED);
  openalError("Could not set distance model");
  
  const glm::vec3 front = glm::vec3(0.0f, 0.0f, 1.0f);
  const glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

  Listener::setPos(glm::vec3(0.0f, 0.0f, 0.0f));
  Listener::setVel(glm::vec3(0.0f, 0.0f, 0.0f));
  Listener::setOrientation(front, up);
  Listener::setGain(1.0f);

//   
//   Source source;
//   source.setPitch(1.0f);
//   source.setGain(0.1f);
//   source.setMaxDist(100.0f);
//   source.setRefDist(1.0f);
//   source.setRolloff(1.0f);
//   source.setMinGain(0.0f);
//   source.setMaxGain(1.0f);
//   source.setPos(glm::vec3(0.0f, 0.0f, 25.0f));
//   source.setVel(glm::vec3(0.0f, 0.0f, 0.0f));
//   source.looping(false);
//   
//   float var = 50.0f;
//   Source source2;
//   source2.setPitch(1.0f);
//   source2.setGain(0.1f);
//   source2.setMaxDist(100.0f);
//   source2.setRefDist(1.0f);
//   source2.setRolloff(1.0f);
//   source2.setMinGain(0.0f);
//   source2.setMaxGain(1.0f);
//   source2.setPos(glm::vec3(var, 0.0f, 0.0f));
//   source2.setVel(glm::vec3(0.0f, 0.0f, 0.0f));
//   source2.looping(false);
//   
//   Buffer buffer;
//   Buffer buffer2;
//   
//   drmp3 mp3;
//   if (!drmp3_init_file(&mp3, "/home/mikuhatsune/Music/15.02.2019/Ephixa & Bossfight - Subside.mp3", NULL)) {
//     throw std::runtime_error("Failed to open file");
//   }
//                                
//   std::cout << "channels " << mp3.channels << " sampleRate " << mp3.sampleRate << "\n";
//   
//   uint64_t pcmFramesCount = drmp3_get_pcm_frame_count(&mp3);
//   uint64_t mp3FramesCount = drmp3_get_mp3_frame_count(&mp3);
//   
// //   openalError("Could not buffer data");
//   
//   std::cout << "pcm frames count " << pcmFramesCount << " mp3 frames count " << mp3FramesCount << "\n";
//   
//   uint64_t framesToRead = 200000;
//   float* pFrames = new float[mp3.channels*framesToRead];
//   size_t size = mp3.channels*framesToRead*sizeof(float);
//   
//   size_t size1 = (mp3.channels*framesToRead/2+1)*sizeof(float);
//   float* pFrames1 = new float[mp3.channels*framesToRead/2+1];
//   
//   size_t size2 = (mp3.channels*framesToRead/2+1)*sizeof(float);
//   float* pFrames2 = new float[mp3.channels*framesToRead/2+1];
//   
//   uint64_t framesRead = drmp3_read_pcm_frames_f32(&mp3, framesToRead, pFrames);
//   if (framesRead == 0) {
//     throw std::runtime_error("Failed to read file");
//   }
//   
//   for (size_t i = 0; i < mp3.channels*framesToRead; ++i) {
//     if (i % 2 == 0) {
//       pFrames1[i/2] = pFrames[i];
//     } else {
//       pFrames2[i/2+1] = pFrames[i];
//     }
//   }
//   
//   ALenum format = to_al_format(mp3.channels, 16);
// //   const char* formatStr = alGetString(format);
//   
// //   openalError("Could not buffer data");
//   
//   std::cout << " ptr: " << pFrames << " size: " << size << " freq: " << mp3.sampleRate << " readed " << framesRead << " frames" << "\n";
//   //alBufferData(buffer, AL_FORMAT_STEREO_FLOAT32, pFrames, size, mp3.sampleRate); //AL_FORMAT_MONO_FLOAT32
//   //openalError("Could not buffer data");
//   
//   buffer.bufferDataStatic(AL_FORMAT_MONO_FLOAT32, pFrames1, size1, mp3.sampleRate);
//   buffer2.bufferDataStatic(AL_FORMAT_MONO_FLOAT32, pFrames2, size2, mp3.sampleRate);
//   
// //   drwav wav;
// //   if (!drwav_init_file(&wav, "/home/mikuhatsune/Downloads/SBDEAD.WAV")) {
// //     throw std::runtime_error("Failed to open file");
// //   }
// //   
// //   std::cout << "channels " << wav.channels << " sampleRate " << wav.sampleRate << " bits per sample " << wav.bitsPerSample << "\n";
// //   
// //   uint64_t pcmFramesCount = wav.totalPCMFrameCount;
// //   std::cout << "pcm frames count " << pcmFramesCount << "\n";
// //   
// //   uint64_t framesToRead = pcmFramesCount;
// //   short* pFrames = new short[wav.channels*framesToRead];
// //   size_t size = wav.channels*framesToRead*sizeof(short);
// //   
// //   //uint64_t framesRead = drwav_read_pcm_frames_f32(&wav, framesToRead, pFrames);
// //   uint64_t framesRead = drwav_read_pcm_frames_s16(&wav, framesToRead, pFrames);
// //   //uint64_t framesRead = drwav_read_raw(&wav, framesToRead, pFrames);
// //   if (framesRead == 0) {
// //     throw std::runtime_error("Failed to read file");
// //   }
// //   
// //   ALenum format = to_al_format(wav.channels, wav.bitsPerSample);
// // //   const char* formatStr = alGetString(format);
// //   
// // //   openalError("Could not buffer data");
// //   
// //   std::cout << " ptr: " << pFrames << " size: " << size << " freq: " << wav.sampleRate << " readed " << framesRead << " frames" << "\n";
// //   alBufferData(buffer, format, pFrames, size, wav.sampleRate);
// //   openalError("Could not buffer data");
//   
//   source.buffer(buffer.id());
//   source2.buffer(buffer2.id());
//   
//   const ALuint sources[] = {source.id(), source2.id()};
//   
// //   alSourcePlay(source);
// //   openalError("Could not play sound");
// //   
// //   
// //   
// //   alSourcePlay(source2);
// //   openalError("Could not play sound");
//   
//   alSourcePlayv(2, sources);
//   openalError("Could not play sound");
//   
//   Source::State source_state = source.state();
//   Source::State source_state2 = source2.state();
//   
//   auto start = std::chrono::steady_clock::now();
//   
//   while (source_state == Source::State::playing || source_state2 == Source::State::playing) {
//     source_state = source.state();
//     source_state2 = source2.state();
//   }
//   
//   auto end = std::chrono::steady_clock::now() - start;
//   auto sec = std::chrono::duration_cast<std::chrono::seconds>(end).count();
//   std::cout << "Seconds: " << sec << "\n";
//   
//   drmp3_uninit(&mp3);
// //   drwav_uninit(&wav);
//   
//   source.~Source();
//   source2.~Source();
//   
//   buffer.~Buffer();
//   buffer2.~Buffer();
//   
//   delete [] pFrames;
//   delete [] pFrames1;
//   delete [] pFrames2;
  
//   device = alcGetContextsDevice(ctx);
  
  error = alGetError();
  openalError(error, "dwvvdwvwlvwvmdvlwvwvmdlmwvlwd");
  while (error == AL_NO_ERROR) {
    uint32_t id;
    alGenSources(1, &id);
    error = alGetError();
    
//     openalError(error, "Could not create sources");
    
    freeSources.emplace_back(id);
  }
  
  //std::cout << "freeSources.size() " << freeSources.size() << "\n";
  freeSources.pop_back();
  if (freeSources.empty()) throw std::runtime_error("Could not create sources");
  if (!freeSources[0].isValid()) throw std::runtime_error("bad sources");
  
  soundtrack.source = freeSources.back();
  freeSources.pop_back();
  
  sourcesCountVar = freeSources.size();
  
  error = AL_NO_ERROR;
  for (size_t i = 0; i < freeSources.size(); ++i) {
    ALCuint ids[2];
    alGenBuffers(2, ids);
    error = alGetError();
    
    openalError(error, "Could not create sound buffers");
    
    sourceBuffers.push_back({Buffer(ids[0]), Buffer(ids[1])});
  }
  
  {
    ALCuint ids[2];
    alGenBuffers(2, ids);
    error = alGetError();
    
    openalError(error, "Could not create sound buffers");
    
    soundtrack.buffers = Buffers(Buffer(ids[0]), Buffer(ids[1]));
    
    soundtrack.gain = 0.1f;
    soundtrack.loadedSize = 0;
    soundtrack.sound = nullptr;
  }
  
  // судя по тому что написано id переменная не будет перезаписана
  // следовательно я спокойно могу удалить последний сорс
}

SoundSystem::~SoundSystem() {
  for (auto &data : datas) {
    soundsPool.deleteElement(data.second);
  }
  
  for (size_t i = 0; i < soundQueue.size(); ++i) {
    if (soundQueue[i]->source.isValid()) {
      freeSources.push_back(soundQueue[i]->source);
      soundQueue[i]->source.stop();
      soundQueue[i]->source.buffer(0);
      soundQueue[i]->source = Source(UINT32_MAX);
    }
    
    if (!soundQueue[i]->data->isCached()) {
      sourceBuffers.push_back(soundQueue[i]->buffers);
    }
    
    queueDataPool.deleteElement(soundQueue[i]);
  }
  
//   for (auto &buffers : cachedBuffers) {
//     if (buffers.second.buffers[0].isValid()) {
//       uint32_t id = buffers.second.buffers[0].id();
//       alDeleteBuffers(1, &id);
//     }
//     
//     if (buffers.second.buffers[1].isValid()) {
//       uint32_t id = buffers.second.buffers[1].id();
//       alDeleteBuffers(1, &id);
//     }
//   }
  
  openalError("Could not delete cached buffers");
  
  for (size_t i = 0; i < sourceBuffers.size(); ++i) {
//     if (sourceBuffers[i].first.isValid()) {
//       uint32_t id = sourceBuffers[i].first.id();
//       alDeleteBuffers(1, &id);
//     }
//     
//     if (sourceBuffers[i].second.isValid()) {
//       uint32_t id = sourceBuffers[i].second.id();
//       alDeleteBuffers(1, &id);
//     }
    
    alDeleteBuffers(2, reinterpret_cast<uint32_t*>(&sourceBuffers[i]));
  }
  
  openalError("Could not delete sources buffers");
  
  alDeleteSources(freeSources.size(), reinterpret_cast<uint32_t*>(freeSources.data()));
  
  openalError("Could not delete sources");
  
//   alDeleteBuffers(soundBuffers.size(), reinterpret_cast<uint32_t*>(soundBuffers.data()));
  
//   sources.clear();
//   soundBuffers.clear();
  
  alcMakeContextCurrent(NULL);
  alcDestroyContext(ctx);
  alcCloseDevice(device);
}

void SoundSystem::updateListener(const ListenerData &data) {
  {
    float arr[4];
    data.pos.storeu(arr);
    Listener::setPos(arr);
  }
  
  {
    float arr[4];
    data.velocity.storeu(arr);
    Listener::setVel(arr);
  }
  
  {
    float arr1[4];
    float arr2[4];
    data.forward.storeu(arr1);
    data.up.storeu(arr2);
    Listener::setOrientation(glm::vec3(arr1[0], arr1[1], arr1[2]), glm::vec3(arr2[0], arr2[1], arr2[2]));
  }
}

void SoundSystem::update(const uint64_t &time) {
  // как будет выглядеть апдейт?
  // у нас должна быть пачка новоприбывших звуков
  // мы смотрим на их приоритет (по идее по дальности от игрока)
  // выбиваем из activeSounds по приоритету (заканчиваем звук, резетим его)
  // ставим новые звуки на освободившиеся места
  // у нас будет несколько звуков с 0 дальностью + саундтрек
  // саундтрек не должен прерываться ни при каких условиях
  
  // все играющие звуки мы проверяем сколлько они проиграли и нужно ли их дозаполнить
  // ifstream и видимо все чтения файлов будут работать в мультипотоке
  
  // я решил переделать звук опять, теперь звук должен проигрываться отсюда
  // и возиться с буферами я тоже должен здесь
  
  {
    const uint32_t processedBuffers = soundtrack.source.processedBuffers();
    if (processedBuffers > 0 && soundtrack.loadedSize < soundtrack.sound->pcmSize()) {
      
      std::cout << "load next buffer" << '\n';
      
      // грузим новые данные
      // нам скорее всего нужно просто передать буфер в лоадНекст
      // для того чтобы не грузить нужно проверить совпадает ли загруженный размер и общим
      // как сделать плавное повторение? нужно ли мне оно?
      // как определить когда звук закончился? кода сорс простаивает? 
      // Source::State::playing
      
      Buffer buf;
      soundtrack.source.unqueueBuffers(1, &buf);
      const size_t readSize = soundtrack.sound->loadNext(soundtrack.loadedSize, buf);
      soundtrack.loadedSize += readSize;
      soundtrack.source.queueBuffers(1, &buf);
    }
  }
  
  // думаю что нужно использовать саундКью как раз для того чтобы раздать сорсы
  // то бишь, сорсы выдаются только первым N звукам, если звук потерял приоритет
  // либо закончился, то должен освободить сорс, как мы должны сообщить саундКомпоненту
  // о конце звука? компонент должен как то следить за состоянием своего звука, 
  // и возможно самостоятельно прикидывать его текущее состояние
  // компонент скорее всего нужно обновлять
  for (size_t i = 0; i < components.size(); ++i) {
    // обновляем компонент каждый кадр
    // в нем обновляется положение звуков запущенных компонентом
    // кроме положения, наверное обновляется что то еще
    // например, если приходит эвент как какой-нибудь звук в очереди
    // зацикливаем его
    
    components[i]->update(time);
  }
  
  const glm::vec3 pos = Listener::getPos();
  const simd::vec4 listenerPos = simd::vec4(pos.x, pos.y, pos.z, 1.0f);
  for (size_t i = 0; i < soundQueue.size(); ++i) {
    const simd::vec4 soundPos = simd::vec4(soundQueue[i]->pos);
    const float d = simd::distance(listenerPos, soundPos);
    soundQueue[i]->priority = (d / (soundQueue[i]->maxDist == 0.0f ? 1.0f : soundQueue[i]->maxDist)) + (soundQueue[i]->loadedSize == SIZE_MAX ? 10000.0f : 0.0f);
    
    if (soundQueue[i]->priority > 1.0f && soundQueue[i]->type.deleteAfterPlayed() && !soundQueue[i]->type.playAnyway()) {
      if (soundQueue[i]->source.isValid()) {
        freeSources.push_back(soundQueue[i]->source);
        soundQueue[i]->source = Source(UINT32_MAX);
        
        sourceBuffers.push_back(soundQueue[i]->buffers);
        soundQueue[i]->buffers.buffers[0] = Buffer(UINT32_MAX);
        soundQueue[i]->buffers.buffers[1] = Buffer(UINT32_MAX);
      }
      
      queueDataPool.deleteElement(soundQueue[i]);
      std::swap(soundQueue[i], soundQueue.back());
      soundQueue.pop_back();
      --i;
    }
    
//     if (soundQueue[i]->data == nullptr) {
//       soundQueue[i]->data = datas[soundQueue[i]->id];
//     }
  }
  
  // сортировать очередь наверное будем здесь
  std::sort(soundQueue.begin(), soundQueue.end(), [] (const QueueSoundData* left, const QueueSoundData* right) {
    return left->priority < right->priority;
  });
  
  float maxPrior = -1.0f, minPrior = 1000000.0f;
  size_t maxPriorIndex = SIZE_MAX, minPriorIndex = 0, sourceCount = 0;
  for (size_t i = 0; i < soundQueue.size(); ++i) {
    QueueSoundData* data = soundQueue[i];
    if (data->priority > 1.0f) continue;
    
    // здесь мы запускаем звуки, загружаем новые куски в буферы
    // в общем следим чтобы сорсы работали а не простаивали
    // как возвращать сорсы? еще раз пройти буфер
    
    // также неплохо было бы объединять несколько звуков,
    // но это скорее всего уже гораздо сложнее
    // в будущем пригодится объединять несколько похожих звуков 
    // или модулирвать какие-нибудь другие звуки из нескольких
    // не уверен что это может пригодится сейчас, поэтому оставлю
    
    // если свободные сорсы есть
    if (!freeSources.empty()) {
      // и если звук сейчас не проигрывается
      if (!data->source.isValid()) {
        // берем свободный сорс и начинаем проигрывать
        data->source = freeSources.back();
        freeSources.pop_back();
        // стоит сразу засунуть сюда буферы
        data->updateSource();
        
        const Buffers &tmp = sourceBuffers.back();
        data->buffers.buffers[0] = tmp.buffers[0];
        data->buffers.buffers[1] = tmp.buffers[1];
        
        sourceBuffers.pop_back();
        
        bool secondBufferHasData = false;
        if (!data->data->isLazyPreloaded()) {
          const ContainerData &containerData = cachedBuffers[data->id];
          
          const int32_t format = to_al_format(data->data->channelCount(), data->data->bitsPerSample());
          data->loadedSize = bytesToPCMFrames(containerData.size1 + containerData.size2, data->data);
          
          data->buffers.buffers[0].bufferData(format, &container[containerData.offset], containerData.size1, data->data->sampleRate());
          
          if (containerData.size2 != 0) {
            data->buffers.buffers[1].bufferData(format, &container[containerData.offset+containerData.size1], containerData.size2, data->data->sampleRate());
            secondBufferHasData = true;
          }
        } else {
          size_t readSize = data->data->loadNext(data->loadedSize, data->buffers.buffers[0]);
          data->loadedSize += readSize;
          if (data->loadedSize < data->data->pcmSize()) {
            data->data->loadNext(data->loadedSize, data->buffers.buffers[1]);
            data->loadedSize += readSize;
            secondBufferHasData = true;
          }
        }
        
        if (secondBufferHasData) {
          data->source.queueBuffers(2, data->buffers.buffers);
          
//           std::cout << "second valid" << "\n";
        } else {
          data->source.queueBuffers(1, data->buffers.buffers);
        }
        data->source.play();
      }
    } else {
      // иначе изменяем мин приоритет
      if (data->priority < minPrior) {
        minPrior = data->priority;
        minPriorIndex = i;
      }
    }
    
    if (data->source.isValid()) {
      if (data->source.state() != Source::State::playing) {
        data->source.buffer(0);
        
        data->loadedSize = SIZE_MAX;
        // нам нужно что то тут сделать чтобы случайно не начать снова проигрывать
        // повторение мы можем сделать с помощью 
        // мне нужно обратно вернуться в то состояние в котором я был
        // если у меня музыка кэширована, то по идее делать особ ничего не надо
        // если у меня часть музыки подгружено, то мне нужно снова часть подгрузить с начала
        // на самом деле я тут подумал. мне бы кешированные данные хранить не в буферах
        // а в голом виде, а потом запихивать это дело в буфер
        // блен так получатся дубликаты, может это не так уж плохо?
        // 
        
//         std::cout << "remove source" << '\n';
        
        freeSources.push_back(data->source);
        data->source = Source(UINT32_MAX);
        
        sourceBuffers.push_back(data->buffers);
        data->buffers.buffers[0] = Buffer(UINT32_MAX);
        data->buffers.buffers[1] = Buffer(UINT32_MAX);
        
        // в многопотоке нужно будет удалять по другому естественно
        if (data->type.deleteAfterPlayed()) {
          queueDataPool.deleteElement(soundQueue[i]);
          std::swap(soundQueue[i], soundQueue.back());
          soundQueue.pop_back();
          --i;
        }
      } else {
        
        // в ином случае чекаем, нужно ли подгрузить следующий кусок данных
        const uint32_t processedBuffers = data->source.processedBuffers();
        if (processedBuffers > 0 && data->loadedSize < data->data->pcmSize()) {
          
          std::cout << "load next buffer" << '\n';
          
          // грузим новые данные
          // нам скорее всего нужно просто передать буфер в лоадНекст
          // для того чтобы не грузить нужно проверить совпадает ли загруженный размер и общим
          // как сделать плавное повторение? нужно ли мне оно?
          // как определить когда звук закончился? кода сорс простаивает? 
          // Source::State::playing
          
          Buffer buf;
          data->source.unqueueBuffers(1, &buf);
          const size_t readSize = data->data->loadNext(data->loadedSize, buf);
          data->loadedSize += readSize;
          data->source.queueBuffers(1, &buf);
        }
        
        if (data->priority > maxPrior) {
          maxPrior = data->priority;
          maxPriorIndex = i;
        }
        ++sourceCount;
      }
    }
    
    // если sourceCount == всего количеству сорсов то выходим
    if (sourcesCountVar == sourceCount) break;
  }
  
  // после этого у нас остается свободный мин приоритет и проигрываемый макс приоритет
  // если мин < макс то выдергиваем у макс сорс и передаем мин
  // таким образом потихоньку заканчиваем не особо нужные звуки
  // хотя я думаю тут нужно ввести некую толерантность
  if (maxPriorIndex != SIZE_MAX && minPrior < maxPrior) {
    // у макс нужно убрать буферы
    
    Source s = soundQueue[maxPriorIndex]->source;
    s.stop();
    s.buffer(0);
    
    Buffers b = soundQueue[maxPriorIndex]->buffers;
    soundQueue[maxPriorIndex]->buffers.buffers[0] = Buffer(UINT32_MAX);
    soundQueue[maxPriorIndex]->buffers.buffers[1] = Buffer(UINT32_MAX);
    
    soundQueue[maxPriorIndex]->source = Source(UINT32_MAX);
    soundQueue[minPriorIndex]->source = s;
    soundQueue[minPriorIndex]->buffers = b;
    
    QueueSoundData* data = soundQueue[minPriorIndex];
    
    // настраиваем на проигрывание
    
    data->updateSource();
    
    bool secondBufferHasData = false;
    if (!data->data->isLazyPreloaded()) {
      const ContainerData &containerData = cachedBuffers[data->id];
      
      const int32_t format = to_al_format(data->data->channelCount(), data->data->bitsPerSample());
      data->loadedSize = bytesToPCMFrames(containerData.size1 + containerData.size2, data->data);
      
      data->buffers.buffers[0].bufferData(format, &container[containerData.offset], containerData.size1, data->data->sampleRate());
      
      if (containerData.size2 != 0) {
        data->buffers.buffers[1].bufferData(format, &container[containerData.offset+containerData.size1], containerData.size2, data->data->sampleRate());
        secondBufferHasData = true;
      }
    } else {
      size_t readSize = data->data->loadNext(data->loadedSize, data->buffers.buffers[0]);
      data->loadedSize += readSize;
      if (data->loadedSize < data->data->pcmSize()) {
        data->data->loadNext(data->loadedSize, data->buffers.buffers[1]);
        data->loadedSize += readSize;
        secondBufferHasData = true;
      }
    }
    
    if (secondBufferHasData) {
      data->source.queueBuffers(2, data->buffers.buffers);
    } else {
      data->source.queueBuffers(1, data->buffers.buffers);
    }
    s.play();
  }
  
  // что нужно передать компоненту чтобы ему было легко обновлять звук?
  // указатель на структуру с данными, там должны быть все необходимые данные
  // то есть позиция, примерное текущее время от начала 
  // (мне нужно чтобы звук был синхронизирован с действием, мы должны начать и закончить ровно в определенный момент)
  // если у нас нет свободных сорсов, то звук не проигрывается
  // если звук начал проигрываться то сорс он отпускает только в том случае если закончился или вышел за переделы дальности
}

void SoundSystem::playBackgroundSound(const ResourceID &id) {
  // останавливаем звук если проигрывался и проигрываем следующий
  ResourceID finalId = id.id() == SIZE_MAX ? soundtrack.id : id;
  
  if (soundtrack.id == finalId && soundtrack.source.state() == Source::State::playing) return;
  else if (soundtrack.id == finalId && soundtrack.source.state() == Source::State::paused) {
    soundtrack.source.play();
    return;
  } else if (soundtrack.id == finalId) {
    soundtrack.loadedSize = 0;
  }
  
  if (soundtrack.id != finalId) {
    soundtrack.id = finalId;
    soundtrack.sound = datas[finalId];
    soundtrack.loadedSize = 0;
    
    soundtrack.source.stop();
    soundtrack.source.buffer(0);
  }
  
  bool secondBufferHasData = false;
  if (!soundtrack.sound->isLazyPreloaded()) {
    const ContainerData &containerData = cachedBuffers[soundtrack.id];
    
    const int32_t format = to_al_format(soundtrack.sound->channelCount(), soundtrack.sound->bitsPerSample());
    soundtrack.loadedSize = bytesToPCMFrames(containerData.size1 + containerData.size2, soundtrack.sound);
    
    soundtrack.buffers.buffers[0].bufferData(format, &container[containerData.offset], containerData.size1, soundtrack.sound->sampleRate());
    
    if (containerData.size2 != 0) {
      soundtrack.buffers.buffers[1].bufferData(format, &container[containerData.offset+containerData.size1], containerData.size2, soundtrack.sound->sampleRate());
      secondBufferHasData = true;
    }
  } else {
    size_t readSize = soundtrack.sound->loadNext(soundtrack.loadedSize, soundtrack.buffers.buffers[0]);
    soundtrack.loadedSize += readSize;
    if (soundtrack.loadedSize < soundtrack.sound->pcmSize()) {
      soundtrack.sound->loadNext(soundtrack.loadedSize, soundtrack.buffers.buffers[1]);
      soundtrack.loadedSize += readSize;
      secondBufferHasData = true;
    }
  }
  
  soundtrack.source.queueBuffers(secondBufferHasData ? 2 : 1, soundtrack.buffers.buffers);
  soundtrack.source.play();
}

void SoundSystem::pauseBackgroundSound() {
  // пауза, никаких особых хитростей
  soundtrack.source.pause();
}

void SoundSystem::stopBackgroundSound() {
  // стоп, должен резетить звук
  soundtrack.source.stop();
  soundtrack.loadedSize = 0;
}

Source::State SoundSystem::getBackgroundSoundState() const {
  return soundtrack.source.state();
}

size_t SoundSystem::getBackgroundSoundDuration() const {
  return soundtrack.sound != nullptr ? size_t(float(soundtrack.sound->pcmSize()) / float(soundtrack.sound->sampleRate()) * 1000000) : 0;
}

float SoundSystem::getBackgroundSoundPosition() const {
  // нужно взять что мы загрузили, вычесть что еще не доиграли, разделить с общим количеством
  //const size_t loaded = pcmFramesToBytes(soundtrack.loadedSize, soundtrack.sound);
  const size_t loaded = soundtrack.loadedSize;
  
  if (loaded == soundtrack.sound->pcmSize() && soundtrack.source.queuedBuffers() == 0) return 1.0f;
  
  //const size_t byte = soundtrack.source.byteOffsetInt();
  const size_t sample = soundtrack.source.sampleOffsetInt();
//   const uint32_t numProcessed = soundtrack.source.processedBuffers();
  const size_t bufferSamples1 = bytesToPCMFrames(soundtrack.buffers.buffers[0].size(), soundtrack.sound);
  const size_t bufferSamples2 = bytesToPCMFrames(soundtrack.buffers.buffers[1].size(), soundtrack.sound);
  
//   const size_t finalByte = byte > bufferSize1 ? byte - bufferSize1 : byte;
  
  const size_t diff = bufferSamples2+bufferSamples1 - sample;
  
  const size_t currentSoundSample = loaded - diff;
  
  return float(currentSoundSample) / float(soundtrack.sound->pcmSize());
}

void SoundSystem::setBackgroundSoundPosition(const float &pos) {
  ASSERT(pos >= 0.0f);
  
  if (pos >= 1.0f) {
    soundtrack.loadedSize = soundtrack.sound->pcmSize();
    
    if (soundtrack.source.state() == Source::State::playing) {
      soundtrack.source.stop();
      soundtrack.source.buffer(0);
    }
    
    return;
  }
  
  const size_t pcmPos = size_t(float(soundtrack.sound->pcmSize()) * pos);
  soundtrack.loadedSize = pcmPos;
  
  bool playing = false;
  if (soundtrack.source.state() == Source::State::playing) {
    playing = true;
    soundtrack.source.stop();
    soundtrack.source.buffer(0);
  }
  
  const size_t readSize = soundtrack.sound->loadNext(soundtrack.loadedSize, soundtrack.buffers.buffers[0]);
  soundtrack.loadedSize += readSize;
  
  bool secondBufferHasData = false;
  if (soundtrack.loadedSize < soundtrack.sound->pcmSize()) {
    const size_t readSize = soundtrack.sound->loadNext(soundtrack.loadedSize, soundtrack.buffers.buffers[1]);
    soundtrack.loadedSize += readSize;
    secondBufferHasData = true;
  }
  
  soundtrack.source.queueBuffers(secondBufferHasData ? 2 : 1, soundtrack.buffers.buffers);
  
  if (playing) soundtrack.source.play();
}

QueueSoundData* SoundSystem::queueSound(const ResourceID &id) {
  auto itr = datas.find(id);
  if (itr == datas.end()) throw std::runtime_error("Sound with name " + id.name() + " is not exist");
  
  QueueSoundData* ptr = queueDataPool.newElement(id);
  ptr->data = itr->second;
  soundQueue.push_back(ptr);
  
  return ptr;
}

void SoundSystem::unqueueSound(QueueSoundData* ptr) {
  for (size_t i = 0; i < soundQueue.size(); ++i) {
    if (soundQueue[i] == ptr) {
      std::swap(soundQueue[i], soundQueue.back());
      soundQueue.pop_back();
      queueDataPool.deleteElement(ptr);
      break;
    }
  }
}

void SoundSystem::loadSound(const ResourceID &id, const SoundLoadingData &data) {
  auto itr = datas.find(id);
  if (itr != datas.end()) throw std::runtime_error("Sound with name " + id.name() + " is already exist");
  
  const SoundData::CreateInfo info{
    id,
    data.path,
    data.type,
    data.cache,
    data.lazyPreload,
//     data.mono,
    data.forceMono,
    data.clearAfterUse,
    data.looped,
//     data.severalMonoChannels
  };
  SoundData* sound = soundsPool.newElement(info);
  
  datas[id] = sound;
  
  // теперь я не смогу удалить один звук и как нибудь на его место присобачить другой
  // в конце уровня придется удалить все
  if (!sound->isLazyPreloaded()) {
//     uint32_t ids[2];
//     alGenBuffers(2, ids);
//     openalError("Could not create sound buffers");
//     
//     Buffers buffers = Buffers(ids[0], ids[1]);
//     cachedBuffers[id] = buffers;
//     
//     sound->loadNext(buffers.buffers[0]);
//     sound->loadNext(buffers.buffers[1]);
    
    ContainerData data;
    data.offset = container.size();
    
    const size_t blockSize = pcmFramesToBytes(sound->sampleRate() * SOUND_LOADING_COEFFICIENT, sound);
    
    container.resize(container.size() + blockSize);
    size_t readedSize = sound->loadNext(0, &container[data.offset]);
    data.size1 = pcmFramesToBytes(readedSize, sound);
    
    if (readedSize != sound->pcmSize()) {
      container.resize(container.size() + blockSize);
      readedSize = sound->loadNext(readedSize, &container[data.offset+data.size1]);
      data.size2 = pcmFramesToBytes(readedSize, sound);
    } else {
      data.size2 = 0;
    }
    
    if (container.size() > data.offset + data.size1 + data.size2) {
      container.resize(data.offset + data.size1 + data.size2);
    }
    
    cachedBuffers[id] = data;
  }
}

void SoundSystem::unloadSound(const ResourceID &id) {
  auto itr = datas.find(id);
  if (itr == datas.end()) throw std::runtime_error("Sound with name " + id.name() + " is not exist");
  
  if (!itr->second->isLazyPreloaded()) {
    auto bItr = cachedBuffers.find(id);
    //alDeleteBuffers(2, reinterpret_cast<uint32_t*>(bItr->second.buffers));
    //openalError("Could not delete sound buffers");
    
    // теперь у нас здесь нет буферов и мы теряем память в контейнере
    
    cachedBuffers.erase(bItr);
  }
  
  soundsPool.deleteElement(itr->second);
  datas.erase(itr);
}

SoundData* SoundSystem::getSound(const ResourceID &id) const {
  auto itr = datas.find(id);
  if (itr == datas.end()) return nullptr;
  
  return itr->second;
}

void SoundSystem::addComponent(SoundComponent* ptr) {
  components.push_back(ptr);
}

void SoundSystem::removeComponent(SoundComponent* ptr) {
  for (size_t i = 0; i < components.size(); ++i) {
    if (components[i] == ptr) {
      std::swap(components.back(), components[i]);
      components.pop_back();
      break;
    }
  }
}

size_t SoundSystem::sourcesCount() const {
  return sourcesCountVar;
}

// тут было бы неплохо пересчитать
size_t SoundSystem::activeSourcesCount() const {
  return std::min(soundQueue.size(), sourcesCountVar);
}
