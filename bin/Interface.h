#ifndef INTERFACE_H
#define INTERFACE_H

#include <vector>
#include "typeless_container.h"

namespace devils_engine {
  namespace interface {
    namespace data {
      struct PressingData {
        uint32_t button;
        uint32_t modifier;
      };

      struct MousePos {
        float x;
        float y;
      };
      
      struct extent {
        float width;
        float height;
      };
      
      struct offset {
        float x;
        float y;
      };
      
      struct rect {
        data::offset offset;
        data::extent extent;
      };
      
      // что вообще мне может потребоваться? 
      // на самом деле это определятся строго теми виджетами которые я использую
      // мне бы 
      enum class command {
        next,
        prev,
        increase,
        decrease,
        choose,
        none
      };
    }

//     class Item {
//     public:
//       Item(const data::Rect &rect) : itemPlace(rect) {}
//       virtual ~Item() {}
//       
//       virtual void draw(const data::MousePos &pos) = 0;
//       virtual Item* feedback(const data::PressingData &data, const data::MousePos &pos) = 0; // ???
//       virtual void correctItemSize(const data::extent &screenSize) = 0;
//       
//       // мне нужно какм то образом выполнить действие при нажатии на кнопку + предусмотреть драг & дроп
//       // я так полагаю все такие манипуляции нужно задать непосредственно в InterfaceItem
//       
//       data::Rect place() const { return itemPlace; }
//       void setPlace(const data::Rect &place) { itemPlace = place; }
//       bool mouseOver(const data::MousePos &pos) {
//         return itemPlace.offset.x >= pos.x && itemPlace.offset.x + itemPlace.extent.width <= pos.x && itemPlace.offset.y >= pos.y && itemPlace.offset.y + itemPlace.extent.height <= pos.y;
//       }
//     protected:
//       data::Rect itemPlace; // это может не потребоваться
//     };
    
    class page {
    public:
      virtual ~page() {}
      
      // еще может потребоваться время
      virtual page* proccess(const data::extent &screen_size, const data::command &command, size_t &focus) = 0; // возвращает либо себя, если еще не закончил, либо следующую страницу
      virtual page* escape(size_t &focus) const = 0;   // виртуал? (это не важно)
    };

    class container {
    public:
      container(const size_t &containerSize);
      ~container();
      
//       void draw(const data::MousePos &pos);
//       void feedback(const data::PressingData &data, const data::MousePos &pos);
//       void correntItemsSize(const data::Extent &screenSize);
      void proccess(const data::extent &screen_size);
      
      void set_default_page(page* page_ptr);
      void open();
      bool is_opened() const;
      
      template <typename T, typename... Args>
      T* add_page(Args&&... args) {
        T* ptr = page_container.create<T>(std::forward<Args>(args)...);
        pages.push_back(ptr);
        
        return ptr;
      }
      
      // команды клавиатуры, это все?
      void next();
      void prev();
      void increase();
      void decrease();
      void choose();
      void escape();
    private:
      // должен определять стиль интерфейса, шрифты, создавать окно, рисовать интерфейс
      page* default_page;
      page* current_page;
      
      data::command last_command;
      size_t focus;
      
      utils::typeless_container page_container;
      std::vector<page*> pages;
    };
  }
}

#endif
