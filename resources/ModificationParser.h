#ifndef MODIFICATION_PARSER_H
#define MODIFICATION_PARSER_H

#include <unordered_map>
#include <vector>

#include "ResourceID.h"

class Conflict;
class Modification;

class ModificationParser {
public:
  virtual ~ModificationParser() = default;

  virtual const Modification* loadModData(const std::string &path) = 0;
  virtual void destroy(const Modification* mod) = 0;

  virtual void parseModification(const Modification* mod) = 0;

  virtual void clean() = 0;

  virtual const Conflict* conflict(const ResourceID &id) const = 0;

  virtual const std::vector<const Modification*> & parsedModifications() const = 0;

  virtual size_t overallSize() const = 0;
  virtual size_t overallGPUSize() const = 0;
};

#endif //MODIFICATION_PARSER_H
