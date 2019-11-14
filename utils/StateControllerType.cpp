#include "StateControllerType.h"

//#define TINY_BEHAVIOUR_MULTITHREADING
//#include "tiny_behaviour/TinyBehavior.h"

// если я хочу использовать дерево поведения
// то мне нужно чтобы обязательно поведение продолжалось с определенного нода?
// можно воспользоваться мемори версиями нодов, в принципе это решит мою проблему
// но у нас еще есть ситуация когда нужно сменить состояние посреди другого
// как быть? нужно обходить все ноды, в первых нодах должны быть состояния
// в которые переход не зависит от текущего состояния
// то есть будет иерархия селектов, которая будет постепенно разворачивать
// дерево состояний, чем дальше тем сложнее описать это дело в json лол
// сами то состояния описать будет несложно, переходы описать гораздо сложнее

// так, а зачем мне дерево поведения то здесь?
// то есть из дерева у меня приходят эвенты на почти все случаи жизни
// а, кажется я пытался понять переходить ли мне при наличие/отсутствии патронов
// то есть нужно ограничить вызов эвента атака при отсутствии патронов, например
// при вызове эвента атака проверяем патроны, возвращаем фейл если все плохо
// как зависимость проверить? зависимости видимо нужно проверять отдельно
//

//class Walking : public tb::Leaf {
//public:
//  // должен быть конст
//  tb::Node::status update(void* const& data, Node** runningPtr) override {
////    if (key_down || speed != 0.0f) {
////      return tb::Node::status::running;
////    }
//
//    return tb::Node::status::failure;
//  }
//};

enum StateFlagsEnum : uint32_t {
  STATE_FLAG_BLOCKING = (1<<0),
  STATE_FLAG_BLOCKING_MOVEMENT = (1<<2),
  STATE_FLAG_LOOPING = (1<<3),
  STATE_FLAG_USE_WEAPON = (1<<4),
  STATE_FLAG_USE_ITEM = (1<<5),
};

StateFlags::StateFlags() : container(0) {}
StateFlags::StateFlags(const bool blocking, const bool blockingMovement, const bool looping, const bool useWeapon, const bool useItem) : container(0) {
  make(blocking, blockingMovement, looping, useWeapon, useItem);
}

void StateFlags::make(const bool blocking, const bool blockingMovement, const bool looping, const bool useWeapon, const bool useItem) {
  container = (STATE_FLAG_BLOCKING * blocking) |
              (STATE_FLAG_BLOCKING_MOVEMENT * blockingMovement) |
              (STATE_FLAG_LOOPING * looping) |
              (STATE_FLAG_USE_WEAPON * useWeapon) |
              (STATE_FLAG_USE_ITEM * useItem);
}

bool StateFlags::blocking() const {
  return (STATE_FLAG_BLOCKING & container) == STATE_FLAG_BLOCKING;
}

bool StateFlags::blockingMovement() const {
  return (STATE_FLAG_BLOCKING_MOVEMENT & container) == STATE_FLAG_BLOCKING_MOVEMENT;
}

bool StateFlags::looping() const {
  return (STATE_FLAG_LOOPING & container) == STATE_FLAG_LOOPING;
}

bool StateFlags::useWeapon() const {
  return (STATE_FLAG_USE_WEAPON & container) == STATE_FLAG_USE_WEAPON;
}

bool StateFlags::useItem() const {
  return (STATE_FLAG_USE_ITEM & container) == STATE_FLAG_USE_ITEM;
}

StateControllerType::StateControllerType(const CreateInfo &info) : container(sizeof(StateData)*info.states.size()), defaultStatePtr(nullptr), states(info.states.size(), nullptr) {
  for (size_t i = 0; i < info.states.size(); ++i) {
    const StateData stateInfo{
      info.states[i].flags,
      info.states[i].time,
      info.states[i].animation,
      info.states[i].sound,
      info.states[i].constantSound,
      info.states[i].relative,
      info.states[i].scalar,
      info.states[i].animationDelay,
      info.states[i].soundDelay,
      info.states[i].speedModificator,
      info.states[i].state,
      nullptr
    };
    states[i] = container.create<StateData>(stateInfo);
  }
  
  for (auto state : states) {
    for (const auto &infoState : info.states) {
      if (state->state == infoState.state) {
        if (!infoState.nextState.valid()) continue;
        
        const size_t index = findState(infoState.nextState);
        if (index == SIZE_MAX) throw std::runtime_error("Bad state controller type creation");
        state->nextState = states[index];
      }
    }
  }
  
  const size_t index = findState(info.defaultState);
  if (index == SIZE_MAX) throw std::runtime_error("Bad state controller type creation");
  defaultStatePtr = states[index];
  
  // алиасы
}

const StateData* StateControllerType::defaultState() const {
  return defaultStatePtr;
}

const StateData* StateControllerType::state(const Type &state) const {
  const size_t index = findState(state);
  if (index != SIZE_MAX) return states[index];

  return nullptr;
}

size_t StateControllerType::statesCount() const {
  return states.size();
}

size_t StateControllerType::findState(const Type &type) const {
  for (size_t i = 0; i < states.size(); ++i) {
    if (states[i]->state == type) return i;
  }
  
  return SIZE_MAX;
}
