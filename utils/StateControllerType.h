#ifndef STATE_CONTROLLER_TYPE_H
#define STATE_CONTROLLER_TYPE_H

#include "Type.h"
#include "ResourceID.h"
#include "Interaction.h"
#include "TypelessContainer.h"

#include <vector>

template <typename T>
class AttributeType;

struct StateFlags {
  uint32_t container;

  StateFlags();
  StateFlags(const bool blocking, const bool blockingMovement, const bool looping, const bool useWeapon, const bool useItem);
  void make(const bool blocking, const bool blockingMovement, const bool looping, const bool useWeapon, const bool useItem);

  bool blocking() const;
  bool blockingMovement() const;
  bool looping() const;
  bool useWeapon() const; // уходит
  bool useItem() const;   // уходит
};

// нужно ли вводить половинки направлений от игрока
// ну и я в итоге прихожу к пониманию что мне везде нужно вставить определяемую в рантайме функцию 
// чтобы не заниматься этой херней, хоть бы функции на луа не были очень медленными
// мне нужны эти функции в нескольких местах, например в абилке
// но тут чаще всего мне нужен скаляр который означает расстояние от игрока в сторону того куда он смотрит
// иные направления скорее всего будут не нужны
// enum class dir {
//   forward,
//   backward,
//   right,
//   left,
//   
//   north,
//   half_east,
//   east,
//   half_south,
//   south,
//   half_west,
//   west,
//   half_north
// };

// посмотрим что еще сюда можно добавить
// + я хотел бликорование вынести в аттрибуты
// это вообще особо не требуется
struct StateDataCreateInfo {
  StateFlags flags;

  size_t time;

  ResourceID animation;
  ResourceID sound;
  
  bool constantSound;
  bool relative;
  float scalar;
  size_t animationDelay;
  size_t soundDelay;
  
  const AttributeType<float>* speedModificator;

  Type state;
  Type nextState; // смысла в нахождении нет
};

struct StateData {
  StateFlags flags;

  size_t time;
  
  ResourceID animation; // делэй? может пригодиться
  ResourceID sound; 
  
  bool constantSound; // звук не меняет положение в зависимости от положения энтити
  bool relative;
  float scalar; // как это задать? скорее всего нас будет интересовать не абсолютная позиция, а скаляр + направление в зависимости от положения источника
  size_t animationDelay;
  size_t soundDelay;
  
  const AttributeType<float>* speedModificator;

  Type state;
  const StateData* nextState;
};

class StateControllerType {
public:
  struct CreateInfo {
    std::vector<StateDataCreateInfo> states;
    Type defaultState;
    std::vector<std::pair<Type, Type>> aliases;
  };
  StateControllerType(const CreateInfo &info);

  const StateData* defaultState() const;
  const StateData* state(const Type &state) const;
  size_t statesCount() const;
private:
  TypelessContainer container;
  
  const StateData* defaultStatePtr;
  std::vector<StateData*> states;
  std::vector<std::pair<Type,const StateData*>> aliases;
  
  size_t findState(const Type &type) const;
};

#endif //STATE_CONTROLLER_TYPE_H
