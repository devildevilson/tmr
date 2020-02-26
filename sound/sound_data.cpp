#include "sound_data.h"

#include "SoundDecoder.h"
#include <cstring>

#define FORCED_MONO_FLAG_PLACE (1<<16)

namespace devils_engine {
  namespace sound {
    channels_data::channels_data() : container(0) {}
    channels_data::channels_data(const uint16_t &channels, const bool forcedMono) : container(0) {
      make(channels, forcedMono);
    }
    
    void channels_data::make(const uint16_t &channels, const bool forcedMono) {
      container |= uint32_t(channels) | (forcedMono * FORCED_MONO_FLAG_PLACE);
    }
    
    uint16_t channels_data::count() const {
      const uint32_t mask = 0x0000ffff;
      return container & mask;
    }
    
    bool channels_data::forced_mono() const {
      return (container & FORCED_MONO_FLAG_PLACE) == FORCED_MONO_FLAG_PLACE;
    }
    
    data::~data() {
      delete [] memory;
    }
    
    size_t data::load(const size_t &loadedSize, const size_t &size, Buffer &buffer) const {
      if (type == type::pcm) {
        const size_t finalSize = std::min(size, pcm_size - loadedSize);
        const size_t bytes = finalSize * channels.count() * (bits_per_sample/8);
        const size_t loadedBytes = loadedSize * channels.count() * (bits_per_sample/8);
        const int32_t format = to_al_format(channels.count(), bits_per_sample);

        buffer.bufferData(format, &memory[loadedBytes], bytes, sample_rate);
        return finalSize;
      }
      
      SoundDecoderInterface* loader = nullptr;
      if (type == type::mp3) {
        loader = new MP3Decoder(memory, size, channels.forced_mono() ? 1 : 0, pcm_size);
      } else if (type == type::wav) {
        loader = new WAVDecoder(memory, size, channels.forced_mono() ? 1 : 0, pcm_size);
      } else if (type == type::flac) {
        loader = new FLACDecoder(memory, size, channels.forced_mono() ? 1 : 0, pcm_size);
      } else if (type == type::ogg) {
        loader = new OGGDecoder(memory, size, channels.forced_mono() ? 1 : 0, pcm_size);
      } else {
        throw std::runtime_error("sound type is not supported");
      }
      
      if (!loader->seek(loadedSize)) {
        delete loader;
        throw std::runtime_error("seek to pcm frame failed");
      }

      const size_t readedFrames = loader->getSamples(buffer, size);
      delete loader;
      return readedFrames;
    }
    
    size_t data::load(const size_t &loadedSize, const size_t &size, char* buffer) const {
      if (type == type::pcm) {
        const size_t finalSize = std::min(size, pcm_size - loadedSize);
        const size_t bytes = finalSize * channels.count() * (bits_per_sample/8);
        const size_t loadedBytes = loadedSize * channels.count() * (bits_per_sample/8);
//         const int32_t format = to_al_format(channels.count(), bits_per_sample);

        memcpy(buffer, &memory[loadedBytes], bytes);
        return finalSize;
      }
      
      SoundDecoderInterface* loader = nullptr;
      if (type == type::mp3) {
        loader = new MP3Decoder(memory, size, channels.forced_mono() ? 1 : 0, pcm_size);
      } else if (type == type::wav) {
        loader = new WAVDecoder(memory, size, channels.forced_mono() ? 1 : 0, pcm_size);
      } else if (type == type::flac) {
        loader = new FLACDecoder(memory, size, channels.forced_mono() ? 1 : 0, pcm_size);
      } else if (type == type::ogg) {
        loader = new OGGDecoder(memory, size, channels.forced_mono() ? 1 : 0, pcm_size);
      } else {
        throw std::runtime_error("sound type is not supported");
      }
      
      if (!loader->seek(loadedSize)) {
        delete loader;
        throw std::runtime_error("seek to pcm frame failed");
      }
      
      const size_t readedFrames = loader->getSamples(buffer, size);
      delete loader;
      return readedFrames;
    }
    
    size_t pcmFramesToBytes(const size_t &pcmFrames, const data* sound) {
      return pcmFrames * sound->channels.count() * (sound->bits_per_sample/8);
    }
    
    size_t bytesToPCMFrames(const size_t &bytes, const data* sound) {
      return (bytes / sound->channels.count()) / (sound->bits_per_sample/8);
    }
    
    size_t secondToPCMSize(const size_t &seconds, const data* sound) {
      return seconds * sound->sample_rate; //  * sound->channels.count() ???
    }
  }
}
