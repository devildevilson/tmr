#ifndef LOADER_H
#define LOADER_H

#include <string>
#include <unordered_map>
#include "ResourceID.h"

// Loader теперь иерархическая стурктура, что это означает?
// означет - Loader СНАЧАЛО загрузит все ресурсы вниз по иерархии
// и только после этого нужный ресурс

class Resource;
class Conflict;
class ModificationParser;

class Loader {
public:
  virtual ~Loader() = default;

  // тут более целесообразно использовать ResourceID при загрузке
  // я планировал что приходящий Resource я смогу как то приводить к локальному типу
  // но у меня чет не получилось догадаться как конкретно сделать это
  virtual bool load(const ModificationParser* modifications, const Resource* resource) = 0;
  virtual bool unload(const ResourceID &id) = 0;
  virtual void end() = 0;
  
  virtual void clear() = 0;
  
  virtual size_t overallState() const = 0;
  virtual size_t loadingState() const = 0;
  virtual std::string hint() const = 0;
};

#endif
