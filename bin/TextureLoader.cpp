#include "TextureLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <fstream>

bool checkTextureJsonValidity(const std::string &pathPrefix, const std::string &path, const nlohmann::json &j, std::string &name, size_t &size, std::vector<ErrorDesc> &error, std::vector<WarningDesc> &warning) {
  bool valid = false;
  bool hasCount = false;
  bool hasRows = false;
  bool hasColumns = false;
  bool hasWidth = false;
  bool hasHeight = false;
  uint32_t count = 0;
  uint32_t rows = 0;
  uint32_t columns = 0;
  uint32_t width = 0;
  uint32_t height = 0;
  std::string texturePath;
  
  int32_t loadedWidth, loadedHeight;
  
  for (auto concreteTIt = j.begin(); concreteTIt != j.end(); ++concreteTIt) {
    if (concreteTIt.value().is_string() && concreteTIt.key().compare("path") == 0) {
      texturePath = concreteTIt.value().get<std::string>();
      valid = true;
      
      int32_t comp;
      int ret = stbi_info((pathPrefix + texturePath).c_str(), &loadedWidth, &loadedHeight, &comp);
//       if (!bool(ret)) throw std::runtime_error("dqsdqwfgqfwfqffqfqfwfavsavaw " + pathPrefix + texturePath);
      
      continue;
    }
    
    if (concreteTIt.value().is_string() && concreteTIt.key().compare("name") == 0) {
      name = concreteTIt.value().get<std::string>();
      continue;
    }
    
    if (concreteTIt.value().is_number_unsigned() && concreteTIt.key().compare("count") == 0) {
      count = concreteTIt.value().get<size_t>();
      hasCount = count > 1;
      continue;
    }
    
    if (concreteTIt.value().is_number_unsigned() && concreteTIt.key().compare("rows") == 0) {
      rows = concreteTIt.value().get<size_t>();
      hasRows = rows > 1;
      continue;
    }
    
    if (concreteTIt.value().is_number_unsigned() && concreteTIt.key().compare("columns") == 0) {
      columns = concreteTIt.value().get<size_t>();
      hasColumns = columns > 1;
      continue;
    }
    
    if (concreteTIt.value().is_object() && concreteTIt.key().compare("size") == 0) {
      for (auto sizeIt = concreteTIt.value().begin(); sizeIt != concreteTIt.value().end(); ++sizeIt) {
        if (sizeIt.value().is_number_unsigned() && sizeIt.key().compare("width") == 0) {
          width = sizeIt.value().get<uint32_t>();
          hasWidth = width > 0;
          continue;
        }
        
        if (sizeIt.value().is_number_unsigned() && sizeIt.key().compare("height") == 0) {
          height = sizeIt.value().get<uint32_t>();
          hasHeight = height > 0;
          continue;
        }
      }
    }
  }
  
  const bool hasSize = hasWidth && hasHeight;
  
  if (hasSize) {
    size = width * height * (hasCount ? count : 1) * 4;
  } else {
    size = loadedWidth * loadedHeight * 4;
  }
  
  if (!valid) {
    // error
    ErrorDesc desc(MISSED_TEXTURE_PATH, std::string("no path.") + " File: " + path);
    error.push_back(desc); // тут наверное нужно заменить на коды ошибок
    return false;
  }
  
  if (hasCount && count > rows * columns) {
    ErrorDesc desc(TOO_MUCH_TEXTURE_COUNT, std::to_string(count) + " > " + std::to_string(rows * columns) + ". File: " + path);
    error.push_back(desc);
    return false;
  }
  
  if (hasCount && !hasSize) {
    ErrorDesc desc(MISSED_TEXTURE_SIZE, std::string("no size.") + " File: " + path);
    error.push_back(desc);
    return false;
  }
  
  if (name.empty()) {
    WarningDesc desc(MISSED_TEXTURE_NAME, "using texture path as name");
    warning.push_back(desc);
    name = texturePath;
  }
  
  return true;
}

bool getTextureData(const std::string &pathPrefix, const nlohmann::json &j, std::string &name, size_t &size, TextureLoader::LoadingData* data) {
  bool valid = false;
  bool hasCount = false;
  bool hasRows = false;
  bool hasColumns = false;
  bool hasWidth = false;
  bool hasHeight = false;
  uint32_t count = 0;
  uint32_t rows = 0;
  uint32_t columns = 0;
  uint32_t width = 0;
  uint32_t height = 0;
  std::string texturePath;
  
  int32_t loadedWidth = 0, loadedHeight = 0;
  
  for (auto concreteTIt = j.begin(); concreteTIt != j.end(); ++concreteTIt) {
    if (concreteTIt.value().is_string() && concreteTIt.key().compare("path") == 0) {
      texturePath = concreteTIt.value().get<std::string>();
      valid = true; 
      
      int32_t comp;
      int ret = stbi_info((pathPrefix + texturePath).c_str(), &loadedWidth, &loadedHeight, &comp);
//       if (!bool(ret)) throw std::runtime_error("dqsdqwfgqfwfqffqfqfwfavsavaw " + pathPrefix + texturePath);
      
      continue;
    }
    
    if (concreteTIt.value().is_string() && concreteTIt.key().compare("name") == 0) {
      name = concreteTIt.value().get<std::string>();
      continue;
    }
    
    if (concreteTIt.value().is_number_unsigned() && concreteTIt.key().compare("count") == 0) {
      count = concreteTIt.value().get<size_t>();
      hasCount = count > 1;
      continue;
    }
    
    if (concreteTIt.value().is_number_unsigned() && concreteTIt.key().compare("rows") == 0) {
      rows = concreteTIt.value().get<size_t>();
      hasRows = rows > 1;
      continue;
    }
    
    if (concreteTIt.value().is_number_unsigned() && concreteTIt.key().compare("columns") == 0) {
      columns = concreteTIt.value().get<size_t>();
      hasColumns = columns > 1;
      continue;
    }
    
    if (concreteTIt.value().is_object() && concreteTIt.key().compare("size") == 0) {
      for (auto sizeIt = concreteTIt.value().begin(); sizeIt != concreteTIt.value().end(); ++sizeIt) {
        if (sizeIt.value().is_number_unsigned() && sizeIt.key().compare("width") == 0) {
          width = sizeIt.value().get<uint32_t>();
          hasWidth = width > 0;
          continue;
        }
        
        if (sizeIt.value().is_number_unsigned() && sizeIt.key().compare("height") == 0) {
          height = sizeIt.value().get<uint32_t>();
          hasHeight = height > 0;
          continue;
        }
      }
    }
  }
  
  const bool hasSize = hasWidth && hasHeight;
  
  if (hasSize) {
    size = width * height * (hasCount ? count : 1) * 4;
  } else {
    size = loadedWidth * loadedHeight * 4;
  }
  
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
  
  if (name.empty()) name = texturePath;
  
  data->name = name;
  data->path = pathPrefix + texturePath;
//   std::cout << "Texture " << data->name << " path: " << data->path << "\n";
  data->sampler = ""; // не уверен на счет него
  data->rows = hasRows ? rows : 1;
  data->columns = hasColumns ? columns : 1;
  data->count = hasCount ? count : 1;
  data->size.width = hasSize ? width : loadedWidth;
  data->size.height = hasSize ? height : loadedHeight;
  data->size.depth = 1;
  
  return true;
}

