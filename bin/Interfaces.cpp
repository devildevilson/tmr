#include "Interfaces.h"

#include <climits>
#include "nuklear_header.h"

namespace devils_engine {
  namespace interface {
    container::container(const size_t &containerSize) : default_page(nullptr), current_page(nullptr), last_command(data::command::none), focus(0), page_container(containerSize) {}
    container::~container() {
      for (auto page : pages) {
        page_container.destroy(page);
      }
    }
    
    void container::proccess(const data::extent &screen_size) {
      if (!is_opened()) return;
      
      current_page = current_page->proccess(screen_size, last_command, focus);
    }
    
//     void container::draw(const data::MousePos &pos) {
//       if (!isOpened()) return;
//       
//       for (auto item : items) {
//         item->draw(pos);
//       }
//     }
//     
//     void container::feedback(const data::PressingData &data, const data::MousePos &pos) {
//       if (!isOpened()) return;
//       
//       currentItem = currentItem->feedback(data, pos);
//     }
//     
//     void container::correntItemsSize(const data::Extent &screenSize) {
//       for (auto item : items) {
//         item->correctItemSize(screenSize);
//       }
//     }
    
    void container::set_default_page(page* page_ptr) {
      default_page = page_ptr;
    }
    
    void container::open() {
      current_page = default_page;
    }
    
    bool container::is_opened() const {
      return current_page != nullptr;
    }
    
    void container::next() {
      last_command = data::command::next;
    }
    
    void container::prev() {
      last_command = data::command::prev;
    }
    
    void container::increase() {
      last_command = data::command::increase;
    }
    
    void container::decrease() {
      last_command = data::command::decrease;
    }
    
    void container::choose() {
      last_command = data::command::choose;
    }
    
    void container::escape() {
      current_page = current_page->escape(focus);
    }
    
//     Button::Button(const CreateInfo &info) : Item(info.rect), name(info.name), nuklear(info.nuklear), next(info.next) {}
//     void Button::draw(const data::MousePos &pos) {
//       // рисуем
//       nk_layout_row_dynamic(&nuklear->ctx, 0, 1);
//       const bool ret = nk_button_label(&nuklear->ctx, "Quit game");
//       
//       
//       nk_layout_space_begin(&nuklear->ctx, NK_DYNAMIC, 500, INT_MAX);
//       nk_layout_space_push(&nuklear->ctx, nk_rect(0.5,0.5,0.1,0.1));
//       nk_widget(...);
//       nk_layout_space_push(&nuklear->ctx, nk_rect(0.7,0.6,0.1,0.1));
//       nk_widget(...);
//       nk_layout_space_end(&nuklear->ctx);
//       
//       if (mouseOver(pos)) {
//         // тултип
//       }
//     }
//     
//     Item* Button::feedback(const data::PressingData &data, const data::MousePos &pos) {
//       // при нажатии на кнопку мыши мы должны вернуть следующий айтем
//       if (mouseOver(pos)) {
//         
//       }
//       
//       return nullptr;
//     }
//     
//     void Button::correctItemSize(const data::Extent &screenSize) {
//       // при изменении размера экрана нам может потребоваться изменить размер объекта
//     }
    
    enum MenuItems {
      MENU_NEW_GAME,
      MENU_SETTINGS,
      MENU_QUIT,
      MENU_COUNT
    };
    
    main_menu::main_menu(const create_info &info) : name(info.name), nuklear(info.nuklear), quit(info.quit), settings(info.settings), new_game(info.new_game), page_size(info.page_size) {}
    page* main_menu::proccess(const data::extent &screen_size, const data::command &command, size_t &focus) {
      switch (command) {
        case data::command::next: {
          focus = (focus+1)%MENU_COUNT;
          break;
        }
        
        case data::command::prev: {
          focus = (focus-1)%MENU_COUNT;
          break;
        }
        
        case data::command::choose: {
          switch (focus) {
            case MENU_NEW_GAME: return new_game;
            case MENU_SETTINGS: return settings;
            case MENU_QUIT: return quit;
          }
        }
        
        default: break;
      }
      
      // тут мы видимо от начала до конца делаем интерфейс
      float w = screen_size.width / 2.0f - page_size.width / 2.0f;
      float h = screen_size.height / 2.0f - page_size.height / 2.0f;
      
      page* next = this;
      if (nk_begin(&nuklear->ctx, name, nk_rect(w, h, page_size.width, page_size.height), NK_WINDOW_NO_SCROLLBAR)) {
        nk_layout_row_dynamic(&nuklear->ctx, 0, 1);
        if (nk_button_label(&nuklear->ctx, "New game")) {
          next = new_game;
        }
        
        if (nk_button_label(&nuklear->ctx, "Settings")) {
          next = settings;
        }
        
        if (nk_button_label(&nuklear->ctx, "Quit game")) {
          next = quit;
        }
      }
      nk_end(&nuklear->ctx);
      
      return next;
    }
    
