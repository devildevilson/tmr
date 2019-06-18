#ifndef MENU_ITEMS_H
#define MENU_ITEMS_H

#include "Menu.h"

struct nuklear_data;

class ButtonItem : public MenuItem {
public:
  ButtonItem(nuklear_data* nuklear, const std::string &name, MenuItem* nextItem, const Extent &extent);
  ~ButtonItem();
  
  void draw() override;
  MenuItem* feedback(const PressingData &data) override;
  bool feedback(const MousePos &data) override;
  
private:
  nuklear_data* nuklear;
  std::string name;
  // размеры
  MenuItem* nextItem;
};

class MainMenu : public MenuItem {
public:
  MainMenu(nuklear_data* nuklear);
  ~MainMenu();
  
  void draw() override;
  MenuItem* feedback(const PressingData &data) override;
  bool feedback(const MousePos &data) override;
  
  void addItem(MenuItem* item);
private:
  nuklear_data* nuklear;
  size_t itemIndex;
  std::vector<MenuItem*> items;
};

class QuitGame : public MenuItem {
public:
  QuitGame(nuklear_data* nuklear, bool* quit, MenuItem* prevItem);
  ~QuitGame();
  
  void draw() override;
  MenuItem* feedback(const PressingData &data) override;
  bool feedback(const MousePos &data) override;
private:
  nuklear_data* nuklear;
  bool* quit;
  MenuItem* prevItem;
  uint8_t focusedAns;
};

class SettingsMenu : public MenuItem {
public:
  
private:
  nuklear_data* nuklear;
  size_t itemIndex;
  std::vector<MenuItem*> items;
};

#endif
