#include "sound_system.h"

#include "AL/al.h"
#include "AL/alc.h"
// #include "AL/alut.h"
#include "AL/alext.h"

#include "alHelpers.h"

#include "EntityComponentSystem.h"
#include "TransformComponent.h"
#include "PhysicsComponent.h"
#include "global_components_indicies.h"
#include <cstring>
#include "Globals.h"
#include "sound_data.h"
#include "core_funcs.h"

#define NULL_SOURCE 0
#define NULL_BUFFFER 0

#define SOUND_LOADING_COEFFICIENT 5.0f

void listAudioDevices(const ALCchar *devices) {
  const ALCchar *device = devices, *next = devices + 1;
  size_t len = 0;

  std::cout << "Devices list:\n";
  std::cout << "----------\n";
  
  while (device && *device != '\0' && next && *next != '\0') {
    std::cout << device << "\n";
    len = strlen(device);
    device += (len + 1);
    next += (len + 2);
  }
  
  std::cout << "----------\n";
}

void listExtensions(const ALCchar *devices) {
  const ALCchar *device = devices, *next = devices + 1;
  size_t len = 0;

  std::cout << "Extensions list:\n";
  std::cout << "----------\n";
  
  while (device && *device != '\0' && next && *next != '\0') {
    std::cout << device << "\n";
    len = strlen(device);
    device += (len + 1);
    next += (len + 2);
  }
  
  std::cout << "----------\n";
}

// ALenum to_al_format(short channels, short samples) {
//   bool stereo = (channels > 1);
//   
//   switch (samples) {
//   case 16:
//     if (stereo)
//       return AL_FORMAT_STEREO16;
//     else
//       return AL_FORMAT_MONO16;
//   case 8:
//     if (stereo)
//       return AL_FORMAT_STEREO8;
//     else
//       return AL_FORMAT_MONO8;
//   default:
//     return -1;
//   }
// }

namespace Listener {
  void setGain(const float &data) {
    alListenerf(AL_GAIN, data);
    openalError("Could not set listener gain");
  }
  
  void setPos(const glm::vec3 &data) {
    alListener3f(AL_POSITION, data.x, data.y, data.z);
    openalError("Could not set listener position");
  }
  
  void setPos(const float* data) {
    alListenerfv(AL_POSITION, data);
    openalError("Could not set listener position");
  }
  
  void setVel(const glm::vec3 &data) {
    alListener3f(AL_VELOCITY, data.x, data.y, data.z);
    openalError("Could not set listener velocity");
  }
  
  void setVel(const float* data) {
    alListenerfv(AL_VELOCITY, data);
    openalError("Could not set listener velocity");
  }
  
  void setOrientation(const glm::vec3 &forward, const glm::vec3 &up) {
    float orient[6] = {forward.x, forward.y, forward.z, up.x, up.y, up.z};
    alListenerfv(AL_ORIENTATION, orient);
    openalError("Could not set listener orientation");
  }
  
  float gatGain() {
    float data;
    alGetListenerf(AL_GAIN, &data);
    openalError("Could not gat listener gain");
    
    return data;
  }
  
  glm::vec3 getPos() {
    glm::vec3 data;
    alGetListener3f(AL_POSITION, &data.x, &data.y, &data.z);
    openalError("Could not get listener position");
    
    return data;
  }
  
  glm::vec3 getVel() {
    glm::vec3 data;
    alGetListener3f(AL_VELOCITY, &data.x, &data.y, &data.z);
    openalError("Could not get listener velocity");
    
    return data;
  }
  
  void getOrientation(glm::vec3 &forward, glm::vec3 &up) {
    float orient[6];
    alGetListenerfv(AL_ORIENTATION, orient);
    openalError("Could not get listener orientation");
    
    forward = glm::vec3(orient[0], orient[1], orient[2]);
    up = glm::vec3(orient[3], orient[4], orient[5]);
  }
}

