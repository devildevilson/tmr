#ifndef RESOURCE_PARSER_H
#define RESOURCE_PARSER_H

#include <vector>
#include <mutex>
#include <iostream>
#include <string>
#include <fstream>
#include "id.h"
#include "resource_container.h"
#include "nlohmann/json.hpp"

// надо исходить из того что все нижеописанные классы мы удалим

namespace devils_engine {
  namespace utils {
    template <typename T>
    class problem_container {
    public:
      template <typename... Args>
      void add(Args&&... args) {
        std::unique_lock<std::mutex> lock(mutex);
        problems.emplace_back(std::forward<Args>(args)...);
        std::cout << problems.back().description << "\n";
      }
      
      const T* at(const size_t &index) const {
        if (index >= problems.size()) return nullptr;
        return &problems[index];
      }
      
      size_t size() const { return problems.size(); }
    private:
      std::mutex mutex;
      std::vector<T> problems;
    };
  }
  
  namespace resources {
    namespace info {
      struct error {
        error(const size_t &loader_mark, const size_t &type, const std::string &desc) : loader_mark(loader_mark), type(type), description(desc) {}

        size_t loader_mark;
        size_t type;
        std::string description;
      };

      struct warning {
        warning(const size_t &mark, const size_t &type, const std::string &desc) : mark(mark), type(type), description(desc) {}

        size_t mark;
        size_t type;
        std::string description;
      };
    }
    
    namespace errors {
      namespace parser {
        enum {
          ERROR_FILE_NOT_FOUND,
          COUNT,
        };
      }
    }
    
    // модификация должна включать в себя прежде всего информацию и картинку
    // ресурсы?
    template <typename T>
    class modification {
    public:
      struct create_info {
        std::string name;
        std::string description;
        std::string m_author;
        std::string m_path;
      };
      modification(const create_info &info) : m_name(info.name), m_description(info.description), m_author(info.m_author), m_path(info.m_path) {}
      
      std::string name() const { return m_name; }
      std::string description() const { return m_description; }
      std::string author() const { return m_author; }
      std::string path() const { return m_path; }
      
      void add_resource(const T* res) {
        std::unique_lock<std::mutex> lock(mutex);
        resources.push_back(res);
      }
      
      const T* at(const size_t &index) const {
        if (index >= resources.size()) return nullptr;
        return &resources[index];
      }
      
      size_t size() const { 
        std::unique_lock<std::mutex> lock(mutex);
        return resources.size(); 
      }
    private:
      std::string m_name;
      std::string m_description;
      std::string m_author;
      std::string m_path;
      
      mutable std::mutex mutex;
      std::vector<const T*> resources;
    };
    
    class parser_interface {
    public:
      virtual ~parser_interface() = default;
      virtual bool can_parse(const std::string &key) const = 0;
      virtual bool parse(const std::string &file_prefix, const std::string &file, modification<core::resource>* mod, const nlohmann::json &data, utils::problem_container<info::error> &errors, utils::problem_container<info::warning> &warnings) = 0;
      virtual void forget(const utils::id &id) = 0;
    };
    
    template <typename T, size_t N>
    class parser : public parser_interface {
    public:
      parser(const std::string &parsing_key);
      virtual ~parser() = default;
      
      size_t mark() const { return reinterpret_cast<size_t>(this); }
      
      bool can_parse(const std::string &key) const override { return key == parsing_key; }
      
      bool parse(const std::string &file_prefix, const std::string &file, modification<core::resource>* mod, const nlohmann::json &data, utils::problem_container<info::error> &errors, utils::problem_container<info::warning> &warnings) override {
        if (data.is_string()) {
          const std::string &path = data.get<std::string>();
          std::ifstream file(file_prefix + path);
          if (!file) {
            errors.add(this, errors::parser::ERROR_FILE_NOT_FOUND, "Could not load file "+file_prefix+path);
            return false;
          }
          
          nlohmann::json json;
          file >> json;
          return parse(file_prefix, path, json, errors, warnings);
        } else if (data.is_array()) {
          bool ret = true;
          for (size_t i = 0; i < data.size(); ++i) {
            ret = ret && parse(file_prefix, file, data[i], errors, warnings);
          }
          return ret;
        } else if (data.is_object()) {
          T info;
          utils::id id = check_json(file_prefix, file, data, mark(), info, errors, warnings);
          if (!id.valid()) return false;
          
          auto ptr = loading_data->create(id);
          (*ptr) = info;
          mod->add_resource(ptr);
          std::cout << "Parsed: " << ptr->id.name() << "\n";
          //resource.push_back(ptr);
          return true;
        }
        
        return false;
      }
      
      void forget(const utils::id &id) override {
        loading_data.destroy(id);
      }
      
      T* resource(const utils::id &id) { return loading_data.get(id); }
      const T* resource(const utils::id &id) const { return loading_data.get(id); }
    protected:
      std::string parsing_key;
      utils::resource_container_array<utils::id, T, N> loading_data;
      // конфликты? пока что не буду их делать
      
      virtual utils::id check_json(const std::string &path_prefix, const std::string &file, const nlohmann::json &data, const size_t &mark, T& info, utils::problem_container<info::error> &errors, utils::problem_container<info::warning> &warnings) const = 0;
    };
    
    class validator {
    public:
      virtual ~validator() = default;
      virtual bool validate(utils::problem_container<info::error> &errors, utils::problem_container<info::warning> &warnings) const = 0;
    };
    
    // этот класс должен работать с какими то внешними контейнерами
    class loader {
    public:
      virtual ~loader() = default;
      virtual bool load(const utils::id &id) = 0;
      virtual bool unload(const utils::id &id) = 0;
      virtual void end() = 0;  
      virtual void clear() = 0;
    };
  }
}

#endif
