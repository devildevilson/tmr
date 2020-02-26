#include "SoundDecoder.h"

#define DR_MP3_IMPLEMENTATION
#include "dr_mp3.h"

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

#define DR_FLAC_IMPLEMENTATION
#include "dr_flac.h"

#include "stb_vorbis.c"

#include "AL/al.h"
#include "AL/alc.h"
#include "AL/alext.h"

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

void* my_malloc(size_t sz, void* pUserData) {
  (void)pUserData;
//   return new char[sz];
  return malloc(sz);
}

void* my_realloc(void* p, size_t sz, void* pUserData) {
  (void)pUserData;
//   auto new_mem = new char[sz];
//   memcpy(new_mem, p, sz);
//   delete [] reinterpret_cast<char*>(p);
//   return new_mem;
  return realloc(p, sz);
}

void my_free(void* p, void* pUserData) {
  (void)pUserData;
//   delete [] reinterpret_cast<char*>(p);
  free(p);
}

SoundDecoderInterface::SoundDecoderInterface(const uint16_t &channels) :
//     m_path(path),
    m_channels(channels),
    m_sampleRate(0),
    m_bitsPerChannel(0),
    m_size(0) {}

SoundDecoderInterface::~SoundDecoderInterface() {}

uint32_t SoundDecoderInterface::channels() const { return m_channels; }
uint32_t SoundDecoderInterface::sampleRate() const { return m_sampleRate; }
uint32_t SoundDecoderInterface::bitsPerChannel() const { return m_bitsPerChannel; }
size_t SoundDecoderInterface::size() const { return m_size; }

// 0 - default channels count
// возможно придется самому вычислять моно звук
MP3Decoder::MP3Decoder(const std::string &path, const uint16_t &channels) : SoundDecoderInterface(channels) {
    const drmp3_config config{
      m_channels,
      0
    };
    
    const drmp3_allocation_callbacks allocation_callbacks {
      nullptr,
      my_malloc,
      my_realloc,
      my_free
    };
    
    if (!drmp3_init_file(&mp3, path.c_str(), &config, &allocation_callbacks)) {
      throw std::runtime_error("Failed to open file " + path);
    }

    m_sampleRate = mp3.sampleRate;
    m_bitsPerChannel = 32;
    m_size = drmp3_get_pcm_frame_count(&mp3);

    if (m_channels == 0) m_channels = mp3.channels;
  }

MP3Decoder::MP3Decoder(const std::string &path, const uint16_t &channels, const size_t &size) : SoundDecoderInterface(channels) {
    const drmp3_config config{
      m_channels,
      0
    };
    
    const drmp3_allocation_callbacks allocation_callbacks {
      nullptr,
      my_malloc,
      my_realloc,
      my_free
    };

    if (!drmp3_init_file(&mp3, path.c_str(), &config, &allocation_callbacks)) {
      throw std::runtime_error("Failed to open file " + path);
    }

    m_sampleRate = mp3.sampleRate;
    m_bitsPerChannel = 32;
    m_size = size;

    if (m_channels == 0) m_channels = mp3.channels;
  }

MP3Decoder::MP3Decoder(std::ifstream* stream, const uint16_t &channels, const size_t &size) : SoundDecoderInterface(channels) {
    const drmp3_config config{
      m_channels,
      0
    };

    const drmp3_allocation_callbacks allocation_callbacks {
      nullptr,
      my_malloc,
      my_realloc,
      my_free
    };
    
    if (!drmp3_init(&mp3, drRead, drSeek, stream, &config, &allocation_callbacks)) {
      throw std::runtime_error("Failed to init mp3 file");
    }

    m_sampleRate = mp3.sampleRate;
    m_bitsPerChannel = 32;
    m_size = size == 0 ? drmp3_get_pcm_frame_count(&mp3) : size;

    if (m_channels == 0) m_channels = mp3.channels;
  }

