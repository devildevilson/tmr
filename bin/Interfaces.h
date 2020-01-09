// наклир предлагает иммедиате мод гуи, что значит что мне нужно просто вызывать
// функции гуи каждый кадр и каждый кадр гуи как бы заново формируется
// в принципе это нормальный подход и он меня устраивает, но мне нужно
// каким то образом быстренько накидывать интерфейс + возможно накидывать его
// в каких нибудь файлах конфигурации? сделать это возможно, но для этого мне
// нужно определить какие то классы хранящие конфигурации интерфейса
// что тут может пойти не так? 

// было бы неплохо настраивать горячие клавиши, сразу в голову приходит создать 
// список возможных действий, по нему находить клавишу и сравнивать ее с тем 
// что нажато, но лучше задействовать тот класс клавиш который у меня уже имеется
// хотя действия в меню строго ограничены, и по идее очень легко определить
// когда и какое произойдет действие в интерфейсе

// вместо всего этого, лучше выделить несколько крупных меню и их полностью задизайнить в одном месте
// нажатие кнопки может привести к отрисовке следующего меню (возращаем указатель)
// сложнее быть с луа, нужно ли мне вообще там задавать интерфейс 
// (но там по прежнему будет проще, чем городить json или xml файлы и из них получать команды для наклира)

#ifndef INTERFACES_H
#define INTERFACES_H

#include "Interface.h"

struct nuklear_data;

namespace devils_engine {
  namespace interface {
//     class Button : public Item {
//     public:
//       struct CreateInfo {
//         data::Rect rect;
//         const char* name;
//         nuklear_data* nuklear;
//         Item* next;
//       };
//       Button(const CreateInfo &info);
//       
//       void draw(const data::MousePos &pos) override;
//       Item* feedback(const data::PressingData &data, const data::MousePos &pos) override;
//       void correctItemSize(const data::Extent &screenSize) override;
//     private:
//       const char* name;
//       nuklear_data* nuklear;
//       Item* next;
//     };
    
    class main_menu : public page {
    public:
      struct create_info {
        const char* name;
        nuklear_data* nuklear;
        page* quit;
        page* settings;
        page* new_game;
        
        data::extent page_size;
      };
      main_menu(const create_info &info);
      
      page* proccess(const data::extent &screen_size, const data::command &command, size_t &focus) override;
      page* escape(size_t &focus) const override;
      
      void set_pointers(page* quit, page* settings, page* new_game);
    private:
      const char* name;
      nuklear_data* nuklear;
      page* quit;
      page* settings;
      page* new_game;
      data::extent page_size;
      // ...
    };
    
    class settings : public page {
    public:
      struct create_info {
        const char* name;
        nuklear_data* nuklear;
        page* main_menu;
        
        data::extent page_size;
      };
      settings(const create_info &info);
      
      page* proccess(const data::extent &screen_size, const data::command &command, size_t &focus) override;
      page* escape(size_t &focus) const override;
      
      void set_pointers(page* main_menu);
    private:
      const char* name;
      nuklear_data* nuklear;
      page* main_menu;
      data::extent page_size;
      // ...
    };
    
    class quit_game : public page {
    public:
      struct create_info {
        const char* name;
        nuklear_data* nuklear;
        page* main_menu;
        bool* quit;
        
        data::extent page_size;
      };
      quit_game(const create_info &info);
      
      page* proccess(const data::extent &screen_size, const data::command &command, size_t &focus) override;
      page* escape(size_t &focus) const override;
      
      void set_pointers(page* main_menu);
      
      // это можно смоделировать с помощью передачей индексов и какой-то энум
//       page* next(); // тут нужно вернуть указатель все же
//       page* prev(); // хотя возможно более правильный вариант это все же использовать энум
//       page* increase();
//       page* decrease();
//       page* choose();
    private:
      const char* name;
      nuklear_data* nuklear;
      page* main_menu;
      bool* quit;
      data::extent page_size;
//       size_t current;
      // ...
    };
  }
}

#endif
