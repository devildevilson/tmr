#ifndef GAME_SYSTEM_CONTAINER_H
#define GAME_SYSTEM_CONTAINER_H

#include "Engine.h"
#include "TypelessContainer.h"

#include <vector>
#include <cstdint>

// это все что нужно скорее всего
// другое дело что некоторые системы у меня еще не готовы
// и вместо них стоят заглушки написанные от руки

class GameSystemContainer {
public:
  GameSystemContainer(const size_t &gameSize) : container(gameSize) {}
  ~GameSystemContainer() {
    for (size_t i = 0; i < engines.size(); ++i) {
      container.destroyStage(engines[i]);
    }
  }
  
  template <typename T, typename Args...>
  T* addSystem(Args&&... args) {
    T* ptr = container.addStage<T>(std::forward<Args>(args)...);
    engines.push_back(ptr);
    
    return ptr;
  }
  
  void updateSystems(const uint64_t &time) {
    for (size_t i = 0; i < engines.size(); ++i) {
      engines[i]->update(time);
    }
  }
private:
  std::vector<Engine*> engines;
  StageContainer container;
};

#endif
