#include "SoundLoader.h"

#include "SoundDecoder.h"

#include <fstream>
#include "filesystem/path.h"

#include "shared_loaders_header.h"

enum SoundErrors {
  SOUND_TYPE_IS_NOT_SUPPORTED,
  COULD_NOT_LOAD_FILE,
  COULD_NOT_DECODE_FILE,
  MISSED_SOUND_ID
};

enum SoundWarnings {
  FLAC_TYPE_MAY_LOSE_PRECISION
};

bool checkSoundValidity(const std::string &pathPrefix, const std::string &path, const nlohmann::json &j, const size_t &mark, SoundResource::CreateInfo &sInfo, std::vector<ErrorDesc> &error, std::vector<WarningDesc> &warning) {
  bool valid = false, cached = false, forsedMono = false;
  std::string id;
  std::string soundPath;
  SupportedSoundType soundType = SOUND_TYPE_COUNT;
  size_t fileSize = 0;
  char* fileContainer = nullptr;
  SoundDecoderInterface* decoderInterface = nullptr;

  for (auto concreteTIt = j.begin(); concreteTIt != j.end(); ++concreteTIt) {
    if (concreteTIt.value().is_string() && concreteTIt.key() == "id") {
      id = concreteTIt.value().get<std::string>();
      continue;
    }

    if (concreteTIt.value().is_string() && concreteTIt.key() == "path") {
      const std::string &tmp = concreteTIt.value().get<std::string>();

      filesystem::path parsing(tmp);
      const auto &ext = parsing.extension();

      if (ext == "mp3") {
        soundType = SOUND_TYPE_MP3;
      } else if (ext == "wav") {
        soundType = SOUND_TYPE_WAV;
      } else if (ext == "ogg") {
        soundType = SOUND_TYPE_OGG;
      } else if (ext == "flac") {
        soundType = SOUND_TYPE_FLAC;
      } else {
        soundType = SOUND_TYPE_COUNT;
      }

      if (soundType == SOUND_TYPE_FLAC) {
        WarningDesc desc(mark, FLAC_TYPE_MAY_LOSE_PRECISION, "Decoder internally converts FLAC type to float32 type, that may decrease sound quality");
        warning.push_back(desc);
      }

      if (soundType == SOUND_TYPE_COUNT) {
        ErrorDesc desc(mark, SOUND_TYPE_IS_NOT_SUPPORTED, "Sound type " + ext + " is not supported");
        error.push_back(desc);
        return false;
      }

      std::ifstream file(pathPrefix+tmp);
      if (!file) {
        ErrorDesc desc(mark, COULD_NOT_LOAD_FILE, "Could not load file. File: " + pathPrefix+tmp);
        error.push_back(desc);
        return false;
      }

      file.seekg(0, std::ifstream::end);
      fileSize = file.tellg();
      file.seekg(0, std::ifstream::beg);

      fileContainer = new char[fileSize];

      file.read(fileContainer, fileSize);

      try {
        if (soundType == SOUND_TYPE_MP3) {
          decoderInterface = new MP3Decoder(fileContainer, fileSize, 0, 0);
        } else if (soundType == SOUND_TYPE_OGG) {
          decoderInterface = new OGGDecoder(fileContainer, fileSize, 0, 0);
        } else if (soundType == SOUND_TYPE_WAV) {
          decoderInterface = new WAVDecoder(fileContainer, fileSize, 0, 0);
        } else if (soundType == SOUND_TYPE_FLAC) {
          decoderInterface = new FLACDecoder(fileContainer, fileSize, 0, 0);
        }
      } catch (const std::runtime_error &e) {
        delete [] fileContainer;
        ErrorDesc desc(mark, COULD_NOT_DECODE_FILE, std::string(e.what()) + ". File: "+pathPrefix+tmp);
        error.push_back(desc);
        return false;
      }

      soundPath = tmp;

      continue;
    }

    if (concreteTIt.value().is_boolean() && concreteTIt.key() == "force_mono") {
      forsedMono = concreteTIt.value().get<bool>();
      continue;
    }

    if (concreteTIt.value().is_boolean() && concreteTIt.key() == "cached") {
      cached = concreteTIt.value().get<bool>();
      continue;
    }
  }

  if (id.empty()) {
    delete decoderInterface;
    delete [] fileContainer;

    ErrorDesc desc(mark, MISSED_SOUND_ID, "Sound must have an id. File: " + pathPrefix + path);
    error.push_back(desc);
    return false;
  }

  // нам бы тут сразу и создать
  sInfo.resInfo.resId = ResourceID::get(id);
  sInfo.resInfo.pathStr = soundPath;
  sInfo.resInfo.resGPUSize = 0;
  sInfo.resInfo.resSize = cached ? decoderInterface->size() : fileSize;
  sInfo.forcedMono = forsedMono;
  sInfo.cached = cached;
  sInfo.type = soundType;
  sInfo.pcmSize = decoderInterface->size();

  delete decoderInterface;
  delete [] fileContainer;
}

