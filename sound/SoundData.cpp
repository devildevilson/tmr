#include "SoundData.h"

#define DR_MP3_IMPLEMENTATION
#include "dr_mp3.h"

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

#define DR_FLAC_IMPLEMENTATION
#include "dr_flac.h"

// #include <vorbis/codec.h>
// #include <vorbis/vorbisfile.h>

#include "stb_vorbis.c"

#include "AL/al.h"
#include "AL/alc.h"
#include "AL/alext.h"

#include "Globals.h"

#include "SoundSystem.h"

#ifdef _DEBUG
  #include <cassert>
  #define ASSERT(expr) assert(expr)
#else
  #define ASSERT(expr)
#endif

size_t drRead(void* pUserData, void* pBufferOut, size_t bytesToRead) {
  std::ifstream* stream = reinterpret_cast<std::ifstream*>(pUserData);
  stream->read(reinterpret_cast<char*>(pBufferOut), bytesToRead);
  return stream->gcount();
}

drflac_bool32 drSeek(void* pUserData, int offset, drflac_seek_origin origin) {
  std::ifstream* stream = reinterpret_cast<std::ifstream*>(pUserData);
  std::ios_base::seekdir dir;
  stream->clear();
  
  switch (origin) {
    case drflac_seek_origin_start: dir = std::ios::beg; break;
    case drflac_seek_origin_current: dir = std::ios::cur; break;
    default: return false;
  }
  
  stream->seekg(std::streamoff(offset), dir);
  return !stream->fail();
}

drflac_bool32 drSeek(void* pUserData, int offset, drmp3_seek_origin origin) {
  std::ifstream* stream = reinterpret_cast<std::ifstream*>(pUserData);
  std::ios_base::seekdir dir;
  stream->clear();
  
  switch (origin) {
    case drmp3_seek_origin_start: dir = std::ios::beg; break;
    case drmp3_seek_origin_current: dir = std::ios::cur; break;
    default: return false;
  }
  
  stream->seekg(std::streamoff(offset), dir);
  return !stream->fail();
}

drflac_bool32 drSeek(void* pUserData, int offset, drwav_seek_origin origin) {
  std::ifstream* stream = reinterpret_cast<std::ifstream*>(pUserData);
  std::ios_base::seekdir dir;
  stream->clear();
  
  switch (origin) {
    case drwav_seek_origin_start: dir = std::ios::beg; break;
    case drwav_seek_origin_current: dir = std::ios::cur; break;
    default: return false;
  }
  
  stream->seekg(std::streamoff(offset), dir);
  return !stream->fail();
}

// size_t readOgg(void *ptr, size_t size, size_t nmemb, void *datasource) {
//   std::istream* file = reinterpret_cast<std::istream*>(datasource);
//   file->read(reinterpret_cast<char*>(ptr), size * nmemb);
//   return file->gcount();
// }
// 
// int32_t seekOgg(void *datasource, ogg_int64_t offset, int whence) {
//   std::istream* file = reinterpret_cast<std::istream*>(datasource);
//   std::ios_base::seekdir Dir;
//   file->clear();
//   
//   switch (whence) {
//     case SEEK_SET: Dir = std::ios::beg; break;
//     case SEEK_CUR: Dir = std::ios::cur; break;
//     case SEEK_END: Dir = std::ios::end; break;
//     default: return -1;
//   }
//   
//   file->seekg(std::streamoff(offset), Dir);
//   return (file->fail() ? -1 : 0);
// }
// 
// long tellOgg(void *datasource) {
//   std::istream* file = reinterpret_cast<std::istream*>(datasource);
//   return file->tellg();
// }
// 
// int32_t closeOgg(void *datasource) {
//   (void)datasource;
//   return 0;
// }

//const uint32_t &sampleRate, const uint32_t &bitsPerChannel, const size_t &size
//const std::string &path
class SoundLoaderInterface {
public:
  SoundLoaderInterface(const uint32_t &channels) : 
//     m_path(path), 
    m_channels(channels), 
    m_sampleRate(0), 
    m_bitsPerChannel(0), 
    m_size(0) {}
    
  virtual ~SoundLoaderInterface() {}
  
  virtual bool seek(const size_t &seekSize) = 0;
  virtual size_t getSamples(char* memory, const size_t &sampleCount) = 0;
  
  //std::string path() const { return m_path; }
  uint32_t channels() const { return m_channels; }
  uint32_t sampleRate() const { return m_sampleRate; }
  uint32_t bitsPerChannel() const { return m_bitsPerChannel; }
  size_t size() const { return m_size; }
protected:
  // путь или файл хендлер
  // не нужно ни то ни другое
  //std::string m_path;
  
  uint32_t m_channels;
  uint32_t m_sampleRate;
  uint32_t m_bitsPerChannel;
  size_t m_size;
};

