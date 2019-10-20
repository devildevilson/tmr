#ifndef TEXTURE_LOADER_H
#define TEXTURE_LOADER_H

#include "Loader.h"
#include "ResourceParser.h"
#include "ImageResourceContainer.h"
#include "RenderStructures.h"
#include "Resource.h"

#include "MemoryPool.h"

#include <vector>
#include <string>
#include <unordered_map>

#define DEFAULT_TEXTURE_DATA_COUNT 100

#define TEXTURE_MAX_LAYER_COUNT 2048
#define IMAGE_POOL_MAX_TEXTURE_LAYER_COUNT uint32_t(512)

VK_DEFINE_HANDLE(VkDescriptorPool)

namespace yavf {
  namespace Internal {
    struct ImageInfo;
  }

  class Image;
  class Buffer;
  class Device;
  class Sampler;

  typedef VkDescriptorPool DescriptorPool;
}

class ImagePool {
public:
  static uint32_t slotsCount();

  struct ImageCreateInfo {
    Vk::Extent2D extent;
    uint32_t   mipLevels;
  };

  struct CreateInfo {
    uint32_t imageLayerCount;
    uint32_t createImages;

    yavf::Device* device;
    ImageCreateInfo imageInfo;
  };
  ImagePool(const CreateInfo &info);
  ~ImagePool();
  
  void destroy();

  Image pop();
  void pop(const size_t &size, Image* memory);
  void release(const Image &img);
  void release(const size_t &size, Image* memory);

  const yavf::Internal::ImageInfo* getInfo() const;
  yavf::Image* getImageArray(const uint32_t &index) const;
  yavf::Image* getImageArray(const Image &data) const;

  constexpr uint32_t layers() const { return imageLayerCount; }
  uint32_t freeImagesCount() const;
  uint32_t imageArraysCount() const;

  void addImageArrays(const uint32_t &count);

  void updateDescriptorData(yavf::DescriptorSet* set);
private:
  struct ImageSlot {
    yavf::Image* image;
    uint32_t slot;
  };

  const uint32_t imageLayerCount;
  yavf::Device* device;
  std::vector<ImageSlot> images;
  std::vector<Image> freePlaces;

  // как быть с дескриптором? мне нужны сторого определенные, но не последовательные слоты для изображений
  // статик переменная? это достаточно простой вариант
  static uint32_t descriptorSlot;
};

namespace std {
  template<>
  struct hash<Vk::Extent2D> {
    size_t operator() (const Vk::Extent2D &extent) const {
      return (extent.width + extent.height) * (extent.width + extent.height + 1) / 2 + extent.height;
    }
  };
  
  template<>
  struct equal_to<Vk::Extent2D> {
    bool operator() (const Vk::Extent2D &right, const Vk::Extent2D &left) const {
      return right.width == left.width && right.height == left.height;
    }
  };
}

bool operator==(const Vk::Extent2D &right, const Vk::Extent2D &left);

// по size и mipLevels мы можем узнать к какому пулу принадлежит изображение
struct ImageContainerData {
  ResourceID id;
  Image* images;
  size_t count;
  Vk::Extent2D size;
  uint32_t mipLevels;
};

struct SamplerContainerData {
  ResourceID id;
  uint32_t sampler;
};

class ImageContainer {
public:
  virtual ~ImageContainer() {}
  
  // в принципе можно здесь подсчитывать количество памяти
  virtual size_t hostSize() const = 0;
  virtual size_t deviceSize() const = 0;
  
  virtual const ImageContainerData* resourceData(const ResourceID &id) const = 0;
  virtual Image image(const ResourceID &id, const uint32_t &index) const = 0;

  virtual uint32_t sampler(const ResourceID &id) const = 0;
};

struct VulkanRelatedData {
  uint32_t imagesCount;
  uint32_t samplersCount;
  yavf::DescriptorSet* set;
  yavf::DescriptorSetLayout layout;

  yavf::DescriptorPool pool;
};

struct ImageData {
  Image image;
  Vk::Extent2D size;
  uint32_t mipLevels;
};

class ImageLoader : public Loader, public ImageResourceContainer, public ImageContainer, public ResourceParser {
public:
  class LoadingData : public Resource {
  public:
    struct CreateInfo {
      Resource::CreateInfo resInfo;

      uint32_t rows;
      uint32_t columns;
      uint32_t count;
      uint32_t mipLevelsVar;
      Vk::Extent3D size;
    };
    LoadingData(const CreateInfo &info);

    uint32_t rows() const;
    uint32_t columns() const;
    uint32_t count() const;
    uint32_t mipLevels() const;
    Vk::Extent3D imageSize() const;
  private:
    uint32_t rowsVar;
    uint32_t columnsVar;
    uint32_t countVar;
    uint32_t mipLevelsVar;
    Vk::Extent3D sizeVar;
  };
  
  struct Data {
    ~Data();
    
//    MemoryPool<Resource, sizeof(Resource)> resourcePool;
//    MemoryPool<Conflict, sizeof(Conflict)> conflictPool;
    MemoryPool<LoadingData, sizeof(LoadingData)> textureDataPool;
    
//     std::vector<Resource*> resources;
//     std::vector<Conflict*> conflicts;
//     std::vector<TextureData*> textureDatas;
    