namespace devils_engine {
  namespace systems {
//     sound::info::info(const yacs::entity* ent, const utils::id &id) : ent(ent), id(id), volume(10000.0f), min_pitch(1.0f), max_pitch(1.0f), max_dist(100.0f), rolloff(1.0f), ref_dist(1.0f) {}
//     sound::info::info(const yacs::entity* ent, const utils::id &id, const float &volume) : ent(ent), id(id), volume(volume), min_pitch(1.0f), max_pitch(1.0f), max_dist(100.0f), rolloff(1.0f), ref_dist(1.0f) {}
//     sound::info::info(const yacs::entity* ent, const utils::id &id, const float &volume, const float &min_pitch, const float &max_pitch) : ent(ent), id(id), volume(volume), min_pitch(min_pitch), max_pitch(max_pitch), max_dist(100.0f), rolloff(1.0f), ref_dist(1.0f) {}
    
    sound::sound() {
      ALCenum error;
      
      master_v = 0.1f;
      music_v = 1.0f;
      menu_v = 1.0f;
      sounds_v = 1.0f;
      
      device = alcOpenDevice(nullptr);
//       device = alcOpenDevice("OpenAL Soft");
      if (device == nullptr) {
        error = alcGetError(device);
        openalcError(device, error, "Could not create OpenAL device");
      }
      
      const ALCchar* actualDeviceName = alcGetString(device, ALC_DEVICE_SPECIFIER);
      Global::console()->printf("Using sound output driver %s", actualDeviceName);
      
      ctx = alcCreateContext(device, nullptr);
      error = alcGetError(device);
      openalcError(device, error, "Could not create OpenAL context");
      
      if (!alcMakeContextCurrent(ctx)) {
        error = alGetError();
        openalcError(device, error, "Could not make current context");
      }
      
      alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED);
      openalError("Could not set distance model");
      
      const glm::vec3 front = glm::vec3(0.0f, 0.0f, 1.0f);
      const glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

      Listener::setPos(glm::vec3(0.0f, 0.0f, 0.0f));
      Listener::setVel(glm::vec3(0.0f, 0.0f, 0.0f));
      Listener::setOrientation(front, up);
      Listener::setGain(1.0f);
      
      error = alGetError();
      openalError(error, "dwvvdwvwlvwvmdvlwvwvmdlmwvlwd");
      while (error == AL_NO_ERROR) {
        uint32_t id;
        alGenSources(1, &id);
        error = alGetError();
        
    //     openalError(error, "Could not create sources");
        
        free_sources.emplace_back(id);
        free_sources.back().setMinGain(0.0f);
        music.source.setGain(master_v * sounds_v);
      }
      
      //std::cout << "freeSources.size() " << freeSources.size() << "\n";
      free_sources.pop_back();
      if (free_sources.empty()) throw std::runtime_error("Could not create sources");
      if (!free_sources[0].isValid()) throw std::runtime_error("bad sources");
      
      music.source = free_sources.back();
      free_sources.pop_back();
      
      error = AL_NO_ERROR;
      for (size_t i = 0; i < free_sources.size(); ++i) {
        ALCuint ids[2];
        alGenBuffers(2, ids);
        error = alGetError();
        
        openalError(error, "Could not create sound buffers");
        
        free_buffers.emplace_back(Buffer(ids[0]), Buffer(ids[1]));
      }
      
      {
        ALCuint ids[2];
        alGenBuffers(2, ids);
        error = alGetError();
        
        openalError(error, "Could not create sound buffers");
        
        music.buffers = std::make_pair(Buffer(ids[0]), Buffer(ids[1]));
        
        music.loaded_size = 0;
        music.sound_data = nullptr;
        
        music.source.relative(true);
        music.source.setPos(glm::vec3(0.0f, 0.0f, 0.0f));
        music.source.setDir(glm::vec3(0.0f, 1.0f, 0.0f));
        music.source.setVel(glm::vec3(0.0f, 0.0f, 0.0f));
        music.source.setGain(master_v * music_v);
        music.source.setMaxDist(100.0f);
        music.source.setPitch(1.0f);
        music.source.setRefDist(1.0f);
        music.source.setRolloff(1.0f);
        music.source.setMinGain(0.0f);
      }
    }
    
