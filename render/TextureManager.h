#ifndef TEXTURE_MANAGER_H
#define TEXTURE_MANAGER_H

#include "../resources/ResourceManager.h"
#include "VulkanRender2.h"

struct FullInfo {
  std::string prefix;
  ImageCreateInfo info;
};

class TextureManager : public PluginParser {
public:
  TextureManager();
  virtual ~TextureManager();
  
  bool addFile(const Conflict & conflict) override;
  void load() override;
  void clear() override;
  
  void print() override;
  
  // собираем загруженные текстуры
  std::map<std::pair<uint32_t, uint32_t>, uint32_t> getTextureMeta() const;
  std::vector<FullInfo> getTexturesInfo() const;
private:
  std::vector<FullInfo> textures; // инфа по загрузке
  std::map<std::pair<uint32_t, uint32_t>, uint32_t> textureMeta;
  //char dummy[8];
  // map<id, Deleter> resourceDeleters; // в будущем удалять ненужные вещи наверное нужно будет отсюда
  // то что мы удаляем, может входить в тот же размер в котором мы не удаляем ресурсы и тип нужно все равно все пересобирать
  
  // почему то вылетает segfault когда textures находится ниже textureMeta
};

#endif
