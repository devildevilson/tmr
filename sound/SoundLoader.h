#ifndef SOUND_LOADER_H
#define SOUND_LOADER_H

#include "ResourceID.h"
#include "Loader.h"
#include "ResourceParser.h"
#include "MemoryPool.h"
#include "SoundData.h"
#include "Resource.h"

#include <cstdint>
#include <cstddef>
#include <vector>
#include <unordered_map> // ??

class SoundResource : public Resource {
public:
  struct CreateInfo {
    Resource::CreateInfo resInfo;

    bool cached;
    bool forcedMono;
    SupportedSoundType type;
    size_t pcmSize;
  };
  SoundResource(const CreateInfo &info);

  bool cached() const;
  bool forcedMono() const;
  SupportedSoundType soundType() const;
  size_t pcmSize() const;
private:
  bool cachedVar;
  bool forcedMonoVar;
  SupportedSoundType type;
  size_t pcmSizeVar;
};

// походу в текущем виде я не смогу адекватно разделить Loader и ResourceParser
// но скоре евсего смогу разделить загрузку от контейнера
class SoundLoader : public Loader, public ResourceParser {
public:
  SoundLoader();
  ~SoundLoader();

  struct LoadData {
    std::string name;
    std::string path;
    bool forcedMono;
    bool cached;
  };
  void load(const LoadData &data);

  const SoundData* getSound(const ResourceID &res) const;

  bool canParse(const std::string &key) const override;

  bool parse(const Modification* mod,
             const std::string &pathPrefix,
             const nlohmann::json &data,
             std::vector<Resource*> &resource,
             std::vector<ErrorDesc> &errors,
             std::vector<WarningDesc> &warnings) override;

  bool forget(const ResourceID &name) override;

  Resource* getParsedResource(const ResourceID &id) override;
  const Resource* getParsedResource(const ResourceID &id) const override;

  bool load(const ModificationParser* modifications, const Resource* resource) override;
  bool unload(const ResourceID &id) override;
  void end() override;

  void clear() override;

  size_t overallState() const override;
  size_t loadingState() const override;
  std::string hint() const override;
private:
  struct ParsingData {
    ~ParsingData();

    MemoryPool<SoundResource, sizeof(SoundResource)*50> resourcePool;
    std::vector<SoundResource*> resources;
  };

  ParsingData* parsingData;

  MemoryPool<SoundData, sizeof(SoundData)*50> soundsPool;
  std::vector<SoundData*> sounds;
};

#endif //SOUND_LOADER_H
