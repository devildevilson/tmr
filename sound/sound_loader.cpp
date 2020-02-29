#include "sound_loader.h"

#include <fstream>
#include "filesystem/path.h"

#include "SoundDecoder.h"

namespace devils_engine {
  namespace resources {
    sound_loader::sound_loader(const create_info &info) : parser("sounds"), container(info.container) {}
    
    bool sound_loader::validate(utils::problem_container<info::error> &errors, utils::problem_container<info::warning> &warnings) const {
      (void)errors;
      (void)warnings;
      return true;
    }
    
    bool sound_loader::load(const utils::id &id) {
      {
        auto s = container->get(id);
        if (s != nullptr) return true;
      }
      
      auto ptr = loading_data.get(id);
      if (ptr == nullptr) return false;
      
      std::ifstream soundFile(ptr->path);
      if (!soundFile) {
        // этого по идее быть не должно
        throw std::runtime_error("Could not load sound file " + ptr->path);
      }

      soundFile.seekg(0, std::ifstream::end);
      const size_t fileSize = soundFile.tellg();
      soundFile.seekg(0, std::ifstream::beg);

      char* fileContainer = new char[fileSize];

      soundFile.read(fileContainer, fileSize);
      
      SoundDecoderInterface* sint = nullptr;
      switch (ptr->type) {
        case sound::type::flac: {
          sint = new FLACDecoder(fileContainer, fileSize, ptr->forced_mono ? 1 : 0, ptr->pcm_size);
          break;
        }
        case sound::type::mp3: {
          sint = new MP3Decoder(fileContainer, fileSize, ptr->forced_mono ? 1 : 0, ptr->pcm_size);
          break;
        }
        case sound::type::wav: {
          sint = new WAVDecoder(fileContainer, fileSize, ptr->forced_mono ? 1 : 0, ptr->pcm_size);
          break;
        }
        case sound::type::ogg: {
          sint = new OGGDecoder(fileContainer, fileSize, ptr->forced_mono ? 1 : 0, ptr->pcm_size);
          break;
        }
        default: throw std::runtime_error("Wrong sound type");
      }

      char* cachedPcm = nullptr;
      size_t sampleSize = ptr->pcm_size*sint->channels()*sint->bitsPerChannel()/8;
      if (ptr->cached) {
        cachedPcm = new char[sampleSize];
        sint->seek(0);
        const size_t redSize = sint->getSamples(cachedPcm, ptr->pcm_size);
        if (redSize != ptr->pcm_size) {
          std::cout << "pcm size " << ptr->pcm_size << " red size " << redSize << '\n';
          throw std::runtime_error("Bad size");
        }
      }
      
      auto sound = container->create(ptr->id);
      sound->id = ptr->id;
      sound->type = ptr->cached ? sound::type::pcm : ptr->type;
      sound->channels = sound::channels_data(sint->channels(), ptr->forced_mono);
      sound->sample_rate = sint->sampleRate();
      sound->bits_per_sample = sint->bitsPerChannel();
      sound->memory = ptr->cached ? cachedPcm : fileContainer;
      sound->size = ptr->cached ? fileSize : sampleSize;
      sound->pcm_size = ptr->pcm_size;
      
      delete sint;
      if (ptr->cached) delete [] fileContainer;

      return true;
    }
    
    bool sound_loader::unload(const utils::id &id) {
      return container->destroy(id);
    }
    
    void sound_loader::end() {
      
    }
    
    void sound_loader::clear() {
      
    }
    
    utils::id sound_loader::check_json(const std::string &path_prefix, const std::string &file, const nlohmann::json &data, const size_t &mark, sound::load_data& info, utils::problem_container<info::error> &errors, utils::problem_container<info::warning> &warnings) const {
      sound::type soundType = sound::type::count;
      size_t fileSize = 0;
      char* fileContainer = nullptr;
      SoundDecoderInterface* decoderInterface = nullptr;
      
      info.forced_mono = false;
      info.cached = false;
      info.pcm_size = 0;
      
      const size_t errors_count = errors.size();

      for (auto concreteTIt = data.begin(); concreteTIt != data.end(); ++concreteTIt) {
        if (concreteTIt.value().is_string() && concreteTIt.key() == "id") {
          info.id = utils::id::get(concreteTIt.value().get<std::string>());
          continue;
        }

        if (concreteTIt.value().is_string() && concreteTIt.key() == "path") {
          const std::string &tmp = concreteTIt.value().get<std::string>();

          filesystem::path parsing(tmp);
          const auto &ext = parsing.extension();

          if (ext == "mp3") {
            soundType = sound::type::mp3;
          } else if (ext == "wav") {
            soundType = sound::type::wav;
          } else if (ext == "ogg") {
            soundType = sound::type::ogg;
          } else if (ext == "flac") {
            soundType = sound::type::flac;
          } else {
            soundType = sound::type::count;
          }

          if (soundType == sound::type::flac) {
            warnings.add(mark, FLAC_TYPE_MAY_LOSE_PRECISION, "Decoder internally converts FLAC type to float32 type, that may decrease sound quality");
          }

          if (soundType == sound::type::count) {
            errors.add(mark, SOUND_TYPE_IS_NOT_SUPPORTED, "Sound type " + ext + " is not supported");
            return utils::id();
          }

          const std::string full_path = path_prefix+tmp;
          std::ifstream file(full_path);
          if (!file) {
            errors.add(mark, COULD_NOT_LOAD_FILE, "Could not load file. File: " + full_path);
            return utils::id();
          }

          file.seekg(0, std::ifstream::end);
          fileSize = file.tellg();
          file.seekg(0, std::ifstream::beg);

          fileContainer = new char[fileSize];

          file.read(fileContainer, fileSize);

          try {
            if (soundType == sound::type::mp3) {
              decoderInterface = new MP3Decoder(fileContainer, fileSize, 0, 0);
            } else if (soundType == sound::type::ogg) {
              decoderInterface = new OGGDecoder(fileContainer, fileSize, 0, 0);
            } else if (soundType == sound::type::wav) {
              decoderInterface = new WAVDecoder(fileContainer, fileSize, 0, 0);
            } else if (soundType == sound::type::flac) {
              decoderInterface = new FLACDecoder(fileContainer, fileSize, 0, 0);
            }
          } catch (const std::runtime_error &e) {
            //delete decoderInterface;
            delete [] fileContainer;
            errors.add(mark, COULD_NOT_DECODE_FILE, std::string(e.what()) + ". File: "+full_path);
            return utils::id();
          }

          info.path = full_path;

          continue;
        }

        if (concreteTIt.value().is_boolean() && concreteTIt.key() == "force_mono") {
          info.forced_mono = concreteTIt.value().get<bool>();
          continue;
        }

        if (concreteTIt.value().is_boolean() && concreteTIt.key() == "cached") {
          info.cached = concreteTIt.value().get<bool>();
          continue;
        }
      }

      if (!info.id.valid()) {
        delete decoderInterface;
        delete [] fileContainer;

        errors.add(mark, MISSED_SOUND_ID, "Sound must have an id. File: " + path_prefix + file);
        return utils::id();
      }
      
      info.gpu_size = 0;
      info.mem_size = info.cached ? decoderInterface->size() : fileSize;
      info.parser_mark = mark;
      info.pcm_size = decoderInterface->size();
      info.type = soundType;

      delete decoderInterface;
      delete [] fileContainer;
      
      return errors_count == errors.size() ? info.id : utils::id();
    }
  }
}
