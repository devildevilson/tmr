#include "ImageLoader.h"

#include "Modification.h"

#include "yavf.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <fstream>
#include <locale>

#include "shared_loaders_header.h"

VkOffset2D toVk(const Vk::Offset2D &offset2D) {
  return {offset2D.x, offset2D.y};
}

VkExtent2D toVk(const Vk::Extent2D &extent2D) {
  return {extent2D.width, extent2D.height};
}

VkRect2D toVk(const Vk::Rect2D &rect2D) {
  return {toVk(rect2D.offset), toVk(rect2D.extent)};
}

VkExtent3D toVk(const Vk::Extent3D &extent3D) {
  return {extent3D.width, extent3D.height, extent3D.depth};
}

bool operator==(const Vk::Extent2D &left, const Vk::Extent2D &right) {
  return left.width == right.width && left.height == right.height;
}

uint32_t ImagePool::slotsCount() {
  return descriptorSlot;
}

ImagePool::ImagePool(const CreateInfo &info) : imageLayerCount(info.imageLayerCount), device(info.device), images(info.createImages, {nullptr, 0}), freePlaces(info.imageLayerCount * info.createImages) {
  if (imageLayerCount > TEXTURE_MAX_LAYER_COUNT) throw std::runtime_error("Too many image layers");
  
  const yavf::ImageCreateInfo imgInfo = yavf::ImageCreateInfo::texture2D(toVk(info.imageInfo.extent),
                                                                         VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                                                         VK_FORMAT_R8G8B8A8_UNORM,
                                                                         imageLayerCount,
                                                                         info.imageInfo.mipLevels);

  for (uint32_t i = 0; i < images.size(); ++i) {
    auto image = device->create(imgInfo, VMA_MEMORY_USAGE_GPU_ONLY);
    image->createView(VK_IMAGE_VIEW_TYPE_2D_ARRAY, VK_IMAGE_ASPECT_COLOR_BIT);

    const uint32_t slot = descriptorSlot;
    ++descriptorSlot;
    images[i] = {image, slot};

    const uint32_t index = imageLayerCount * i;
    for (uint32_t j = 0; j < imageLayerCount; ++j) {
      freePlaces[index + j] = Image{slot, j};
    }
  }
}

ImagePool::~ImagePool() {}

void ImagePool::destroy() {
  for (auto & pair : images) {
    device->destroy(pair.image);
    pair.image = nullptr;
  }
}

Image ImagePool::pop() {
  // честно говоря бездумно создавать изображения = смерть
  // нужно постараться сделать так чтобы addImage() вызывался как можно реже
  if (freePlaces.empty()) {
    addImageArrays(1);
  }

  Image img = freePlaces.back();
  freePlaces.pop_back();
  return img;
}

void ImagePool::pop(const size_t &size, Image* memory) {
#ifdef _DEBUG
  ASSERT(size != 0);
  ASSERT(memory != nullptr);
#endif

  if (freePlaces.size() < size) {
    const size_t more = size - freePlaces.size();
    const size_t count = std::ceil(float(more) / float(imageLayerCount));
    addImageArrays(count);
  }

  const size_t place = freePlaces.size() - size;
  memcpy(memory, &freePlaces[place], sizeof(Image)*size);
  freePlaces.erase(freePlaces.begin()+place, freePlaces.end());
}

void ImagePool::release(const Image &img) {
  freePlaces.push_back(img);
}

void ImagePool::release(const size_t &size, Image* memory) {
#ifdef _DEBUG
  ASSERT(getImageArray(memory[0]) != nullptr);
  ASSERT(memory != nullptr);
#endif

  const size_t oldSize = freePlaces.size();
  freePlaces.resize(oldSize + size);
  memcpy(&freePlaces[oldSize], memory, sizeof(Image)*size);
}

const yavf::Internal::ImageInfo* ImagePool::getInfo() const {
  return &images[0].image->info();
}

yavf::Image* ImagePool::getImageArray(const uint32_t &index) const {
  return images[index].image;
}

yavf::Image* ImagePool::getImageArray(const Image &data) const {
  for (const auto &image : images) {
    if (image.slot == data.index) return image.image;
  }

  return nullptr;
}

//constexpr uint32_t ImagePool::layers() const {
//  return imageLayerCount;
//}

uint32_t ImagePool::freeImagesCount() const {
  return freePlaces.size();
}

uint32_t ImagePool::imageArraysCount() const {
  return images.size();
}

void ImagePool::addImageArrays(const uint32_t &count) {
  const VkExtent2D ext{
    getInfo()->extent.width,
    getInfo()->extent.height
  };

  const yavf::ImageCreateInfo imgInfo = yavf::ImageCreateInfo::texture2D(ext,
                                                                         VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                                                         VK_FORMAT_R8G8B8A8_UNORM,
                                                                         imageLayerCount,
                                                                         getInfo()->mipLevels);

  for (uint32_t i = 0; i < count; ++i) {
    auto image = device->create(imgInfo, VMA_MEMORY_USAGE_GPU_ONLY);
    image->createView(VK_IMAGE_VIEW_TYPE_2D_ARRAY, VK_IMAGE_ASPECT_COLOR_BIT);

    const uint32_t slot = descriptorSlot;
    ++descriptorSlot;
    images.push_back({image, slot});

    for (uint32_t j = 0; j < imageLayerCount; ++j) {
      freePlaces.push_back(Image{slot, j});
    }
  }
}

void ImagePool::updateDescriptorData(yavf::DescriptorSet* set) {
  for (const auto &image : images) {
#ifdef _DEBUG
    if (image.slot >= set->size()) throw std::runtime_error("Small set");
#endif

    set->at(image.slot).imageData.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    set->at(image.slot).imageData.imageView = image.image->view();
    set->at(image.slot).bindingNum = 0;
    set->at(image.slot).arrayElement = image.slot;
    set->at(image.slot).descType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
  }
}