MP3Decoder::MP3Decoder(const char* memory, const size_t &memorySize, const uint16_t &channels, const size_t &size) : SoundDecoderInterface(channels) {
  const drmp3_config config{
    m_channels,
    0
  };
  
  const drmp3_allocation_callbacks allocation_callbacks {
      nullptr,
      my_malloc,
      my_realloc,
      my_free
    };

  if (!drmp3_init_memory(&mp3, memory, memorySize, &config, &allocation_callbacks)) {
    throw std::runtime_error("Failed to init mp3 file");
  }

  m_sampleRate = mp3.sampleRate;
  m_bitsPerChannel = 32;
  m_size = size == 0 ? drmp3_get_pcm_frame_count(&mp3) : size;

  if (m_channels == 0) m_channels = mp3.channels;
}

MP3Decoder::~MP3Decoder() {
  drmp3_uninit(&mp3);
}

bool MP3Decoder::seek(const size_t &seekSize) {
  return drmp3_seek_to_pcm_frame(&mp3, seekSize);
}

size_t MP3Decoder::getSamples(char* memory, const size_t &sampleCount) {
  auto* finalData = reinterpret_cast<float*>(memory);

  const size_t blockSize = sampleCount * mp3.channels;
  auto* blockData = new float[blockSize];

  // читаем один блок
  const size_t readedFramesVar = drmp3_read_pcm_frames_f32(&mp3, sampleCount, reinterpret_cast<float*>(memory));

  if (channels() == 1 && mp3.channels != 1) {
    size_t dataIndex = 0;
    for (size_t i = 0; i < blockSize; ++i) {
      const size_t channelIndex = i % mp3.channels;
      dataIndex += size_t(channelIndex == 0 && i > 0);
      blockData[dataIndex] = channelIndex == 0 ? blockData[i] : blockData[dataIndex] + blockData[i];
    }

    const size_t tmp = blockSize / mp3.channels;
    for (size_t i = 0; i < tmp; ++i) {
      //if (blockData[i] != 0) blockData[i] /= wav.channels;
      //finalData[i] = blockData[i] != 0 ? blockData[i] / wav.channels : 0;
      finalData[i] = blockData[i] / mp3.channels;
    }
  } else {
    memcpy(finalData, blockData, blockSize * sizeof(float));
  }

  delete [] blockData;

  return readedFramesVar;

  //return drmp3_read_pcm_frames_f32(&mp3, sampleCount, reinterpret_cast<float*>(memory));
}

size_t MP3Decoder::getSamples(Buffer &buffer, const size_t &sampleCount) {
  const size_t blockSize = sampleCount * mp3.channels;
  auto* blockData = new float[blockSize];

  // читаем один блок
  const size_t readedFramesVar = drmp3_read_pcm_frames_f32(&mp3, sampleCount, blockData);

  if (channels() == 1 && mp3.channels != 1) {
    size_t dataIndex = 0;
    for (size_t i = 0; i < blockSize; ++i) {
      const size_t channelIndex = i % mp3.channels;
      dataIndex += size_t(channelIndex == 0 && i > 0);
//           blockData[dataIndex] += channelIndex == 0 ? 0 : blockData[dataIndex+channelIndex];
      blockData[dataIndex] = channelIndex == 0 ? blockData[i] : blockData[dataIndex] + blockData[i];
    }

    const size_t tmp = blockSize / mp3.channels;
    for (size_t i = 0; i < tmp; ++i) {
      //if (blockData[i] != 0) blockData[i] /= wav.channels;
      //blockData[i] = blockData[i] != 0 ? blockData[i] / wav.channels : 0;
      blockData[i] = blockData[i] / mp3.channels;
    }

    buffer.bufferData(to_al_format(channels(), bitsPerChannel()), blockData, tmp * sizeof(float), sampleRate());
  } else {
    buffer.bufferData(to_al_format(channels(), bitsPerChannel()), blockData, blockSize * sizeof(float), sampleRate());
//      memcpy(finalData, blockData, blockSize * sizeof(uint8_t));
  }

  delete [] blockData;

  return readedFramesVar;

//  const size_t soundBlock = sampleCount * m_channels;
//  auto* blockData = new float[soundBlock];
//
//  const size_t finalSize = drmp3_read_pcm_frames_f32(&mp3, sampleCount, blockData);
//  buffer.bufferData(to_al_format(m_channels, m_bitsPerChannel), blockData, finalSize, m_sampleRate);
//
//  return finalSize;
}

