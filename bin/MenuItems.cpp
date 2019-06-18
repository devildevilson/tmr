#include "MenuItems.h"

#include "nuklear_header.h"
#include "GLFW/glfw3.h"

#include "Globals.h"

ButtonItem::ButtonItem(nuklear_data* nuklear, const std::string &name, MenuItem* nextItem, const Extent &extent) : MenuItem(extent), nuklear(nuklear), name(name), nextItem(nextItem) {}
ButtonItem::~ButtonItem() {}

void ButtonItem::draw() {
  //nk_layout_row_static(&nuklear->ctx, 30.0f, 300, 1);
  nk_layout_row_static(&nuklear->ctx, extent.height, extent.width, 1);
  nk_label(&nuklear->ctx, name.c_str(), NK_TEXT_LEFT);
}

MenuItem* ButtonItem::feedback(const PressingData &data) {
  // что конкретно сюда должно приходить?
  if (data.button == GLFW_KEY_ENTER) return nextItem;
  
  return nullptr;
}

bool ButtonItem::feedback(const MousePos &data) {
  // тут наверное все же нужно возвращать bool
  // для того чтобы проверить пересекает ли указатель мышки кнопку
  return true;
}

MainMenu::MainMenu(nuklear_data* nuklear) : nuklear(nuklear), itemIndex(0) {}
MainMenu::~MainMenu() {}

void MainMenu::draw() {
  // нам сильно не помешает отрисовать все кнопки по центру
  
  // как курсор то рисовать? мне для этого нужно понять где расположен объект
  // нужно короч добавить размеры для каждого айтема видимо (то есть width, height)
  // наверное нужно еще передавать откуда начинать рисовать 
  
  if (nk_begin(&nuklear->ctx, "main_menu_window", nk_rect(400, 200, 300, 300), NK_WINDOW_NO_SCROLLBAR)) {
    // вначале какую нибудь картинку отрисуем
    
    for (auto item : items) {
      item->draw();
    }
  }
  nk_end(&nuklear->ctx);
}

MenuItem* MainMenu::feedback(const PressingData &data) {
  if (data.button == GLFW_KEY_ESCAPE) {
    Global::data()->focusOnInterface = false;
    return nullptr;
  }
  
  MenuItem* ptr = items[itemIndex]->feedback(data);
  if (ptr != nullptr) return ptr;
  
  return this;
}

bool MainMenu::feedback(const MousePos &data) {
  for (size_t i = 0; i < items.size(); ++i) {
    if (items[i]->feedback(data)) {
      itemIndex = i;
      break;
    }
  }
  
  return false;
}

void MainMenu::addItem(MenuItem* item) {
  items.push_back(item);
}

QuitGame::QuitGame(nuklear_data* nuklear, bool* quit, MenuItem* prevItem) : nuklear(nuklear), quit(quit), prevItem(prevItem) {}
QuitGame::~QuitGame() {}

void QuitGame::draw() {
  // тут мы тип будем рисовать текст с описанием каким нибудь и спрашивать ВЫ УВЕРЕНЫ???????
  // хотя может это и вообще не нужно
  
  if (nk_begin(&nuklear->ctx, "quit_game_window", nk_rect(400, 200, 300, 300), NK_WINDOW_NO_SCROLLBAR)) {
    nk_layout_row_static(&nuklear->ctx, 30.0f, 300, 1);
    nk_label(&nuklear->ctx, "no", NK_TEXT_CENTERED);
//     nk_layout_row_static(&nuklear->ctx, 30.0f, 300, 1);
    nk_label(&nuklear->ctx, "yes", NK_TEXT_CENTERED);
  }
  nk_end(&nuklear->ctx);
}

MenuItem* QuitGame::feedback(const PressingData &data) {
  // если пришел энтер то мы должны как то завершить всю игру
  
  if (data.button == GLFW_KEY_ENTER && focusedAns == 1) *quit = true;
  else if (data.button == GLFW_KEY_ENTER && focusedAns == 0) return prevItem;
  else if (data.button == GLFW_KEY_ESCAPE) return prevItem;
  
  if (data.button == GLFW_KEY_DOWN) {
    focusedAns = (focusedAns + 1) % 2;
  }
  
  if (data.button == GLFW_KEY_UP) {
    focusedAns = uint8_t(std::abs(int32_t(focusedAns) - 1) % 2);
  }
  
  return this;
}

bool QuitGame::feedback(const MousePos &data) {
  // чекаем пересечение с курсором
}
