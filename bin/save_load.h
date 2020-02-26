#ifndef SAVE_LOAD_H
#define SAVE_LOAD_H

#include <string>

namespace devils_engine {
  namespace utils {
     // самые базовые функции сохранения/загрузки
     // по идее требуют еще мод лоадер, но это кажется потом будут требовать
    bool save_game(const std::string &path);
    bool load_game(const std::string &path);
  }
}

#endif