// 0 - default channels count
// возможно придется самому вычислять моно звук
class MP3Loader : public SoundLoaderInterface {
public:
  MP3Loader(const std::string &path, const uint32_t &channels) : SoundLoaderInterface(channels) {
    const drmp3_config config{
      m_channels,
      0
    };
    
    if (!drmp3_init_file(&mp3, path.c_str(), &config)) {
      throw std::runtime_error("Failed to open file " + path);
    }
    
    m_sampleRate = mp3.sampleRate;
    m_bitsPerChannel = 32;
    m_size = drmp3_get_pcm_frame_count(&mp3);
    
    if (m_channels == 0) m_channels = mp3.channels;
  }
  
  MP3Loader(const std::string &path, const uint32_t &channels, const size_t &size) : SoundLoaderInterface(channels) {
    const drmp3_config config{
      m_channels,
      0
    };
    
    if (!drmp3_init_file(&mp3, path.c_str(), &config)) {
      throw std::runtime_error("Failed to open file " + path);
    }
    
    m_sampleRate = mp3.sampleRate;
    m_bitsPerChannel = 32;
    m_size = size;
    
    if (m_channels == 0) m_channels = mp3.channels;
  }
  
  MP3Loader(std::ifstream* stream, const uint32_t &channels, const size_t &size = 0) : SoundLoaderInterface(channels) {
    const drmp3_config config{
      m_channels,
      0
    };
    
    if (!drmp3_init(&mp3, drRead, drSeek, stream, &config)) {
      throw std::runtime_error("Failed to init mp3 file");
    }
    
    m_sampleRate = mp3.sampleRate;
    m_bitsPerChannel = 32;
    m_size = size == 0 ? drmp3_get_pcm_frame_count(&mp3) : size;
    
    if (m_channels == 0) m_channels = mp3.channels;
  }
  
  MP3Loader(const std::vector<uint8_t> &memory, const uint32_t &channels, const size_t &size = 0) : SoundLoaderInterface(channels) {
    const drmp3_config config{
      m_channels,
      0
    };
    
    if (!drmp3_init_memory(&mp3, memory.data(), memory.size(), &config)) {
      throw std::runtime_error("Failed to init mp3 file");
    }
    
    m_sampleRate = mp3.sampleRate;
    m_bitsPerChannel = 32;
    m_size = size == 0 ? drmp3_get_pcm_frame_count(&mp3) : size;
    
    if (m_channels == 0) m_channels = mp3.channels;
  }
  
  ~MP3Loader() {
    drmp3_uninit(&mp3);
  }
  
  bool seek(const size_t &seekSize) override {
    return drmp3_seek_to_pcm_frame(&mp3, seekSize);
  }
  
  size_t getSamples(char* memory, const size_t &sampleCount) override {
    return drmp3_read_pcm_frames_f32(&mp3, sampleCount, reinterpret_cast<float*>(memory));
  }
private:
  drmp3 mp3;
};

class WAVLoader : public SoundLoaderInterface {
public:
  WAVLoader(const std::string &path, const uint32_t &channels) : SoundLoaderInterface(channels) {
    if (!drwav_init_file(&wav, path.c_str())) {
      throw std::runtime_error("Failed to open file " + path);
    }
    
    // по всей видимости vaw файл нужно будет самому приводить к моно если потребуется
    
    m_sampleRate = wav.sampleRate;
    m_bitsPerChannel = wav.bitsPerSample;
    m_size = wav.totalPCMFrameCount;
    
    if (m_channels == 0) m_channels = wav.channels;
  }
  
  WAVLoader(const std::string &path, const uint32_t &channels, const size_t &size) : SoundLoaderInterface(channels) {
    if (!drwav_init_file(&wav, path.c_str())) {
      throw std::runtime_error("Failed to open file " + path);
    }
    
    m_sampleRate = wav.sampleRate;
    m_bitsPerChannel = wav.bitsPerSample;
    m_size = size;
    
    if (m_channels == 0) m_channels = wav.channels;
  }
  
  WAVLoader(std::ifstream* stream, const uint32_t &channels, const size_t &size = 0) : SoundLoaderInterface(channels) {
    if (!drwav_init(&wav, drRead, drSeek, stream)) {
      throw std::runtime_error("Failed to init wav file");
    }
    
    m_sampleRate = wav.sampleRate;
    m_bitsPerChannel = wav.bitsPerSample;
    m_size = size == 0 ? wav.totalPCMFrameCount : size;
    
    if (m_channels == 0) m_channels = wav.channels;
  }
  
  WAVLoader(const std::vector<uint8_t> &memory, const uint32_t &channels, const size_t &size = 0) : SoundLoaderInterface(channels) {
    if (!drwav_init_memory(&wav, memory.data(), memory.size())) {
      throw std::runtime_error("Failed to init wav file");
    }
    
    m_sampleRate = wav.sampleRate;
    m_bitsPerChannel = wav.bitsPerSample;
    m_size = size == 0 ? wav.totalPCMFrameCount : size;
    
    if (m_channels == 0) m_channels = wav.channels;
  }
  
