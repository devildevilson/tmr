#ifndef TEXTURE_LOADER_H
#define TEXTURE_LOADER_H

#include "Loader.h"
#include "ImageResourceContainer.h"
#include "RenderStructures.h"
#include "Manager.h"

#include "MemoryPool.h"

#include <vector>
#include <string>
#include <unordered_map>

#define DEFAULT_TEXTURE_DATA_COUNT 100

#define TEXTURE_MAX_LAYER_COUNT 2048

namespace std {
  template<>
  struct hash<VkExtent2D> {
    size_t operator() (const VkExtent2D &extent) const {
      return (extent.width + extent.height) * (extent.width + extent.height + 1) / 2 + extent.height;
    }
  };
  
  template<>
  struct equal_to<VkExtent2D> {
    bool operator() (const VkExtent2D &right, const VkExtent2D &left) const {
      return right.width == left.width && right.height == left.height;
    }
  };
}

class TextureContainer {
public:
  virtual ~TextureContainer() {}
  
  // в принципе можно здесь подсчитывать количество памяти
  virtual size_t hostSize() const = 0;
  virtual size_t deviceSize() const = 0;
  
  virtual std::vector<Texture> getTextures(const std::string &name) const = 0;
  virtual Texture getTexture(const std::string &name, const uint32_t &index) const = 0;
  // тут еще нужно разобраться с сэмплерами
};

class TextureLoader : public Loader, public ImageResourceContainer, public TextureContainer, public ResourceParser {
public:
  // по идее это нам тоже не особ нужно после загрузки
  // так как все эти данные мы должны восстановить из json'а
  struct LoadingData {
    std::string name;
    std::string path;
    std::string sampler;
    uint32_t rows;
    uint32_t columns;
    uint32_t count;
    VkExtent3D size;
    
    // что еще?
  };
  
  struct Data {
    ~Data();
    
    MemoryPool<Resource, sizeof(Resource)> resourcePool;
    MemoryPool<Conflict, sizeof(Conflict)> conflictPool;
    MemoryPool<LoadingData, sizeof(LoadingData)> textureDataPool;
    
//     std::vector<Resource*> resources;
//     std::vector<Conflict*> conflicts;
//     std::vector<TextureData*> textureDatas;
    
    // по идее нам не особ нужен мемори пул,
    // хотя нет нужен
    
    // нужно ли тут именовать эти данные?
    std::unordered_map<std::string, Resource*> resources;
    std::unordered_map<std::string, Conflict*> conflicts;
    std::unordered_map<std::string, LoadingData*> textureDatas;
    
    std::vector<std::string> load;
    std::vector<std::string> unload;
    
    std::unordered_map<VkExtent2D, uint32_t> layerCount;
    
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
  
  TextureLoader(const CreateInfo &info);
  ~TextureLoader();
  
  // тут еще нужно сделать возможность пересобрать новые данные о текстуре
  // то есть у нас есть Texture в котором записаны данные об хранении текстуры в одном блоке текстур в памяти компа
  // и при каждой перезагрузки нам нужно передать изменения, как это сделать?
  
  bool parse(const std::string &pathPrefix, 
             const std::string &forcedNamePrefix, 
             const nlohmann::json &data, 
             std::vector<Resource*> &resources, 
             std::vector<ErrorDesc> &errors, 
             std::vector<WarningDesc> &warnings) override;
  bool forget(const std::string &name) override;

  std::unordered_map<std::string, Resource*> getLoadedResource() override;
  std::unordered_map<std::string, Conflict*> getConflicts() override;
  
  bool load(const std::string &name) override;
  bool unload(const std::string &name) override;
  void end() override;
  
  void clear() override;
  
  size_t overallState() const override;
  size_t loadingState() const override;
  std::string hint() const override;
  
  size_t hostSize() const override;
  size_t deviceSize() const override;
  
  std::vector<Texture> getTextures(const std::string &name) const override;
  Texture getTexture(const std::string &name, const uint32_t &index) const override;
  
  uint32_t imageCount() const override;
  yavf::DescriptorSet* imageDescriptor() const override;
  yavf::DescriptorSetLayout imageSetLayout() const override;
  
  uint32_t samplerCount() const override;
  yavf::DescriptorSet* samplerDescriptor() const override;
  yavf::DescriptorSetLayout samplerSetLayout() const override;
private:
  struct ImageArrayData {
    yavf::Image* texture;
    uint32_t currentLayer;
    uint32_t descriptorIndex;
    // тут наверное еще нужно указать где хранится эта картинка
    // хотя мы можем это узнать неявно из texture при наличие ptr()
  };
  
  // тут нужно удалить Resource'ы после конца загрузки
  
  //MemoryPool<Resource, DEFAULT_RESOURCE_COUNT*sizeof(Resource)> resourcePool;
  //MemoryPool<TextureData, DEFAULT_TEXTURE_DATA_COUNT*sizeof(TextureData)> resourcePool;
  
  // неплохой вариант сохранить данные после загрузки для дебага например или еще чего нибудь
  Data* data;
  
  std::unordered_map<std::string, std::vector<Texture>> textureMap;
  std::unordered_map<std::string, uint32_t> samplerMap;
  std::unordered_map<VkExtent2D, std::vector<size_t>> arrayIndices;
  std::vector<ImageArrayData> arrays;
  
  yavf::Device* device;
  
  yavf::Sampler sampler;
  
  yavf::DescriptorPool pool;
  
  uint32_t imagesCount;
  uint32_t samplersCount;
  yavf::DescriptorSet* images;
  yavf::DescriptorSet* samplers;
  yavf::DescriptorSetLayout imageLayout;
  yavf::DescriptorSetLayout samplerLayout;
  
  void createData();
  
  void recreateDescriptorPool();
};

#endif