TextureLoader::Data::~Data() {
  for (const auto &resource : resources) {
    resourcePool.deleteElement(resource.second);
  }
  
  for (const auto &conflict : conflicts) {
    conflictPool.deleteElement(conflict.second);
  }
  
  for (const auto &textureData : textureDatas) {
    textureDataPool.deleteElement(textureData.second);
  }
}


TextureLoader::TextureLoader(const CreateInfo &info) : 
  data(nullptr), 
  device(nullptr),
  sampler(VK_NULL_HANDLE),
  pool(VK_NULL_HANDLE),
  imagesCount(0), 
  samplersCount(0), 
  images(VK_NULL_HANDLE), 
  samplers(VK_NULL_HANDLE), 
  imageLayout(VK_NULL_HANDLE), 
  samplerLayout(VK_NULL_HANDLE) {
    
  this->device = info.device;
  
//   {
//     yavf::DescriptorPoolMaker dpm(device);
//     
//     pool = dpm.flags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
//               .poolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1)
//               .poolSize(VK_DESCRIPTOR_TYPE_SAMPLER, 1)
//               .create("texture_descriptor_pool");
//   }
  
  {
    yavf::SamplerMaker sm(device);
    
    sampler = sm.addressMode(VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT)
                .anisotropy(VK_TRUE, 16.0f)
                .borderColor(VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK)
                .compareOp(VK_FALSE, VK_COMPARE_OP_MAX_ENUM)
                .filter(VK_FILTER_NEAREST, VK_FILTER_NEAREST)
                .lod(0, 1000.0f)
                .mipmapMode(VK_SAMPLER_MIPMAP_MODE_NEAREST)
                .unnormalizedCoordinates(VK_FALSE)
                .create("default_sampler");
  }
  
//   {
//     yavf::DescriptorLayoutMaker dlm(device);
//     
//     samplerLayout = dlm.binding(0, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1).create("sampler_array_layout");
//   }
  
//   {
//     yavf::DescriptorMaker dm(device);
//     
//     samplers = dm.layout(samplerLayout).create(pool)[0];
//     
//     yavf::DescriptorUpdater du(device);
//   
//     du.currentSet(samplers).begin(0, 0, VK_DESCRIPTOR_TYPE_SAMPLER).sampler(sampler.handle()).update();
//   }
  
  samplersCount = 1;
}

TextureLoader::~TextureLoader() {
  // удалить все
  
  if (data != nullptr) {
    delete data;
  }
  
  for (size_t i = 0; i < arrays.size(); ++i) {
    device->destroy(arrays[i].texture);
  }
  
  device->destroy(sampler);
  device->destroy(pool);
  device->destroy(imageLayout);
  device->destroy(samplerLayout);
}