  ~WAVLoader() {
    drwav_uninit(&wav);
  }
  
  bool seek(const size_t &seekSize) override {
    return drwav_seek_to_pcm_frame(&wav, seekSize);
  }
  
  size_t getSamples(char* memory, const size_t &sampleCount) override {
    if (wav.bitsPerSample == 8) {
      uint8_t* finalData = reinterpret_cast<uint8_t*>(memory);
      
      const size_t blockSize = sampleCount * wav.channels;
      uint8_t* blockData = new uint8_t[blockSize];
      
      // читаем один блок
      const size_t readedFramesVar = drwav_read_pcm_frames(&wav, sampleCount, blockData);
      
      if (m_channels == 1 && wav.channels != 1) {
        size_t dataIndex = 0;
        for (size_t i = 0; i < blockSize; ++i) {
          const size_t channelIndex = i % wav.channels;
          dataIndex += size_t(channelIndex == 0 && i > 0);
//           blockData[dataIndex] += channelIndex == 0 ? 0 : blockData[dataIndex+channelIndex];
          blockData[dataIndex] = channelIndex == 0 ? blockData[i] : blockData[dataIndex] + blockData[i];
        }
        
        for (size_t i = 0; i < blockSize / wav.channels; ++i) {
          //if (blockData[i] != 0) blockData[i] /= wav.channels;
          finalData[i] = blockData[i] != 0 ? blockData[i] / wav.channels : 0;
        }
      } else {
        memcpy(finalData, blockData, blockSize * sizeof(uint8_t));
      }
      
      delete [] blockData;
      
      //readedFrames = readedFramesVar;
      return readedFramesVar;
    } else if (wav.bitsPerSample == 16) {
      int16_t* finalData = reinterpret_cast<int16_t*>(memory);
      
      const size_t blockSize = sampleCount * wav.channels;
      int16_t* blockData = new int16_t[blockSize];
      
      // читаем один блок
      const size_t readedFramesVar = drwav_read_pcm_frames(&wav, sampleCount, blockData);
      
      if (m_channels == 1 && wav.channels != 1) {
        size_t dataIndex = 0;
        for (size_t i = 0; i < blockSize; ++i) {
          const size_t channelIndex = i % wav.channels;
          dataIndex += size_t(channelIndex == 0 && i > 0);
          //blockData[dataIndex] += channelIndex == 0 ? 0 : blockData[dataIndex+channelIndex];
          blockData[dataIndex] = channelIndex == 0 ? blockData[i] : blockData[dataIndex] + blockData[i];
        }
        
        for (size_t i = 0; i < blockSize / wav.channels; ++i) {
          finalData[i] = blockData[i] != 0 ? blockData[i] / wav.channels : 0;
        }
      } else {
        memcpy(finalData, blockData, blockSize * sizeof(int16_t));
      }
      
      delete [] blockData;
      
      return readedFramesVar;
    } else if (wav.bitsPerSample == 32) {
      float* finalData = reinterpret_cast<float*>(memory);
      
      const size_t blockSize = sampleCount * wav.channels;
      float* blockData = new float[blockSize];
      
      // читаем один блок
      const size_t readedFramesVar = drwav_read_pcm_frames_f32(&wav, sampleCount, blockData);
      
      if (m_channels == 1 && wav.channels != 1) {
        size_t dataIndex = 0;
        for (size_t i = 0; i < blockSize; ++i) {
          const size_t channelIndex = i % wav.channels;
          dataIndex += size_t(channelIndex == 0 && i > 0);
//           blockData[dataIndex] += channelIndex == 0 ? 0 : blockData[dataIndex+channelIndex];
          blockData[dataIndex] = channelIndex == 0 ? blockData[i] : blockData[dataIndex] + blockData[i];
        }
        
        for (size_t i = 0; i < blockSize / wav.channels; ++i) {
          //if (blockData[i] != 0) blockData[i] /= wav.channels;
          finalData[i] = blockData[i] != 0 ? blockData[i] / wav.channels : 0;
        }
      } else {
        memcpy(finalData, blockData, blockSize * sizeof(float));
      }
      
      delete [] blockData;
      
      return readedFramesVar;
    } else {
      throw std::runtime_error("wav format is not supported");
    }
    
    return SIZE_MAX;
  }
private:
  drwav wav;
};

class FLACLoader : public SoundLoaderInterface {
public:
  FLACLoader(const std::string &path, const uint32_t &channels) : SoundLoaderInterface(channels), flac(nullptr) {
    flac = drflac_open_file(path.c_str());
    if (flac == nullptr) throw std::runtime_error("Failed to open file " + path);
    
    m_sampleRate = flac->sampleRate;
    m_bitsPerChannel = flac->bitsPerSample;
    m_size = flac->totalPCMFrameCount;
    
    if (m_channels == 0) m_channels = flac->channels;
  }
  