uint32_t ImagePool::descriptorSlot = 0;

enum Errors {
  COULD_NOT_LOAD_FILE = 0,
  MISSED_TEXTURE_PATH,
  TOO_MUCH_TEXTURE_COUNT,
  MISSED_TEXTURE_SIZE,
  MISSED_TEXTURE_ID,
  TEXTURE_ERRORS_COUNT
};

enum Warnings {

};

bool operator==(const VkExtent2D &right, const VkExtent2D &left) {
  return right.width == left.width && right.height == left.height;
}

bool checkTextureJsonValidity(const std::string &pathPrefix, const std::string &path, const nlohmann::json &j, const size_t &mark, std::vector<ErrorDesc> &error, std::vector<WarningDesc> &warning) {
  bool valid = false, hasCount = false, hasWidth = false, hasHeight = false;
  uint32_t count = 0, rows = 0, columns = 0, width = 0, height = 0;
  std::string texturePath;
  std::string name;
  
  int32_t loadedWidth, loadedHeight;
  
  for (auto concreteTIt = j.begin(); concreteTIt != j.end(); ++concreteTIt) {
    if (concreteTIt.value().is_string() && concreteTIt.key() == "path") {
      texturePath = concreteTIt.value().get<std::string>();
      
      int32_t comp;
      int ret = stbi_info((pathPrefix + texturePath).c_str(), &loadedWidth, &loadedHeight, &comp);
      (void)ret;
//       if (!bool(ret)) throw std::runtime_error("dqsdqwfgqfwfqffqfqfwfavsavaw " + pathPrefix + texturePath);
      
      continue;
    }
    
    if (concreteTIt.value().is_string() && concreteTIt.key() == "id") {
      name = concreteTIt.value().get<std::string>();
      continue;
    }
    
    if (concreteTIt.value().is_number_unsigned() && concreteTIt.key() == "count") {
      count = concreteTIt.value().get<size_t>();
      hasCount = count > 1;
      continue;
    }
    
    if (concreteTIt.value().is_number_unsigned() && concreteTIt.key() == "rows") {
      rows = concreteTIt.value().get<size_t>();
//       hasRows = rows > 1;
      continue;
    }
    
    if (concreteTIt.value().is_number_unsigned() && concreteTIt.key() == "columns") {
      columns = concreteTIt.value().get<size_t>();
//       hasColumns = columns > 1;
      continue;
    }
    
    if (concreteTIt.value().is_object() && concreteTIt.key() == "size") {
      for (auto sizeIt = concreteTIt.value().begin(); sizeIt != concreteTIt.value().end(); ++sizeIt) {
        if (sizeIt.value().is_number_unsigned() && sizeIt.key() == "width") {
          width = sizeIt.value().get<uint32_t>();
          hasWidth = width > 0;
          continue;
        }
        
        if (sizeIt.value().is_number_unsigned() && sizeIt.key() == "height") {
          height = sizeIt.value().get<uint32_t>();
          hasHeight = height > 0;
          continue;
        }
      }
    }
  }
  
  const bool hasSize = hasWidth && hasHeight;

  if (!texturePath.empty() && !name.empty()) valid = true;
  
  if (!valid) {
    ErrorDesc desc(mark, MISSED_TEXTURE_PATH, std::string("Missed texture path.") + " File: " + path);
    // в консоль поди надо выводить?
    std::cout << "Error: " << desc.description << '\n';
    error.push_back(desc);
    return false;
  }
  
  if (hasCount && count > rows * columns) {
    ErrorDesc desc(mark, TOO_MUCH_TEXTURE_COUNT, "Texture count " + std::to_string(count) + " > rows*columns " + std::to_string(rows * columns) + ". File: " + path);
    std::cout << "Error: " << desc.description << '\n';
    error.push_back(desc);
    return false;
  }
  
  if (hasCount && !hasSize) {
    ErrorDesc desc(mark, MISSED_TEXTURE_SIZE, std::string("Missed texture size.") + " File: " + path);
    std::cout << "Error: " << desc.description << '\n';
    error.push_back(desc);
    return false;
  }
  
  if (name.empty()) {
    ErrorDesc desc(mark, MISSED_TEXTURE_ID, std::string("Resource must have an id.") + " File: " + path);
    std::cout << "Error: " << desc.description << '\n';
    error.push_back(desc);
  }
  
  return true;
}

