#ifndef MODIFICATION_CONTAINER_H
#define MODIFICATION_CONTAINER_H

#include "Modification.h"
#include "ModificationParser.h"
#include "ResourceParser.h"
#include "Conflict.h"

#include "TypelessContainer.h"
#include "MemoryPool.h"

#include <unordered_map>
#include <vector>

class Loader;

class ModificationContainer : public ModificationParser {
public:
  struct CreateInfo {
    size_t parserSize;
  };
  ModificationContainer(const CreateInfo &info);
  ~ModificationContainer();

  const Modification* loadModData(const std::string &path) override;
  void destroy(const Modification* mod) override;

  void parseModification(const Modification* mod) override;

  void clean() override;

  // TODO: добавить обход конфликтов

  // мы можем сделать не пересекающиеся группы конфликтов используя указатель на парсер
  const Conflict* conflict(const ResourceID &id) const override;
//  std::unordered_map<ResourceID, const Conflict*> & conflicts() override;
//  const std::unordered_map<ResourceID, const Conflict*> & conflicts() const override;

  const std::vector<const Modification*> & parsedModifications() const override;

  size_t overallSize() const override;
  size_t overallGPUSize() const override;

//  template <typename T, typename ...Args>
//  T* createParser(Args&& ...args) {
//    auto ptr = parserContainer.create<T>(std::forward<Args>(args)...);
//    parsers.push_back(ptr);
//    return ptr;
//  }

  void addParser(ResourceParser* parser);

  void addTextureSupport(ResourceParser* textureParser, Loader* textureLoader);
private:
  ResourceParser* textureParser;
  Loader* textureLoader;

//  TypelessContainer parserContainer;
  std::vector<ResourceParser*> parsers;

  MemoryPool<Modification, sizeof(Modification)*20> modsPool;
  std::vector<Modification*> createdModInfos;
  std::vector<const Modification*> parsedMods;


  MemoryPool<Conflict, sizeof(Conflict)*100> conflictPool;
  std::vector<Conflict*> conflictsArray;
  // если мы свяжем конфликт мапы с парсером который это дело обработал, то у нас получатся не пересекающиеся множества
  std::unordered_map<ResourceID, const Conflict*> conflictsMap;
};

#endif //MODIFICATION_CONTAINER_H
