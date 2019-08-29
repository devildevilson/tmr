#include "SoundData.h"

// #include <vorbis/codec.h>
// #include <vorbis/vorbisfile.h>

#include "AL/al.h"
#include "AL/alc.h"
#include "AL/alext.h"

#include "Globals.h"

#include "SoundSystem.h"
#include "SoundDecoder.h"

#ifdef _DEBUG
  #include <cassert>
  #define ASSERT(expr) assert(expr)
#else
  #define ASSERT(expr)
#endif

#define FORCED_MONO_FLAG_PLACE (1<<16)

ChannelData::ChannelData() : container(0) {}
ChannelData::ChannelData(const uint16_t &channels, const bool forcedMono) : container(0) {
  make(channels, forcedMono);
}

void ChannelData::make(const uint16_t &channels, const bool forcedMono) {
  container |= uint32_t(channels) | (forcedMono * FORCED_MONO_FLAG_PLACE);
}

uint16_t ChannelData::count() const {
  const uint32_t mask = 0x0000ffff;
  return container & mask;
}

bool ChannelData::forcedMono() const {
  return (container & FORCED_MONO_FLAG_PLACE) == FORCED_MONO_FLAG_PLACE;
}

SoundData::SoundData(const CreateInfo &info) : 
  soundId(info.id),
  soundData(info.soundData){

  // к сожалению тогда decoder нельзя использовать в мультипотоке
//  if (soundData.type == SOUND_TYPE_MP3) {
//    decoder = new MP3Decoder(soundData.data, soundData.size, isForcedMono() ? 1 : 0, soundData.pcmSize);
//  } else if (soundData.type == SOUND_TYPE_WAV) {
//    decoder = new WAVDecoder(soundData.data, soundData.size, isForcedMono() ? 1 : 0, soundData.pcmSize);
//  } else if (soundData.type == SOUND_TYPE_FLAC) {
//    decoder = new FLACDecoder(soundData.data, soundData.size, isForcedMono() ? 1 : 0, soundData.pcmSize);
//  } else if (soundData.type == SOUND_TYPE_OGG) {
//    decoder = new OGGDecoder(soundData.data, soundData.size, isForcedMono() ? 1 : 0, soundData.pcmSize);
//  }
}

SoundData::~SoundData() {
  delete [] soundData.data;
}

size_t SoundData::load(const size_t &loadedSize, const size_t &size, Buffer &buffer) const {
  if (soundData.type == SOUND_TYPE_PCM) {
    const size_t finalSize = std::min(size, soundData.pcmSize - loadedSize);
    const size_t bytes = finalSize * soundData.channels.count() * (soundData.bitsPerSample/8);
    const size_t loadedBytes = loadedSize * soundData.channels.count() * (soundData.bitsPerSample/8);
    const int32_t format = to_al_format(soundData.channels.count(), soundData.bitsPerSample);

    buffer.bufferData(format, &soundData.data[loadedBytes], bytes, soundData.sampleRate);
    return finalSize;
  }

  SoundDecoderInterface* loader = nullptr;
  if (soundData.type == SOUND_TYPE_MP3) {
    loader = new MP3Decoder(soundData.data, soundData.size, isForcedMono() ? 1 : 0, soundData.pcmSize);
  } else if (soundData.type == SOUND_TYPE_WAV) {
    loader = new WAVDecoder(soundData.data, soundData.size, isForcedMono() ? 1 : 0, soundData.pcmSize);
  } else if (soundData.type == SOUND_TYPE_FLAC) {
    loader = new FLACDecoder(soundData.data, soundData.size, isForcedMono() ? 1 : 0, soundData.pcmSize);
  } else if (soundData.type == SOUND_TYPE_OGG) {
    loader = new OGGDecoder(soundData.data, soundData.size, isForcedMono() ? 1 : 0, soundData.pcmSize);
  } else {
    throw std::runtime_error("sound type is not supported");
  }
  
  if (!loader->seek(loadedSize)) {
    delete loader;
    throw std::runtime_error("seek to pcm frame failed");
  }

//  const size_t bytes = size * (loader->bitsPerChannel()/8);
//  char* memory = new char[bytes];
//
//  const size_t readedFrames = loader->getSamples(memory, size);
//
//  const int32_t format = to_al_format(loader->channels(), loader->bitsPerChannel());
//  buffer.bufferData(format, memory, bytes, loader->sampleRate());
//
//  delete [] memory;

  const size_t readedFrames = loader->getSamples(buffer, size);

  delete loader;
  
  return readedFrames;
}