bool getTextureData(const std::string &pathPrefix, const nlohmann::json &j, ImageLoader::LoadingData::CreateInfo &info) {
  bool valid = false, hasCount = false, hasRows = false, hasColumns = false, hasWidth = false, hasHeight = false, mipmaps = false;
  uint32_t count = 0, rows = 0, columns = 0, width = 0, height = 0;
  int32_t loadedWidth = 0, loadedHeight = 0;
  std::string texturePath;

  for (auto concreteTIt = j.begin(); concreteTIt != j.end(); ++concreteTIt) {
    if (concreteTIt.value().is_string() && concreteTIt.key() == "path") {
      texturePath = concreteTIt.value().get<std::string>();
      valid = true;

      int32_t comp;
      int ret = stbi_info((pathPrefix + texturePath).c_str(), &loadedWidth, &loadedHeight, &comp);
      (void)ret;
//       if (!bool(ret)) throw std::runtime_error("dqsdqwfgqfwfqffqfqfwfavsavaw " + pathPrefix + texturePath);

      continue;
    }

    if (concreteTIt.value().is_string() && concreteTIt.key() == "id") {
      info.resInfo.resId = ResourceID::get(concreteTIt.value().get<std::string>());
      continue;
    }

    if (concreteTIt.value().is_number_unsigned() && concreteTIt.key() == "count") {
      count = concreteTIt.value().get<size_t>();
      hasCount = count > 1;
      continue;
    }

    if (concreteTIt.value().is_number_unsigned() && concreteTIt.key() == "rows") {
      rows = concreteTIt.value().get<size_t>();
      hasRows = rows > 1;
      continue;
    }

    if (concreteTIt.value().is_number_unsigned() && concreteTIt.key() == "columns") {
      columns = concreteTIt.value().get<size_t>();
      hasColumns = columns > 1;
      continue;
    }

    if (concreteTIt.value().is_boolean() && concreteTIt.key() == "create_mipmaps") {
      mipmaps = concreteTIt.value().get<bool>();
      continue;
    }

    if (concreteTIt.value().is_object() && concreteTIt.key() == "size") {
      for (auto sizeIt = concreteTIt.value().begin(); sizeIt != concreteTIt.value().end(); ++sizeIt) {
        if (sizeIt.value().is_number_unsigned() && sizeIt.key() == "width") {
          width = sizeIt.value().get<uint32_t>();
          hasWidth = width > 0;
          continue;
        }

        if (sizeIt.value().is_number_unsigned() && sizeIt.key() == "height") {
          height = sizeIt.value().get<uint32_t>();
          hasHeight = height > 0;
          continue;
        }
      }
    }
  }

  const bool hasSize = hasWidth && hasHeight;

  if (hasSize) {
    info.resInfo.resGPUSize = width * height * (hasCount ? count : 1) * 4;
  } else {
    info.resInfo.resGPUSize = loadedWidth * loadedHeight * 4;
  }

  info.resInfo.resGPUSize += mipmaps ? info.resInfo.resGPUSize / 3 : 0;

  if (!valid) {
    std::cout << "not valid" << "\n";
    return false;
  }

  if (hasCount && count > rows * columns) {
    std::cout << "count > rows * columns" << "\n";
    return false;
  }

  if (hasCount && !hasSize) {
    std::cout << "hasCount && !hasSize" << "\n";
    return false;
  }

  if (!info.resInfo.resId.valid()) {
    std::cout << "must have an id" << "\n";
    return false;
  }

  info.size = {hasSize ? width : loadedWidth, hasSize ? height : loadedHeight, 1};
  info.count = hasCount ? count : 1;
  info.columns = columns == 0 ? 1 : columns;
  info.rows = rows == 0 ? 1 : rows;
  info.resInfo.pathStr = pathPrefix + texturePath;
  info.resInfo.resSize = 0;
  info.mipLevelsVar = mipmaps ? static_cast<uint32_t>(std::floor(std::log2(std::max(info.size.width, info.size.height)))) + 1 : 0;

  return true;
}

ImageLoader::LoadingData::LoadingData(const CreateInfo &info) : Resource(info.resInfo), rowsVar(info.rows), columnsVar(info.columns), countVar(info.count), mipLevelsVar(info.mipLevelsVar), sizeVar(info.size) {}

uint32_t ImageLoader::LoadingData::rows() const {
  return rowsVar;
}

uint32_t ImageLoader::LoadingData::columns() const {
  return columnsVar;
}

uint32_t ImageLoader::LoadingData::count() const {
  return countVar;
}

uint32_t ImageLoader::LoadingData::mipLevels() const {
  return mipLevelsVar;
}

Vk::Extent3D ImageLoader::LoadingData::imageSize() const {
  return sizeVar;
}

ImageLoader::Data::~Data() {  
  for (const auto &textureData : textureDatas) {
    textureDataPool.deleteElement(textureData.second);
  }
}

ImageLoader::ImageLoader(const CreateInfo &info) :
  data(nullptr), 
  device(info.device),
  samplers(nullptr),
  oldImagesCount(0),
  renderData{
    0,
    0,
    nullptr,
    VK_NULL_HANDLE,
    VK_NULL_HANDLE,
  } {

  renderData.samplersCount = 2;
  samplers = new yavf::Sampler[2];
  
  {
    yavf::SamplerMaker sm(device);
    
    samplers[0] = sm.addressMode(VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT)
                    .anisotropy(VK_TRUE, 16.0f)
                    .borderColor(VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK)
                    .compareOp(VK_FALSE, VK_COMPARE_OP_MAX_ENUM)
                    .filter(VK_FILTER_NEAREST, VK_FILTER_NEAREST)
                    .lod(0, 1000.0f)
                    .mipmapMode(VK_SAMPLER_MIPMAP_MODE_NEAREST)
                    .unnormalizedCoordinates(VK_FALSE)
                    .create("default_sampler");

    samplers[1] = sm.addressMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER)
                    .anisotropy(VK_TRUE, 16.0f)
                    .borderColor(VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK)
                    .compareOp(VK_FALSE, VK_COMPARE_OP_MAX_ENUM)
                    .filter(VK_FILTER_NEAREST, VK_FILTER_NEAREST)
                    .lod(0, 1000.0f)
                    .mipmapMode(VK_SAMPLER_MIPMAP_MODE_NEAREST)
                    .unnormalizedCoordinates(VK_FALSE)
                    .create("clamp_to_border_sampler");
  }

  samplersContainer = {
    {
      ResourceID::get("default_sampler"),
      0
    },
    {
      ResourceID::get("clamp_to_border_sampler"),
      1
    }
  };
}

ImageLoader::~ImageLoader() {
  if (data != nullptr) {
    delete data;
  }

  for (size_t i = 0; i < renderData.samplersCount; ++i) {
    device->destroy(samplers[i]);
  }
  
  for (auto & array : arrays) {
    array.destroy();
  }

  delete [] samplers;

  device->destroy(renderData.pool);
  device->destroy(renderData.layout);
}