  FLACLoader(const std::string &path, const uint32_t &channels, const size_t &size) : SoundLoaderInterface(channels), flac(nullptr) {
    flac = drflac_open_file(path.c_str());
    if (flac == nullptr) throw std::runtime_error("Failed to open file " + path);
    
    m_sampleRate = flac->sampleRate;
    m_bitsPerChannel = flac->bitsPerSample;
    m_size = size;
    
    if (m_channels == 0) m_channels = flac->channels;
  }
  
  FLACLoader(std::ifstream* stream, const uint32_t &channels, const size_t &size) : SoundLoaderInterface(channels) {
    flac = drflac_open(drRead, drSeek, stream);
    if (flac == nullptr) throw std::runtime_error("Failed to init flac file");
    
    m_sampleRate = flac->sampleRate;
    m_bitsPerChannel = flac->bitsPerSample;
    m_size = size;
    
    if (m_channels == 0) m_channels = flac->channels;
  }
  
  FLACLoader(const std::vector<uint8_t> &memory, const uint32_t &channels, const size_t &size) : SoundLoaderInterface(channels) {
    flac = drflac_open_memory(memory.data(), memory.size());
    if (flac == nullptr) throw std::runtime_error("Failed to init flac file");
    
    m_sampleRate = flac->sampleRate;
    m_bitsPerChannel = flac->bitsPerSample;
    m_size = size;
    
    if (m_channels == 0) m_channels = flac->channels;
  }
  
  ~FLACLoader() {
    drflac_close(flac);
  }
  
  bool seek(const size_t &seekSize) override {
    return drflac_seek_to_pcm_frame(flac, seekSize);
  }
  
  size_t getSamples(char* memory, const size_t &sampleCount) override {
    if (m_bitsPerChannel == 16) {
      int16_t* finalData = reinterpret_cast<int16_t*>(memory);
      
      const size_t soundBlock = sampleCount * flac->channels;
      int16_t* blockData = new int16_t[soundBlock];
      
      // у флака числа с плавающей точкой это потеря качества
      // но в опенал я не понял как сделать формат int32
      const size_t readedFrames = drflac_read_pcm_frames_s16(flac, soundBlock, blockData);
      
      if (m_channels == 1 && flac->channels != 1) {
        size_t dataIndex = 0;
        for (size_t i = 0; i < soundBlock; ++i) {
          const size_t channelIndex = i % flac->channels;
          dataIndex += size_t(channelIndex == 0 && i > 0);
//           blockData[dataIndex] += channelIndex == 0 ? 0 : blockData[dataIndex+channelIndex];
          blockData[dataIndex] = channelIndex == 0 ? blockData[i] : blockData[dataIndex] + blockData[i];
        }
        
        for (size_t i = 0; i < soundBlock / flac->channels; ++i) {
          finalData[i] = blockData[i] != 0 ? blockData[i] / flac->channels : 0;
        }
      } else {
        memcpy(finalData, blockData, soundBlock * sizeof(int16_t));
      }
      
      delete [] blockData;
      
      return readedFrames;
    } else if (m_bitsPerChannel == 32) {
      float* finalData = reinterpret_cast<float*>(memory);
      
      const size_t soundBlock = sampleCount * flac->channels;
      float* blockData = new float[soundBlock];
      
      // у флака числа с плавающей точкой это потеря качества
      // но в опенал я не понял как сделать формат int32
      const size_t readedFrames = drflac_read_pcm_frames_f32(flac, soundBlock, blockData);
      
      if (m_channels == 1 && flac->channels != 1) {
        size_t dataIndex = 0;
        for (size_t i = 0; i < soundBlock; ++i) {
          const size_t channelIndex = i % flac->channels;
          dataIndex += size_t(channelIndex == 0 && i > 0);
//           blockData[dataIndex] += channelIndex == 0 ? 0 : blockData[dataIndex+channelIndex];
          blockData[dataIndex] = channelIndex == 0 ? blockData[i] : blockData[dataIndex] + blockData[i];
        }
        
        for (size_t i = 0; i < soundBlock / flac->channels; ++i) {
          finalData[i] = blockData[i] != 0 ? blockData[i] / flac->channels : 0;
        }
      } else {
        memcpy(finalData, blockData, soundBlock * sizeof(float));
      }
      
      delete [] blockData;
      
      return readedFrames;
    } else {
      throw std::runtime_error("flac format is not supported");
    }
    
    return SIZE_MAX;
  }
private:
  drflac* flac;
};

