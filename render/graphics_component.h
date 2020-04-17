#ifndef GRAPHICS_COMPONENT_H
#define GRAPHICS_COMPONENT_H

#include <cstddef>
#include <cstdint>
#include <queue>

#include "ring_buffer.h"
#include "ArrayInterface.h"
#include "shared_structures.h"
#include "state.h"
#include "shared_time_constant.h"
#include "interface_context.h"
// #include "interface.h"
// #include "nuklear_header.h"

// 5 разных штук нужно отрисовать
// спрайт монстра, стена, свет, руки игрока, интерфейс

namespace yacs {
  class entity;
}

namespace devils_engine {
  namespace components {
//     struct graphics {
//       virtual ~graphics() {}
//       virtual void draw() = 0;
//     };
    
    struct sprite_graphics {
      yacs::entity* ent;
//       uint32_t container_index;
      
      void draw();
      // нужен цвет
      void debug_draw(); // рисуем ббокс
      
      // сам спрайт мы по идее берем из состояний
      // здесь нам нужно только отправить данные оптимизеру
      // 
      
      //static Container<Texture>* textureContainer;
    };
    
    // не очень удачное название
    struct indexed_graphics {
      yacs::entity* ent;
      
      uint32_t offset;
      uint32_t count;
      uint32_t index;
      
      void draw();
      void debug_draw();
      
      // по идее нам проще передать точки заново в оптимизер
      // чем то как было (оффсеты и размеры в константном буфере)
      
//       static Container<Texture>* textureContainer;
    };
    
    struct point_light {
      yacs::entity* ent;
      
      
      
      void draw();
      
      // тут мы должны уметь рисовать/не рисовать свет
      // задавать ему произвольную позицию
      // 
    };
    
    // эти компоненты будут в единственном экземпляре, сомневаюсь что они вообще нужны
    // с другой стороны player_sprite будет удобно держать близко чтобы сделать покачивание оружия игроку
    // player_interface будет разным для разных персонажей игрока, тогда лучше компонентами
    struct player_sprite {
      yacs::entity* ent;
      
      float movu;
      float movv;
      const core::state_t* player_state;
      
      void draw(const size_t &time);
      
      // нужно указать смещение для спрайта, для того чтобы эффекты разные моделировать
      // например покачивание рук при движении
      // как задать? абсолютная позиция? смещение?
      // у меня вызов функции происходит каждое константное время, 
      // то есть я могу задавать "константное" смещение
      // смещение чего? текстурных координат? абсолютной позиции?
      // текстурные координаты - скорее всего возникнут проблемы на границе картинки 
      // (сэмплер будет срезать часть картинки), вообще мне нужно видимо придумать некую 
      // квадратную область на экране (1024на1024 или 720на720)
      // и это будет область в которой должен спрайт игрока находиться
      // в этом случае координаты мы можем задать относительно наклир картинки
    };
    
    struct player_interface {
      static const size_t info_message_time = ONE_SECOND*10;
      static const size_t game_message_time = ONE_SECOND*2;
      
      yacs::entity* ent;
      utils::ring_buffer<std::string, 3> strings;
      //std::array<std::string, 10> game_messages;
      std::queue<std::string> game_messages;
      const core::state_t* face_state;
      size_t current_time_info;
      size_t current_time_game;
      
      player_interface(yacs::entity* ent);
      void draw(const size_t &time, const interface::data::extent &screen_size);
      void info_message(const std::string &string); // подбор предметов
      void game_message(const std::string &string); // какая нибудь информация по центру экрана (для этой двери нужен такой то ключ)
      // адекватного способа поделить сообщения и понять сколько времени требуется я так понимаю не существует
      // поэтому нужно придумать какой то способ последовательно выдавать несколько сообщений
      
      // как именно должен выглядеть интерфейс игрока я еще не понял
      // но определенно там должна быть возможность писать сообщения на экране
      // начем с того что для разных персонажей будет разный интерфейс
      // это значит что нужно предусмотреть во первых несколько интерфейсов для своих персонажей 
      // так еще и возможные пользовательские варианты
      // как пользовательский вариант организовать? мне нужно выделить функцию
      // для того чтобы игрок мог что нибудь нарисовать на экране
      // понятное дело все функции наклира передавать глупо, значит нужно
      // продумать целый апи
    };
  }
}

#endif
