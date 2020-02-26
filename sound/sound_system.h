#ifndef SOUND_SYSTEM_H
#define SOUND_SYSTEM_H

#include "id.h"
#include "Source.h"
#include "Buffer.h"
#include "sound_info.h"
#include "Engine.h"
#include <vector>
#include <mutex>

// мультитрединг кажется тут ни к чему, но нужно синхронизовать play(info) и stop(entity);
// эти функции наверное единственные которые должны быть доступны в функциях состояний

typedef struct ALCdevice ALCdevice;
typedef struct ALCcontext ALCcontext;

class SoundDecoderInterface;

namespace yacs {
  class entity;
}

namespace devils_engine {
  namespace sound {
    struct data;
  }
  
  namespace systems {
    class sound : public Engine {
    public:
      struct queued_sound {
        devils_engine::sound::info user_info;
        const devils_engine::sound::data* sound_data;
        size_t loaded_size;
        float priority;
        Source source;
        std::pair<Buffer, Buffer> buffers;
        
        void update_source(const float &master_v, const float &sounds_v);
        void update_buffers();
        void queue_buffers();
        float playing_position() const;
      };
      
      sound();
      ~sound();
      
      bool play(const devils_engine::sound::info &info);
      bool stop(const yacs::entity* ent);
      
      bool start_music(const utils::id &id = utils::id());
      void stop_music();
      
      void pause_sounds();
      void resume_sounds();
      
      void update_listener(const yacs::entity* ent);
      void update(const size_t &time) override;
      
      float master_volume();
      void  master_volume(const float &value);
      float music_volume();
      void  music_volume(const float &value);
      float menu_volume(); // как отделить эти звуки от других
      void  menu_volume(const float &value);
      float sounds_volume();
      void  sounds_volume(const float &value);
    private:
      struct music_data {
        const devils_engine::sound::data* sound_data;
        size_t loaded_size;
        Source source;
        std::pair<Buffer, Buffer> buffers;
        
        bool play(const utils::id &id);
        void pause();
        void stop();
        Source::State state() const;
        size_t duration() const;
        float position() const;
        void set_position(const float &pos);
      };
      
      ALCdevice* device;
      ALCcontext* ctx;
      
      std::vector<Source> free_sources;
      std::vector<std::pair<Buffer, Buffer>> free_buffers;
      std::vector<queued_sound> sounds;
      std::mutex mutex;
      
      music_data music;
      
      float master_v;
      float music_v;
      float menu_v;
      float sounds_v;
      
      size_t find(const yacs::entity* ent, const utils::id &id) const;
      size_t find(const yacs::entity* ent) const;
      void remove(const size_t &index);
    };
  }
}

#endif
