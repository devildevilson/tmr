#ifndef SOUND_SYSTEM_H
#define SOUND_SYSTEM_H

#include "Engine.h"
#include "Utility.h"
#include "MemoryPool.h"
#include "EntityComponentSystem.h"
#include "ThreadPool.h"

#include <vector>
#include <string>
#include <map>
#include <unordered_map>

#include "Source.h"
#include "Buffer.h"

#include "ResourceID.h"
#include "SoundData.h"

// я чет не могу понять как это все у меня будет выглядеть в итоге
// то есть мне нужно будет связать сорс с непосредственно данными
// а для этого мне эти данные нужно заполнить
// и если с хешированными данными все более менее понятно
// то со звуками которые сложно хешировать что делать?
// а, все просто, вполне нормально будет использовать одинаковый интерфейс для всех ресурсов
// вопрос возник с тем, как хранить многоканальные данные
// проще их всех хранить в multimap'е

// class Source;
// class Buffer;
//class BufferAllocator;

struct ALCdevice_struct;
typedef struct ALCdevice_struct ALCdevice;
struct ALCcontext_struct;
typedef struct ALCcontext_struct ALCcontext;

// а как загружать данные из зип архива
// придется же его постоянно держать включенным для того чтобы грузить по кускам
// также меня волнует ограничение на количество хендлов открытых одновременно в системе
// зип архив придется дергать каждый раз когда нам нужно подгрузить еще немног музыки
// в принципе ничего страшного, кроме того что мне нужно пилить некий класс который я могу использовать как контейнер для указателей на зип архив

// в общем основная проблема как обработать два (и более) моно канала вместо одного
// эти каналы, понятное дело должны взять два (и более) источников
// что делать если место одно? выкидывать по мелкому приоритету, если приоритета не достаточно то не проигрывать

// как загружать музыку для двух каналов так чтобы не хранить лишние данные в памяти?
// абстракция над SoundChannelData, там нам чисто потребуется загрузить новые данные, да и все

// для того чтобы заполнить channelCount и channel мне нужно сначала подгрузить всю метаинформацию
// то есть скорее всего я могу это сделать и до непосредственного создания SoundData, надо добавить в конструктор

// информация о том где и как проигрывается звук будет передаваться видимо из саунд компонента

// также нужно продумать момент с тем, когда несколько звуков проигрываются одновременно
// тут вообще не очень сложно, добавляем в пул, потом проверяем есть ли такой звук уже
// если есть то проигрываем его чуть громче (+ может быть пересчитываем положение)
// что делать если звуки проигрываются не совсем одновременно, но очень близко к этому?
// если звук проигрывается прямо сейчас + есть еще несколько в пуле, то те что в пуле нужно сменить на другой (например на несколько падающих вещей)
// с этим все же могут быть проблемы + ситуация подобная этой в чистом виде редка
// у нас также будет каналов 15 - 30, я думаю этого должно хватить за глаза 
// звуки сами по себе будут очень мелкими, секунда две примерно

// я не очень понимаю что делать со звуками которые выбиваются по приоритету, ну то есть паузить их? (не выйдет)
// думаю что их просто надо выкидывать и резетить

// class SoundChannelData;
// class SoundData;

class SoundLoader;

struct ListenerData {
  simd::vec4 pos;
  simd::vec4 velocity;
  simd::vec4 forward;
  simd::vec4 up;
};

struct Buffers {
  Buffers() : buffers{Buffer(UINT32_MAX), Buffer(UINT32_MAX)} {}
//  Buffers(const Buffers &buffers) : buffers{buffers.buffers[0], buffers.buffers[1]} {}
  Buffers(const Buffer &b1, const Buffer &b2) : buffers{b1, b2} {}
  ~Buffers() {}
  
//  Buffers & operator=(const Buffers &buffers) {
//    this->buffers[0] = buffers.buffers[0];
//    this->buffers[1] = buffers.buffers[1];
//    return *this;
//  }
  
  Buffer buffers[2];
};

struct SoundOutput {
  ResourceID id;
  const SoundData* sound;
  
  Source source;
  Buffers buffers;
  float gain;
  size_t loadedSize;
};

struct QueueSoundType {
  uint32_t container;
  
  QueueSoundType();
  QueueSoundType(const bool relative, const bool deleteAfterPlayed, const bool playAnyway, const bool looping);
  
  void makeType(const bool relative, const bool deleteAfterPlayed, const bool playAnyway, const bool looping);
  
  bool isRelative() const;
  bool deleteAfterPlayed() const;
  bool playAnyway() const; // это в основном нужно чтобы проиграть хотя бы раз (супер редко)
  bool looping() const;
  // запустить звук с какого нибудь определенного места
  // для этого скорее всего достаточно будет одного байта
  
