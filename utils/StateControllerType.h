#ifndef STATE_CONTROLLER_TYPE_H
#define STATE_CONTROLLER_TYPE_H

#include "Type.h"
#include "ResourceID.h"
#include "Interaction.h"

#include <vector>

struct StateFlags {
  uint32_t container;

  StateFlags();
  StateFlags(const bool blocking, const bool blockingMovement, const bool looping, const bool useWeapon, const bool useItem);
  void make(const bool blocking, const bool blockingMovement, const bool looping, const bool useWeapon, const bool useItem);

  bool blocking() const;
  bool blockingMovement() const;
  bool looping() const;
  bool useWeapon() const;
  bool useItem() const;
};

// посмотрим что еще сюда можно добавить
// + я хотел бликорование вынести в аттрибуты
// это вообще особо не требуется
struct StateDataCreateInfo {
  StateFlags flags;

  size_t time;

  ResourceID animation;
  ResourceID sound;
//  enum Interaction::type interactionType;
//  Type interactionEvent;

  Type state;
  Type nextState; // смысла в нахождении нет
};

struct StateData {
  StateFlags flags;

  size_t time;

  // часть вещей мы должны взять из оружия
  // точнее большую часть вещей
  ResourceID animation;
  ResourceID sound;
  // тут еще поди может потребоваться указать relative sound pos

  // нужны доп данные для анимаций и звука

//   size_t animationDelay;
//   size_t soundDelay;
//   size_t interactionDelay;

//  enum Interaction::type interactionType;
//  Type interactionEvent;
  // двигать оружее по синусоиде - анимационный компонент
  // все анимационные состояния должны быть в оружии
  // смена оружия? тоже анимация но при этом разбита на два этапа
  // запускаем одну анимацию, после меняем указатель, запускаем другую анимацию
  // как вызвать интеракцию? просто флажок поставить? ну требование оч простое

  Type state;
  const StateData* nextState;
};

class StateControllerType {
public:
  struct CreateInfo {
    std::vector<StateDataCreateInfo> states;
    size_t defaultIndex;
    std::vector<std::pair<Type, Type>> aliases;
  };
  StateControllerType(const CreateInfo &info);

  const StateData* defaultState() const;
  const StateData* state(const Type &state) const;
  size_t statesCount() const;
private:
  StateData* defaultStatePtr;
  std::unordered_map<Type, StateData> states;
  std::unordered_map<Type, const StateData*> aliases;
};

#endif //STATE_CONTROLLER_TYPE_H
