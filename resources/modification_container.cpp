#include "modification_container.h"

#include "filesystem/path.h"

namespace devils_engine {
  namespace resources {
    modification_container::modification_container(const size_t &container_size) : container(container_size) {}
    modification_container::~modification_container() {
      for (auto parser : parsers) {
        container.destroy(parser);
      }
      
      for (auto mod : mods_array) {
        mods_pool.deleteElement(mod);
      }
    }
    
    modification<core::resource>* modification_container::load_mod(const std::string &path) {
      std::ifstream file(path);
      if (!file) {
        throw std::runtime_error("Could not load mod");
      }

      nlohmann::json json;

      file >> json;
      
      modification<core::resource>::create_info info;
      info.m_path = path;
      for (auto itr = json.begin(); itr != json.end(); ++itr) {
        // узнаем данные мода
        // нужно загрузить название, описание, автора, версию, какую-нибудь ссылку на мод
        // также мы загрузим здесь изображение для мода

        //textureParser->parse(asagsgag);

        if (itr.value().is_string() && itr.key() == "version") {
          // версию нужно получить из стринга
        }

        if (itr.value().is_string() && itr.key() == "name") {
          info.name = itr.value().get<std::string>();
        }

        if (itr.value().is_string() && itr.key() == "description") {
          info.description = itr.value().get<std::string>();
        }

        if (itr.value().is_string() && itr.key() == "author") {
          info.m_author = itr.value().get<std::string>();
        }
      }
      
      auto mod = mods_pool.newElement(info);
      mods_array.push_back(mod);
      
      return mod;
    }
    
    bool modification_container::parse_mod(modification<core::resource>* mod) {
      filesystem::path modPath = mod->path();

      const std::string ext = modPath.extension();

      PRINT_VAR("ext", ext)
      if (ext == "zip") {
        // в этом случае нам по идее не нужен префикс пути
        // но нужен зато указатель на открытый зип архив
      } else {
        // нужно взять префикс пути
      }

      const std::string prefix = modPath.parent_path().str() + "/";
      PRINT_VAR("prefix", prefix)

      std::ifstream file(mod->path());
      if (!file) {
        throw std::runtime_error("Could not load mod");
      }

      nlohmann::json json;
      file >> json;

      if (mod == nullptr) throw std::runtime_error("Mod not found");

      utils::problem_container<info::error> errors; 
      utils::problem_container<info::warning> warnings;
      for (auto itr = json.begin(); itr != json.end(); ++itr) {

        if (itr.value().is_object() && itr.key() == "data") {
          for (auto parser : parsers) {
            for (auto dataIt = itr.value().begin(); dataIt != itr.value().end(); ++dataIt) {
              if (parser->can_parse(dataIt.key())) {
                std::cout << "Parsing " << dataIt.key() << "\n";
                // этот парсер нужно поставить как текущий

                // если обрабатываем zip то мы должны передать указатель на него, вместо префикса

                parser->parse(prefix, "main.json", mod, dataIt.value(), errors, warnings);
                break;
              }
            }
          }

          break;
        }
      }

      // где мы выводим ошибки на экран?
      // наверное сразу при парсинге
      
    //   for (const auto &error : errors) {
    //     std::cout << error.description << "\n";
    //   }

      if (errors.size() != 0) throw std::runtime_error("There are "+std::to_string(errors.size())+" parsing errors");

      if (mod->size() == 0) throw std::runtime_error("Empty mod "+mod->name());

//       for (auto res : _mod->resources()) {
//         auto itr = conflictsMap.find(res->id());
//         if (itr == conflictsMap.end()) {
//           auto conflict = conflictPool.newElement(Conflict::CreateInfo{res});
//           conflictsArray.push_back(conflict);
//           conflictsMap[res->id()] = conflict;
//           continue;
//         }
// 
//         for (auto _conflict : conflictsArray) {
//           if (itr->second == _conflict) {
//             _conflict->add(res);
//             break;
//           }
//         }
//       }
      return true;
    }
    
    bool modification_container::validate() const {
      utils::problem_container<info::error> errors; 
      utils::problem_container<info::warning> warnings;
      for (auto validator : validators) {
        validator->validate(errors, warnings);
      }
      
      if (errors.size() != 0) throw std::runtime_error("There are "+std::to_string(errors.size())+" validate errors");
      
      return true;
    }
    
    bool modification_container::load_data() const {
      for (auto mod : mods_array) {
        for (size_t i = 0; i < mod->size(); ++i) {
          for (auto loader : loaders) {
            const bool ret = loader->load(mod->at(i)->id);
            if (ret) break;
          }
        }
      }
      
      return true;
    }
    
    void modification_container::end() const {
      for (auto loader : loaders) {
        loader->end();
      }
    }
  }
}
