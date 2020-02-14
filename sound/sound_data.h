#ifndef SOUND_DATA_H
#define SOUND_DATA_H

#include <cstdint>
#include <cstddef>
#include "id.h"
#include "resource_container.h"
#include "Buffer.h"

namespace devils_engine {
  namespace sound {
    enum class type {
      flac,
      mp3,
      wav,
      ogg,
      pcm,
      count
    };
    
    struct channels_data {
      uint32_t container;
      
      channels_data();
      channels_data(const uint16_t &channels, const bool forcedMono);
      void make(const uint16_t &channels, const bool forcedMono);
      uint16_t count() const;
      bool forced_mono() const;
    };
    
    struct data {
      utils::id id; // ???
      enum type type;
      //int32_t al_format;
      channels_data channels;
      uint32_t sample_rate;
      uint32_t bits_per_sample;

      char* memory;
      size_t size; // размер данных в data, то есть сайзОф(формат) * сэмплРейт * чаннелКаунт
      size_t pcm_size; // может быть равен size
      //SoundDecoderInterface* decoder;
      
      ~data();
      size_t load(const size_t &loadedSize, const size_t &size, Buffer &buffer) const;
      size_t load(const size_t &loadedSize, const size_t &size, char* buffer) const;
    };
    
    size_t pcmFramesToBytes(const size_t &pcmFrames, const data* sound);
    size_t bytesToPCMFrames(const size_t &bytes, const data* sound);
    size_t secondToPCMSize(const size_t &seconds, const data* sound);
  }
  
  namespace game {
    using sounds_container = const utils::resource_container_array<utils::id, sound::data, 50>;
    using sounds_container_load = utils::resource_container_array<utils::id, sound::data, 50>;
  }
}

#endif