class OGGLoader : public SoundLoaderInterface {
public:
  OGGLoader(const std::string &path, const uint32_t &channels) : SoundLoaderInterface(channels), file(nullptr) {
    int32_t err;
    file = stb_vorbis_open_filename(path.c_str(), &err, nullptr);
    if (file == nullptr) throw std::runtime_error("Failed to open file " + path);
    
    const stb_vorbis_info &info = stb_vorbis_get_info(file);
    
    m_sampleRate = info.sample_rate;
    m_bitsPerChannel = 32;
    m_size = stb_vorbis_stream_length_in_samples(file);
    
    if (m_channels == 0) m_channels = info.channels;
  }
  
  OGGLoader(const std::string &path, const uint32_t &channels, const size_t &size) : SoundLoaderInterface(channels), file(nullptr) {
    int32_t err;
    file = stb_vorbis_open_filename(path.c_str(), &err, nullptr);
    if (file == nullptr) throw std::runtime_error("Failed to open file " + path);
    
    const stb_vorbis_info &info = stb_vorbis_get_info(file);
    
    m_sampleRate = info.sample_rate;
    m_bitsPerChannel = 32;
    m_size = size;
    
    if (m_channels == 0) m_channels = info.channels;
  }
  
  OGGLoader(const std::vector<uint8_t> &memory, const uint32_t &channels, const size_t &size) : SoundLoaderInterface(channels), file(nullptr) {
    int32_t err;
    file = stb_vorbis_open_memory(memory.data(), memory.size(), &err, nullptr);
    if (file == nullptr) throw std::runtime_error("Failed to init ogg file");
    
    const stb_vorbis_info &info = stb_vorbis_get_info(file);
    
    m_sampleRate = info.sample_rate;
    m_bitsPerChannel = 32;
    m_size = size;
    
    if (m_channels == 0) m_channels = info.channels;
  }
  
  ~OGGLoader() {
    stb_vorbis_close(file);
  }
  
  bool seek(const size_t &seekSize) override {
    return stb_vorbis_seek_frame(file, seekSize);
  }
  
  size_t getSamples(char* memory, const size_t &sampleCount) override {
    return stb_vorbis_get_samples_float_interleaved(file, m_channels, reinterpret_cast<float*>(memory), sampleCount);
  }
private:
  stb_vorbis* file;
};

int32_t to_al_format(const uint32_t &channels, const uint32_t &bitsPerChannel) {
  const bool stereo = channels > 1;
  
  switch (bitsPerChannel) {
    case 32:
      if (stereo) return AL_FORMAT_STEREO_FLOAT32;
      else return AL_FORMAT_MONO_FLOAT32;
    case 16:
      if (stereo) return AL_FORMAT_STEREO16;
      else return AL_FORMAT_MONO16;
    case 8:
      if (stereo) return AL_FORMAT_STEREO8;
      else return AL_FORMAT_MONO8;
  }
  
  return -1;
}

SoundData::SoundData(const CreateInfo &info) : 
  soundId(info.id), 
  pathStr(info.path), 
  type(0), 
  dataType(info.type), 
  channelCountVar(0),
  pcmSizeVar(0) {
  if (info.cache) type |= DATA_CACHED;
  if (info.lazyPreload) type |= DATA_LAZY_PRELOADED;
  if (info.forceMono) { type |= DATA_FORCE_MONO; type |= DATA_MONO; }
  if (info.clearAfterUse) type |= DATA_CLEARED_AFTER_USE;
  if (info.looped) type |= DATA_LOOPED;
  
  //std::ifstream file(path(), std::ios::binary);
  //file
  
  // сюда скорее всего будет приходить memory
  // мемори будет формироваться из распакованного зип архива
  // мне нужно будет получать мемори каждый раз как я захочу подгрузить следующий кусочек
  // как сделать так чтобы не проседать по фпс из-за этого?
  
  SoundLoaderInterface* loader = nullptr;
  if (dataType == SOUND_TYPE_MP3) {
    loader = new MP3Loader(pathStr, isForcedMono() ? 1 : 0);
  } else if (dataType == SOUND_TYPE_WAV) {
    loader = new WAVLoader(pathStr, isForcedMono() ? 1 : 0);
  } else if (dataType == SOUND_TYPE_FLAC) {
    loader = new FLACLoader(pathStr, isForcedMono() ? 1 : 0);
  } else if (dataType == SOUND_TYPE_OGG) {
    loader = new OGGLoader(pathStr, isForcedMono() ? 1 : 0);
  } else {
    throw std::runtime_error("sound type is not supported");
  }
  
  if (loader->channels() == 1) type |= DATA_MONO;
  
  channelCountVar = loader->channels();
  sampleRateVar = loader->sampleRate();
  bitsPerSampleVar = loader->bitsPerChannel();
  pcmSizeVar = loader->size();
  
  delete loader;
}

SoundData::~SoundData() {}

