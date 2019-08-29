#ifndef RESOURCE_PARSER_H
#define RESOURCE_PARSER_H

#include <unordered_map>
#include "ResourceID.h"

#include "nlohmann/json.hpp"

struct ErrorDesc {
  ErrorDesc() {}
  ErrorDesc(const size_t &loaderMark, const size_t &type, const std::string &desc) : loaderMark(loaderMark), type(type), description(desc) {}

  size_t loaderMark;
  size_t type;
  std::string description;
};

struct WarningDesc {
  WarningDesc() {}
  WarningDesc(const size_t &mark, const size_t &type, const std::string &desc) : mark(mark), type(type), description(desc) {}

  size_t mark;
  size_t type;
  std::string description;
};

class Resource;
class Modification;

class ResourceParser {
public:
  virtual ~ResourceParser() = default;

  virtual bool canParse(const std::string &key) const = 0;

  virtual bool parse(const Modification* mod,
                     const std::string &pathPrefix,
                     const nlohmann::json &data,
                     std::vector<Resource*> &resource,
                     std::vector<ErrorDesc> &errors,
                     std::vector<WarningDesc> &warnings) = 0;

  virtual bool forget(const ResourceID &name) = 0;

  virtual Resource* getParsedResource(const ResourceID &id) = 0;
  virtual const Resource* getParsedResource(const ResourceID &id) const = 0;
};

#endif //RESOURCE_PARSER_H