bool ImageLoader::canParse(const std::string &key) const {
  return key == "texture" || key == "textures"; // у меня так и не получилось привести к нижнему регистру
}

bool ImageLoader::parse(const Modification *mod,
                        const std::string &pathPrefix,
                        const nlohmann::json &data,
                        std::vector<Resource*> &resources,
                        std::vector<ErrorDesc> &errors,
                        std::vector<WarningDesc> &warnings) {
  // чтобы сделать проверку нужно передать сюда еще и ключ

  // создать
  if (this->data == nullptr) createData();
  
  // тут мы чекаем json, вытаскиваем данные о текстурах, засовываем общие данные в resources
  
  // во первых сюда скорее всего будет приходить еще zip архив размещенный в памяти
  // во вторых мне нужно сделать парсинг разных уровней данных + данных в разных файлах
  // то есть сюда может придти как отдельное описание одной текстурки, так и массив json'ов каждый из которых обозначает текстурку
  
  // может придти объект textures: new, update
  // может придти объект с описанием текстурки (path, name, count и тд)

  // никаких массивов new и объектов update
  // использую только ресурсИД
  // сейчас у меня ресурсИД которые будут пересекаться при разных типах ресурсов
  // нужно ли сделать так чтобы они не пересекались?

  // мы можем вполне использовать объект (то есть перечисление вида id : data)

  bool validData = false;
  for (auto itr = data.begin(); itr != data.end(); ++itr) {
//    std::string name;
    std::string debugPath;
    nlohmann::json json;

    if (itr.value().is_string()) {
      // грузим новый json
      //std::string path = texturesIt.value()[tInfoIt].get<std::string>();
      debugPath = itr.value().get<std::string>();

      std::ifstream file(pathPrefix + debugPath);
      if (!file.is_open()) {
        ErrorDesc desc(IMAGE_LOADER_TYPE_ID, COULD_NOT_LOAD_FILE, "could not load file " + pathPrefix + debugPath);
        errors.push_back(desc);
        continue;
      }

      file >> json;

//                   if (!checkTextureJsonValidity(path, json, name, error, warning)) {
//                     continue;
//                   }
    } else if (itr.value().is_object()) {
      // обходим информацию о текстуре
      json = itr.value();
    }

    if (!checkTextureJsonValidity(pathPrefix, debugPath, json, IMAGE_LOADER_TYPE_ID, errors, warnings)) {
      continue;
    }

    ImageLoader::LoadingData::CreateInfo info{
      {
        ResourceID(),
        "",
        0,
        0,
        this,
        mod
      },
      0,
      0,
      0,
      0,
      {0, 0, 0}
    };
    getTextureData(pathPrefix, json, info);

    LoadingData* l = this->data->textureDataPool.newElement(info);
    this->data->textureDatas[info.resInfo.resId] = l;
    resources.push_back(l);

    std::cout << "parsed: " << info.resInfo.resId.name() << " mod: " << mod->name() << "\n";
    ASSERT(l->count() > 0);
  }
  
  return true;
}

bool ImageLoader::forget(const ResourceID &id) {
  // тут в случае когда у нас есть data мы должны забыть текстурку по имени name
  // этот метод должен вызываться в основном в том случае, если мы отменяем выбор мода
  // тогда мы должны пройтись по лоадерам и удалить мета данные из них
  // так как по этим мета данным будет потом происходить загрузка игры
  
  // по идее когда мы вызываем этот метод текстура не должна быть загружена в память
  // то есть это вызывается только в момент когда кто-то ошибся и хочет что то другое

  // скорее всего в этом нет нужды
  
  // выпиливаем ресурсы по названию
  if (data == nullptr) throw std::runtime_error("Not in loading state");
  
  auto itr = data->textureDatas.find(id);
  // по идее этого достаточно для того чтобы определить есть ли у нас такой ресурс
  if (itr == data->textureDatas.end()) return false;
  
  data->textureDataPool.deleteElement(itr->second);
  data->textureDatas.erase(itr);
  
  // из конфликтов нужно убирать не здесь
  
  return true;
}

Resource* ImageLoader::getParsedResource(const ResourceID &id) {
  if (data == nullptr) throw std::runtime_error("Not in loading state");
  auto itr = data->textureDatas.find(id);
  if (itr == data->textureDatas.end()) return nullptr;

  return itr->second;
}

const Resource* ImageLoader::getParsedResource(const ResourceID &id) const {
  if (data == nullptr) throw std::runtime_error("Not in loading state");
  auto itr = data->textureDatas.find(id);
  if (itr == data->textureDatas.end()) return nullptr;

  return itr->second;
}

bool equal(const ImagePool &pool, const Vk::Extent2D &size, const uint32_t &mipLevels) {
  return pool.getInfo()->extent.width == size.width && pool.getInfo()->extent.height == size.height && pool.getInfo()->mipLevels == mipLevels;
}