bool TextureLoader::parse(const std::string &pathPrefix, 
                          const std::string &forcedNamePrefix, 
                          const nlohmann::json &data, 
                          std::vector<Resource*> &resources, 
                          std::vector<ErrorDesc> &errors, 
                          std::vector<WarningDesc> &warnings) {
  // создать
  if (this->data == nullptr) createData();
  
  // тут мы чекаем json, вытаскиваем данные о текстурах, засовываем общие данные в resources
  
  // во первых сюда скорее всего будет приходить еще zip архив размещенный в памяти
  // во вторых мне нужно сделать парсинг разных уровней данных + данных в разных файлах
  // то есть сюда может придти как отдельное описание одной текстурки, так и массив json'ов каждый из которых обозначает текстурку
  
  // может придти объект textures: new, update
  // может придти объект с описанием текстурки (path, name, count и тд)
  
  // мы должны проверить массив new и объект update
  // или обнаружить хотя бы path, name, count
  bool validData = false;
  for (auto itr = data.begin(); itr != data.end(); ++itr) {
    if (itr.value().is_array() && itr.key().compare("new") == 0) {
      validData = true;
      
      for (size_t tInfoIt = 0; tInfoIt < itr.value().size(); ++tInfoIt) {
        std::string name;
        std::string debugPath;
        nlohmann::json json;
        
        if (itr.value()[tInfoIt].is_string()) {
          // грузим новый json
          //std::string path = texturesIt.value()[tInfoIt].get<std::string>();
          debugPath = itr.value()[tInfoIt].get<std::string>();

          std::ifstream file(pathPrefix + debugPath);
          if (!file.is_open()) {
            ErrorDesc desc(COULD_NOT_LOAD_FILE, "could not load file " + pathPrefix + debugPath);
            errors.push_back(desc);
            continue;
          }
          
          file >> json;
          
//                   if (!checkTextureJsonValidity(path, json, name, error, warning)) {
//                     continue;
//                   }
        } else if (itr.value()[tInfoIt].is_object()) {
          // обходим информацию о текстуре                  
          json = itr.value()[tInfoIt];
        }
        
        size_t resourceSize;
        
        if (!checkTextureJsonValidity(pathPrefix, debugPath, json, name, resourceSize, errors, warnings)) {
          continue;
        }
        
        const std::string &realName = forcedNamePrefix.empty() ? name : forcedNamePrefix + "." + name;
        
        TextureLoader::LoadingData* tData = this->data->textureDataPool.newElement();
        bool b = getTextureData(pathPrefix, json, name, resourceSize, tData);
        tData->name = realName;
        
//         if (!b) throw std::runtime_error("getTextureData error");
        
        auto tDataItr = this->data->textureDatas.find(realName);
        if (tDataItr == this->data->textureDatas.end()) {
          this->data->textureDatas[realName] = tData;
        } else {
          this->data->textureDataPool.deleteElement(tData);
          
          ErrorDesc desc(DUBLICATE_ERROR, "data with name " + realName + " already exist");
          errors.push_back(desc);
          
          continue;
        }
        
        //Conflict con("texturePath", &mod, TEXTURE);
        //std::string name = prefix + "." + info.name;
        //name = forcedNamePrefix.empty() ? name : forcedNamePrefix + "." + name;
        auto resItr = this->data->resources.find(realName);
        Resource* res = nullptr;
        if (resItr == this->data->resources.end()) {
          res = this->data->resourcePool.newElement();
          res->name = realName;
          res->data = json;
          //res->path = 
          res->size = resourceSize;
          
          this->data->resources[realName] = res;
        } else {
          //res = resItr->second;
          
          this->data->textureDatas.erase(realName);
          this->data->textureDataPool.deleteElement(tData);
          
          ErrorDesc desc(DUBLICATE_ERROR, "data with name " + realName + " already exist");
          errors.push_back(desc);
          
          continue;
        }
        
        auto conItr = this->data->conflicts.find(realName);
        if (conItr == this->data->conflicts.end()) {
          //conItr = this->data->conflicts.insert(std::make_pair(name, this->data->conflictPool.newElement(res))).first;
          this->data->conflicts.insert(std::make_pair(realName, this->data->conflictPool.newElement(res)));
          std::cout << "inserted " << realName << "\n";
        } else {
          if (conItr->second->mainData()) {
            this->data->textureDatas.erase(realName);
            this->data->textureDataPool.deleteElement(tData);
            
            this->data->resources.erase(realName);
            this->data->resourcePool.deleteElement(res);
            
            ErrorDesc desc(DUBLICATE_ERROR, "data with name " + realName + " already exist");
            errors.push_back(desc);
            
            continue;
          }
          
          
          conItr->second->setMainData(res);
//           for (int64_t i = lateConflict.size()-1; i >= 0; --i) {
//             //if (&conItr->second == lateConflict[i]) {
//             if (conItr->second.getName().compare(lateConflict[i]) == 0) {
//               lateConflict[i] = lateConflict.back();
//               lateConflict.pop_back();
//               break;
//             }
//           }
          
          //if (conItr->second.exist() && !conItr->second.solved()) nonSolved.push_back(conItr->second.getName());
        }
        
      }
    } else if (itr.value().is_object() && itr.key().compare("update") == 0) {
      validData = true;
      
      for (auto tInfoIt = itr.value().begin(); tInfoIt != itr.value().end(); ++tInfoIt) {
        std::string name;
        std::string debugPath;
        nlohmann::json textureJson;
        
        if (tInfoIt.value().is_string()) {
          // грузим новый json
          debugPath = tInfoIt.value().get<std::string>();
          
          std::ifstream file(pathPrefix + debugPath);
          if (!file.is_open()) {
            ErrorDesc desc(COULD_NOT_LOAD_FILE, "could not load file " + debugPath);
            errors.push_back(desc);
            continue;
          }
          
          file >> textureJson;
          
//                   if (!loadTextureJson(path, textureJson, info, error, warning)) {
//                     continue;
//                   }
        } else if (tInfoIt.value().is_object()) {
          // обходим информацию о текстуре
          textureJson = tInfoIt.value().is_object();
        }
        
        size_t resourceSize;
        
        if (!checkTextureJsonValidity(pathPrefix, debugPath, textureJson, name, resourceSize, errors, warnings)) {
          continue;
        }
        
        const std::string &realName = forcedNamePrefix.empty() ? name : forcedNamePrefix + "." + name;
        
        TextureLoader::LoadingData* tData = this->data->textureDataPool.newElement();
        bool b = getTextureData(pathPrefix, textureJson, name, resourceSize, tData);
        tData->name = realName;
        
//         if (!b) throw std::runtime_error("getTextureData error");
        
        auto tDataItr = this->data->textureDatas.find(realName);
        if (tDataItr == this->data->textureDatas.end()) {
          this->data->textureDatas[realName] = tData;
        } else {
          this->data->textureDataPool.deleteElement(tData);
          
          ErrorDesc desc(DUBLICATE_ERROR, "data with name " + realName + " already exist");
          errors.push_back(desc);
          
          continue;
        }
        
        //name = forcedNamePrefix.empty() ? name : forcedNamePrefix + "." + name;
        Resource* res = nullptr;
        auto resItr = this->data->resources.find(realName);
        if (resItr == this->data->resources.end()) {
          res = this->data->resourcePool.newElement();
          res->name = realName;
          res->data = textureJson;
          //res->path = 
          res->size = resourceSize;
          
          this->data->resources[realName] = res;
        } else {
          //res = resItr->second;
          
          this->data->textureDatas.erase(realName);
          this->data->textureDataPool.deleteElement(tData);
          
          ErrorDesc desc(DUBLICATE_ERROR, "data with name " + realName + " already exist");
          errors.push_back(desc);
          
          continue;
        }
        
        auto conItr = this->data->conflicts.find(tInfoIt.key()); // это будет типо game.textureName
        if (conItr == this->data->conflicts.end()) {
          Conflict* con = this->data->conflictPool.newElement(tInfoIt.key());
          this->data->conflicts[tInfoIt.key()] = con;
          con->addVariant(res);
          res->conflict = con;
          
          // сначало проверяем есть ли уже апдейт
//           auto updateIt = updateConflicts.find(tInfoIt.key());
//           if (updateIt != updateConflicts.end()) {
//             conItr = conflicts.find(updateIt->second);
//           } else {
//             // если апдейта нет, то создаем
//             conItr = conflicts.insert(std::make_pair(tInfoIt.key(), Conflict(tInfoIt.key(), TEXTURE))).first;
//             //lateConflict.push_back(&conItr->second); // отвалиться при перехешировании. Переделать
//             lateConflict.push_back(tInfoIt.key());
//           }
        } else {
          conItr->second->addVariant(res);
          res->conflict = conItr->second;
        }
        
        // здесь все же нужно создавать не отдельный конфликт, а вариант
        // короче: 
        // 1. префиксы наверное нужно указывать в депендансисах пользователю (модеру)
        // 2. мод от мода отличает 100% только воршоп id
        // 3. что с луа в этом случае? как регистрировать функции луа? как отделять один мод от другого? и еще миллион вопросов
        // 4. каждая new запись должен создавать конфликт, каждая update запись должна создавать вариант
        // 5. работать буду здесь (и в конфликтах) с json
        
        //std::string name = prefix + "." + info.name;
//         name = prefix + "." + name;
//         conItr->second.addVariant(Variant(name, tInfoIt.value(), &gameDataItr->second, &conItr->second));
//         updateConflicts.insert(std::make_pair(name, tInfoIt.key()));
//         if (conItr->second.exist() && !conItr->second.solved()) nonSolved.push_back(tInfoIt.key());
        
        std::cout << "inserted " << name << " to " << tInfoIt.key() << "\n";
      }
    }
  }
  
  if (!validData) {
    // тут мы по идее должны проверить ситуацию когда к нам пришли
    // данные об отдельной текстуре
    
    std::string name;
    size_t resourceSize;
    
    TextureLoader::LoadingData* tData = this->data->textureDataPool.newElement();
//     if (checkTextureJsonValidity(pathPrefix, "", data, name, resourceSize, errors, warnings)) {
//       validData = true;
//     }
    
    if (getTextureData(pathPrefix, data, name, resourceSize, tData)) {
      validData = true;
      
      name = forcedNamePrefix.empty() ? name : forcedNamePrefix + "." + name;
      tData->name = name;
      
      auto tDataItr = this->data->textureDatas.find(name);
      if (tDataItr == this->data->textureDatas.end()) {
        this->data->textureDatas[name] = tData;
      } else {
        this->data->textureDataPool.deleteElement(tData);
        
        ErrorDesc desc(DUBLICATE_ERROR, "data with name " + name + " already exist");
        errors.push_back(desc);
        
        return true;
      }
      
      // если текстурДата есть то это ошибка вообще то!!
      // она может быть если мы текстурку с таким именем уже обработали
      
      Resource* res = nullptr;
      auto itr = this->data->resources.find(name);
      if (itr == this->data->resources.end()) {
        res = this->data->resourcePool.newElement();
        
        res->name = name;
        res->data = data;
        res->size = resourceSize;
        
        this->data->resources[name] = res;
      } else {
        // если tDat'ы нет а ресурс есть это очень странно
        // но тем не менее ее нужно тогда удалить
        this->data->textureDatas.erase(name);
        this->data->textureDataPool.deleteElement(tData);
        
        ErrorDesc desc(DUBLICATE_ERROR, "data with name " + name + " already exist");
        errors.push_back(desc);
        
        return true;
        //res = itr->second;
      }
      
      auto conItr = this->data->conflicts.find(name);
      if (conItr == this->data->conflicts.end()) {
        Conflict* con = this->data->conflictPool.newElement(res);
        res->conflict = con;
        
        this->data->conflicts[name] = con;
      } else {
        if (conItr->second->mainData()) {
          this->data->textureDatas.erase(name);
          this->data->textureDataPool.deleteElement(tData);
          
          this->data->resources.erase(name);
          this->data->resourcePool.deleteElement(res);
          
          ErrorDesc desc(DUBLICATE_ERROR, "data with name " + name + " already exist");
          errors.push_back(desc);
          
          return true;
        }
        
        conItr->second->setMainData(res);
        res->conflict = conItr->second;
      }
    } else {
      this->data->textureDataPool.deleteElement(tData);
    }
  }
  
  // также мне нужно заполнить TextureData для каждой текстурки
  // по идее это не сложно, достаточно лишь передать TextureData в checkTextureJsonValidity
  // вроде бы теперь заполняю
  
  if (!validData) return false;
  
  return true;
}

