#include "ModificationContainer.h"

#include <fstream>
#include "nlohmann/json.hpp"

#include "filesystem/path.h"

#include "Utility.h"

#include "Loader.h"
#include "Resource.h"

ModificationContainer::ModificationContainer(const CreateInfo &info) : textureParser(nullptr), textureLoader(nullptr) {}

ModificationContainer::~ModificationContainer() {
//  for (auto parser : parsers) {
//    parserContainer.destroy(parser);
//  }

  for (auto mod : createdModInfos) {
    modsPool.deleteElement(mod);
  }

  for (auto conflict : conflictsArray) {
    conflictPool.deleteElement(conflict);
  }
}

const Modification* ModificationContainer::loadModData(const std::string &path) {
  std::ifstream file(path);
  if (!file) {
    throw std::runtime_error("Could not load mod");
  }

  nlohmann::json json;

  file >> json;

  Modification::CreateInfo modInfo;
  modInfo.pathStr = path;
  for (auto itr = json.begin(); itr != json.end(); ++itr) {
    // узнаем данные мода
    // нужно загрузить название, описание, автора, версию, какую-нибудь ссылку на мод
    // также мы загрузим здесь изображение для мода

    //textureParser->parse(asagsgag);

    if (itr.value().is_string() && itr.key() == "version") {
      // версию нужно получить из стринга
    }

    if (itr.value().is_string() && itr.key() == "name") {
      modInfo.nameStr = itr.value().get<std::string>();
    }

    if (itr.value().is_string() && itr.key() == "description") {
      modInfo.descriptionStr = itr.value().get<std::string>();
    }

    if (itr.value().is_string() && itr.key() == "author") {
      modInfo.authorStr = itr.value().get<std::string>();
    }
  }

  //textureLoader->load(resource);

  Modification* mod = modsPool.newElement(modInfo);
  createdModInfos.push_back(mod);

  return mod;
}

void ModificationContainer::destroy(const Modification* mod) {
  for (size_t i = 0; i < createdModInfos.size(); ++i) {
    if (createdModInfos[i] == mod) {
      modsPool.deleteElement(createdModInfos[i]);

      std::swap(createdModInfos[i], createdModInfos.back());
      createdModInfos.pop_back();

      return;
    }
  }

  throw std::runtime_error("Bad mod pointer");
}

void ModificationContainer::parseModification(const Modification* mod) {
  filesystem::path modPath = mod->path();

  const std::string ext = modPath.extension();

  PRINT_VAR("ext", ext)
  if (ext == "zip") {
    // в этом случае нам по идее не нужен префикс пути
    // но нужен зато указатель на открытый зип архив
  } else {
    // нужно взять префикс пути
  }

  const std::string prefix = modPath.parent_path().str();
  PRINT_VAR("prefix", prefix)

  std::ifstream file(mod->path());
  if (!file) {
    throw std::runtime_error("Could not load mod");
  }

  nlohmann::json json;

  file >> json;

  Modification* _mod = nullptr;
  for (auto pmod : createdModInfos) {
    if (pmod == mod) {
      _mod = pmod;
    }
  }

  if (_mod == nullptr) throw std::runtime_error("Mod not found");

  std::vector<ErrorDesc> errors;
  std::vector<WarningDesc> warnings;
  for (auto itr = json.begin(); itr != json.end(); ++itr) {

    if (itr.value().is_object() && itr.key() == "data") {
      for (auto parser : parsers) {
        for (auto dataIt = itr.value().begin(); dataIt != itr.value().end(); ++dataIt) {
          if (parser->canParse(dataIt.value())) {
            // этот парсер нужно поставить как текущий

            // если обрабатываем zip то мы должны передать указатель на него, вместо префикса

            parser->parse(mod, prefix, dataIt.value(), _mod->resources(), errors, warnings);
            break;
          }
        }
      }

      break;
    }
  }

  // где мы выводим ошибки на экран?
  // наверное сразу при парсинге

  if (!errors.empty()) throw std::runtime_error("There are "+std::to_string(errors.size())+" parsing errors");

  if (_mod->resources().empty()) throw std::runtime_error("Empty mod "+mod->name());

  for (auto res : _mod->resources()) {
    auto itr = conflictsMap.find(res->id());
    if (itr == conflictsMap.end()) {
      auto conflict = conflictPool.newElement(Conflict::CreateInfo{res});
      conflictsArray.push_back(conflict);
      conflictsMap[res->id()] = conflict;
      continue;
    }

    for (auto _conflict : conflictsArray) {
      if (itr->second == _conflict) {
        _conflict->add(res);
        break;
      }
    }
  }

  parsedMods.push_back(mod);
}

void ModificationContainer::clean() {
  for (size_t i = 0; i < createdModInfos.size(); ++i) {
    bool parsedFlag = false;
    for (auto parsed : parsedMods) {
      if (parsed == createdModInfos[i]) {
        parsedFlag = true;
        break;
      }
    }

    if (!parsedFlag) {
      modsPool.deleteElement(createdModInfos[i]);

      std::swap(createdModInfos[i], createdModInfos.back());
      createdModInfos.pop_back();
    }
  }
}

const Conflict* ModificationContainer::conflict(const ResourceID &id) const {
  auto itr = conflictsMap.find(id);
  if (itr != conflictsMap.end()) return itr->second;

  return nullptr;
}

//std::unordered_map<ResourceID, const Conflict*> & ModificationContainer::conflicts() {
//  return conflictsMap;
//}
//
//const std::unordered_map<ResourceID, const Conflict*> & ModificationContainer::conflicts() const {
//  return conflictsMap;
//}

const std::vector<const Modification*> & ModificationContainer::parsedModifications() const {
  return parsedMods;
}

size_t ModificationContainer::overallSize() const {
  return 0;
}

size_t ModificationContainer::overallGPUSize() const {
  return 0;
}

void ModificationContainer::addParser(ResourceParser* parser) {
  parsers.push_back(parser);
}

void ModificationContainer::addTextureSupport(ResourceParser* textureParser, Loader* textureLoader) {
  this->textureParser = textureParser;
  this->textureLoader = textureLoader;
}