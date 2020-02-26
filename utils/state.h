#ifndef STATE_H
#define STATE_H

#include <cstddef>
#include <cstdint>
#include <functional>
#include "id.h"

namespace yacs {
  class entity;
}

namespace devils_engine {
  namespace core {
    struct state_t {
      using action_func = std::function<void(yacs::entity*, const size_t&)>;
      struct frame_t {
        uint32_t texture_offset;
        uint32_t images_count;
      };
      
      utils::id id;
      frame_t frame;
      size_t time;
      action_func action;
      const state_t* next;
    };
  }
}

// вот что я узнал из doom'а
// смещение + индекс = верная сторона, нужны данные о количестве сторон
// входные данные? у меня по идее все в энтити записано
// следующий стейт, если стейт не указан, что по окончанию времени энтити удаляется из игры
//uint32_t m1, m2;     // зарезервированные, но не используемые переменные (мне поди они тоже не пригодятся)

#endif