bool TextureLoader::forget(const std::string &name) {
  // тут в случае когда у нас есть data мы должны забыть текстурку по имени name
  // этот метод должен вызываться в основном в том случае, если мы отменяем выбор мода
  // тогда мы должны пройтись по лоадерам и удалить мета данные из них
  // так как по этим мета данным будет потом происходить загрузка игры
  
  // по идее когда мы вызываем этот метод текстура не должна быть загружена в память
  // то есть это вызывается только в момент когда кто-то ошибся и хочет что то другое
  
  // выпиливаем ресурсы по названию
  if (data == nullptr) throw std::runtime_error("Not in loading state");
  
  auto itr = data->resources.find(name);
  // по идее этого достаточно для того чтобы определить есть ли у нас такой ресурс
  if (itr == data->resources.end()) return false;
  
  Conflict* con = itr->second->conflict;
  con->removeVariant(itr->second);
  
  data->resourcePool.deleteElement(itr->second);
  data->resources.erase(itr);
  
  data->textureDatas.erase(name);
  
  if (con->getVariants().empty()) {
    data->conflicts.erase(con->name());
    data->conflictPool.deleteElement(con);
  }
  
  // как быть с конфликтами? из конфликтов нужно убирать ресурс
  // как найти конфликт? нужно в ресурсах указать к какому конфликту принадлежит 
  // как указать? указателя по идее должно быть достаточно
  
  return true;
}

std::unordered_map<std::string, Resource*> TextureLoader::getLoadedResource() {
  if (data == nullptr) throw std::runtime_error("Not in loading state");
  
  return data->resources;
}

std::unordered_map<std::string, Conflict*> TextureLoader::getConflicts() {
  if (data == nullptr) throw std::runtime_error("Not in loading state");
  
  return data->conflicts;
}

bool TextureLoader::load(const std::string &name) {
  // тут в случае TextureLoader мы помечаем определенные ресурс для того чтобы потом загрузить масимально оптимально
  
  // так как это сделать? как сделать загрузку/удаление?
  // ну то есть я примерно знаю как пометить на загрузку
  
  // мне нужно пересчитать количество текстур и уровней в них
  // и после это поможет мне все это пересобрать
  // а ну и да это тоже не будет работать без data
  
  // вообще наверное именно здесь мы будем заполнять data->textureDatas
  // так как до этого мы не можем точно сказать что нас интересует в конфликте
  
  if (data == nullptr) throw std::runtime_error("Not in loading state");
  
//   for (const auto &pair : data->textureDatas) {
//     std::cout << pair.first << "\n";
//   }
  
  auto itr = data->textureDatas.find(name);
  if (itr == data->textureDatas.end()) return false;
  
  data->load.push_back(name);
  
  const VkExtent2D e{
    itr->second->size.width, 
    itr->second->size.height
  };
  
  auto layerCountItr = data->layerCount.find(e);
  if (layerCountItr == data->layerCount.end()) {
    data->layerCount[e] = itr->second->count;
  } else {
    layerCountItr->second += itr->second->count;
  }
  
  return true;
}

