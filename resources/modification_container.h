#ifndef MODIFICATION_CONTAINER_H
#define MODIFICATION_CONTAINER_H

#include "MemoryPool.h"
#include "resource.h"
#include "resource_parser.h"
#include "image_loader.h"
#include "typeless_container.h"

namespace devils_engine {
  namespace resources {
    class modification_container {
    public:
      modification_container(const size_t &container_size);
      ~modification_container();
      
      template <typename T, typename... Args>
      T* add_loader(Args&& ...args) {
        T* ptr = container.create<T>(std::forward<Args>(args)...);
        parsers.push_back(ptr);
        validators.push_back(ptr);
        loaders.push_back(ptr);
        return ptr;
      }
      
      modification<core::resource>* load_mod(const std::string &path);
      bool parse_mod(modification<core::resource>* mod);
      bool validate() const;
      bool load_data() const;
      void end() const;
    private:
      utils::typeless_container container;
      std::vector<resources::parser_interface*> parsers;
      std::vector<resources::validator*> validators;
      std::vector<resources::loader*> loaders;
      resources::image_loader* images;
      MemoryPool<modification<core::resource>, sizeof(modification<core::resource>)*20> mods_pool;
      std::vector<modification<core::resource>*> mods_array;
    };
  }
}

#endif