SoundResource::SoundResource(const CreateInfo &info) : Resource(info.resInfo), cachedVar(info.cached), forcedMonoVar(info.forcedMono) {}

bool SoundResource::cached() const {
  return cachedVar;
}

bool SoundResource::forcedMono() const {
  return forcedMonoVar;
}

SupportedSoundType SoundResource::soundType() const {
  return type;
}

size_t SoundResource::pcmSize() const {
  return pcmSizeVar;
}

SoundLoader::SoundLoader() {}
SoundLoader::~SoundLoader() {
  clear();

  for (auto sound : sounds) {
    soundsPool.deleteElement(sound);
  }
}

void SoundLoader::load(const LoadData &data) {
  const ResourceID resId = ResourceID::get(data.name);

  for (const auto sound : sounds) {
    if (sound->id() == resId) return;
  }

  filesystem::path parsing(data.path);
  const auto &ext = parsing.extension();
  SupportedSoundType soundType = SOUND_TYPE_COUNT;

  if (ext == "mp3") {
    soundType = SOUND_TYPE_MP3;
  } else if (ext == "wav") {
    soundType = SOUND_TYPE_WAV;
  } else if (ext == "ogg") {
    soundType = SOUND_TYPE_OGG;
  } else if (ext == "flac") {
    soundType = SOUND_TYPE_FLAC;
  } else {
    soundType = SOUND_TYPE_COUNT;
  }

  if (soundType == SOUND_TYPE_FLAC) {
    std::cout << "Decoder internally converts FLAC type to float32 type, that may decrease sound quality" << "\n";
  }

  if (soundType == SOUND_TYPE_COUNT) {
    throw std::runtime_error("Sound type " + ext + " is not supported");
  }

  std::ifstream file(data.path);
  if (!file) {
    throw std::runtime_error("Could not load file. File: " + data.path);
  }

  file.seekg(0, std::ifstream::end);
  const size_t fileSize = file.tellg();
  file.seekg(0, std::ifstream::beg);

  char* fileContainer = new char[fileSize];

  file.read(fileContainer, fileSize);

  SoundDecoderInterface* decoderInterface = nullptr;
  if (soundType == SOUND_TYPE_MP3) {
    decoderInterface = new MP3Decoder(fileContainer, fileSize, 0, 0);
  } else if (soundType == SOUND_TYPE_OGG) {
    decoderInterface = new OGGDecoder(fileContainer, fileSize, 0, 0);
  } else if (soundType == SOUND_TYPE_WAV) {
    decoderInterface = new WAVDecoder(fileContainer, fileSize, 0, 0);
  } else if (soundType == SOUND_TYPE_FLAC) {
    decoderInterface = new FLACDecoder(fileContainer, fileSize, 0, 0);
  }

  char* cachedPcm = nullptr;
  size_t sampleSize = decoderInterface->size()*decoderInterface->channels()*decoderInterface->bitsPerChannel()/8;
  if (data.cached) {
    cachedPcm = new char[sampleSize];
    decoderInterface->seek(0);
    const size_t redSize = decoderInterface->getSamples(cachedPcm, decoderInterface->size());
    if (redSize != decoderInterface->size()) {
      std::cout << "pcm size " << decoderInterface->size() << " red size " << redSize << '\n';
      throw std::runtime_error("Bad size");
    }
  }

  const LoadedSoundData sData {
    data.cached ? SOUND_TYPE_PCM : soundType,
    to_al_format(decoderInterface->channels(), decoderInterface->bitsPerChannel()),
    ChannelData(decoderInterface->channels(), data.forcedMono),
    decoderInterface->sampleRate(),
    decoderInterface->bitsPerChannel(),
    data.cached ? cachedPcm : fileContainer,
    data.cached ? fileSize : sampleSize,
    decoderInterface->size()
  };

  SoundData* sdata = soundsPool.newElement(SoundData::CreateInfo{resId, sData});
  sounds.push_back(sdata);
}