size_t SoundData::loadNext(const size_t &loadedSize, Buffer &buffer) {
  SoundLoaderInterface* loader = nullptr;
  if (dataType == SOUND_TYPE_MP3) {
    loader = new MP3Loader(pathStr, isForcedMono() ? 1 : 0, pcmSizeVar);
  } else if (dataType == SOUND_TYPE_WAV) {
    loader = new WAVLoader(pathStr, isForcedMono() ? 1 : 0, pcmSizeVar);
  } else if (dataType == SOUND_TYPE_FLAC) {
    loader = new FLACLoader(pathStr, isForcedMono() ? 1 : 0, pcmSizeVar);
  } else if (dataType == SOUND_TYPE_OGG) {
    loader = new OGGLoader(pathStr, isForcedMono() ? 1 : 0, pcmSizeVar);
  } else {
    throw std::runtime_error("sound type is not supported");
  }
  
  if (!loader->seek(loadedSize)) throw std::runtime_error("seek to pcm frame failed");
  
  const size_t soundBlock = isCached() ? pcmSizeVar : std::min(size_t(loader->sampleRate() * SOUND_LOADING_COEFFICIENT), pcmSizeVar);
  char* memory = new char[soundBlock * loader->channels() * (loader->bitsPerChannel()/8)];
  
  const size_t readedFrames = loader->getSamples(memory, soundBlock);
  
  const int32_t format = to_al_format(loader->channels(), loader->bitsPerChannel());
  buffer.bufferData(format, memory, readedFrames * loader->channels() * (loader->bitsPerChannel()/8), loader->sampleRate());
  
  delete [] memory;
  delete loader;
  
  return readedFrames;
}

size_t SoundData::loadNext(const size_t &loadedSize, char* buffer) {
  SoundLoaderInterface* loader = nullptr;
  if (dataType == SOUND_TYPE_MP3) {
    loader = new MP3Loader(pathStr, isForcedMono() ? 1 : 0, pcmSizeVar);
  } else if (dataType == SOUND_TYPE_WAV) {
    loader = new WAVLoader(pathStr, isForcedMono() ? 1 : 0, pcmSizeVar);
  } else if (dataType == SOUND_TYPE_FLAC) {
    loader = new FLACLoader(pathStr, isForcedMono() ? 1 : 0, pcmSizeVar);
  } else if (dataType == SOUND_TYPE_OGG) {
    loader = new OGGLoader(pathStr, isForcedMono() ? 1 : 0, pcmSizeVar);
  } else {
    throw std::runtime_error("sound type is not supported");
  }
  
  if (!loader->seek(loadedSize)) throw std::runtime_error("seek to pcm frame failed");
  
  const size_t soundBlock = isCached() ? pcmSizeVar : std::min(size_t(loader->sampleRate() * SOUND_LOADING_COEFFICIENT), pcmSizeVar);
  
  const size_t readedFrames = loader->getSamples(buffer, soundBlock);

  delete loader;
  
  return readedFrames;
}

// void SoundData::reset() {
//   loadedPcmSizeVar = 0;
// }

// void SoundData::prepareNextFrame() {
//   
// }

// void SoundData::attachToSource(const std::vector<Source> &sources) {
//   this->sources = sources;
// }
// 
// void SoundData::deattach() {
//   alSourceRewindv(sources.size(), reinterpret_cast<uint32_t*>(sources.data()));
//   for (uint32_t i = 0; i < sources.size(); ++i) {
//     alSourcei(sources[i].id(), AL_BUFFER, 0);
//   }
//   
//   sources.clear();
// }
// 
// bool SoundData::isAttached() const {
//   return !sources.empty();
// }

bool SoundData::isCached() const {
  return (type & DATA_CACHED) == DATA_CACHED;
}

bool SoundData::isMono() const {
  return (type & DATA_MONO) == DATA_MONO;
}

bool SoundData::isForcedMono() const {
  return (type & DATA_FORCE_MONO) == DATA_FORCE_MONO;
}

bool SoundData::isLazyPreloaded() const {
  return (type & DATA_LAZY_PRELOADED) == DATA_LAZY_PRELOADED;
}

bool SoundData::isClearedAfterUse() const {
  return (type & DATA_CLEARED_AFTER_USE) == DATA_CLEARED_AFTER_USE;
}

bool SoundData::isLooped() const {
  return (type & DATA_LOOPED) == DATA_LOOPED;
}

ResourceID SoundData::id() const {
  return soundId;
}

std::string SoundData::path() const {
  return pathStr;
}

size_t SoundData::pcmSize() const {
  return pcmSizeVar;
}

// size_t SoundData::loadedPcmSize() const {
//   return loadedPcmSizeVar;
// }

uint32_t SoundData::sampleRate() const {
  return sampleRateVar;
}

uint32_t SoundData::bitsPerSample() const {
  return bitsPerSampleVar;
}

uint32_t SoundData::channelCount() const {
  return channelCountVar;
}

size_t pcmFramesToBytes(const size_t &pcmFrames, const SoundData* sound) {
  return pcmFrames * sound->channelCount() * (sound->bitsPerSample()/8);
}

