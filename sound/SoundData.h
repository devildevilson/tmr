#ifndef SOUND_DATA_H
#define SOUND_DATA_H

#include "ResourceID.h"
#include "Buffer.h"
#include "Source.h"
#include "MemoryPool.h"
//#include "SoundLoader.h"

#include <cstdint>
#include <vector>
#include <string>

#define SOUND_LOADING_COEFFICIENT 5.0f

enum SupportedSoundType {
  SOUND_TYPE_FLAC,
  SOUND_TYPE_MP3,
  SOUND_TYPE_WAV,
  SOUND_TYPE_OGG,
  SOUND_TYPE_PCM,
  SOUND_TYPE_COUNT
};

struct ChannelData {
  uint32_t container;

  ChannelData();
  ChannelData(const uint16_t &channels, const bool forcedMono);

  void make(const uint16_t &channels, const bool forcedMono);

  uint16_t count() const;
  bool forcedMono() const;
};

// наверное потом добавлю имя и где находится звук
// данные добавятся еще скорее всего
struct LoadedSoundData {
  SupportedSoundType type;

  int32_t alFormat;
  ChannelData channels;
  uint32_t sampleRate;
  uint32_t bitsPerSample;

  char* data;
  size_t size; // размер данных в data, то есть сайзОф(формат) * сэмплРейт * чаннелКаунт
  size_t pcmSize; // может быть равен size
};

// можно конечно использовать ifstream, но зачем?
// придет время когда мне может потребоваться стриминг ресурсов
class SoundData {
public:
  struct CreateInfo {
    ResourceID id;

    LoadedSoundData soundData;
  };
  SoundData(const CreateInfo &info);
  ~SoundData();
  
  // тут наверное лучше еще указать сколько нужно загрузить
  size_t load(const size_t &loadedSize, const size_t &size, Buffer &buffer) const;
  size_t load(const size_t &loadedSize, const size_t &size, char* buffer) const;
  
  bool isCached() const;
  bool isMono() const;
  bool isForcedMono() const;
  
  ResourceID id() const;
//  std::string path() const;
  size_t pcmSize() const;
  uint32_t sampleRate() const;
  uint32_t bitsPerSample() const;
  uint16_t channelCount() const;
private:
  ResourceID soundId;

  LoadedSoundData soundData;

  // декодер нужно использовать в мультипотоке, это значит что в нем не должно быть изменяемого стейта
  // для этого придется создавать уникальный стейт
//  SoundDecoderInterface* decoder;
};

size_t pcmFramesToBytes(const size_t &pcmFrames, const SoundData* sound);
size_t bytesToPCMFrames(const size_t &bytes, const SoundData* sound);
size_t secondToPCMSize(const size_t &seconds, const SoundData* sound);

#endif