bool TextureLoader::unload(const std::string &name) {
  // тут мы помечаем ресурс чтобы потом его выгрузить и возможно на его место загрузить что то другое
  // либо отдать эту память отдать обратно в кучу
  
  // как пометить на удаление? то есть мне нужно понять что должно быть в текстурке
  
  if (data == nullptr) throw std::runtime_error("Not in loading state");
  
  data->unload.push_back(name);
  
  // этот метод тоже видимо будет использоваться крайне редко
  
  // не знаю пока накой мне это нужно... дальше возможно будет понятнее
  
  return false;
}

// yavf::Image* createImage(yavf::Device* device, const uint32_t &width, const uint32_t &height, const uint32_t &layerCount) {
//   // почему то не содает картинку с параметрами VK_IMAGE_TILING_OPTIMAL и VMA_MEMORY_USAGE_CPU_ONLY
//   // линейный тайлинг требует не больше 1 слоя
//   // короч наверное придется создавать линейную текстурку с одним слоем
//   // и копировать так
//   // и вообще почему я вообще сюда попадаю, по идее это должно вызываться когда у меня уже есть текстурки
//   
//   const yavf::ImageCreateInfo info{
//     0,
//     VK_IMAGE_TYPE_2D,
//     VK_FORMAT_R8G8B8A8_UNORM,
//     {width, height, 1},
//     1,
//     layerCount,
//     VK_SAMPLE_COUNT_1_BIT,
//     VK_IMAGE_TILING_OPTIMAL,
//     VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
//     VK_IMAGE_ASPECT_COLOR_BIT,
//     VMA_MEMORY_USAGE_CPU_ONLY
//   };
//   
//   return device->createImage(info);
// }

yavf::Image* createImage(yavf::Device* device, const uint32_t &width, const uint32_t &height, const uint32_t &mipLevels, const uint32_t &layerCount) {
//   const yavf::ImageCreateInfo imageInfo{
//     0,
//     VK_IMAGE_TYPE_2D,
//     VK_FORMAT_R8G8B8A8_UNORM,
//     {uint32_t(width), uint32_t(height), 1},
//     mipLevels, // не помешало бы вынести в ImageCreateInfo
//     layerCount,
//     VK_SAMPLE_COUNT_1_BIT,
//     VK_IMAGE_TILING_OPTIMAL,
//     VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
//     VK_IMAGE_ASPECT_COLOR_BIT,
//     VMA_MEMORY_USAGE_GPU_ONLY
//   };
  
  return device->create(yavf::ImageCreateInfo::texture2D({uint32_t(width), uint32_t(height)}, 
                                                         VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
                                                         VK_FORMAT_R8G8B8A8_UNORM, 
                                                         layerCount, 
                                                         mipLevels), 
                        VMA_MEMORY_USAGE_GPU_ONLY);
}

void copy(const uint32_t &startingLayer, const uint32_t &startIndex, const uint32_t &count, const uint32_t &rows, const uint32_t &columns, const VkExtent3D &size, std::vector<VkImageCopy> &copies) {
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
        size
      };

      copies.push_back(copy);
    }
  }
}