bool ImageLoader::load(const ModificationParser* modifications, const Resource* resource) {
  (void)modifications;
  // мы должны определить где данный ресурс будет располагаться
  // то есть разметить место под ресурс сгенерить информацию об изображениях
  // на этом этапе ничего не будет загружено, но уже можно брать инфу об изображениях
  
  if (data == nullptr) throw std::runtime_error("Not in loading state");
  
  if (arrays.empty()) {
    for (const auto &pair : data->textureDatas) {
      const ImagePool::CreateInfo info{
        
      };
    }
  }

  //if (resource->loaderMark() != mark()) return false;
  if (resource->parser() != this) return false;
  
  auto itr = data->textureDatas.find(resource->id());
  if (itr == data->textureDatas.end()) return false;

  const LoadingData &loadingData = *itr->second;

  // во первых проверить загружены ли мы уже
  for (const auto &loadData : data->loadData) {
    if (loadData.id == loadingData.id()) return true;
  }

  for (auto & i : imagesContainer) {
    if (i.id == loadingData.id()) return true;
  }

  const Vk::Extent2D imageSize{
    loadingData.imageSize().width,
    loadingData.imageSize().height
  };

  // возможно нам нужно загрузиться на место удаленного изображения
  // проблема в том что у меня могут быть разные количества изображений
  // правильно сделать пересоздание не выйдет, нужно чет другое придумать
  // можно создать все необходимые хост изображения, а потом в другой функции их загрузить
  // или даже загрузить на одном из этапов рендеринга
  // но нам к сожалению нужно еще удалить старое, и проследить чтобы предыдущие встали опять на свои места
  // мы можем сделать специальный пул изображений, при удалении будем в этот пул складывать,
  // при загрузке брать оттуда - идея хорошая, теперь нужно разделить полную перезагрузку
  // и подгрузку постепенно, в этом случае я добиваюсь необходимого поведения
  // но что делать если нам требуется больше изображений? здесь ничего особо и не придумаешь, кроме зависания
  // то есть по сути это будет примерно тот же мемори пул

  ASSERT(loadingData.count() > 0);
  
  ImageContainerData imageConcrete{
    loadingData.id(),
    new Image[loadingData.count()],
    loadingData.count(),
    imageSize,
    loadingData.mipLevels()
  };

//  for (size_t i = 0; i < imageConcrete.count; ++i) {
//    imageConcrete.images[i] = arrays[index].pop();
//  }

  data->loadData.push_back(imageConcrete);
  
  // ошибка в том что arrays в этот момент не инициализирован
//   size_t index = SIZE_MAX;
//   for (size_t i = 0; i < arrays.size(); ++i) {
//     if (equal(arrays[i], imageSize, loadingData.mipLevels())) {
//       index = i;
//       break;
//     }
//   }
//   
//   arrays[index].pop(imageConcrete.count, imageConcrete.images);

  // как то так мы подготавливаем данные к дальнейшему копированию
  
  return true;
}

bool ImageLoader::unload(const ResourceID &id) {
  // тут мы должны действительно отдать память обратно в пул
  // скорее всего удалять мы ничего не будем, то есть пул будет только увеличиваться
  // это может привести к проблемам, можно ли что то придумать для того чтобы исправить это?
  // в текущей реализации - вряд ли, с другой стороны если сделать вместо
  // Image некий ResourceHandle, который будет предоставлять метаинформацию об
  // изображении, то в этом случае можно придумать какой механизм для того чтобы удалять
  // не нужные изображения, решение этой проблемы может потребоваться по идее только
  // в опенворлде, поэтому пока не нужно

  bool ret = false;

  for (size_t i = 0; i < imagesContainer.size(); ++i) {
    if (id == imagesContainer[i].id) {
      ret = true;

      size_t index = SIZE_MAX;
      for (size_t j = 0; j < arrays.size(); ++j) {
        if (equal(arrays[j], imagesContainer[i].size, imagesContainer[i].mipLevels)) {
          index = j;
          break;
        }
      }

//      for (size_t i = 0; i < imagesContainer[i].count; ++i) {
//        arrays[index].release(imagesContainer[i].images[i]);
//      }

      arrays[index].release(imagesContainer[i].count, imagesContainer[i].images);

      delete [] imagesContainer[i].images;
      std::swap(imagesContainer[i], imagesContainer.back());
      imagesContainer.pop_back();
    }
  }

  if (data == nullptr) return ret;

  for (size_t i = 0; i < data->loadData.size(); ++i) {
    if (id == data->loadData[i].id) {
      ret = true;

      size_t index = SIZE_MAX;
      for (size_t j = 0; j < arrays.size(); ++j) {
        if (equal(arrays[j], data->loadData[i].size, data->loadData[i].mipLevels)) {
          index = j;
          break;
        }
      }

      arrays[index].release(data->loadData[i].count, data->loadData[i].images);

      delete [] data->loadData[i].images;
      std::swap(data->loadData[i], data->loadData.back());
      data->loadData.pop_back();
    }
  }
  
  return ret;
}

yavf::Image* createImage(yavf::Device* device, const uint32_t &width, const uint32_t &height, const uint32_t &mipLevels, const uint32_t &layerCount) {
  return device->create(yavf::ImageCreateInfo::texture2D({uint32_t(width), uint32_t(height)}, 
                                                         VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
                                                         VK_FORMAT_R8G8B8A8_UNORM, 
                                                         layerCount, 
                                                         mipLevels), 
                        VMA_MEMORY_USAGE_GPU_ONLY);
}

void copy(const uint32_t &startingLayer, const uint32_t &startIndex, const uint32_t &count, const uint32_t &rows, const uint32_t &columns, const Vk::Extent3D &size, std::vector<VkImageCopy> &copies) {
  uint32_t counter = 0;
  uint32_t width = 0;
  uint32_t height = 0;
  
  //std::cout << "count " << count << "\n";
  for (uint32_t i = 0; i < rows; ++i) {
    if (counter == count) break;
    height = size.height * i;
    // std::cout << "Height: " << height << "\n";
    width = 0;

    for (uint32_t j = 0; j < columns; ++j) {
      if (counter == count) break;
      
      ++counter;
      if (counter < startIndex) continue;
      
      width = size.width * j;
      // std::cout << "Width: " << width << "\n";

      const VkImageCopy copy{
        {
          VK_IMAGE_ASPECT_COLOR_BIT,
          0, 0, 1
        },
        {int32_t(width), int32_t(height), 0},
        {
          VK_IMAGE_ASPECT_COLOR_BIT,
          0, (counter-1) + startingLayer, 1
        },
        {0, 0, 0},
        toVk(size)
      };

      copies.push_back(copy);
    }
  }
}