const SoundData* SoundLoader::getSound(const ResourceID &res) const {
  for (const auto sound : sounds) {
    if (sound->id() == res) return sound;
  }

  return nullptr;
}

bool SoundLoader::canParse(const std::string &key) const {
  return key == "sounds";
}

bool SoundLoader::parse(const Modification* mod,
                        const std::string &pathPrefix,
                        const nlohmann::json &data,
                        std::vector<Resource*> &resource,
                        std::vector<ErrorDesc> &errors,
                        std::vector<WarningDesc> &warnings) {
  if (!data.is_array()) return false;

  if (parsingData == nullptr) parsingData = new ParsingData;

  for (size_t i = 0; i < data.size(); ++i) {
    std::string debugPath;
    nlohmann::json json;

    if (data[i].is_object()) {
      json = data[i];
    } else if (data[i].is_string()) {
      debugPath = data[i].get<std::string>();

      std::ifstream file(pathPrefix + debugPath);
      if (!file.is_open()) {
        ErrorDesc desc(SOUND_LOADER_TYPE_ID, COULD_NOT_LOAD_FILE, "could not load file " + pathPrefix + debugPath);
        errors.push_back(desc);
        continue;
      }

      file >> json;
    }

    SoundResource::CreateInfo sInfo {
      {
        ResourceID(),
        "",
        0,
        0,
        this,
        mod
      },
      false,
      false,
      SOUND_TYPE_COUNT,
      0
    };

    if (!checkSoundValidity(pathPrefix, "", json, SOUND_LOADER_TYPE_ID, sInfo, errors, warnings)) {
      continue;
    }

    auto res = parsingData->resourcePool.newElement(sInfo);
    parsingData->resources.push_back(res);
    resource.push_back(res);
  }
}

bool SoundLoader::forget(const ResourceID &name) {
  if (parsingData == nullptr) throw std::runtime_error("Not in loading state");

  for (size_t i = 0; i < parsingData->resources.size(); ++i) {
    if (parsingData->resources[i]->id() == name) {
      parsingData->resourcePool.deleteElement(parsingData->resources[i]);
      std::swap(parsingData->resources[i], parsingData->resources.back());
      parsingData->resources.pop_back();
      break;
    }
  }
}

Resource* SoundLoader::getParsedResource(const ResourceID &id) {
  if (parsingData == nullptr) return nullptr;

  for (auto resource : parsingData->resources) {
    if (resource->id() == id) return resource;
  }

  return nullptr;
}

const Resource* SoundLoader::getParsedResource(const ResourceID &id) const {
  if (parsingData == nullptr) return nullptr;

  for (auto resource : parsingData->resources) {
    if (resource->id() == id) return resource;
  }

  return nullptr;
}