size_t SoundData::load(const size_t &loadedSize, const size_t &size, char* buffer) const {
  if (soundData.type == SOUND_TYPE_PCM) {
    const size_t finalSize = std::min(size, soundData.pcmSize - loadedSize);
    const size_t bytes = finalSize * soundData.channels.count() * (soundData.bitsPerSample/8);
    const size_t loadedBytes = loadedSize * soundData.channels.count() * (soundData.bitsPerSample/8);

    memcpy(buffer, &soundData.data[loadedBytes], bytes);
    return finalSize;
  }

  // декодер мы можем создать и хранить не пересоздавая из раза в раз
  SoundDecoderInterface* loader = nullptr;
  if (soundData.type == SOUND_TYPE_MP3) {
    loader = new MP3Decoder(soundData.data, soundData.size, isForcedMono() ? 1 : 0, soundData.pcmSize);
  } else if (soundData.type == SOUND_TYPE_WAV) {
    loader = new WAVDecoder(soundData.data, soundData.size, isForcedMono() ? 1 : 0, soundData.pcmSize);
  } else if (soundData.type == SOUND_TYPE_FLAC) {
    loader = new FLACDecoder(soundData.data, soundData.size, isForcedMono() ? 1 : 0, soundData.pcmSize);
  } else if (soundData.type == SOUND_TYPE_OGG) {
    loader = new OGGDecoder(soundData.data, soundData.size, isForcedMono() ? 1 : 0, soundData.pcmSize);
  } else {
    throw std::runtime_error("sound type is not supported");
  }
  
  if (!loader->seek(loadedSize)) {
    delete loader;
    throw std::runtime_error("seek to pcm frame failed");
  }

//  const size_t bytes = size * loader->channels() * (loader->bitsPerChannel()/8);
//  const size_t readedFrames = loader->getSamples(buffer, bytes);
  const size_t readedFrames = loader->getSamples(buffer, size);

  delete loader;
  
  return readedFrames;
}

bool SoundData::isCached() const {
  return soundData.type == SOUND_TYPE_PCM;
}

bool SoundData::isMono() const {
  return soundData.channels.count() == 1;
}

bool SoundData::isForcedMono() const {
  return soundData.channels.forcedMono();
}

ResourceID SoundData::id() const {
  return soundId;
}

//std::string SoundData::path() const {
//  return pathStr;
//}

size_t SoundData::pcmSize() const {
  return soundData.pcmSize;
}

// size_t SoundData::loadedPcmSize() const {
//   return loadedPcmSizeVar;
// }

uint32_t SoundData::sampleRate() const {
  return soundData.sampleRate;
}

uint32_t SoundData::bitsPerSample() const {
  return soundData.bitsPerSample;
}

uint16_t SoundData::channelCount() const {
  return soundData.channels.count();
}

size_t pcmFramesToBytes(const size_t &pcmFrames, const SoundData* sound) {
  return pcmFrames * sound->channelCount() * (sound->bitsPerSample()/8);
}

size_t bytesToPCMFrames(const size_t &bytes, const SoundData* sound) {
  return (bytes / sound->channelCount()) / (sound->bitsPerSample()/8);
}

size_t secondToPCMSize(const size_t &seconds, const SoundData* sound) {
  return seconds * sound->sampleRate() * sound->channelCount();
}