    sound::~sound() {
      for (size_t i = 0; i < sounds.size(); ++i) {
        if (sounds[i].source.isValid()) {
          free_sources.push_back(sounds[i]->source);
          sounds[i].source.stop();
          sounds[i].source.buffer(0);
          sounds[i].source = Source(UINT32_MAX);
          free_buffers.push_back(sounds[i].buffers);
          
//           if (!sounds[i].sound_data->type == devils_engine::sound::data::type::pcm) {
//             free_buffers.push_back(sounds[i].buffers);
//           }
        }
      }
      
      openalError("Could not delete cached buffers");
      
      uint32_t buf[2] = {music.buffers.first.id(), music.buffers.second.id()};
      alDeleteBuffers(2, buf);
      
      openalError("Could not delete soundtrack buffers");
      
    //   std::cout << "sources size: " << freeSources.size() << " buffers size: " << sourceBuffers.size() << "\n";
      
      for (size_t i = 0; i < free_buffers.size(); ++i) {
        uint32_t buf[2] = {free_buffers[i].first.id(), free_buffers[i].second.id()};
        alDeleteBuffers(2, buf);
      }
      
      openalError("Could not delete sources buffers");
      
      alDeleteSources(freeSources.size(), reinterpret_cast<uint32_t*>(freeSources.data()));
      
      openalError("Could not delete sources");
      openalcError(device, "Error");
      
      alcMakeContextCurrent(nullptr);
      openalcError(device, "Error");
      alcDestroyContext(ctx);
      openalcError(device, "Error");
      alcCloseDevice(device);
    }
    
    // короче нужно сразу выдавать источники
    
    bool sound::play(const info &info) {
      auto new_sound_data = Global::get<devils_engine::sound::sound_container>()->get(info.id);
      if (new_sound_data == nullptr) return false;
      
      std::unique_lock<std::mutex> lock(mutex);
//       const size_t index = find(info.ent, info.id);
//       if (index != SIZE_MAX) return false;
      
      if (free_sources.empty()) return false;
      
      sounds.push_back({
        info,
        new_sound_data,
        0,
        0.0f,
        free_sources.back(),
        free_buffers.back()
      });
      
      free_sources.pop_back();
      free_buffers.pop_back();
      
      sounds.back().source.relative(info.ent == nullptr); // если еще не задана конкретная позиция
      sounds.back().source.setPitch(1.0f); // случайная величина
      sounds.back().source.setGain(std::min(master_v * sounds_v, master_v * info.volume));
      sounds.back().source.setMaxDist(info.max_dist);
      sounds.back().source.setMaxGain(master_v * sounds_v);
      sounds.back().source.setRefDist(info.ref_dist);
      sounds.back().source.setRolloff(info.rolloff);
      
      if (info.ent == nullptr) {
        sounds.back().source.setPos(glm::vec3(0.0f, 0.0f, 0.0f));
        sounds.back().source.setDir(glm::vec3(0.0f, 1.0f, 0.0f));
        sounds.back().source.setVel(glm::vec3(0.0f, 0.0f, 0.0f));
      }
      
      // нужно ли проверять имеется ли этот звук уже?
      // точнее эта пара звук + источник?
      // несколько раз подряд вызваная функция с одинаковым источником и звуком - это скорее баг
      // по факту мне особо не имеет смысла держать звуков больше чем free_sources.size()
      // 
      
      return true;
    }
    
    bool sound::stop(const yacs::entity* ent) {
      std::unique_lock<std::mutex> lock(mutex);
      const size_t index = find(ent);
      if (index == SIZE_MAX) return false;
      
      if (sounds[index].source.isValid()) {
        free_sources.push_back(sounds[index].source);
        free_buffers.push_back(sounds[index].buffers);
        sounds[index].source = Source();
      }
      std::swap(sounds[index], sounds.back());
      sounds.pop_back();
      return true;
    }
    
    bool sound::start_music(const utils::id &id) {
      return music.play(id);
    }
    
    void sound::stop_music() {
      music.stop();
    }
    
    void sound::pause_sounds() {
      music.pause();
      
      for (auto &sound : sounds) {
        sound.source.pause();
      }
    }
    
    void sound::resume_sounds() {
      music.source.play();
      
      for (auto &sound : sounds) {
        sound.source.play();
      }
    }
    
