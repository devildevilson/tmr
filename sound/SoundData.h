#ifndef SOUND_DATA_H
#define SOUND_DATA_H

#include "ResourceID.h"
#include "Buffer.h"
#include "Source.h"
#include "MemoryPool.h"

#include <cstdint>
#include <vector>
#include <string>

#define SOUND_LOADING_COEFFICIENT 5.0f

class SoundData;

enum SupportedSoundType {
  SOUND_TYPE_FLAC,
  SOUND_TYPE_MP3,
  SOUND_TYPE_WAV,
  SOUND_TYPE_OGG,
  SOUND_TYPE_COUNT
};

// короче никаких 3-ех буферов мне не нужно
// достаточно двух, просто отработанный ставим в конец
// alSourceUnqueueBuffers ВОЗВРАЩАЕТ отцепленные буферы

// в будущем я буду использовать zip архивы для хранения данных игры
// что мне нужно сделать чтобы мне было удобно пользоваться ими для загрузки
// звуков? как вообще это должно выглядеть? несколько открытых файлов на протяжении работы игры?
// в варианте по умолчанию это еще более менее, так как будет открыт всего 1 дескриптор
// а когда добавятся моды что делать? я не могу сформулировать даже проблему доконца

// в общем скорее всего будет примерно так, для каждого файла который мне нужно загрузить
// я могу создать fstream, я должен передать fstream позицию меня интересующую и размер
// затем загрузить из памяти по позиции и размеру ресурсы
// для этого мне нужен какой-то контейнер из которого я смогу поличить размер файла и позицию начала
// по всей видимости в ресурсах должны хранится именно эти данные
class SoundData {
public:
  enum Type {
    DATA_CACHED            = (1<<0),
    DATA_MONO              = (1<<1),
    DATA_FORCE_MONO        = (1<<2),
    DATA_LAZY_PRELOADED    = (1<<3),
    DATA_CLEARED_AFTER_USE = (1<<4),
    DATA_LOOPED            = (1<<5),
  };
  
  struct CreateInfo {
    ResourceID id;
    // также информация о том как найти этот файл (в будущем скорее всего будет неким хендлом)
    // + метаинформация (размер например), может быть даже метаинфа самого аудиофайла
    std::string path;
    
//     size_t pcmSize; // скорее всего точно пригодится
    
//     uint32_t channelCount;
    
    SupportedSoundType type;
    bool cache;
    bool lazyPreload; // nothing atually loaded from start
//     bool mono;
    bool forceMono; // по идее это означает что это может быть использовано как 3д звук (помоему это так называется)
    bool clearAfterUse;
    bool looped;
//     bool severalMonoChannels;
  };
  
  SoundData(const CreateInfo &info);
  ~SoundData();
  
  // тут наверное лучше еще указать сколько нужно загрузить
  size_t loadNext(const size_t &loadedSize, Buffer &buffer);
  size_t loadNext(const size_t &loadedSize, char* buffer);
  
  bool isCached() const;
  bool isMono() const;
  bool isForcedMono() const;
  bool isLazyPreloaded() const;
  bool isClearedAfterUse() const;
  bool isLooped() const;
  
  ResourceID id() const;
  std::string path() const;
  size_t pcmSize() const;
  uint32_t sampleRate() const;
  uint32_t bitsPerSample() const;
  uint32_t channelCount() const;
private:
  ResourceID soundId;
  
  std::string pathStr;
  
  uint32_t type;
  SupportedSoundType dataType;

  uint32_t channelCountVar;
//   uint32_t bufferProcessedIndex;
//   uint32_t bufferCountPerChannelVar;
  uint32_t sampleRateVar;
  uint32_t bitsPerSampleVar;
  float soundLoadinfCoefficient;
  
  // одна секунда звука вычисляется как sampleRate * channels * (bitsPerSample / 8)
  // как получить длительность? как получить проигранную длительность?
  // проигранная длительность получается из AL_BYTE_OFFSET, AL_SAMPLE_OFFSET, AL_SEC_OFFSET
  // но это по всей видимости длительность одного буфера, поэтому нужно еще + проигранные буферы + проигранный размер файла
  size_t pcmSizeVar;
  //size_t loadedPcmSizeVar; // для того чтобы верно подгрузить следующий кусок
  // нужна ли переменная для того чтобы определить сколько подгружать каждый раз?
  
//   std::vector<Buffer> buffers;
//   std::vector<Source> sources;
  
  //std::vector<SoundChannel> channels;
  
  // чаще всего у меня будет либо 1 канал либо 2 (в крайне редких случаях их будет 3 и более)
  // вектор мне бы использовать не хотелось (много лишней фигни)
  // нужно разместить все каналы в одном большом буфере и здесь хранить только указатель на первый и количество
//   SoundChannelData* channel;
  
//   static MemoryPool<SoundChannelData, sizeof(SoundChannelData)*100> channelsPool;
};

int32_t to_al_format(const uint32_t &channels, const uint32_t &bitsPerChannel);
size_t pcmFramesToBytes(const size_t &pcmFrames, const SoundData* sound);
size_t bytesToPCMFrames(const size_t &bytes, const SoundData* sound);

#endif
