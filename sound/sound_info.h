#ifndef SOUND_INFO_H
#define SOUND_INFO_H

#include "id.h"

namespace yacs {
  class entity;
}

namespace devils_engine {
  namespace sound {
    // может быть тут стоит сразу сам звук передать? 
    // где мы возьмем указатель? найдем его по id, можем использовать static чтобы запомнить конкретный звук
    // в луа мы по идее можем запомнить все указатели на звуки
    // все это нужно чтобы сократить количество поисков, как можно сделать в плюсах?
    // в плюсах можно хранить указатели в статиках внутри функции, но нужно как то изменять иногда эти указатели
    // значит нужен какой то быстрый контейнейр, анордеред_мап?
    struct info {
      const yacs::entity* ent;
      utils::id id;
      float volume;
      float min_pitch;
      float max_pitch;
      float max_dist; // звуки то поди поступают все, но некоторые из них будут выбывать по дальности
      float rolloff; // полезная вещь, но нужно много проверять как именно работает
      float ref_dist; // полезная вещь, но нужно много проверять как именно работает
      
      inline info(const yacs::entity* ent, const utils::id &id) : ent(ent), id(id), volume(10000.0f), min_pitch(1.0f), max_pitch(1.0f), max_dist(100.0f), rolloff(1.0f), ref_dist(1.0f) {}
      inline info(const yacs::entity* ent, const utils::id &id, const float &volume) : ent(ent), id(id), volume(volume), min_pitch(1.0f), max_pitch(1.0f), max_dist(100.0f), rolloff(1.0f), ref_dist(1.0f) {}
      inline info(const yacs::entity* ent, const utils::id &id, const float &volume, const float &min_pitch, const float &max_pitch) : ent(ent), id(id), volume(volume), min_pitch(min_pitch), max_pitch(max_pitch), max_dist(100.0f), rolloff(1.0f), ref_dist(1.0f) {}
    };
  }
  
//   namespace particle {
//     struct info {
//       // картинка
//       float gravity;
//       float scale;
//       float vel[4];
//       float pos[4];
//       // ???
//     };
//   }
}

#endif