    void sound::update_listener(const yacs::entity* ent) {
      auto trans = ent->at<TransformComponent>(game::entity::transform);
      auto phys = ent->at<PhysicsComponent>(game::entity::physics);
      
      {
        float arr[4];
        trans->pos().storeu(arr);
        Listener::setPos(arr);
      }
      
      {
        float arr[4];
        phys->getVelocity().storeu(arr);
        Listener::setVel(arr);
      }
      
      {
        float arr1[4];
        float arr2[4];
        trans->rot().storeu(arr1);
        auto vec = -PhysicsEngine::getGravityNorm();
        vec.storeu(arr2);
        Listener::setOrientation(glm::vec3(arr1[0], arr1[1], arr1[2]), glm::vec3(arr2[0], arr2[1], arr2[2]));
      }
    }
    
    struct QueueLess {
      bool operator()(const sound::queued_sound &left, const sound::queued_sound &right) const {
        return left.priority < right.priority;
      }
    };
    
    void sound::update(const size_t &time) {
      (void)time;
      
      // синхронизация? скорее всего потребуется
      std::unique_lock<std::mutex> lock(mutex);
      
      // sound.user_info.ent всегда определен? нет
      // мне нужно еще сделать звук в определенном месте (без привязки к энтити)
      // и звук ровно в точке персонажа (например звуки меню, но еще это пригодится при подборе предмета)
      // в точке персонажа мы можем сделать звук передав собственно указатель персонажа
      // да но нам теперь нужно будет делить звуки от персонажа в мультиплеере от звуков подбора предмета
      // nullptr - хороший выход
      // нужно сделать отдельную функцию для звуков меню
      // звук от энтити который вот вот удалиться, должен ли он прекращаться?
      // думаю что нет, но тогда нужно как то отделить звук который пришел с nullptr 
      // от того который пришел с энтити, было бы проще если бы я выдавал сорсы сразу при добавлении в очередь
      // в думе сразу выдается канал
      
      auto pos = Listener::getPos();
      auto lpos = simd::vec4(pos.x, pos.y, pos.z, 1.0f);
      for (size_t i = 0; i < sounds.size(); ++i) {
        if (core::deleted_state(sounds[i].user_info.ent)) {
          sounds[i].user_info.ent = nullptr;
        }
        
        sounds[i].update_buffers();
        sounds[i].update_source(master_v, sounds_v);
        
        if (sounds[i].user_info.ent != nullptr) {
          auto trans = sounds[i].user_info.ent->at<TransformComponent>(game::entity::transform);
          const float dist = simd::distance(lpos, trans->pos());
          sounds[i].priority = dist / sounds[i].user_info.max_dist;
          if (sounds[i].priority >= 1.0f) {
            remove(i);
            --i;
            continue;
          }
        }
        
        if (sounds[i].source.isValid() && sounds[i].loaded_size >= sounds[i].sound_data->pcm_size && sounds[i].source.processedBuffers() > 1) {
          remove(i);
          --i;
        }
      }
      
      // сортировка не нужна
      //std::sort(sounds.begin(), sounds.end(), QueueLess());
      
//       for (size_t i = 0; i < sounds.size(); ++i) {
//         if (sounds[i].priority >= 1.0f) {
//           remove(i);
//           --i;
//           continue;
//         }
//         
//         if (sounds[i].source.isValid() && sounds[i].loaded_size >= sounds[i].sound_data->pcm_size && sounds[i].source.processedBuffers() > 1) {
//           remove(i);
//           --i;
//         }
//       }
//       
//       for (auto &sound : sounds) {
//         if (!sound.source.isValid()) {
//           sound.source = free_sources.back();
//           sound.buffers = free_buffers.back();
//           free_sources.pop_back();
//           free_buffers.pop_back();
//           sound.update_buffers();
//           sound.update_source(master_v, sounds_v);
//           sound.source.play();
//         }
//       }
    }
    
    float sound::master_volume() {
      return master_v;
    }
    
    void  sound::master_volume(const float &value) {
      master_v = value;
      music.source.setGain(master_v * music_v);
    }
    
