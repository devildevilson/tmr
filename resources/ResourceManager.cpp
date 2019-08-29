#include "ResourceManager.h"

#include "TextureLoader.h"
#include "HardcodedLoaders.h"

#include <fstream>

PluginParserImpl::PluginParserImpl()/* : loading(nullptr)*/ {
  
}

PluginParserImpl::~PluginParserImpl() {
//   if (loading != nullptr) {
//     delete loading;
//   }
}

void PluginParserImpl::add( ResourceParser* loader) {
  parsers.push_back(loader);
}

void PluginParserImpl::set(const std::vector<ResourceParser*> &parsers) {
  this->parsers = parsers;
}

void PluginParserImpl::parseJSON(const nlohmann::json &data) {
  // тут нам нужно разбирать определенное количество даты
  // только информацию о моде (плагине)
  // остальные вещи уходят в лоадеры
  // тут в будуещем нужно передавать файловый дескриптор
  // для того чтобы читать данные из zip архива
  
//   std::vector<ErrorDesc> error;
//   std::vector<WarningDesc> warning;
//   GameData mod;
  
  bool hasPrefix = false;
  bool hasVersion = false;
  bool validData = false;
  std::string prefix;
  uint32_t version = 1;
  for (auto it = data.begin(); it != data.end(); ++it) {
    if (it.value().is_string() && it.key().compare("prefix") == 0) {
      prefix = it.value();
      hasPrefix = true;
    }
    
    if (it.value().is_number_unsigned() && it.key().compare("version") == 0) {
      version = it.value().get<uint32_t>();
      hasVersion = true;
    }
  }
  
  //Plugin p;
  auto itr = pluginsMap.insert(std::make_pair(prefix, Plugin{})).first;
  itr->second.name = prefix;
  
  //nlohmann::json newData = data;
  // загружаем мод...
  for (auto it = data.begin(); it != data.end(); ++it) {
    if (it.value().is_object() && it.key().compare("data") == 0) {
      for (auto dataIt = it.value().begin(); dataIt != it.value().end(); ++dataIt) {
        bool parsed = false;
        //size_t resourceSize = loading.resourceArray.size();
        for (size_t i = 0; i < parsers.size(); ++i) {
//           parsed = parsed || loaders[i]->parse(this->prefix, "", dataIt.value(), resources, temporal->errors, temporal->warnings);
          if (parsed) {
            parsed = false;
            break;
          }
        }
      }
    }
    
    if (it.value().is_object() && it.key().compare("info") == 0) {
      for (auto infoIt = it.value().begin(); infoIt != it.value().end(); ++infoIt) {
        if (infoIt.value().is_string() && infoIt.key().compare("description") == 0) {
          itr->second.desc = infoIt.value();
        }
      }
    }
  }
  
  // как то так это выглядит
  // нужно еще ошибки ловить 
  // также пока на первое время тут надо бы уметь также обрабатывать какие-то куски мода
  // хотя может и ненадо
  // тут неплохо было бы добавить еще кое какие данные
}

void PluginParserImpl::unloadJSON(const std::string &modName) {
  auto itr = pluginsMap.find(modName);
  if (itr == pluginsMap.end()) return;
  
  const std::vector<Resource*> &res = itr->second.relatedResources;
  for (size_t i = 0; i < res.size(); ++i) {
    bool parsed = false;
    for (size_t j = 0; j < parsers.size(); ++j) {
      parsed = parsed || parsers[j]->forget(res[i]->name);
      if (parsed) {
        parsed = false; 
        break;
      }
    }
  }
  
  pluginsMap.erase(itr);
}

std::vector<Resource*> PluginParserImpl::getResources() const {
  //if (loading == nullptr) throw std::runtime_error("Not in loading state");
  
  //return loading.resourceArray;
  
  std::vector<Resource*> res;
  for (size_t i = 0; i < parsers.size(); ++i) {
    const auto &tmp = parsers[i]->getLoadedResource();
    for (const auto &pair : tmp) {
      res.push_back(pair.second);
    }
  }
  
  return res;
}

size_t PluginParserImpl::getOverallSize() const {
  // тут поди нужно просто сложить сайзы всех ресурсов
  // как определить хост/не хост?
  // может быть размер должен быть доступен всегда?
  //if (loading == nullptr) throw std::runtime_error("Not in loading state");
  
  size_t sum = 0;
  for (const auto &pair : pluginsMap) {
    for (size_t i = 0; i < pair.second.relatedResources.size(); ++i) {
      sum += pair.second.relatedResources[i]->size;
    }
  }
  
  return sum;
}

size_t PluginParserImpl::getHostSize() const {
  // как определить хост/не хост?
  
  return 0;
}

std::vector<Resource*> PluginParserImpl::getPluginResources(const std::string &name) const {
  //if (loading == nullptr) throw std::runtime_error("Not in loading state");
  
//   auto itr = plugins.find(name);
//   if (itr == plugins.end()) throw std::runtime_error("Cannot find plugin with that name");
//   
//   std::vector<Resource*> res;
//   for (const auto name : itr->second.relatedResources) {
//     res.push_back(loading.resourceNames.at(name));
//   }
//   
//   return res;
  
  auto itr = pluginsMap.find(name);
  if (itr == pluginsMap.end()) throw std::runtime_error("Mod with name " + name + " is not loaded");
  
  return itr->second.relatedResources;
}