WAVDecoder::WAVDecoder(const std::string &path, const uint16_t &channels) : SoundDecoderInterface(channels) {
  const drwav_allocation_callbacks allocation_callbacks {
    nullptr,
    my_malloc,
    my_realloc,
    my_free
  };
  if (!drwav_init_file(&wav, path.c_str(), &allocation_callbacks)) {
    throw std::runtime_error("Failed to open file " + path);
  }

  // по всей видимости vaw файл нужно будет самому приводить к моно если потребуется

  m_sampleRate = wav.sampleRate;
  m_bitsPerChannel = wav.bitsPerSample;
  m_size = wav.totalPCMFrameCount;

  if (m_channels == 0) m_channels = wav.channels;
}

WAVDecoder::WAVDecoder(const std::string &path, const uint16_t &channels, const size_t &size) : SoundDecoderInterface(channels) {
  const drwav_allocation_callbacks allocation_callbacks {
    nullptr,
    my_malloc,
    my_realloc,
    my_free
  };
  
  if (!drwav_init_file(&wav, path.c_str(), &allocation_callbacks)) {
    throw std::runtime_error("Failed to open file " + path);
  }

  m_sampleRate = wav.sampleRate;
  m_bitsPerChannel = wav.bitsPerSample;
  m_size = size;

  if (m_channels == 0) m_channels = wav.channels;
}

WAVDecoder::WAVDecoder(std::ifstream* stream, const uint16_t &channels, const size_t &size) : SoundDecoderInterface(channels) {
  const drwav_allocation_callbacks allocation_callbacks {
    nullptr,
    my_malloc,
    my_realloc,
    my_free
  };
  if (!drwav_init(&wav, drRead, drSeek, stream, &allocation_callbacks)) {
    throw std::runtime_error("Failed to init wav file");
  }

  m_sampleRate = wav.sampleRate;
  m_bitsPerChannel = wav.bitsPerSample;
  m_size = size == 0 ? wav.totalPCMFrameCount : size;

  if (m_channels == 0) m_channels = wav.channels;
}

WAVDecoder::WAVDecoder(const char* memory, const size_t &memorySize, const uint16_t &channels, const size_t &size) : SoundDecoderInterface(channels) {
  const drwav_allocation_callbacks allocation_callbacks {
    nullptr,
    my_malloc,
    my_realloc,
    my_free
  };
  if (!drwav_init_memory(&wav, memory, memorySize, &allocation_callbacks)) {
    throw std::runtime_error("Failed to init wav file");
  }

  m_sampleRate = wav.sampleRate;
  m_bitsPerChannel = wav.bitsPerSample;
  m_size = size == 0 ? wav.totalPCMFrameCount : size;

  if (m_channels == 0) m_channels = wav.channels;
}

WAVDecoder::~WAVDecoder() {
  drwav_uninit(&wav);
}

bool WAVDecoder::seek(const size_t &seekSize) {
  return drwav_seek_to_pcm_frame(&wav, seekSize);
}

template <typename T>
size_t getSamplesWAV(drwav &wav, const WAVDecoder* decoder, char* memory, const size_t &sampleCount) {
  T* finalData = reinterpret_cast<T*>(memory);

  const size_t blockSize = sampleCount * wav.channels;
  auto* blockData = new T[blockSize];

  // читаем один блок
  const size_t readedFramesVar = drwav_read_pcm_frames(&wav, sampleCount, blockData);

  if (decoder->channels() == 1 && wav.channels != 1) {
    size_t dataIndex = 0;
    for (size_t i = 0; i < blockSize; ++i) {
      const size_t channelIndex = i % wav.channels;
      dataIndex += size_t(channelIndex == 0 && i > 0);
      blockData[dataIndex] = channelIndex == 0 ? blockData[i] : blockData[dataIndex] + blockData[i];
    }

    for (size_t i = 0; i < blockSize / wav.channels; ++i) {
      //if (blockData[i] != 0) blockData[i] /= wav.channels;
      //finalData[i] = blockData[i] != 0 ? blockData[i] / wav.channels : 0;
      finalData[i] = blockData[i] / wav.channels;
    }
  } else {
    memcpy(finalData, blockData, blockSize * sizeof(T));
  }

  delete [] blockData;

  return readedFramesVar;
}