    float sound::music_volume() {
      return music_v;
    }
    
    void  sound::music_volume(const float &value) {
      music_v = value;
      music.source.setGain(master_v * music_v);
    }
    
    float sound::menu_volume() {
      return menu_v;
    }
    
    void  sound::menu_volume(const float &value) {
      menu_v = value;
    }
    
    float sound::sounds_volume() {
      return sounds_v;
    }
    
    void  sound::sounds_volume(const float &value) {
      sounds_v = value;
    }
    
    void sound::queued_sound::update_source(const float &master_v, const float &sounds_v) {
      if (!source.isValid()) return;

//       source.relative(false);
//       source.setPitch(1.0f);
      source.setGain(std::min(master_v * sounds_v, master_v * user_info.volume));
//       source.setMaxDist(user_info.max_dist);
//       source.setMaxGain(master_v * sounds_v);
//       source.setRefDist(user_info.ref_dist);
//       source.setRolloff(user_info.rolloff);

      if (user_info.ent == nullptr) return;
      
      auto trans = user_info.ent->at<TransformComponent>(game::entity::transform);
      auto phys = user_info.ent->at<PhysicsComponent>(game::entity::physics);
      
      float pos[4], dir[4], vel[4];
      trans->pos().storeu(pos);
      trans->rot().storeu(dir);
      phys->getVelocity().storeu(vel);
      
      source.setPos(pos);
      source.setDir(dir);
      source.setVel(vel);
    }
    
    void sound::queued_sound::update_buffers() {
      if (!source.isValid()) return;
      
      if (source.queuedBuffers() == 0) {
        ASSERT(sound_data != nullptr);
        const size_t size = sound_data->type == devils_engine::sound::type::pcm ? ((sound_data->pcm_size - loaded_size) / 2) + 1 : secondToPCMSize(SOUND_LOADING_COEFFICIENT, sound_data);
        loaded_size += sound_data->load(loaded_size, size, buffers.first);
        loaded_size += sound_data->load(loaded_size, size, buffers.second);
        Buffer b[2] = {buffers.first, buffers.second};
        source.queueBuffers(2, b);
        return;
      }
      
      if (source.processedBuffers() > 0 && loaded_size < sound_data->pcm_size) {
        ASSERT(sound_data != nullptr);
        Buffer buf;
        source.unqueueBuffers(1, &buf);
        const size_t size = secondToPCMSize(SOUND_LOADING_COEFFICIENT, sound_data);
        loaded_size += sound_data->load(loaded_size, size, buf);
        source.queueBuffers(1, &buf);
      }
    }
    
    void sound::queued_sound::queue_buffers() {
      if (!source.isValid()) return;
      Buffer b[2] = {buffers.first, buffers.second};
      source.queueBuffers(2, b);
    }
    
    float sound::queued_sound::playing_position() const {
      if (!source.isValid()) return 1.0f;

      const size_t loaded = loaded_size;
      if (loaded == sound_data->pcm_size && source.processedBuffers() == 2) return 1.0f;

      const size_t sample = source.sampleOffsetInt();
      const size_t bufferSamples1 = bytesToPCMFrames(buffers.first.size(), sound_data);
      const size_t bufferSamples2 = bytesToPCMFrames(buffers.second.size(), sound_data);

    //   const size_t finalByte = byte > bufferSize1 ? byte - bufferSize1 : byte;

      const size_t diff = bufferSamples2+bufferSamples1 - sample;

      const size_t currentSoundSample = loaded - diff;

      return float(currentSoundSample) / float(sound_data->pcmSize());
    }
    
    bool sound::music_data::play(const utils::id &id) {
      if (sound_data != nullptr && (!id.valid() || id == sound_data->id)) {
        source.play();
        return true;
      }
      
      auto new_sound_data = Global::get<devils_engine::sound::sound_container>()->get(id);
      if (new_sound_data == nullptr) return false;
      
      loaded_size = 0;
      
      bool playing = false;
      if (source.state() == Source::State::playing) {
        playing = true;
        source.stop();
        source.buffer(0);
      }
      
      sound_data = new_sound_data;
      const size_t size = sound_data->type == devils_engine::sound::type::pcm ? ((sound_data->pcm_size - loaded_size) / 2) + 1 : secondToPCMSize(SOUND_LOADING_COEFFICIENT, sound_data);
      loaded_size += sound_data->load(loaded_size, size, buffers.first);
      loaded_size += sound_data->load(loaded_size, size, buffers.second);
      Buffer b[2] = {buffers.first, buffers.second};
      source.queueBuffers(2, b);
      if (playing) source.play();
    }
    
