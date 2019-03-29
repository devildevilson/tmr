#ifndef SOUND_SYSTEM_H
#define SOUND_SYSTEM_H

#include "Engine.h"
#include "Utility.h"

#include <vector>
#include <string>
#include <map>
#include <unordered_map>

class SoundID {
public:
  static SoundID get(const std::string &name);
  static bool has(const std::string &name);
  
  SoundID(const std::string &name);
  
  size_t id() const;
  
  bool operator== (const SoundID &other) const;
private:
  size_t soundId;
  
  static std::unordered_map<std::string, size_t> idx;
};

namespace std {
  template<>
  struct hash<SoundID> {
    size_t operator() (const SoundID &soundId) const {
      return hash<size_t>()(soundId.id());
    }
  };
}

// я чет не могу понять как это все у меня будет выглядеть в итоге
// то есть мне нужно будет связать сорс с непосредственно данными
// а для этого мне эти данные нужно заполнить
// и если с хешированными данными все более менее понятно
// то со звуками которые сложно хешировать что делать?
// а, все просто, вполне нормально будет использовать одинаковый интерфейс для всех ресурсов
// вопрос возник с тем, как хранить многоканальные данные
// проще их всех хранить в multimap'е

class Source;
class Buffer;
class BufferAllocator;

class ALCdevice;
class ALCcontext;

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

class SoundChannelData;

// имя?
class SoundData {
public:
  struct CreateInfo {
    SoundID id;
    // также информация о том как найти этот файл (в будущем скорее всего будет неким хендлом)
    // + метаинформация (размер например), может быть даже метаинфа самого аудиофайла
    std::string path;
    
    size_t pcmSize; // скорее всего точно пригодится
    
    uint32_t channelCount;
    SoundChannelData* channel; // первый канал
  };
  
  SoundData(const CreateInfo &info);
  ~SoundData();
  
  void update(); // время? скорее всего здесь не нужно, так как решение о перезагрузке буферов мы делаем исходя из того что уже обработано
  
  size_t id() const;
  std::string path() const;
  size_t pcmSize() const;
  size_t loadedPcmSize() const;
  uint32_t channelCount() const;
private:
  SoundID soundId;
  
  std::string pathStr;
  
  size_t pcmSizeVar;
  size_t loadedPcmSizeVar; // для того чтобы верно подгрузить следующий кусок
  // нужна ли переменная для того чтобы определить сколько подружать каждый раз?
  
  // чаще всего у меня будет либо 1 канал либо 2 (в крайне редких случаях их будет 3 и более)
  // вектор мне бы использовать не хотелось (много лишней фигни)
  // нужно разместить все каналы в одном большом буфере и здесь хранить только указатель на первый и количество
  uint32_t channelCountVar;
  SoundChannelData* channel;
};

class SoundChannelData {
public:
  enum Type {
    DATA_CACHED            = (1<<0),
    DATA_MONO              = (1<<1),
    DATA_FORCE_MONO        = (1<<2),
    DATA_LAZY_PRELOADED    = (1<<3),
    DATA_CLEARED_AFTER_USE = (1<<4),
    DATA_LOOPED            = (1<<5),
  };
  
  struct Settings {
    bool cache;
    bool lazyPreload; // nothing atually loaded from start
    bool forceMono;
    bool clearAfterUse;
    bool looped;
    uint32_t channelIndex;
  };
  SoundChannelData(SoundData* relatedSound, const Settings &settings);
  ~SoundChannelData();
  
  void loadSoundData(); // loads full data if cached, otherwise only several buffers (usable only with DATA_LAZY_PRELOADED)
  void clearSoundData(); // deletes data from buffers
  bool isSoundDataPrepared();
  
  void attachToSource(Source* source);
  void deattach();
  bool isAttached() const;
  
//   std::string name() const;
//   std::string path() const;
  SoundData* sound() const;
  size_t offset() const;
  size_t count() const;
  
  bool isCached() const;
  bool isMono() const;
  bool isForcedMono() const;
  bool isClearedAfterUse() const;
  bool isLooped() const;
private:
//   std::string nameStr;
  // переменной с путем у нас здесь наверное не будет
  // вместо этого указатель на абстрактный хендл, либо ничего
//   std::string pathStr;
  
  SoundData* relatedSound;
  
  uint32_t channelIndex;
  uint32_t type;
  
