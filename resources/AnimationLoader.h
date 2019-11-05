#ifndef ANIMATION_LOADER_H
#define ANIMATION_LOADER_H

#include "Type.h"
#include "Loader.h"
#include "ResourceParser.h"
#include "Resource.h"

#include "LoadingTemporaryData.h"

// class Animation {
// public:
//   struct Side {
//     Image image;
//     bool flipU;
//     bool flipV;
//   };
//   
//   
// private:
//   uint8_t animFrameSize;
//   uint32_t animFrameStart;
//   uint32_t animFrameCount;
//   // если animStart > animSize то мы идем в обратную сторону
// 
//   //size_t imageOffset;
//   Side* array; // указатель на массив начиная с оффсет
// };

class ImageLoader;

class AnimationLoader : public Loader, public ResourceParser {
public:
  enum Errors {
    ERROR_COULD_NOT_LOAD_FILE = 0,
    ERROR_BAD_FRAME_DATA,
    ERROR_BAD_FLIP_DATA,
    ERROR_SIDE_MUST_HAVE_AN_IMAGE_ID,
    ERROR_FRAME_MUST_HAVE_AT_LEAST_ONE_SIDE,
    ERROR_ANIMATION_MUST_HAVE_AN_ID,
    ERROR_EVERY_ANIMATION_FRAME_MUST_HAVE_EQUAL_NUMBER_OF_SIDES
  };
  
  class LoadData : public Resource {
  public:
    struct Side {
      ResourceID image;
      size_t index;
      bool flipU;
      bool flipV;
    };
    
    struct Frame {
      std::vector<Side> sides;
    };
    
    struct CreateInfo {
      Resource::CreateInfo resInfo;
      
      std::vector<Frame> frames;
    };
    LoadData(const CreateInfo &info);
    
    const std::vector<Frame> & frames() const;
  private:
    std::vector<Frame> m_frames;
  };
  
  static LoadData::Side parseSideString(const std::string &str);
  
  struct CreateInfo {
    const ImageLoader* imageLoader;
  };
  AnimationLoader(const CreateInfo &info);
  ~AnimationLoader();
  
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

  // ресурс будет означать подготовку entityType, который мы потом используем непосредственно для создания энтити
  bool load(const ModificationParser* modifications, const Resource* resource) override;
  bool unload(const ResourceID &id) override;
  void end() override;
  
  void clear() override;
  
  size_t overallState() const override;
  size_t loadingState() const override;
  std::string hint() const override;
  
  // сейчас все анимации у меня лежат в системе анимаций
  // вполне возможно указатель на анимацию перенести в состояние
  // пока что наверное так оставлю, как проверить что мы загрузили анимацию? getParsedResource?
  // я тут подумал, а че с конфликтами? их то как делать? понятие не имею
  // видимо придется делать в самих загрузчиках конфликты 
private:
  const ImageLoader* imageLoader;
  
  LoadingTemporaryData<LoadData, 30>* tempData;
  
  
  size_t findTempData(const ResourceID &id) const;
};

#endif
