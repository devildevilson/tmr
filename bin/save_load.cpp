#include "save_load.h"

#include <fstream>
#include "Globals.h"
#include "global_components_indicies.h"
#include "EntityComponentSystem.h"
#include "TransformComponent.h"
#include "lz4hc.h"

namespace devils_engine {
  namespace utils {
    bool save_game(const std::string &path) {
      // создаем протобаф файл
      // сначала задаем хедер
      
      auto world = Global::get<yacs::world>();
      for (size_t i = 0; i < world->size(); ++i) {
        auto ent = world->get_entity(i);
        // запихиваем каждый энтити
      }
      
      // дополнительно сжимаем память
      const int size = 132512262;
      const size_t dst_capacity = LZ4_compressBound(size);
      const int out = LZ4_compress_HC(src, dst, size, dst_capacity, LZ4HC_CLEVEL_MAX);
      if (out == 0) return false;
      
      std::fstream file(path, std::ios::in | std::ios::binary);
      if (!file) return false;
      
      // пихаем полученные данные в файл
    }
    
    // имеет смысл сначала распарсить хедер, чтоб получить название сохранения, время создания, карту, картинку
    bool load_game(const std::string &path) {
      
    }
  }
}