  // у всех звуков должны быть приоритеты по которым мы принимаем решение проигрывать их сейчас или может быть в какой то другой момент
  // скорее не у каждого звука должен быть приоритет, а он должен вычисляться на основе расстояния
  // может ли возникнуть ситуация, когда важный звук не может "вернуться" из-за того что дохрена других?
  // во-первых вряд ли, если у звука маленький приоритет, то он нахрен не нужен, если у звука нормальный приоритет, то он вернется в очередь
  
  // для буфферов
  size_t offsetVar;
  size_t countVar;
  
  Source* source;
  BufferAllocator* allocator;
};

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
  
  struct CreateInfo {
    float pitch;
    float gain;
    float maxDist;
    float rolloff;
    float refDist;
    float minGain;
    float maxGain;
    bool looping;
    bool relative;
  };
  
  Source();
  Source(const CreateInfo &info);
  Source(Source &source);
  ~Source();
  
  void play();
  void pause();
  void stop();
  void rewind();
  // буду ли я хранить указатели на буферы в сорсе?
  void queueBuffers(const int32_t &size, uint32_t* buffers);
  void unqueueBuffers(const int32_t &size, uint32_t* buffers);
  
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
  
//   // в спеках чет нигде это не указано
//   void setSecOffset(const float &data);
//   void setSampleOffset(const float &data);
//   void setByteOffset(const float &data);
  
  void buffer(const uint32_t &data);
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
  
  glm::vec3 getPos() const;
  glm::vec3 getVel() const;
  glm::vec3 getDir() const;
  
  uint32_t getBuffer() const;
  
  int32_t queueBuffers() const;
  int32_t processedBuffers() const;
  
  Type type() const;
  State state() const;
  
  bool isRelative() const;
  bool isLooping() const;
  
//   void setSecOffset(const float &data);
//   void setSampleOffset(const float &data);
//   void setByteOffset(const float &data);
  
  uint32_t id() const;
  
  Source & operator= (Source &other);
private:
  uint32_t alId;
  
  // указатели на буферы (нужны ли?)
  // указатель на то что сейчас играет
};

class Buffer {
public:
  static void setBufferDataStatic(const bool &data);
  static bool isBufferDataStaticPresent();
  
  Buffer();
  Buffer(Buffer &other);
  ~Buffer();
  
  void bufferData(const int32_t &format, void* data, const int32_t &size, const int32_t &freq);
  void bufferDataStatic(const int32_t &format, void* data, const int32_t &size, const int32_t &freq);
  
  int32_t frequency() const;
  int32_t bits() const;
  int32_t channels() const;
  int32_t size() const;
  
  uint32_t id() const;
  
  Buffer & operator= (Buffer &other);
private:
  uint32_t alId;
  
  // судя по спекам данные в буфер КОПИРУЮТСЯ
  // с alBufferDataStatic данные не копируются, а значит нужно использовать свое хранилище
  static bool bufferDataStaticEnabled;
  // походу нет у меня этого расширения
};

struct SoundOutput {
  Source* source;
  SoundChannelData* data;
  
  float priority; // вычисляется исходя из дальности до звука
};

struct ListenerData {
  glm::vec3 pos;
  glm::vec3 velocity;
  glm::vec3 forward;
  glm::vec3 up;
};

// звук удобнее всего будет доделать после того как у меня появятся зачатки игровой логики

// тут нужно бы мультитрединг запилить
class SoundSystem : public Engine {
public:
  SoundSystem();
  ~SoundSystem();
  
  void updateListener(const ListenerData &data);
  
  void update(const uint64_t &time) override;
  
  // тут еще будут методы для того чтобы поставить звук в очередь, для этого нам потребуется 
  // позиция, скорость, направление, максимальная дистанция и прочее
  // и нам еще потребуется обновить местоположение звука, как это сделать?
  // передавать какой-нибудь уникальный индентификатор именно звука в очереди???
  // это несложно по идее
  
  // методы для загрузки и удаления звуков
private:
  ALCdevice* device;
  ALCcontext* ctx;
  
  // суть в том что у меня будет ограниченное количество источников
  //std::vector<Source*> sources;
  std::vector<SoundOutput> sources;
//   std::vector<SoundOutput*> currentSounds;
  
  // тут наверное нужно использовать хеш названия
//   std::multimap<std::string, SoundChannelData*> soundDatas;
  std::unordered_map<SoundID, SoundData*> datas;
};

#endif
