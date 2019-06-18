#ifndef MENU_H
#define MENU_H

#include <vector>
#include "StageContainer.h"

struct PressingData {
  uint32_t button;
  uint32_t modifier;
};

struct MousePos {
  float x;
  float y;
};

struct Extent {
  float width;
  float height;
};

// тут нужно наверное все же добавить x y 
class MenuItem {
public:
  MenuItem() {}
  MenuItem(const Extent extent) : extent(extent) {}
  virtual ~MenuItem() {}
  
  virtual void draw() = 0;
  virtual MenuItem* feedback(const PressingData &data) = 0;
  virtual bool feedback(const MousePos &data) = 0;
  Extent size() const { return extent; }
protected:
  Extent extent;
};

class MenuStateMachine {
public:
  MenuStateMachine(const size_t &size);
  ~MenuStateMachine();
  
  // по идее запускаем nk_begin и затем обходим меню айтем и там тоже запускаем драв
  void draw();
  void feedback(const PressingData &data);
  void feedback(const MousePos &data);
  
  void setDefaultItem(MenuItem* item);
  void openMenu();
  bool isOpened() const;
  
  template <typename T, typename... Args>
  T* addMenuItem(Args&&... args) {
    T* ptr = menuContainer.addStage<T>(std::forward<Args>(args)...);
    items.push_back(ptr);
    
    return ptr;
  }
private:
  MenuItem* defaultItem;
  MenuItem* currentItem;
  
//   nuklear_data* nuklear;
  
  StageContainer menuContainer;
  std::vector<MenuItem*> items;
};

#endif
