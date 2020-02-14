#ifndef RESOURCE_PARSER_H
#define RESOURCE_PARSER_H

#include <unordered_map>
#include "ResourceID.h"

#include "nlohmann/json.hpp"
#include <mutex>

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

// template <typename LoadData>
// class ResourceParser2 {
// public:
//   struct CreateInfo {
//     std::string key1;
//     std::string key2;
//   };
//   ResourceParser2(const CreateInfo &info) : key1(info.key1), key2(info.key2) {}
//   virtual ~ResourceParser2() {
//     delete loadData;
//   }
//   
//   bool canParse(const std::string &key) const {
//     return (!key1.empty() && key1 == key) || (!key2.empty() && key2 == key);
//   };
//   
//   bool parse(const Modification* mod,
//              const std::string &pathPrefix,
//              const nlohmann::json &data,
//              std::vector<Resource*> &resource,
//              std::vector<ErrorDesc> &errors,
//              std::vector<WarningDesc> &warnings) {
//     if (loadData == nullptr) loadData = new LoadData;
//     
//     if (data.is_string()) {
//       const std::string &path = data.get<std::string>();
//       std::ifstream file(pathPrefix + path);
//       if (!file) {
//         ErrorDesc desc(4123, ERROR_FILE_NOT_FOUND, "Could not load file "+pathPrefix+path);
//         std::cout << "Error: " << desc.description << "\n";
//         errors.push_back(desc);
//         return false;
//       }
//       
//       nlohmann::json json;
//       file >> json;
//       return parse(mod, pathPrefix, json, resource, errors, warnings);
//     } else if (data.is_array()) {
//       bool ret = true;
//       for (size_t i = 0; i < data.size(); ++i) {
//         ret = ret && parse(mod, pathPrefix, data[i], resource, errors, warnings);
//       }
//       return ret;
//     } else if (data.is_object()) {
//       return validate(mod, pathPrefix, data, resource, errors, warnings);
//     }
//     
//     return false;
//   }
//   
//   bool forget(const ResourceID &name) {
//     if (loadData == nullptr) throw std::runtime_error("Not in loading state");
//     
//     return loadData->remove(name);
//   }
// 
//   Resource* getParsedResource(const ResourceID &id) {
//     if (loadData == nullptr) throw std::runtime_error("Not in loading state");
//     
//     return loadData->find(id);
//   }
//   
//   const Resource* getParsedResource(const ResourceID &id) const {
//     if (loadData == nullptr) throw std::runtime_error("Not in loading state");
//     
//     return loadData->find(id);
//   }
//   
//   virtual bool validate(const Modification* mod,
//                         const std::string &pathPrefix,
//                         const nlohmann::json &data,
//                         std::vector<Resource*> &resource,
//                         std::vector<ErrorDesc> &errors,
//                         std::vector<WarningDesc> &warnings) = 0;
// protected:
//   std::string key1;
//   std::string key2;
//   
//   LoadData* loadData;
// };
// 
// template <typename T>
// class ValidateVariable {
// public:
//   virtual ~ValidateVariable() = default;
//   
//   virtual void validate(const nlohmann::json::iterator &itr, T& var, std::vector<ErrorDesc> &errors, std::vector<WarningDesc> &warnings) = 0;
// };
// 
// class ValidateId : public ValidateVariable<Type> {
// public:
//   void validate(const nlohmann::json::iterator &itr, Type &var, std::vector<ErrorDesc> &errors, std::vector<WarningDesc> &warnings) override {
//     if (itr.value().is_string() && itr.key() == "id") {
//       var = Type::get(itr.value().get<std::string>());
//     }
//   }
// };

#endif //RESOURCE_PARSER_H