size_t WAVDecoder::getSamples(char* memory, const size_t &sampleCount) {
  if (wav.bitsPerSample == 8) {
    return getSamplesWAV<uint8_t>(wav, this, memory, sampleCount);
  } else if (wav.bitsPerSample == 16) {
    return getSamplesWAV<int16_t>(wav, this, memory, sampleCount);
  } else if (wav.bitsPerSample == 32) {
    return getSamplesWAV<float>(wav, this, memory, sampleCount);
  } else {
    throw std::runtime_error("wav format is not supported");
  }

  return SIZE_MAX;
}

template <typename T>
size_t getSamplesWAVBuffer(drwav &wav, const WAVDecoder* decoder, Buffer &buffer, const size_t &sampleCount) {
  const size_t blockSize = sampleCount * wav.channels;
  auto* blockData = new T[blockSize];

  // читаем один блок
  const size_t readedFramesVar = drwav_read_pcm_frames(&wav, sampleCount, blockData);

  if (decoder->channels() == 1 && wav.channels != 1) {
    size_t dataIndex = 0;
    for (size_t i = 0; i < blockSize; ++i) {
      const size_t channelIndex = i % wav.channels;
      dataIndex += size_t(channelIndex == 0 && i > 0);
//           blockData[dataIndex] += channelIndex == 0 ? 0 : blockData[dataIndex+channelIndex];
      blockData[dataIndex] = channelIndex == 0 ? blockData[i] : blockData[dataIndex] + blockData[i];
    }

    const size_t tmp = blockSize / wav.channels;
    for (size_t i = 0; i < tmp; ++i) {
      //if (blockData[i] != 0) blockData[i] /= wav.channels;
      //blockData[i] = blockData[i] != 0 ? blockData[i] / wav.channels : 0;
      blockData[i] = blockData[i] / wav.channels;
    }

    buffer.bufferData(to_al_format(decoder->channels(), decoder->bitsPerChannel()), blockData, tmp * sizeof(T), decoder->sampleRate());
  } else {
    buffer.bufferData(to_al_format(decoder->channels(), decoder->bitsPerChannel()), blockData, blockSize * sizeof(T), decoder->sampleRate());
//      memcpy(finalData, blockData, blockSize * sizeof(uint8_t));
  }

  delete [] blockData;

  return readedFramesVar;
}

size_t WAVDecoder::getSamples(Buffer &buffer, const size_t &sampleCount) {
  if (wav.bitsPerSample == 8) {
    return getSamplesWAVBuffer<uint8_t>(wav, this, buffer, sampleCount);
  } else if (wav.bitsPerSample == 16) {
    return getSamplesWAVBuffer<int16_t>(wav, this, buffer, sampleCount);
  } else if (wav.bitsPerSample == 32) {
    return getSamplesWAVBuffer<float>(wav, this, buffer, sampleCount);
//    float* finalData = reinterpret_cast<float*>(memory);
//
//    const size_t blockSize = sampleCount * wav.channels;
//    float* blockData = new float[blockSize];
//
//    // читаем один блок
//    const size_t readedFramesVar = drwav_read_pcm_frames_f32(&wav, sampleCount, blockData);
//
//    if (m_channels == 1 && wav.channels != 1) {
//      size_t dataIndex = 0;
//      for (size_t i = 0; i < blockSize; ++i) {
//        const size_t channelIndex = i % wav.channels;
//        dataIndex += size_t(channelIndex == 0 && i > 0);
////           blockData[dataIndex] += channelIndex == 0 ? 0 : blockData[dataIndex+channelIndex];
//        blockData[dataIndex] = channelIndex == 0 ? blockData[i] : blockData[dataIndex] + blockData[i];
//      }
//
//      for (size_t i = 0; i < blockSize / wav.channels; ++i) {
//        //if (blockData[i] != 0) blockData[i] /= wav.channels;
//        finalData[i] = blockData[i] != 0 ? blockData[i] / wav.channels : 0;
//      }
//    } else {
//      memcpy(finalData, blockData, blockSize * sizeof(float));
//    }
//
//    delete [] blockData;
//
//    return readedFramesVar;
  } else {
    throw std::runtime_error("wav format is not supported");
  }

  return SIZE_MAX;
}