void copy(const size_t &imageCount, const Image* memory, const uint32_t &count, const uint32_t &rows, const uint32_t &columns, const Vk::Extent3D &size, std::vector<VkImageCopy> &copies) {
  uint32_t width = 0, height = 0, counter = 0;

  for (uint32_t i = 0; i < rows; ++i) {
    height = size.height * i;
    // std::cout << "Height: " << height << "\n";
    width = 0;

    for (uint32_t j = 0; j < columns; ++j) {
      width = size.width * j;

      if (counter == count || counter == imageCount) throw std::runtime_error("copy error");

      copies[counter] = {
        {
          VK_IMAGE_ASPECT_COLOR_BIT,
          0, 0, 1
        },
        {int32_t(width), int32_t(height), 0},
        {
          VK_IMAGE_ASPECT_COLOR_BIT,
          0, memory[counter].layer, 1
        },
        {0, 0, 0},
        toVk(size)
      };

      ++counter;
    }
  }
}

void blit(const uint32_t &mipLevels, const uint32_t &layersCount, const VkExtent3D &size, std::vector<VkImageBlit> &blits) {
#ifdef _DEBUG
  const uint32_t mipLevelsCheck = static_cast<uint32_t>(std::floor(std::log2(std::max(size.width, size.height)))) + 1;
  //  эта формула - это макисальное количество мип уровней? похоже на то
  // по такой формуле посчитаются мип уровни вплоть до 1х1 по идее
  // дольше в них нет необходимости
  
  ASSERT(mipLevelsCheck == mipLevels);
#endif

  int32_t mipWidth = size.width;
  int32_t mipHeight = size.height;
  for (uint32_t i = 1; i < mipLevels; ++i) {
    const VkImageBlit b{
      {
        VK_IMAGE_ASPECT_COLOR_BIT,
        i-1, 0, layersCount
      },
      {
        {0, 0, 0}, {mipWidth, mipHeight, 1}
      },
      {
        VK_IMAGE_ASPECT_COLOR_BIT,
        i, 0, layersCount
      },
      {
        {0, 0, 0}, {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1}
      }
    };

    blits[i-1] = b;

    if (mipWidth > 1) mipWidth /= 2;
    if (mipHeight > 1) mipHeight /= 2;
  }
}