bool SoundLoader::load(const ModificationParser* modifications, const Resource* resource) {
  (void)modifications;
  if (parsingData == nullptr) throw std::runtime_error("Not in loading state");

  for (auto sound : sounds) {
    if (sound->id() == resource->id()) return true;
  }

  SoundResource* res = nullptr;
  for (auto r : parsingData->resources) {
    if (r->id() == resource->id()) {
      res = r;
      break;
    }
  }

  if (res == nullptr) return false;

  const SoundResource &localResource = *res;

  std::ifstream soundFile(localResource.path());
  if (!soundFile) {
    // этого по идее быть не должно
    throw std::runtime_error("Could not load sound file " + localResource.path());
  }

  soundFile.seekg(0, std::ifstream::end);
  const size_t fileSize = soundFile.tellg();
  soundFile.seekg(0, std::ifstream::beg);

  char* fileContainer = new char[fileSize];

  soundFile.read(fileContainer, fileSize);

  SoundDecoderInterface* sint = nullptr;
  switch (localResource.soundType()) {
    case SOUND_TYPE_FLAC: {
      sint = new FLACDecoder(fileContainer, fileSize, localResource.forcedMono() ? 1 : 0, localResource.pcmSize());
      break;
    }
    case SOUND_TYPE_MP3: {
      sint = new MP3Decoder(fileContainer, fileSize, localResource.forcedMono() ? 1 : 0, localResource.pcmSize());
      break;
    }
    case SOUND_TYPE_WAV: {
      sint = new WAVDecoder(fileContainer, fileSize, localResource.forcedMono() ? 1 : 0, localResource.pcmSize());
      break;
    }
    case SOUND_TYPE_OGG: {
      sint = new OGGDecoder(fileContainer, fileSize, localResource.forcedMono() ? 1 : 0, localResource.pcmSize());
      break;
    }
    default: throw std::runtime_error("Wrong sound type");
  }

  char* cachedPcm = nullptr;
  size_t sampleSize = localResource.pcmSize()*sint->channels()*sint->bitsPerChannel()/8;
  if (localResource.cached()) {
    cachedPcm = new char[sampleSize];
    sint->seek(0);
    const size_t redSize = sint->getSamples(cachedPcm, localResource.pcmSize());
    if (redSize != localResource.pcmSize()) {
      std::cout << "pcm size " << localResource.pcmSize() << " red size " << redSize << '\n';
      throw std::runtime_error("Bad size");
    }
  }

  const LoadedSoundData sData {
    localResource.cached() ? SOUND_TYPE_PCM : localResource.soundType(),
    to_al_format(sint->channels(), sint->bitsPerChannel()),
    ChannelData(sint->channels(), localResource.forcedMono()),
    sint->sampleRate(),
    sint->bitsPerChannel(),
    localResource.cached() ? cachedPcm : fileContainer,
    localResource.cached() ? fileSize : sampleSize,
    localResource.pcmSize()
  };

  SoundData* data = soundsPool.newElement(SoundData::CreateInfo{localResource.id(), sData});
  sounds.push_back(data);

  return true;
}

bool SoundLoader::unload(const ResourceID &id) {
  for (size_t i = 0; i < sounds.size(); ++i) {
    if (sounds[i]->id() == id) {
      soundsPool.deleteElement(sounds[i]);
      std::swap(sounds[i], sounds.back());
      sounds.pop_back();
      return true;
    }
  }

  return false;
}

void SoundLoader::end() {

}

void SoundLoader::clear() {
  if (parsingData != nullptr) {
    delete parsingData;
    parsingData = nullptr;
  }
}

size_t SoundLoader::overallState() const {

}

size_t SoundLoader::loadingState() const {

}

std::string SoundLoader::hint() const {

}

SoundLoader::ParsingData::~ParsingData() {
  for (auto res : resources) {
    resourcePool.deleteElement(res);
  }
}