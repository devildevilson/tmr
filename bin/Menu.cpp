#include "Menu.h"

//#include "nuklear_header.h"

MenuStateMachine::MenuStateMachine(const size_t &size) : defaultItem(nullptr), currentItem(nullptr), menuContainer(size) {}
MenuStateMachine::~MenuStateMachine() {
  for (auto item : items) {
    menuContainer.destroyStage(item);
  }
}

// по идее запускаем nk_begin и затем обходим меню айтем и там тоже запускаем драв
void MenuStateMachine::draw() {
  if (currentItem == nullptr) return;
  
  // нуклир не здесь!!
  
  //if (nk_begin(&nuklear->ctx, "Basic window", nk_rect(10, 10, 300, 240), NK_WINDOW_NO_SCROLLBAR)) {
    currentItem->draw();
  //}
  //nk_end(&nuklear->ctx);
}

void MenuStateMachine::feedback(const PressingData &data) {
  if (currentItem == nullptr) return;
  
//   MenuItem* item = currentItem->feedback(data);
//   if (item != nullptr) currentItem = item;
  
  currentItem = currentItem->feedback(data);
}

void MenuStateMachine::feedback(const MousePos &data) {
  if (currentItem == nullptr) return;
  
  currentItem->feedback(data);
}

void MenuStateMachine::setDefaultItem(MenuItem* item) {
  defaultItem = item;
}

void MenuStateMachine::openMenu() {
  currentItem = defaultItem;
}

bool MenuStateMachine::isOpened() const {
  return currentItem != nullptr;
}