FLACDecoder::FLACDecoder(const std::string &path, const uint16_t &channels) : SoundDecoderInterface(channels), flac(nullptr) {
  const drflac_allocation_callbacks allocation_callbacks{
    nullptr,
    my_malloc,
    my_realloc,
    my_free
  };
  flac = drflac_open_file(path.c_str(), &allocation_callbacks);
  if (flac == nullptr) throw std::runtime_error("Failed to open file " + path);

  m_sampleRate = flac->sampleRate;
  m_bitsPerChannel = flac->bitsPerSample;
  m_size = flac->totalPCMFrameCount;

  if (m_channels == 0) m_channels = flac->channels;
}

FLACDecoder::FLACDecoder(const std::string &path, const uint16_t &channels, const size_t &size) : SoundDecoderInterface(channels), flac(nullptr) {
  const drflac_allocation_callbacks allocation_callbacks{
    nullptr,
    my_malloc,
    my_realloc,
    my_free
  };
  flac = drflac_open_file(path.c_str(), &allocation_callbacks);
  if (flac == nullptr) throw std::runtime_error("Failed to open file " + path);

  m_sampleRate = flac->sampleRate;
  m_bitsPerChannel = flac->bitsPerSample;
  m_size = size;

  if (m_channels == 0) m_channels = flac->channels;
}

FLACDecoder::FLACDecoder(std::ifstream* stream, const uint16_t &channels, const size_t &size) : SoundDecoderInterface(channels) {
  const drflac_allocation_callbacks allocation_callbacks{
    nullptr,
    my_malloc,
    my_realloc,
    my_free
  };
  flac = drflac_open(drRead, drSeek, stream, &allocation_callbacks);
  if (flac == nullptr) throw std::runtime_error("Failed to init flac file");

  m_sampleRate = flac->sampleRate;
  m_bitsPerChannel = flac->bitsPerSample;
  m_size = size;

  if (m_channels == 0) m_channels = flac->channels;
}

FLACDecoder::FLACDecoder(const char* memory, const size_t &memorySize, const uint16_t &channels, const size_t &size) : SoundDecoderInterface(channels) {
  const drflac_allocation_callbacks allocation_callbacks{
    nullptr,
    my_malloc,
    my_realloc,
    my_free
  };
  flac = drflac_open_memory(memory, memorySize, &allocation_callbacks);
  if (flac == nullptr) throw std::runtime_error("Failed to init flac file");

  m_sampleRate = flac->sampleRate;
  m_bitsPerChannel = flac->bitsPerSample;
  m_size = size;

  if (m_channels == 0) m_channels = flac->channels;
}

FLACDecoder::~FLACDecoder() {
  drflac_close(flac);
  flac = nullptr;
}

bool FLACDecoder::seek(const size_t &seekSize) {
  return drflac_seek_to_pcm_frame(flac, seekSize);
}

typedef drflac_uint64 (*flac_read_pcm_frames)(drflac* pFlac, drflac_uint64 framesToRead, void* pBufferOut);