    void sound::music_data::pause() {
      source.pause();
    }
    
    void sound::music_data::stop() {
      source.stop();
      loaded_size = 0;
    }
    
    Source::State sound::music_data::state() const {
      return source.state();
    }
    
    size_t sound::music_data::duration() const {
      return sound_data != nullptr ? size_t(float(sound_data->pcm_size) / float(sound_data->sample_rate * sound_data->channels.count()) * 1000000) : 0;
    }
    
    float sound::music_data::position() const {
      // нужно взять что мы загрузили, вычесть что еще не доиграли, разделить с общим количеством
      //const size_t loaded = pcmFramesToBytes(soundtrack.loadedSize, soundtrack.sound);
      const size_t loaded = loaded_size;
      
      if (loaded == sound_data->pcm_size && source.processedBuffers() == 2) return 1.0f;
      
      //const size_t byte = soundtrack.source.byteOffsetInt();
      const size_t sample = source.sampleOffsetInt();
    //   const uint32_t numProcessed = soundtrack.source.processedBuffers();
      const size_t bufferSamples1 = bytesToPCMFrames(buffers.first.size(), sound_data);
      const size_t bufferSamples2 = bytesToPCMFrames(buffers.second.size(), sound_data);
      
    //   const size_t finalByte = byte > bufferSize1 ? byte - bufferSize1 : byte;

      ASSERT(bufferSamples2+bufferSamples1 > sample);
      
      const size_t diff = bufferSamples2+bufferSamples1 - sample;
      
      const size_t currentSoundSample = loaded - diff;
      
      return float(currentSoundSample) / float(sound_data->pcm_size);
    }
    
    void sound::music_data::set_position(const float &pos) {
      ASSERT(pos >= 0.0f);
      
      if (pos >= 1.0f) {
        loaded_size = sound_data->pcm_size;
        
        if (source.state() == Source::State::playing) {
          source.stop();
          source.buffer(0);
        }
        
        return;
      }
      
      const size_t pcmPos = size_t(float(sound_data->pcm_size) * pos);
      loaded_size = pcmPos;
      
      bool playing = false;
      if (source.state() == Source::State::playing) {
        playing = true;
        source.stop();
        source.buffer(0);
      }

      const size_t size = sound_data->type == devils_engine::sound::type::pcm ? ((sound_data->pcm_size - loaded_size) / 2) + 1 : secondToPCMSize(SOUND_LOADING_COEFFICIENT, sound_data);
      loaded_size += sound_data->load(loaded_size, size, buffers.first);
      loaded_size += sound_data->load(loaded_size, size, buffers.second);
      Buffer b[2] = {buffers.first, buffers.second};
      source.queueBuffers(2, b);
      
      if (playing) source.play();
    }
    
    size_t sound::find(const yacs::entity* ent, const utils::id &id) const {
      for (size_t i = 0; i < sounds.size(); ++i) {
        if (sounds[i].user_info.ent == ent && sounds[i].user_info.id == id) return i;
      }
      return SIZE_MAX;
    }
    
    size_t sound::find(const yacs::entity* ent) const {
      for (size_t i = 0; i < sounds.size(); ++i) {
        if (sounds[i].user_info.ent == ent) return i;
      }
      return SIZE_MAX;
    }
    
    void sound::remove(const size_t &index) {
      if (sounds[index].source.isValid()) {
        sounds[index].source.stop();
        sounds[index].source.buffer(0);
        free_sources.push_back(sounds[index].source);
        free_buffers.push_back(sounds[index].buffers);
        sounds[index].source = Source();
      }
      
      std::swap(sounds[index], sounds.back());
      sounds.pop_back();
    }
  }
}
