#ifndef MEMORY_COMPONENT_H
#define MEMORY_COMPONENT_H

#include "id.h"
#include <atomic>

// основная задача это хранить некую статистику игрока (только игрока?)
// при этом данные отсюда скорее всего считывать могут все
// а изменять? думаю что можно сделать с помощью атомиков все
// предварительно заполнить? можно в json указать 
// но это все нужно чтобы не использовать аттрибуты
// может статистику через аттрибуты собрать?
// было бы все же лучше разделять эти вещи

namespace devils_engine {
  namespace components {
    struct memory {
      //static const size_t max_memory = 16;
      struct piece {
        utils::id id;
        std::atomic<size_t> mem;
      };
      
      //piece pieces[max_memory];
      size_t size;
      piece* pieces;
      
      memory(const size_t &size);
      ~memory();
      size_t get(const utils::id &id) const;
      size_t set(const utils::id &id, const size_t &value);
      size_t inc(const utils::id &id, const size_t &value);
      size_t dec(const utils::id &id, const size_t &value);
      size_t find(const utils::id &id) const;
    };
  }
}

#endif