std::vector<Conflict*> PluginParserImpl::conflicts() const {
  //if (loading == nullptr) throw std::runtime_error("Not in loading state");
  
  std::vector<Conflict*> con;
  for (size_t i = 0; i < parsers.size(); ++i) {
    const auto &tmp = parsers[i]->getConflicts();
    for (const auto &pair : tmp) {
      con.push_back(pair.second);
    }
  }
  
  return con;
}

std::vector<Plugin> PluginParserImpl::plugins() const {
  std::vector<Plugin> tmp;
  for (const auto plugin : pluginsMap) {
    tmp.push_back(plugin.second);
  }
  
  return tmp;
}

ParserHelper::ParserHelper(const CreateInfo &info) : temporal(nullptr) {
  this->prefix = info.prefix;
  this->textureLoader = info.textureLoader;
  this->entityLoader = info.entityLoader;
  this->mapLoader = info.mapLoader;
  
  loaders.push_back(textureLoader);
}

ParserHelper::~ParserHelper() {
  clear();
}
  
void ParserHelper::loadPlugin(const std::string &path) {
  std::ifstream file(prefix + path);
  
  if (!file) {
    throw std::runtime_error("Bad file path " + path);
  }
  
  const size_t index = path.find_last_of("/");
  const std::string folder = path.substr(0, index+1);
  const std::string realPrefix = prefix + folder;
  
  if (temporal == nullptr) {
    temporal = new TemporalData();
  }
  
  nlohmann::json data;
  file >> data;
  
  bool hasPrefix = false;
  bool hasVersion = false;
  bool validData = false;
  std::string prefix;
  uint32_t version = 1;
  for (auto it = data.begin(); it != data.end(); ++it) {
    if (it.value().is_string() && it.key().compare("prefix") == 0) {
      prefix = it.value();
      hasPrefix = true;
    }
    
    if (it.value().is_number_unsigned() && it.key().compare("version") == 0) {
      version = it.value().get<uint32_t>();
      hasVersion = true;
    }
  }
  
  // проверка на невалидные данные мода
  
  //Plugin p;
  auto itr = plugins.insert(std::make_pair(prefix, PluginCommonData{})).first;
  itr->second.name = prefix;
  
  std::vector<Resource*> resources;
  //nlohmann::json newData = data;
  // загружаем мод...
  for (auto it = data.begin(); it != data.end(); ++it) {
    if (it.value().is_object() && it.key().compare("data") == 0) {
      for (auto dataIt = it.value().begin(); dataIt != it.value().end(); ++dataIt) {
        bool parsed = false;
        //size_t resourceSize = loading.resourceArray.size();
        for (size_t i = 0; i < loaders.size(); ++i) {
          parsed = parsed || loaders[i]->parse(realPrefix, "", dataIt.value(), resources, temporal->errors, temporal->warnings);
          if (parsed) {
            parsed = false;
            break;
          }
        }
      }
    }
    
    if (it.value().is_object() && it.key().compare("info") == 0) {
      for (auto infoIt = it.value().begin(); infoIt != it.value().end(); ++infoIt) {
        if (infoIt.value().is_string() && infoIt.key().compare("description") == 0) {
          itr->second.desc = infoIt.value();
        }
      }
    }
  }
  
  for (const auto &res : resources) {
//     itr->second.conflicts.push_back(res->conflict);
    temporal->conflicts[prefix].insert(std::make_pair(res->name, res->conflict));
  }
  
  for (size_t i = 0; i < temporal->warnings.size(); ++i) {
    std::cout << "Parser warning " << temporal->warnings[i].description << "\n";
  }
  
  for (size_t i = 0; i < temporal->errors.size(); ++i) {
    std::cout << "Parser error " << temporal->errors[i].description << "\n";
  }
  
  if (!temporal->errors.empty()) throw std::runtime_error("There are errors");
  
  // возможно тут нужно дополнительно распарсить мод
  // то есть например нет ничего криминального в том чтобы загрузить игру сохраненную, например, с модом хд текстур
  // собственно без него
  // как это можно предусмотреть? по идее если в моде содержатся только апдейт данные то его можно без риска отключить
}

void ParserHelper::loadPlugin(const PluginCommonData &path) {
  // тут пока ничего
}

void ParserHelper::parsePlugins() {
  // тут чекаем всех в папке
  // и загружаем основные данные
  // также нужно подумать куда пойдут иконки модов?
}

void ParserHelper::clear() {
//   for (size_t i = 0; i < loaders.size(); ++i) {
//     // было бы неплохо их тут очистить
//     //loaders[i]->
//   }
  
  textureLoader->clear();
  entityLoader->clear();
  mapLoader->clear();
  
  if (temporal != nullptr) {
    delete temporal;
    temporal = nullptr;
  }
}

std::unordered_map<std::string, PluginCommonData> ParserHelper::getPluginData() const {
  return plugins;
}

TemporalData* ParserHelper::getTemporalData() const {
  return temporal;
}