size_t bytesToPCMFrames(const size_t &bytes, const SoundData* sound) {
  return (bytes / sound->channelCount()) / (sound->bitsPerSample()/8);
}

// if (dataType == SOUND_TYPE_MP3) {
//     drmp3 mp3;
//     const drmp3_config config{
//       isForcedMono() ? uint32_t(1) : 0,
//       0
//     };
//     
//     if (!drmp3_init_file(&mp3, pathStr.c_str(), &config)) {
//       throw std::runtime_error("Failed to open file " + pathStr);
//     }
//     
//     const bool seek = drmp3_seek_to_pcm_frame(&mp3, loadedSize);
//     if (!seek) {
//       throw std::runtime_error("seek to pcm frame failed");
//     }
//     
//     const size_t soundBlock = isCached() ? pcmSizeVar : std::min(size_t(mp3.sampleRate * SOUND_LOADING_COEFFICIENT), pcmSizeVar); //  * sizeof(float) //  * mp3.channels
//     float* blockData = new float[soundBlock * mp3.channels];
//     
// //     std::cout << "channels " << mp3.channels << '\n';
//     
//     // читаем один блок
//     const size_t readedFrames = drmp3_read_pcm_frames_f32(&mp3, soundBlock, blockData);
//     
//     ASSERT(isForcedMono() == (mp3.channels == 1));
//     
//     // если у нас форседМоно, то по идее у нас обязантельно приходит один канал
//     const int32_t format = to_al_format(mp3.channels, 32);
//     
//     buffer.bufferData(format, blockData, readedFrames * mp3.channels * sizeof(float), mp3.sampleRate);
//     
//     drmp3_uninit(&mp3);
//     
//     delete [] blockData;
//     
//     //loadedPcmSizeVar += readedFrames;
//     return readedFrames;
//   }
//   
//   if (dataType == SOUND_TYPE_WAV) {
//     drwav wav;
//     if (!drwav_init_file(&wav, pathStr.c_str())) {
//       throw std::runtime_error("Failed to open file " + pathStr);
//     }
//     
//     // по всей видимости vaw файл нужно будет самому приводить к моно если потребуется
//     
//     const bool seek = drwav_seek_to_pcm_frame(&wav, loadedSize);
//     if (!seek) {
//       throw std::runtime_error("seek to pcm frame failed");
//     }
//     
//     const size_t soundBlock = isCached() ? pcmSizeVar : std::min(size_t(wav.sampleRate * SOUND_LOADING_COEFFICIENT), pcmSizeVar); //  * sizeof(float) //  * mp3.channels
//     
//     void* data = nullptr;
//     size_t readedFrames = 0;
//     
//     if (wav.bitsPerSample == 8) {
//       const size_t blockSize = soundBlock * wav.channels;
//       uint8_t* blockData = new uint8_t[blockSize];
//       
//       // читаем один блок
//       const size_t readedFramesVar = drwav_read_pcm_frames(&wav, soundBlock, blockData);
//       
//       if (isForcedMono() && wav.channels != 1) {
//         size_t dataIndex = 0;
//         for (size_t i = 0; i < blockSize; ++i) {
//           const size_t channelIndex = i % wav.channels;
//           dataIndex += size_t(channelIndex == 0 && i > 0);
//           blockData[dataIndex] += channelIndex == 0 ? 0 : blockData[dataIndex+channelIndex];
//         }
//         
//         for (size_t i = 0; i < blockSize / wav.channels; ++i) {
//           if (blockData[i] != 0) blockData[i] /= wav.channels;
//         }
//       }
//       
//       const uint32_t channels = isForcedMono() ? 1 : wav.channels;
//       const int32_t format = to_al_format(channels, wav.bitsPerSample);
//       
//       buffer.bufferData(format, blockData, readedFrames * channels, wav.sampleRate);
//       
//       delete [] blockData;
//       
//       data = blockData;
//       readedFrames = readedFramesVar;
//     } else if (wav.bitsPerSample == 16) {
//       const size_t blockSize = soundBlock * wav.channels;
//       int16_t* blockData = new int16_t[blockSize];
//       
//       // читаем один блок
//       const size_t readedFramesVar = drwav_read_pcm_frames(&wav, soundBlock, blockData);
//       
//       if (isForcedMono() && wav.channels != 1) {
//         size_t dataIndex = 0;
//         for (size_t i = 0; i < blockSize; ++i) {
//           const size_t channelIndex = i % wav.channels;
//           dataIndex += size_t(channelIndex == 0 && i > 0);
//           blockData[dataIndex] += channelIndex == 0 ? 0 : blockData[dataIndex+channelIndex];
//         }
//         
//         for (size_t i = 0; i < blockSize / wav.channels; ++i) {
//           if (blockData[i] != 0) blockData[i] /= wav.channels;
//         }
//       }
//       
//       const uint32_t channels = isForcedMono() ? 1 : wav.channels;
//       const int32_t format = to_al_format(channels, wav.bitsPerSample);
//       
//       buffer.bufferData(format, blockData, readedFrames * channels * sizeof(int16_t), wav.sampleRate);
//       
//       delete [] blockData;
//       
//       data = blockData;
//       readedFrames = readedFramesVar;
//     } else if (wav.bitsPerSample == 32) {
//       const size_t blockSize = soundBlock * wav.channels;
//       float* blockData = new float[blockSize];
//       
//       // читаем один блок
//       const size_t readedFramesVar = drwav_read_pcm_frames_f32(&wav, soundBlock, blockData);
//       
//       if (isForcedMono() && wav.channels != 1) {
//         size_t dataIndex = 0;
//         for (size_t i = 0; i < blockSize; ++i) {
//           const size_t channelIndex = i % wav.channels;
//           dataIndex += size_t(channelIndex == 0 && i > 0);
//           blockData[dataIndex] += channelIndex == 0 ? 0 : blockData[dataIndex+channelIndex];
//         }
//         
//         for (size_t i = 0; i < blockSize / wav.channels; ++i) {
//           if (blockData[i] != 0) blockData[i] /= wav.channels;
//         }
//       }
//       
//       const uint32_t channels = isForcedMono() ? 1 : wav.channels;
//       const int32_t format = to_al_format(channels, wav.bitsPerSample);
//       
//       buffer.bufferData(format, blockData, readedFrames * channels * sizeof(float), wav.sampleRate);
//       
//       delete [] blockData;
//       
//       data = blockData;
//       readedFrames = readedFramesVar;
//     } else {
//       throw std::runtime_error("wav format is not supported");
//     }
//     
//     drwav_uninit(&wav);
//     
//     return readedFrames;
//   }
//   
//   if (dataType == SOUND_TYPE_FLAC) {
//     drflac* flac = drflac_open_file(pathStr.c_str());
//     if (flac == nullptr) throw std::runtime_error("Failed to open file " + pathStr);
//     
//     const bool seek = drflac_seek_to_pcm_frame(flac, loadedSize);
//     if (!seek) throw std::runtime_error("seek to pcm frame failed");
//     
//     const size_t soundBlock = isCached() ? pcmSizeVar : std::min(size_t(flac->sampleRate * SOUND_LOADING_COEFFICIENT), pcmSizeVar);
//     float* blockData = new float[soundBlock * flac->channels];
//     
//     // у флака числа с плавающей точкой это потеря качества
//     // но в опенал я не понял как сделать формат int32
//     const size_t readedFrames = drflac_read_pcm_frames_f32(flac, soundBlock, blockData);
//     
//     if (isForcedMono() && flac->channels != 1) {
//       size_t dataIndex = 0;
//       for (size_t i = 0; i < soundBlock * flac->channels; ++i) {
//         const size_t channelIndex = i % flac->channels;
//         dataIndex += size_t(channelIndex == 0 && i > 0);
//         blockData[dataIndex] += channelIndex == 0 ? 0 : blockData[dataIndex+channelIndex];
//       }
//       
//       for (size_t i = 0; i < soundBlock; ++i) {
//         if (blockData[i] != 0) blockData[i] /= flac->channels;
//       }
//     }
//     
//     const uint32_t channels = isForcedMono() ? 1 : flac->channels;
//     const int32_t format = to_al_format(channels, 32);
//     
//     buffer.bufferData(format, blockData, readedFrames * channels * sizeof(float), flac->sampleRate);
//     
//     delete [] blockData;
//     drflac_close(flac);
//     
//     return readedFrames;
//   }
//   
//   if (dataType == SOUND_TYPE_OGG) {
//     stb_vorbis* file = nullptr;
//     int32_t err;
//     file = stb_vorbis_open_filename(pathStr.c_str(), &err, nullptr);
//     if (file == nullptr) throw std::runtime_error("Failed to open file " + pathStr);
//     
//     if (!stb_vorbis_seek_frame(file, loadedSize)) throw std::runtime_error("seek to pcm frame failed");
//     
//     const stb_vorbis_info &info = stb_vorbis_get_info(file);
//     
//     const uint32_t channels = isForcedMono() ? 1 : info.channels;
//     const size_t soundBlock = isCached() ? pcmSizeVar : std::min(size_t(info.sample_rate * SOUND_LOADING_COEFFICIENT), pcmSizeVar);
//     float* blockData = new float[soundBlock * channels];
//     
//     const size_t readedFrames = stb_vorbis_get_samples_float_interleaved(file, channels, blockData, soundBlock);
//     
//     const int32_t format = to_al_format(channels, 32);
//     
//     buffer.bufferData(format, blockData, readedFrames * channels * sizeof(float), info.sample_rate);
//     
//     delete [] blockData;
//     stb_vorbis_close(file);
//     
//     return readedFrames;
//   }