    // по идее нам не особ нужен мемори пул,
    // хотя нет нужен
    
    // нужно ли тут именовать эти данные?
//    std::unordered_map<ResourceID, Resource*> resources;
//    std::unordered_map<std::string, Conflict*> conflicts;
    std::unordered_map<ResourceID, LoadingData*> textureDatas;
    
    //std::vector<ResourceID> load;
    //std::vector<ResourceID> unload;

    // тут еще должны быть стейджинг изображения которые и будут хранить непосредственно данные
    // или потом создать?
    std::vector<ImageContainerData> loadData;


    // я подозреваю что map можно заменить на вектор
    // и будет достаточно быстро
    std::unordered_map<Vk::Extent2D, uint32_t> layerCount;
    
    // вообще можно оставить TextureData для того чтобы проще блоы бы загружать новые уровни
    // тогда по идее не придется репарсить все моды
    // но с другой стороны это, во первых, не нужные данные, а во вторых... что во вторых? 
    // (я хотел здесб написать о том что мне все равно придется парсить json, но это не так)
    // вообще это возможно здорово упростило бы мне жизнь
    // эти данные мне нужны ТОЛЬКО в момент загрузки
  };
  
  struct CreateInfo {
    yavf::Device* device;
  };
  ImageLoader(const CreateInfo &info);
  ~ImageLoader();
  
  // тут еще нужно сделать возможность пересобрать новые данные о текстуре
  // то есть у нас есть Texture в котором записаны данные об хранении текстуры в одном блоке текстур в памяти компа
  // и при каждой перезагрузки нам нужно передать изменения, как это сделать?

  bool canParse(const std::string &key) const override;
  
  bool parse(const Modification *mod,
             const std::string &pathPrefix,
             const nlohmann::json &data, 
             std::vector<Resource*> &resources, 
             std::vector<ErrorDesc> &errors, 
             std::vector<WarningDesc> &warnings) override;
  bool forget(const ResourceID &id) override;

  Resource* getParsedResource(const ResourceID &id) override;
  const Resource* getParsedResource(const ResourceID &id) const override;
  
  bool load(const ModificationParser* modifications, const Resource* resource) override;
  bool unload(const ResourceID &id) override;
  void end() override;
  
  void clear() override;
  
  size_t overallState() const override;
  size_t loadingState() const override;
  std::string hint() const override;
  
  size_t hostSize() const override;
  size_t deviceSize() const override;

  const ImageContainerData* resourceData(const ResourceID &id) const override;
  Image image(const ResourceID &id, const uint32_t &index) const override;

  uint32_t sampler(const ResourceID &id) const override;

  uint32_t imagesCount() const override;
  uint32_t samplersCount() const override;

  yavf::DescriptorSet* resourceDescriptor() const override;
  yavf::DescriptorSetLayout resourceLayout() const override;

  // лучше наверное вынести на уровень выше
  bool needRecreatePipelines() const;
private:
//  struct ImageArrayData {
//    yavf::Image* texture;
//    uint32_t currentLayer;
//    uint32_t descriptorIndex;
//    // тут наверное еще нужно указать где хранится эта картинка
//    // хотя мы можем это узнать неявно из texture при наличие ptr()
//    // не device local текстурки сильно ограничены, поэтому это бессмысленно
//
//    size_t prevDataIndex;
//  };

//  struct CurrentSizeIndex {
//    VkExtent2D size;
//    uint32_t mipLevels;
//    size_t currentIndex;
//
//    bool equal(const VkExtent2D &otherSize, const uint32_t &otherMip) const;
//  };
  
  // тут нужно удалить Resource'ы после конца загрузки
  
  //MemoryPool<Resource, DEFAULT_RESOURCE_COUNT*sizeof(Resource)> resourcePool;
  //MemoryPool<TextureData, DEFAULT_TEXTURE_DATA_COUNT*sizeof(TextureData)> resourcePool;
  
  // неплохой вариант сохранить данные после загрузки для дебага например или еще чего нибудь
  Data* data;

  // можем ли мы и тут вектор сделать? у сэмлеров легко, текстуры?
  // часто ли у нас вообще обращение к обычным текстурам происходит? не очень
  std::vector<ImageContainerData> imagesContainer;
  std::vector<SamplerContainerData> samplersContainer;
//  std::vector<CurrentSizeIndex> currentIndices;

//  std::unordered_map<ResourceID, std::vector<Texture>> textureMap;
//  std::unordered_map<ResourceID, uint32_t> samplerMap;
//  std::unordered_map<VkExtent2D, std::vector<size_t>> arrayIndices; // это понятно для чего, но я и это заменю наверное
  std::vector<ImagePool> arrays;
  
  yavf::Device* device;

  yavf::Sampler* samplers;

  uint32_t oldImagesCount;
  VulkanRelatedData renderData;

  // у нас здесь много что поменяется, нам нужен пример способ получить информацию ОБ изображении
  // размеры, есть ли мип уровни, ??
  
  void createData();
  
  void recreateDescriptorPool();
};

#endif