template <typename T>
size_t getSamplesFLAC(drflac* flac, const FLACDecoder* decoder, flac_read_pcm_frames func, char* memory, const size_t &sampleCount) {
  T* finalData = reinterpret_cast<T*>(memory);

  const size_t soundBlock = sampleCount * flac->channels;
  T* blockData = new T[soundBlock];

  // у флака числа с плавающей точкой это потеря качества
  // но в опенал я не понял как сделать формат int32
  const size_t readedFrames = func(flac, soundBlock, blockData);

  if (decoder->channels() == 1 && flac->channels != 1) {
    size_t dataIndex = 0;
    for (size_t i = 0; i < soundBlock; ++i) {
      const size_t channelIndex = i % flac->channels;
      dataIndex += size_t(channelIndex == 0 && i > 0);
//           blockData[dataIndex] += channelIndex == 0 ? 0 : blockData[dataIndex+channelIndex];
      blockData[dataIndex] = channelIndex == 0 ? blockData[i] : blockData[dataIndex] + blockData[i];
    }

    const size_t tmp = soundBlock / flac->channels;
    for (size_t i = 0; i < tmp; ++i) {
      //finalData[i] = blockData[i] != 0 ? blockData[i] / flac->channels : 0;
      finalData[i] = blockData[i] / flac->channels;
    }
  } else {
    memcpy(finalData, blockData, soundBlock * sizeof(T));
  }

  delete [] blockData;

  return readedFrames;
}

template <typename T>
size_t getSamplesFLACBuffer(drflac* flac, const FLACDecoder* decoder, flac_read_pcm_frames func, Buffer &buffer, const size_t &sampleCount) {
  const size_t soundBlock = sampleCount * flac->channels;
  T* blockData = new T[soundBlock];

  // у флака числа с плавающей точкой это потеря качества
  // но в опенал я не понял как сделать формат int32
  const size_t readedFrames = func(flac, soundBlock, blockData);

  if (decoder->channels() == 1 && flac->channels != 1) {
    size_t dataIndex = 0;
    for (size_t i = 0; i < soundBlock; ++i) {
      const size_t channelIndex = i % flac->channels;
      dataIndex += size_t(channelIndex == 0 && i > 0);
//           blockData[dataIndex] += channelIndex == 0 ? 0 : blockData[dataIndex+channelIndex];
      blockData[dataIndex] = channelIndex == 0 ? blockData[i] : blockData[dataIndex] + blockData[i];
    }

    const size_t tmp = soundBlock / flac->channels;
    for (size_t i = 0; i < tmp; ++i) {
      blockData[i] = blockData[i] / flac->channels;
    }

    buffer.bufferData(to_al_format(decoder->channels(), decoder->bitsPerChannel()), blockData, tmp * sizeof(T), decoder->sampleRate());
  } else {
    buffer.bufferData(to_al_format(decoder->channels(), decoder->bitsPerChannel()), blockData, soundBlock * sizeof(T), decoder->sampleRate());
//    memcpy(finalData, blockData, soundBlock * sizeof(T));
  }

  delete [] blockData;

  return readedFrames;
}

size_t FLACDecoder::getSamples(char* memory, const size_t &sampleCount) {
  if (m_bitsPerChannel == 16) {
    return getSamplesFLAC<int16_t>(flac, this, reinterpret_cast<flac_read_pcm_frames>(drflac_read_pcm_frames_s16), memory, sampleCount);
  } else if (m_bitsPerChannel == 32) {
    return getSamplesFLAC<float>(flac, this, reinterpret_cast<flac_read_pcm_frames>(drflac_read_pcm_frames_f32), memory, sampleCount);
  } else {
    throw std::runtime_error("flac format is not supported");
  }

  return SIZE_MAX;
}