    page* main_menu::escape(size_t &focus) const {
      focus = 0;
      return nullptr;
    }
    
    void main_menu::set_pointers(page* quit, page* settings, page* new_game) {
      this->quit = quit;
      this->settings = settings;
      this->new_game = new_game;
    }
    
    enum QuitItem {
      QUIT_GAME_NO,
      QUIT_GAME_YES,
      QUIT_COUNT
    };
    
    quit_game::quit_game(const create_info &info) : name(info.name), nuklear(info.nuklear), main_menu(info.main_menu), quit(info.quit), page_size(info.page_size) {}
    page* quit_game::proccess(const data::extent &screen_size, const data::command &command, size_t &focus) {
      switch (command) {
        case data::command::next: {
          focus = (focus+1)%QUIT_COUNT;
          break;
        }
        
        case data::command::prev: {
          focus = (focus-1)%QUIT_COUNT;
          break;
        }
        
        case data::command::choose: {
          if (focus == QUIT_GAME_NO) return escape(focus);
          else {
            *quit = true;
            return nullptr;
          }
          break;
        }
        
        default: break;
      }
      
      float w = screen_size.width / 2.0f - page_size.width / 2.0f;
      float h = screen_size.height / 2.0f - page_size.height / 2.0f;
      
      page* next = this;
      if (nk_begin(&nuklear->ctx, name, nk_rect(w, h, page_size.width, page_size.height), NK_WINDOW_NO_SCROLLBAR)) {
        nk_layout_row_dynamic(&nuklear->ctx, 0, 1);
//         const float ratio[] = {0.25f, 0.75f};
//         nk_layout_row(&nuklear->ctx, NK_DYNAMIC, 0, 2, ratio);
        // нам нужно нарисовать изображение в том месте куда указывает курсор
        // нам нужно обработать нажатия стрелочек (функция обработчик или просто задать индекс?)
        // и обработать движение мышки
        // я так полагаю нужно сюда это дело передавать
        
        // основная задача это получать фокус на конкретный виджет и по фокусу с ним взаимодействовать
        // + должен быть кастомный фокус + 
        
        // нужно задать верное изображение, либо можно использовать null изображение
//         struct nk_image fsaf; 
//         struct nk_image null;
        
//         if (focus == QUIT_GAME_NO) nk_image(&nuklear->ctx, fsaf);
//         else nk_image(&nuklear->ctx, null);
        if (nk_button_label(&nuklear->ctx, "No")) {
          next = main_menu;
        }
        
//         if (focus == QUIT_GAME_YES) nk_image(&nuklear->ctx, fsaf);
//         else nk_image(&nuklear->ctx, null);
        if (nk_button_label(&nuklear->ctx, "Yes")) {
          *quit = true;
          next = nullptr;
        }
      }
      nk_end(&nuklear->ctx);
      
      return next;
    }
    
    page* quit_game::escape(size_t &focus) const {
      focus = 0;
      return main_menu;
    }
    
    void quit_game::set_pointers(page* main_menu) {
      this->main_menu = main_menu;
    }
//     
//     page* quit_game::next() {
//       current = (current+1)%2;
//       return this;
//     }
//     
//     page* quit_game::prev() {
//       current = (current-1)%2;
//       return this;
//     }
//     
//     page* quit_game::increase() { return this; }
//     page* quit_game::decrease() { return this; }
//     page* quit_game::choose() {
//       if (current == 0) return escape();
//       *quit = true;
//       return nullptr;
//     }
  }
}
