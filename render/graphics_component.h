#ifndef GRAPHICS_COMPONENT_H
#define GRAPHICS_COMPONENT_H

#include <cstddef>
#include <cstdint>

#include "ring_buffer.h"
#include "ArrayInterface.h"
#include "RenderStructures.h"
#include "state.h"

// 5 разных штук нужно отрисовать
// спрайт монстра, стена, свет, руки игрока, интерфейс

namespace yacs {
  class entity;
}

namespace devils_engine {
  namespace components {
    struct graphics {
      virtual ~graphics() {}
      virtual void draw() = 0;
    };
    
    struct sprite_graphics : public graphics {
      yacs::entity* ent;
//       uint32_t container_index;
      
      void draw() override;
      // нужен цвет
      void debug_draw(); // рисуем ббокс
      
      // сам спрайт мы по идее берем из состояний
      // здесь нам нужно только отправить данные оптимизеру
      // 
      
      //static Container<Texture>* textureContainer;
    };
    
    // не очень удачное название
    struct indexed_graphics : public graphics {
      yacs::entity* ent;
      
      uint32_t offset;
      uint32_t count;
      uint32_t index;
      
      void draw() override;
      void debug_draw();
      
      // по идее нам проще передать точки заново в оптимизер
      // чем то как было (оффсеты и размеры в константном буфере)
      
      static Container<Texture>* textureContainer;
    };
    
    struct point_light {
      yacs::entity* ent;
      
      
      
      void draw();
      
      // тут мы должны уметь рисовать/не рисовать свет
      // задавать ему произвольную позицию
      // 
    };
    
    struct player_sprite {
      yacs::entity* ent;
      
      float movu;
      float movv;
      const core::state_t* player_state;
      
      void draw(const size_t &time);
      
      // нужно указать смещение для спрайта, для того чтобы эффекты разные моделировать
      // например покачивание рук при движении
    };
    
    struct player_interface {
      yacs::entity* ent;
      
      utils::ring_buffer<std::string, 3> strings;
      const core::state_t* face_state;
      
      void draw(const size_t &time);
      void message(const std::string &string);
      
      // как именно должен выглядеть интерфейс игрока я еще не понял
      // но определенно там должна быть возможность писать сообщения на экране
      // начем с того что для разных персонажей будет разный интерфейс
      // это значит что нужно предусмотреть во первых несколько интерфейсов для своих персонажей 
      // так еще и возможные пользовательские варианты
    };
  }
}

#endif