void ImageLoader::end() {
  // именно здесь мы будем загружать/выгружать/что-угодно-делать
  // то есть нам нужно масимально компактно расположить текстурки в памяти
  
  // сюда приходит список того что нужно загрузить среди этого списка скорее всего есть то что уже загружено
  // надо это просто скопировать, а остальное загрузить
  
  // так что нам нужно тут сделать
  // нужно составить список того что я скопирую из текущих текстур - это будут как бы текстуры которые я решил оставить
  // 

  // в общем мне нужен механизм где я могу примерно прикинуть какие текстурки должны обязательно попасть
  // в девайс память, а какие могут и на хосте посидеть, в общем то это зависит от размера текстур
  // то есть мелкие текстуры могут и на хосте посидеть, следовательно нужно начать с больших текстур при загрузке
  // но при этом может возникнуть ситуация когда после загрузки может отстаться место для нескольких мелких,
  // но при этом не остаться места для остальных больших текстур
  // короче, я так понял VMA по умолчанию пытается впихнуть в дефолтный пул, если не получается то создает пул на хосте
  // а значит что если мы просто будем последовательно пытаться создать текстуры от большой к малой, то так или иначе 
  // у нас получится предпочтительное поведение
  // в дальнейшем нужно будет подумать о том чтобы делать еще и уровни текстурки для того чтобы идеально входила на устройство
  
  // вообще конечно проще тут с нуля все сделать
  // зачем мне копировать сначало на хост а потом обратно?
  // проще заново загрузить и не париться
  // блен
  
  // короч, я не могу создать картинки с оптимальным тайлингом и при этом на цпу
  // поэтому нужно придумать что то другое
  // скорее всего я буду заново грузить все с диска, так как это самый легкий вариант
  // а что делать если у игрока банально закончится память?
  // тип в нормальных движках реализована динамическая подгрузка мира
  
  struct LoadingTexture {
    yavf::Image* src;
    yavf::Image* dst;
    std::vector<VkImageCopy> copies;
    std::vector<VkImageBlit> blits;
  };

  if (data == nullptr) throw std::runtime_error("Not in loading state");

  struct Tmp {
    Vk::Extent2D extent;
    uint32_t mipLevels;
    uint32_t count;
  };
  std::vector<Tmp> tmpArray;
  
  for (const auto &loadingData : data->loadData) {
    bool founded = false;
    for (size_t i = 0; i < tmpArray.size(); ++i) {
      if (tmpArray[i].extent == loadingData.size && tmpArray[i].mipLevels == loadingData.mipLevels) {
        tmpArray[i].count += loadingData.count;
        founded = true;
        break;
      }
    }
    
    if (!founded) tmpArray.push_back({loadingData.size, loadingData.mipLevels, static_cast<uint32_t>(loadingData.count)});
  }
  
  // нужно будет когда нибудь потом сделать свой вектор с плейсмент нью и без вечного оператора присвоения
  for (const auto &tmp : tmpArray) {
    arrays.emplace_back(ImagePool::CreateInfo{std::min(tmp.count, IMAGE_POOL_MAX_TEXTURE_LAYER_COUNT), static_cast<uint32_t>(std::ceil(float(tmp.count) / float(IMAGE_POOL_MAX_TEXTURE_LAYER_COUNT))), device, {tmp.extent, tmp.mipLevels}});
  }
  
//   for (size_t i = 0; i < arrays.size(); ++i) {
//     for (size_t k = 0; k < arrays[i].imageArraysCount(); ++k) {
//       std::cout << "image handle " << arrays[i].getImageArray(k)->handle() << "\n";
//     }
//     
//     std::cout << "free images count " << arrays[i].freeImagesCount() << "\n";
//   }

  std::vector<LoadingTexture> dataToLoad;
  for (auto & loadData : data->loadData) {
    const LoadingData &resource = *data->textureDatas[loadData.id];
    
//     const Vk::Extent2D imageSize{
//      resource.imageSize().width,
//      resource.imageSize().height
//    };
    
    size_t index = SIZE_MAX;
    for (size_t i = 0; i < arrays.size(); ++i) {
      if (equal(arrays[i], loadData.size, loadData.mipLevels)) {
        index = i;
        break;
      }
    }
    
//     std::cout << "loadData.count " << loadData.count << "\n";
//     std::cout << "loadData.id.name() " << loadData.id.name() << "\n";
//     std::cout << "loadData.images " << loadData.images << "\n";
//     std::cout << "loadData.mipLevels " << loadData.mipLevels << "\n";
//     std::cout << "loadData.size " << loadData.size.width << " " << loadData.size.height << "\n";
    
    arrays[index].pop(loadData.count, loadData.images);

    // грузим изображениие в дополнительную память
    int texWidth, texHeight, texChannels;
    uint8_t* pixels = stbi_load(resource.path().c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    const VkDeviceSize imageBytes = texWidth * texHeight * 4;
    
    if (pixels == nullptr) {
      throw std::runtime_error("failed to load image " + resource.id().name() + " path: " + resource.path());
    }
    
    yavf::Image* staging = device->create(yavf::ImageCreateInfo::texture2DStaging({uint32_t(texWidth), uint32_t(texHeight)}), VMA_MEMORY_USAGE_CPU_ONLY);
    memcpy(staging->ptr(), pixels, imageBytes);
    stbi_image_free(pixels);

    // собираем VkImageCopy в кучу
    std::vector<VkImageCopy> copies(loadData.count);
    copy(loadData.count, loadData.images, resource.count(), resource.rows(), resource.columns(), resource.imageSize(), copies);

    // теперь нам придется обойти каждый Image так как мы не можем гарантировать что все они будут в одном слоте
    yavf::Image* oldImage = nullptr;
    uint32_t oldSlot = UINT32_MAX;
    for (size_t i = 0; i < loadData.count; ++i) {
      yavf::Image* dst = nullptr;

      // найдем нужное изображение среди массивов
      if (loadData.images[i].index == oldSlot) {
        dst = oldImage;
      } else {
        for (auto & array : arrays) {
          if (equal(array, loadData.size, loadData.mipLevels)) {
            dst = array.getImageArray(loadData.images[i]);
            oldSlot = loadData.images[i].index;
            oldImage = dst;
            break;
          }
        }
      }

      if (dst == nullptr) throw std::runtime_error("dslmkvwdvpajiovjdiopvw");
      
      // найдем уже созданный LoadingTexture
      size_t index = SIZE_MAX;
      for (size_t j = 0; j < dataToLoad.size(); ++j) {
        if (dataToLoad[j].dst == dst && dataToLoad[j].src == staging) {
          index = j;
          break;
        }
      }

      if (index == SIZE_MAX) {
        index = dataToLoad.size();
        dataToLoad.push_back({staging, dst, {copies[i]}, std::vector<VkImageBlit>()}); //dst->info().mipLevels-1
      } else {
        dataToLoad[index].copies.push_back(copies[i]);
      }

      if (dataToLoad[index].dst->info().mipLevels > 1 && dataToLoad[index].blits.empty()) {
        dataToLoad[index].blits.resize(dataToLoad[index].dst->info().mipLevels-1);
        blit(dataToLoad[index].dst->info().mipLevels, dataToLoad[index].dst->info().arrayLayers, dataToLoad[index].dst->info().extent, dataToLoad[index].blits);
      }
      
      ASSERT(dataToLoad[index].dst->info().mipLevels > 1 && !dataToLoad[index].blits.empty());
    }

    imagesContainer.push_back(loadData);
  }
  
  // проблема с мипмаппингом, ниарест фильтрация выглядит всрато
  // а линейная - размывает текстуру

  yavf::GraphicTask* task = device->allocateGraphicTask();

  task->begin();

  for (auto & i : dataToLoad) {
    task->setBarrier(i.src, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    task->setBarrier(i.dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    task->copy(i.src, i.dst, i.copies);
//    task->setBarrier(i.dst, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    for (uint32_t j = 1; j < i.dst->info().mipLevels; ++j) {
      const VkImageSubresourceRange range{
        VK_IMAGE_ASPECT_COLOR_BIT,
        j-1, 1, 0, i.dst->info().arrayLayers
      };

      task->setBarrier(i.dst->handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, range);
      task->copyBlit(i.dst, i.dst, i.blits[j-1], VK_FILTER_NEAREST); //VK_FILTER_NEAREST
    }
    
    if (i.dst->info().mipLevels > 1) {
      const VkImageSubresourceRange range{
        VK_IMAGE_ASPECT_COLOR_BIT,
        i.dst->info().mipLevels-1, 1, 0, i.dst->info().arrayLayers
      };
      task->setBarrier(i.dst->handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, range);
    }

    i.dst->info().initialLayout = i.dst->info().mipLevels > 1 ? VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL : i.dst->info().initialLayout;
    task->setBarrier(i.dst, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  }

  task->end();

  task->start();
  task->wait();

  // на этом все
  device->deallocate(task);
  for (auto & i : dataToLoad) {
    device->destroy(i.src);
  }

//  renderData.imagesCount = 0;
//  for (size_t i = 0; i < arrays.size(); ++i) {
//    renderData.imagesCount += arrays[i].imageArraysCount();
//  }

  // этого достаточно чтобы переделать, если я собираюсь делать версию с освобождением памяти
  // то нужна более сложная проверка

  // как понять нужно ли мне пересоздавать пайплайны?
  oldImagesCount = renderData.imagesCount;
  if (renderData.imagesCount != ImagePool::slotsCount()) {
    renderData.imagesCount = ImagePool::slotsCount();
    recreateDescriptorPool();
  }
}

void ImageLoader::clear() {
  // здесь мы удалим data, для того чтобы освободить память не особ нужными данными
  
  if (data != nullptr) {
    delete data;
    data = nullptr;
  }
}

size_t ImageLoader::overallState() const {
  // тут мы должны примерно прикинуть какое-нибудь число которое примерно показывает сколько нужно всего загрузить
  return data != nullptr ? data->loadData.size() : 0;
}

size_t ImageLoader::loadingState() const {
  // тут нужно примерную степень загрузки выдать
}

std::string ImageLoader::hint() const {
  // тут просто смешной хинт какой-нибудь
}

size_t ImageLoader::hostSize() const {
  // размер на хосте
  return sizeof(ImageLoader) +
         sizeof(ImageContainerData) * imagesContainer.size() +
         sizeof(SamplerContainerData) * samplersContainer.size() +
         sizeof(ImagePool) * arrays.size();
}

size_t ImageLoader::deviceSize() const {
  // размер на устройстве
  size_t size = 0;
  for (const auto &pool : arrays) {
    const size_t &oneImageSize = pool.getInfo()->extent.height * pool.getInfo()->extent.width * pool.getInfo()->extent.depth * 4;
    size += oneImageSize * pool.layers() + (oneImageSize * pool.getInfo()->mipLevels) / 3;
  }

  return size;
}

const ImageContainerData* ImageLoader::resourceData(const ResourceID &id) const {
  for (auto & data : imagesContainer) {
    if (data.id == id) return &data;
  }

  return nullptr;
}

Image ImageLoader::image(const ResourceID &id, const uint32_t &index) const {
  for (auto & data : imagesContainer) {
    if (data.id == id) {
      //ASSERT(data.count > index);
      if (index >= data.count) return {UINT32_MAX, UINT32_MAX};

      return data.images[index];
    }
  }

  return {UINT32_MAX, UINT32_MAX};
}

uint32_t ImageLoader::sampler(const ResourceID &id) const {
  for (const auto &sampler : samplersContainer) {
    if (id == sampler.id) return sampler.sampler;
  }

  return UINT32_MAX;
}

uint32_t ImageLoader::imagesCount() const {
  // по идее это количество слотов, то есть
  return renderData.imagesCount;
}

uint32_t ImageLoader::samplersCount() const {
  return samplersContainer.size();
}

yavf::DescriptorSet* ImageLoader::resourceDescriptor() const {
  return renderData.set;
}

yavf::DescriptorSetLayout ImageLoader::resourceLayout() const {
  return renderData.layout;
}

bool ImageLoader::needRecreatePipelines() const {
  return oldImagesCount != renderData.imagesCount;
}

void ImageLoader::createData() {
  data = new Data();
}

void ImageLoader::recreateDescriptorPool() {
  if (renderData.pool != VK_NULL_HANDLE) {
    device->destroy(renderData.pool);
    renderData.pool = VK_NULL_HANDLE;
  }

  if (renderData.layout != VK_NULL_HANDLE) {
    device->destroy(renderData.layout);
  }
  
  {
    yavf::DescriptorPoolMaker dpm(device);

    renderData.pool = dpm.flags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
                         .poolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, renderData.imagesCount)
                         .poolSize(VK_DESCRIPTOR_TYPE_SAMPLER, renderData.samplersCount)
                         .create("texture_descriptor_pool");
  }
  
//   std::cout << "Creating descriptor pool" << "\n";
  
  {
    yavf::DescriptorLayoutMaker dlm(device);
    
    //imagesCount
//    imageLayout = dlm.binding(0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT, imagesCount).create("image_array_layout");
//    samplerLayout = dlm.binding(0, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, samplersCount).create("sampler_array_layout");
    renderData.layout = dlm.binding(0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT, renderData.imagesCount)
                           .binding(1, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, renderData.samplersCount)
                           .create("texture_data_array_layout");
  }
  
//   std::cout << "Creating descriptor layout" << "\n";
  
//   if (this->images != VK_NULL_HANDLE) {
//     yavf::vkCheckError("vkFreeDescriptorSets", nullptr, 
//     vkFreeDescriptorSets(device->handle(), pool, 1, &this->images));
//     this->images = VK_NULL_HANDLE;
//     // нужно еще попробовать vkResetDescriptorPool
//   }
  
  {
    yavf::DescriptorMaker dm(device);

    renderData.set = dm.layout(renderData.layout).create(renderData.pool)[0];

    renderData.set->resize(renderData.imagesCount + renderData.samplersCount);

    for (auto & array : arrays) {
      array.updateDescriptorData(renderData.set);
    }

    for (size_t i = 0; i < renderData.samplersCount; ++i) {
      renderData.set->at(i + renderData.imagesCount) = {samplers[i], nullptr, VK_IMAGE_LAYOUT_MAX_ENUM, i, 1, VK_DESCRIPTOR_TYPE_SAMPLER};
    }

    renderData.set->update();
  }
}