size_t FLACDecoder::getSamples(Buffer &buffer, const size_t &sampleCount) {
  if (m_bitsPerChannel == 16) {
    return getSamplesFLACBuffer<int16_t>(flac, this, reinterpret_cast<flac_read_pcm_frames>(drflac_read_pcm_frames_s16), buffer, sampleCount);
  } else if (m_bitsPerChannel == 32) {
    return getSamplesFLACBuffer<float>(flac, this, reinterpret_cast<flac_read_pcm_frames>(drflac_read_pcm_frames_f32), buffer, sampleCount);
//    const size_t soundBlock = sampleCount * flac->channels;
//    auto* blockData = new float[soundBlock];
//
//    // у флака числа с плавающей точкой это потеря качества
//    // но в опенал я не понял как сделать формат int32
//    const size_t readedFrames = drflac_read_pcm_frames_f32(flac, soundBlock, blockData);
//
//    if (m_channels == 1 && flac->channels != 1) {
//      size_t dataIndex = 0;
//      for (size_t i = 0; i < soundBlock; ++i) {
//        const size_t channelIndex = i % flac->channels;
//        dataIndex += size_t(channelIndex == 0 && i > 0);
////           blockData[dataIndex] += channelIndex == 0 ? 0 : blockData[dataIndex+channelIndex];
//        blockData[dataIndex] = channelIndex == 0 ? blockData[i] : blockData[dataIndex] + blockData[i];
//      }
//
//      for (size_t i = 0; i < soundBlock / flac->channels; ++i) {
////        blockData[i] = blockData[i] != 0 ? blockData[i] / flac->channels : 0;
//        blockData[i] = blockData[i] / flac->channels;
//      }
//
//      buffer.bufferData(to_al_format(m_channels, m_bitsPerChannel), blockData, (soundBlock / flac->channels) * sizeof(float), m_sampleRate);
//    } else {
//      buffer.bufferData(to_al_format(m_channels, m_bitsPerChannel), blockData, soundBlock * sizeof(float), m_sampleRate);
//    }
//
//    delete [] blockData;
//
//    return readedFrames;
  } else {
    throw std::runtime_error("flac format is not supported");
  }

  return SIZE_MAX;
}

OGGDecoder::OGGDecoder(const std::string &path, const uint16_t &channels) : SoundDecoderInterface(channels), file(nullptr) {
  int32_t err;
  file = stb_vorbis_open_filename(path.c_str(), &err, nullptr);
  if (file == nullptr) throw std::runtime_error("Failed to open file " + path);

  const stb_vorbis_info &info = stb_vorbis_get_info(file);

  m_sampleRate = info.sample_rate;
  m_bitsPerChannel = 32;
  m_size = stb_vorbis_stream_length_in_samples(file);

  if (m_channels == 0) m_channels = info.channels;
}

OGGDecoder::OGGDecoder(const std::string &path, const uint16_t &channels, const size_t &size) : SoundDecoderInterface(channels), file(nullptr) {
  int32_t err;
  file = stb_vorbis_open_filename(path.c_str(), &err, nullptr);
  if (file == nullptr) throw std::runtime_error("Failed to open file " + path);

  const stb_vorbis_info &info = stb_vorbis_get_info(file);

  m_sampleRate = info.sample_rate;
  m_bitsPerChannel = 32;
  m_size = size;

  if (m_channels == 0) m_channels = info.channels;
}

OGGDecoder::OGGDecoder(const char* memory, const size_t &memorySize, const uint16_t &channels, const size_t &size) : SoundDecoderInterface(channels), file(nullptr) {
  int32_t err;
  auto finMem = reinterpret_cast<const uint8_t*>(memory);
  file = stb_vorbis_open_memory(finMem, memorySize, &err, nullptr);
  if (file == nullptr) throw std::runtime_error("Failed to init ogg file");

  const stb_vorbis_info &info = stb_vorbis_get_info(file);

  m_sampleRate = info.sample_rate;
  m_bitsPerChannel = 32;
  m_size = size == 0 ? stb_vorbis_stream_length_in_samples(file) : size;

  if (m_channels == 0) m_channels = info.channels;
}

OGGDecoder::~OGGDecoder() {
  stb_vorbis_close(file);
}

bool OGGDecoder::seek(const size_t &seekSize) {
  return stb_vorbis_seek_frame(file, seekSize);
}

size_t OGGDecoder::getSamples(char* memory, const size_t &sampleCount) {
  return stb_vorbis_get_samples_float_interleaved(file, m_channels, reinterpret_cast<float*>(memory), sampleCount);
}

size_t OGGDecoder::getSamples(Buffer &buffer, const size_t &sampleCount) {
  const size_t soundBlock = sampleCount * m_channels;
  auto* blockData = new float[soundBlock];

  const size_t finalSize = stb_vorbis_get_samples_float_interleaved(file, m_channels, blockData, sampleCount);
  buffer.bufferData(to_al_format(m_channels, m_bitsPerChannel), blockData, finalSize, m_sampleRate);

  return finalSize;
}

int32_t to_al_format(const uint16_t &channels, const uint32_t &bitsPerChannel) {
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
