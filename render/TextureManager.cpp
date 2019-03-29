#include "TextureManager.h"

#include "Globals.h"
#include "VulkanRender2.h"

#include "stbi_image_header.h"

bool loadTextureJson(const nlohmann::json &j, ImageCreateInfo &info) {
  for (auto concreteTIt = j.begin(); concreteTIt != j.end(); ++concreteTIt) {
    if (concreteTIt.value().is_string() && concreteTIt.key().compare("path") == 0) {
      info.path = concreteTIt.value().get<std::string>();
      continue;
    }
    
    if (concreteTIt.value().is_string() && concreteTIt.key().compare("name") == 0) {
      info.name = concreteTIt.value().get<std::string>();
      continue;
    }
    
    if (concreteTIt.value().is_number_unsigned() && concreteTIt.key().compare("count") == 0) {
      info.count = concreteTIt.value().get<size_t>();
      continue;
    }
    
    if (concreteTIt.value().is_number_unsigned() && concreteTIt.key().compare("rows") == 0) {
      info.rows = concreteTIt.value().get<size_t>();
      continue;
    }
    
    if (concreteTIt.value().is_number_unsigned() && concreteTIt.key().compare("columns") == 0) {
      info.columns = concreteTIt.value().get<size_t>();
      continue;
    }
    
    if (concreteTIt.value().is_object() && concreteTIt.key().compare("size") == 0) {
      for (auto sizeIt = concreteTIt.value().begin(); sizeIt != concreteTIt.value().end(); ++sizeIt) {
        if (sizeIt.value().is_number_unsigned() && sizeIt.key().compare("width") == 0) {
          info.size.width = sizeIt.value().get<uint32_t>();
          continue;
        }
        
        if (sizeIt.value().is_number_unsigned() && sizeIt.key().compare("height") == 0) {
          info.size.height = sizeIt.value().get<uint32_t>();
          continue;
        }
      }
    }
  }
  
  if (info.name.empty()) info.name = info.path;
  if (info.count == 0) info.count = 1;
  if (info.rows == 0) info.rows = 1;
  if (info.columns == 0) info.columns = 1;
  
  return true;
}

TextureManager::TextureManager() {}

TextureManager::~TextureManager() {}

bool TextureManager::addFile(const Conflict & conflict) {
  if (conflict.getType() != TEXTURE) return false;
  
  FullInfo info{
    "",
    {
      "", "", "", 1, 0, 0, {0, 0, 1}
    }
  };
  
//   ImageCreateInfo info{
//     "", "", "", 1, 0, 0, {0, 0, 1}
//   };
  
  const Variant &var = conflict.getSelectedVariant();
  //loadTextureJson(conflict.getSelected(), info);
  loadTextureJson(var.get(), info.info);
  
  info.prefix = var.relatedMod()->pathPrefix;
  
  int32_t width = info.info.size.width, height = info.info.size.height, channels;
  if (width <= 1 || height <= 1) {
    stbi_info((info.prefix + info.info.path).c_str(), &width, &height, &channels);
  }
  // скорее всего мне будет сложно избавиться от чтения два раза
  // если необходимо читать один раз то тогда придется кратковременно держать ВСЕ текстурки в памяти компа
  // возможно нужно будет держать текстурки и так и сяк, но в случае с дискретными видеокартами
  // они почти мгновенно улетают в память карточки
  
  auto it = textureMeta.find(std::make_pair(width, height));
  if (it == textureMeta.end()) {
    it = textureMeta.insert(std::make_pair(std::make_pair(width, height), 0)).first;
  }
  
  it->second += info.info.count;
  
  textures.push_back(info);
  
  return true;
}

void TextureManager::load() {
  for (const auto &pair : textureMeta) {
    Global::render()->precacheLayers({pair.first.first, pair.first.second}, pair.second);
  }
  
  for (size_t i = 0; i < textures.size(); ++i) {
    Global::render()->loadTexture(textures[i].prefix, textures[i].info);
    //stbi_image_free(textures[i].pixels);
  }
}

void TextureManager::clear() {
  textureMeta.clear();
  textures.clear();
}

void TextureManager::print() {
  size_t count = 0;
  for (const auto &pair : textureMeta) {
    std::cout << count << ". Images size: w " << pair.first.first << " h " << pair.first.second << " count " << pair.second << "\n";
    ++count;
  }
  
  std::cout << "Textures: " << "\n";
  for (size_t i = 0; i < textures.size(); ++i) {
    std::cout << textures[i].info.name << " count " << (textures[i].info.count == 0 ? 1 : textures[i].info.count) << "\n";
  }
}

std::map<std::pair<uint32_t, uint32_t>, uint32_t> TextureManager::getTextureMeta() const {
  return textureMeta;
}

std::vector<FullInfo> TextureManager::getTexturesInfo() const {
  return textures;
}