void TextureLoader::end() {
  // именно здесь мы будем загружать/выгружать/что-угодно-делать
  // то есть нам нужно масимально компактно расположить текстурки в памяти
  
  // сюда приходит список того что нужно загрузить среди этого списка скорее всего есть то что уже загружено
  // надо это просто скопировать, а остальное загрузить
  
  // так что нам нужно тут сделать
  // нужно составить список того что я скопирую из текущих текстур - это будут как бы текстуры которые я решил оставить
  // 
  
//   struct TemporalCopyData {
//     yavf::Image* img;
//     VkImageCopy copy;
//   };
//   
//   // тут еще одна картинка нужна (куда копируем то?)
//   struct TemporalLoadData {
//     yavf::Image* src;
//     yavf::Image* dst;
//     std::vector<VkImageCopy> copies;
//   };
//   
//   struct TemporalImage {
//     yavf::Image* img;
//     uint32_t currentLayer;
//   };
  
  //const size_t deviceSize = 10;
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
  };
  
  for (uint32_t i = 0; i < arrays.size(); ++i) {
    device->destroy(arrays[i].texture);
  }
  arrays.clear();
  arrayIndices.clear();
  textureMap.clear();
  
  std::vector<LoadingTexture> textures;
  for (const auto &name : data->load) {
    auto dataItr = this->data->textureDatas.find(name);
    if (dataItr == this->data->textureDatas.end()) throw std::runtime_error("wtf");
    
    const LoadingData &data = *dataItr->second;
    const VkExtent2D e{
      data.size.width,
      data.size.height
    };
    
    int texWidth, texHeight, texChannels;
    unsigned char* pixels = stbi_load((data.path).c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    const VkDeviceSize imageSize = texWidth * texHeight * 4;
    
    if (pixels == nullptr) {
      throw std::runtime_error("failed to load image " + name + " path: " + data.path + "!");
    }
    
    // в будущем нам потребуется еще грузить сжатые текстурки
    // нужно прикинуть что нам нужно поменять здесь

    yavf::Image* staging = device->create(yavf::ImageCreateInfo::texture2DStaging({uint32_t(texWidth), uint32_t(texHeight)}), VMA_MEMORY_USAGE_CPU_ONLY);
    memcpy(staging->ptr(), pixels, imageSize);
    
    stbi_image_free(pixels);
    
    std::vector<Texture> textureVector;
    
    auto imageItr = arrayIndices.find(e);
    if (imageItr == arrayIndices.end()) {
      imageItr = arrayIndices.insert(std::make_pair(e, std::vector<size_t>())).first;
      uint32_t layerCount = this->data->layerCount[e];
      
      while (layerCount > 0) {
        const uint32_t layers = std::min(layerCount, uint32_t(TEXTURE_MAX_LAYER_COUNT));
        //yavf::Image* img = createImage(device, texWidth, texHeight, 1, layers);
        yavf::Image* img = createImage(device, e.width, e.height, 1, layers);
      
        imageItr->second.push_back(arrays.size());
        arrays.push_back({img, 0, 0});
        
        //layerCount > TEXTURE_MAX_LAYER_COUNT ? TEXTURE_MAX_LAYER_COUNT : layerCount;
        layerCount -= layers;
      }
    }
    
    uint32_t imageCount = data.count;
    uint32_t lastCount = 0;
    size_t currentIndex = 0;
    while (imageCount > 0) {
      // собираем здесь VkImageCopy
      
      const size_t imageArrayIndex = imageItr->second[currentIndex];
      const uint32_t currentLayer = arrays[imageArrayIndex].currentLayer;
      ++currentIndex;
      if (currentLayer == TEXTURE_MAX_LAYER_COUNT) continue;
      
      uint32_t newCount = ((TEXTURE_MAX_LAYER_COUNT - currentLayer) > imageCount) ? imageCount : TEXTURE_MAX_LAYER_COUNT - currentLayer;
      
      std::vector<VkImageCopy> copies;
      copy(currentLayer, lastCount, newCount, data.rows, data.columns, data.size, copies);
      arrays[imageArrayIndex].currentLayer += newCount;
      
      textures.push_back({staging, arrays[imageArrayIndex].texture, copies});
      
      for (size_t i = 0; i < newCount; ++i) {
        textureVector.push_back({static_cast<uint32_t>(imageArrayIndex), static_cast<uint32_t>(currentLayer + i), 0});
      }
      
      imageCount -= newCount;
      lastCount = newCount;
    }
    
    textureMap[name] = textureVector;
  }
  
//   std::cout << "Allocating images" << "\n";
  
  yavf::TransferTask* task = device->allocateTransferTask();
  
  task->begin();
//   for (uint32_t i = 0; i < arrays.size(); ++i) {
//     task->setBarrier(arrays[i].texture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
//   }
  
  // здесь мы все последовательно копируем
  for (uint32_t i = 0; i < textures.size(); ++i) {
    task->setBarrier(textures[i].src, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    task->setBarrier(textures[i].dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    task->copy(textures[i].src, textures[i].dst, textures[i].copies);
    task->setBarrier(textures[i].dst, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  }
  
//   for (uint32_t i = 0; i < arrays.size(); ++i) {
//     task->setBarrier(arrays[i].texture, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
//   }
  
//   for (const auto &src : alreadyLoaded) {
//     //task->setBarrier(src.first, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
//     for (const auto &dst : src.second) {
//       //task->setBarrier(needLoad[i].dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
//       task->copy(src.first, dst.first, dst.second);
//       //task->setBarrier(needLoad[i].dst, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
//     }
//   }
  
  task->end();
  
  task->start();
  task->wait();
  
  // в конце чистим все
  device->deallocate(task);
  for (size_t i = 0; i < textures.size(); ++i) {
    device->destroy(textures[i].src);
  }
  
//   std::cout << "Coping images" << "\n";
  
  for (size_t i = 0; i < arrays.size(); ++i) {
    //arrays[i].texture->createView(VK_IMAGE_VIEW_TYPE_2D_ARRAY, {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, arrays[i].currentLayer});
    arrays[i].texture->createView(VK_IMAGE_VIEW_TYPE_2D_ARRAY, VK_IMAGE_ASPECT_COLOR_BIT);
  }
  
//   std::cout << "Creating views" << "\n";
  
  // и пересобираем дескриптор
  // короче походу мне нужно пересобирать весь дескриптор пул
  // просто сказать нечего, я не понимаю как работает дескриптор ДО СИХ ПОР смцйузбоплцаощ
  imagesCount = arrays.size();
  recreateDescriptorPool();
  
//   std::cout << "Creating descriptor pools" << "\n";
  
//   if (imageLayout != VK_NULL_HANDLE) {
//     device->destroy(imageLayout);
//     imageLayout = VK_NULL_HANDLE;
//     throw std::runtime_error("che za huinya");
//   }
//   
//   {
//     yavf::DescriptorLayoutMaker dlm(device);
//     
//     //imagesCount
//     imageLayout = dlm.binding(0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT).create("image_array_layout");
//   }
//   
//   if (this->images != VK_NULL_HANDLE) {
//     yavf::vkCheckError("vkFreeDescriptorSets", nullptr, 
//     vkFreeDescriptorSets(device->handle(), pool, 1, &this->images));
//     this->images = VK_NULL_HANDLE;
//     throw std::runtime_error("che za huinya");
//     // нужно еще попробовать vkResetDescriptorPool
//   }
//   
//   {
//     yavf::DescriptorMaker dm(device);
//     
//     this->images = dm.layout(imageLayout).create(pool)[0];
//     
//     yavf::DescriptorUpdater du(device);
//   
//     du.currentSet(this->images).begin(0, 0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
//     for (size_t i = 0; i < arrays.size(); ++i) {
//       du.image(arrays[i].texture);
//     }
//     du.currentSet(this->samplers).begin(0, 0, VK_DESCRIPTOR_TYPE_SAMPLER).sampler(sampler.handle());
//     
//     du.update();
//   }
//   
  //throw std::runtime_error("dqwfqfqwfqfafwf");
}

void TextureLoader::clear() {
  // здесь мы удалим data, для того чтобы освободить память не особ нужными данными
  
  if (data != nullptr) {
    delete data;
    data = nullptr;
  }
}

size_t TextureLoader::overallState() const {
  // тут мы должны примерно прикинуть какое-нибудь число которое примерно показывает сколько нужно всего загрузить
}

size_t TextureLoader::loadingState() const {
  // тут нужно примерную степень загрузки выдать
}

std::string TextureLoader::hint() const {
  // тут просто смешной хинт какой-нибудь
}

size_t TextureLoader::hostSize() const {
  // размер на хосте
}

size_t TextureLoader::deviceSize() const {
  // размер на устройстве
}

std::vector<Texture> TextureLoader::getTextures(const std::string &name) const {
  auto itr = textureMap.find(name);
  if (itr == textureMap.end()) throw std::runtime_error("Could not find textures with name " + name);
  
  return itr->second;
}

Texture TextureLoader::getTexture(const std::string &name, const uint32_t &index) const {
  auto itr = textureMap.find(name);
  if (itr == textureMap.end()) throw std::runtime_error("Could not find texture with name " + name);
  
  return itr->second[index];
}

uint32_t TextureLoader::imageCount() const {
  return imagesCount;
}

yavf::DescriptorSet* TextureLoader::imageDescriptor() const {
  return images;
}

yavf::DescriptorSetLayout TextureLoader::imageSetLayout() const {
  return imageLayout;
}

uint32_t TextureLoader::samplerCount() const {
  return samplersCount;
}

yavf::DescriptorSet* TextureLoader::samplerDescriptor() const {
  return samplers;
}

yavf::DescriptorSetLayout TextureLoader::samplerSetLayout() const {
  return samplerLayout;
}

void TextureLoader::createData() {
  data = new Data();
}

void TextureLoader::recreateDescriptorPool() {
  if (pool != VK_NULL_HANDLE) {
    device->destroy(pool);
    pool = VK_NULL_HANDLE;
  }
  
  if (imageLayout != VK_NULL_HANDLE) {
    device->destroy(imageLayout);
    imageLayout = VK_NULL_HANDLE;
  }
  
  if (samplerLayout != VK_NULL_HANDLE) {
    device->destroy(samplerLayout);
    samplerLayout = VK_NULL_HANDLE;
  }
  
  {
    yavf::DescriptorPoolMaker dpm(device);
    
    pool = dpm.flags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
              .poolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, imagesCount)
              .poolSize(VK_DESCRIPTOR_TYPE_SAMPLER, samplersCount)
              .create("texture_descriptor_pool");
  }
  
//   std::cout << "Creating descriptor pool" << "\n";
  
  {
    yavf::DescriptorLayoutMaker dlm(device);
    
    //imagesCount
    imageLayout = dlm.binding(0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT, imagesCount).create("image_array_layout");
    samplerLayout = dlm.binding(0, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, samplersCount).create("sampler_array_layout");
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
    
    this->images = dm.layout(imageLayout).create(pool)[0];
    this->samplers = dm.layout(samplerLayout).create(pool)[0];
    
//     std::cout << "Creating descriptors" << "\n";
    
    for (size_t i = 0; i < arrays.size(); ++i) {
      images->add({VK_NULL_HANDLE, arrays[i].texture->view(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, i, 0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE});
    }
    images->update();
    
//     std::cout << "Updating image descriptors" << "\n";
    
    samplers->add({sampler, nullptr, VK_IMAGE_LAYOUT_MAX_ENUM, 0, 0, VK_DESCRIPTOR_TYPE_SAMPLER});
    samplers->update();
    
//     std::cout << "Updating descriptors" << "\n";
    
//     yavf::DescriptorUpdater du(device);
//   
//     du.currentSet(this->images->handle()).begin(0, 0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
//     for (size_t i = 0; i < arrays.size(); ++i) {
//       du.image(arrays[i].texture);
//     }
//     du.currentSet(this->samplers->handle()).begin(0, 0, VK_DESCRIPTOR_TYPE_SAMPLER).sampler(sampler.handle());
//     
//     du.update();
  }
}

// std::unordered_map<std::string, std::vector<Texture>> changingTextureMap;
//   
//   std::unordered_map<yavf::Image*, std::unordered_map<yavf::Image*, std::vector<VkImageCopy>>> alreadyLoaded;
//   std::vector<TemporalLoadData> needLoad;
//   std::unordered_map<VkExtent2D, std::vector<size_t>> localArrayIndices;
//   std::vector<TemporalImage> images;
//   
//   for (const auto &name : data->load) {
//     auto itr = textureMap.find(name);
//     if (itr == textureMap.end()) {
//       // это новая текстурка
//       
//       const TextureData &data = *this->data->textureDatas[name];
//       const VkExtent2D e{
//         data.size.width,
//         data.size.height
//       };
//       
//       auto imageItr = localArrayIndices.find(e);
//       if (imageItr == localArrayIndices.end()) {
//         auto layerItr = this->data->layerCount.find(e);
//         if (layerItr == this->data->layerCount.end()) throw std::runtime_error("Bad parser work");
//         
//         uint32_t layerCount = layerItr->second;
//         for (uint32_t i = 0; i < layerCount; ++i) {
//           yavf::Image* img = createImage(device, e.width, e.height, layers);
//           
//           images.push_back({img, 0});
//           localArrayIndices[e].push_back(images.size()-1);
//         }
//         
//         while (layerCount > 0) {
//           uint32_t layers = glm::min(uint32_t(TEXTURE_MAX_LAYER_COUNT), layerCount);
//           yavf::Image* img = createImage(device, e.width, e.height, layers);
//         
//           images.push_back({img, 0});
//           localArrayIndices[e].push_back(images.size()-1);
//           
//           layerCount -= layers;
//           // тут нужно будет запилить создание нескольких текстурок если количество слоев будет больше чем TEXTURE_MAX_LAYER_COUNT
//         }
//         
//         imageItr = localArrayIndices.find(e);
//         // тут нужно будет запилить создание нескольких текстурок если количество слоев будет больше чем TEXTURE_MAX_LAYER_COUNT
//       }
//       
//       // так в общем тут так же как и в старом вулкан рендере
//       // то есть нужно сначало загрузить текстурку в хост память
//       // а потом с помощью VkImageCopy раскидать по слоям
//       // загружаем (но мне тут наверное придется загружать из памяти)
//       int texWidth, texHeight, texChannels;
//       unsigned char* pixels = stbi_load((data.path).c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
//       VkDeviceSize imageSize = texWidth * texHeight * 4;
// 
//       // std::cout << "Image width: " << texWidth << " height: " << texHeight << "\n";
//     
//       if (pixels == nullptr) {
//         throw std::runtime_error("failed to load image " + name + " path: " + data.path + "!");
//       }
//       
//       // в будущем нам потребуется еще грузить сжатые текстурки
//       // нужно прикинуть что нам нужно поменять здесь
//       
//       yavf::ImageCreateInfo imageInfo{
//         0,
//         VK_IMAGE_TYPE_2D,
//         VK_FORMAT_R8G8B8A8_UNORM,
//         {uint32_t(texWidth), uint32_t(texHeight), 1},
//         1, // не помешало бы вынести в ImageCreateInfo
//         1,
//         VK_SAMPLE_COUNT_1_BIT, // по идее должно быть определенно в настройках
//         VK_IMAGE_TILING_LINEAR,
//         VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
//         VK_IMAGE_ASPECT_COLOR_BIT,
//         VMA_MEMORY_USAGE_CPU_ONLY
//       };
// 
//       yavf::Image* staging = device->createImage(imageInfo);
//       memcpy(staging->ptr(), pixels, imageSize);
//       
//       stbi_image_free(pixels);
//       
//       std::vector<Texture> textureVector;
//       
//       uint32_t imageCount = data.count;
//       uint32_t lastCount = 0;
//       size_t currentIndex = 0;
//       while (imageCount > 0) {
//         // собираем здесь VkImageCopy
//         
//         const size_t imageArrayIndex = imageItr->second[currentIndex];
//         const uint32_t currentLayer = images[imageArrayIndex].currentLayer;
//         ++currentIndex;
//         if (currentLayer == TEXTURE_MAX_LAYER_COUNT) continue;
//         
//         uint32_t newCount = ((TEXTURE_MAX_LAYER_COUNT - currentLayer) > imageCount) ? imageCount : TEXTURE_MAX_LAYER_COUNT - currentLayer;
//         
//         std::vector<VkImageCopy> copies;
//         copy(currentLayer, lastCount, newCount, data.rows, data.columns, data.size, copies);
//         
//         needLoad.push_back({staging, images[imageArrayIndex].img, copies});
//         
//         for (size_t i = 0; i < newCount; ++i) {
//           textureVector.push_back({static_cast<uint32_t>(imageArrayIndex), static_cast<uint32_t>(currentLayer + i), 0});
//         }
//         
//         imageCount -= newCount;
//         lastCount = newCount;
//       }
//       
//       changingTextureMap[name] = textureVector;
//       
//       continue;
//     }
//     
//     // пытаемся загрузить уже имеющиеся текстуры
//     for (size_t i = 0; i < itr->second.size(); ++i) {
//       const ImageArrayData &data = arrays[itr->second[i].imageArrayIndex];
//       
//       const VkExtent2D e{
//         data.texture->param().size.width,
//         data.texture->param().size.height
//       };
//       
//       auto imageItr = localArrayIndices.find(e);
//       if (imageItr == localArrayIndices.end()) {
//         // мне еще нужно количество слоев 
//         // это легко можно узнать из TextureData
//         // которые у нас подготавливаются в load'е
//         
//         auto layerItr = this->data->layerCount.find(e);
//         if (layerItr == this->data->layerCount.end()) throw std::runtime_error("Bad parser work");
//         
//         uint32_t layerCount = layerItr->second;
//         while (layerCount > 0) {
//           uint32_t layers = glm::min(uint32_t(TEXTURE_MAX_LAYER_COUNT), layerCount);
//           yavf::Image* img = createImage(device, e.width, e.height, layers);
//         
//           images.push_back({img, 0});
//           localArrayIndices[e].push_back(images.size()-1);
//           
//           layerCount -= layers;
//           // тут нужно будет запилить создание нескольких текстурок если количество слоев будет больше чем TEXTURE_MAX_LAYER_COUNT
//         }
//         
//         imageItr = localArrayIndices.find(e);
//       }
//       
//       // копирование тоже происходит в разные места, нужно это учесть
//       size_t imageIndex = 0;
//       for (uint32_t i = 0; i < imageItr->second.size(); ++i) {
//         const size_t index = imageItr->second[i];
//         if (images[index].currentLayer == TEXTURE_MAX_LAYER_COUNT) continue;
//         
//         imageIndex = index;
//         break;
//       }
//       
//       const uint32_t layer = images[imageIndex].currentLayer;
//       const VkImageCopy copy{
//         {
//           VK_IMAGE_ASPECT_COLOR_BIT,
//           0,
//           itr->second[i].imageArrayLayer,
//           1
//         },
//         {0, 0, 0},
//         {
//           VK_IMAGE_ASPECT_COLOR_BIT,
//           0,
//           layer,
//           1
//         },
//         {0, 0, 0},
//         data.texture->param().size
//       };
//       
//       ++images[imageIndex].currentLayer;
//       
//       alreadyLoaded[data.texture][images[imageIndex].img].push_back(copy);
//       
//       //auto textureItr = changingTextureMap.insert(std::make_pair(name, std::vector<Texture>())).first;
//       //textureItr->second.push_back({static_cast<uint32_t>(imageIndex), layer, textureMap[name][i].samplerIndex});
//       
//       changingTextureMap[name].push_back({static_cast<uint32_t>(imageIndex), layer, textureMap[name][i].samplerIndex});
//       
//       // как то так это выглядит
//       // значит images нужно загрузить строго в последовательности разрешения текстурок
//     }
//   }

// теперь удаляем старые картинки
//   for (uint32_t i = 0; i < needLoad.size(); ++i) {
//     device->destroy(needLoad[i].src);
//   }
//   
//   for (uint32_t i = 0; i < arrays.size(); ++i) {
//     device->destroy(arrays[i].texture);
//   }
//   arrays.clear();
//   
//   // затем создаем новые в интересующем нас порядке
//   std::vector<size_t> idx(images.size());
//   for (size_t i = 0; i < images.size(); ++i) {
//     idx[i] = i;
//   }
//   
//   std::sort(idx.begin(), idx.end(), [&] (const size_t &first, const size_t &second) {
//     const auto &firstSize = images[first].img->param().size;
//     const auto &secondSize = images[second].img->param().size;
//     
//     const auto &mulFirst = firstSize.width * firstSize.height;
//     const auto &mulSecond = secondSize.width * secondSize.height;
//     
//     if (mulFirst > mulSecond) return true;
//     else if (mulFirst == mulSecond) {
//       if (images[first].currentLayer > images[second].currentLayer) return true;
//     }
//     
//     return false;
//   });
//   
//   arrays.resize(images.size());
//   for (size_t i = 0; i < images.size(); ++i) {
//     const size_t index = idx[i];
//     const TemporalImage &tmp = images[index];
//     
//     yavf::ImageCreateInfo imageInfo{
//       0,
//       VK_IMAGE_TYPE_2D,
//       VK_FORMAT_R8G8B8A8_UNORM,
//       tmp.img->param().size,
//       1, // это должно контролироваться с помощью настроек
//       tmp.currentLayer,
//       VK_SAMPLE_COUNT_1_BIT,
//       VK_IMAGE_TILING_OPTIMAL,
//       VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
//       VK_IMAGE_ASPECT_COLOR_BIT,
//       VMA_MEMORY_USAGE_GPU_ONLY
//     };
//     
//     arrays[index].texture = device->createImage(imageInfo);
//     arrays[index].currentLayer = tmp.currentLayer;
//     arrays[index].descriptorIndex = 0;
//   }
//   
//   task->begin();
//   
//   // и копируем туда все ресурсы
//   for (size_t i = 0; i < images.size(); ++i) {
//     task->setBarrier(images[i].img, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
//     task->setBarrier(arrays[i].texture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
//     task->copy(images[i].img, arrays[i].texture);
//     task->setBarrier(arrays[i].texture, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
//   }
//   
//   task->end();
//   
//   task->start();
//   task->wait();
//   
//   textureMap = changingTextureMap;
//   arrayIndices = localArrayIndices;
//   
//   imagesCount = arrays.size();