  void setRelative(const bool enable);
  void setDeleteAfterPlayed(const bool enable);
  void setPlayAnyway(const bool enable);
  void setLooping(const bool enable);
};

struct QueueSoundData {
  ResourceID id;
  
  size_t loadedSize;
//   size_t time;
  
  float pitch;
  float gain;
  float maxDist;
  float maxGain;
  float rolloff;
  float refDist;
  
  float pos[4];
  float vel[4];
  float dir[4];
  
  QueueSoundType type;
  
  const SoundData* data;
  
  float priority;
  Source source;
  Buffers buffers;
  
  QueueSoundData();
  QueueSoundData(const ResourceID &id);
  
  ~QueueSoundData();
  
  void updateSource();
  void queueBuffers();
  float playingPosition() const;
  
  // может быть пригодится еще тип звука
  // приоритет посчитаем на основе позиции
  // так же нужно присылать звуки от игрока, то есть относительно слушателя
  // передать примерное начало звука
};

// звук удобнее всего будет доделать после того как у меня появятся зачатки игровой логики
struct SoundLoadingData {
  std::string path;
  
  // настройки
  SupportedSoundType type; // по идее мы это легко определим из пути
  bool cache;
  bool lazyPreload; // грузим звук только в тот момент когда мы пытаемся его проиграть
  bool forceMono;
  bool clearAfterUse; // не используется
  bool looped;        // не используется
//   uint32_t forceSoundChannel; // грузим звук только из конкретного канала
};

class SoundComponent;

// тут нужно бы мультитрединг запилить
class SoundSystem : public Engine, public yacs::system {
public:
  static void updateListener(const ListenerData &data);

  struct CreateInfo {
    dt::thread_pool* pool;
    SoundLoader* loader;
    // добавятся методы загрузки звуков
    // да и в общем изменится подход ко звукам
  };
  SoundSystem(const CreateInfo &info);
  ~SoundSystem();
  
  // нужно еще сделать выбор разных устройств вывода звука
  
  // да и вообще у каждого engine наверное нужно сделать какой-нибудь релоад
  
  void update(const uint64_t &time) override;
  
  // нужно еще узнать сколько звук проиграл
  // playedSize / pcmSize, как то так. как узнать playedSize?
  void playBackgroundSound(const ResourceID &id = ResourceID());
  void pauseBackgroundSound();
  void stopBackgroundSound(); // стоп = возвращаем к 0 секунде
  Source::State getBackgroundSoundState() const;
  size_t getBackgroundSoundDuration() const; // микросекунды
  float getBackgroundSoundPosition() const;
  void setBackgroundSoundPosition(const float &pos);

  QueueSoundData* queueSound(const ResourceID &id);
  void unqueueSound(QueueSoundData* ptr);
  
  // методы для загрузки и удаления звуков
  // теперь если звук не isLazyPreloaded, то мне сложно его удалить
  //void loadSound(const ResourceID &id, const SoundLoadingData &data);
//  void loadSound(const ResourceID &id, const LoadedSoundData &data);
//  void unloadSound(const ResourceID &id);
//  const SoundData* getSound(const ResourceID &id) const;
  
//  void addComponent(SoundComponent* ptr);
//  void removeComponent(SoundComponent* ptr);
  
  size_t sourcesCount() const;
  size_t activeSourcesCount() const;
private:
//  struct ContainerData {
//    size_t offset;
//    size_t size1; // это по идее в байтах должно быть
//    size_t size2;
//  };

  dt::thread_pool* pool;
  
  ALCdevice* device;
  ALCcontext* ctx;
  
  size_t sourcesCountVar;
  
  // один стерео канал для фоновых звуков, нужно сделать панель управления ими
  // скорее всего нужно явно указать какой звук саундтрек а какой нет
  SoundOutput soundtrack;
  
  std::vector<QueueSoundData*> soundQueue;
//  std::vector<SoundComponent*> components;

  std::mutex mutex;
  std::vector<Source> freeSources;
  std::vector<Buffers> sourceBuffers;
  
//  std::vector<char> container; // данные подгруженных заранее звуков
  SoundLoader* loader;

  // тут наверное нужно использовать хеш названия
//  std::unordered_map<ResourceID, SoundData*> datas;
//  std::unordered_map<ResourceID, ContainerData> cachedBuffers;
//  MemoryPool<SoundData, sizeof(SoundData)*50> soundsPool;
  MemoryPool<QueueSoundData, sizeof(QueueSoundData)*50> queueDataPool;

  void freeSource(QueueSoundData* ptr);
  void setSource(QueueSoundData* ptr);
};

#endif
