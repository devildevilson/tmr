#include "StateControllerType.h"

enum {
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

StateControllerType::StateControllerType(const CreateInfo &info) : defaultStatePtr(nullptr) {
  for (const auto state : info.states) {
    states[state.state] = {
      state.flags,
      state.time,
      state.animation,
      state.sound,
      state.state,
      nullptr
    };
  }

  for (auto &state : states) {
    for (const auto infoState : info.states) {
      if (state.second.state == infoState.state) {
        if (!infoState.nextState.valid()) continue;

        auto itr = states.find(infoState.nextState);
        if (itr == states.end()) throw std::runtime_error("Bad state controller type creation");
        state.second.nextState = &itr->second;
      }
    }
  }

  const Type &defaultType = info.states[info.defaultIndex].state;
  auto itr = states.find(defaultType);
  if (itr == states.end()) throw std::runtime_error("Bad state controller type creation");
  defaultStatePtr = &itr->second;
}

const StateData* StateControllerType::defaultState() const {
  return defaultStatePtr;
}

const StateData* StateControllerType::state(const Type &state) const {
  auto itr = states.find(state);
  if (itr == states.end()) return nullptr;

  return &itr->second;
}

size_t StateControllerType::statesCount() const {
  return states.size();
}
