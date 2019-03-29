#ifndef CONTAINERS_H
#define CONTAINERS_H

#include "Engine.h"
#include "StageContainer.h"
#include "Optimizer.h"
#include "Manager.h"
#include "ResourceManager.h"

#include "ArrayInterface.h"

#include "Physics.h"
#include "BroadphaseInterface.h"
#include "Solver.h"
#include "NarrowphaseInterface.h"
#include "PhysicsSorter.h"

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
  
  template <typename T, typename ...Args>
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
  
  size_t size() const {
    return container.size();
  }
private:
  std::vector<Engine*> engines;
  StageContainer container;
};

class OptimiserContainer {
public:
  OptimiserContainer(const size_t &size) : container(size) {}
  ~OptimiserContainer() {
    for (size_t i = 0; i < optimizers.size(); ++i) {
      container.destroyStage(optimizers[i]);
    }
  }
  
  template <typename T, typename ...Args>
  T* add(Args&&... args) {
    T* ptr = container.addStage<T>(std::forward<Args>(args)...);
    optimizers.push_back(ptr);
    
    return ptr;
  }
  
  size_t size() const {
    return container.size();
  }
private:
  StageContainer container;
  std::vector<Optimizer*> optimizers;
};

class ParserContainer {
public:
  ParserContainer(const size_t &size) : ptr(nullptr), parserContainers(size) {}
  ~ParserContainer() {
    if (ptr != nullptr) {
      delete ptr;
    }
    
    for (size_t i = 0; i < parsers.size(); ++i) {
      parserContainers.destroyStage(parsers[i]);
    }
  }
  
  template <typename T, typename ...Args>
  T* add(Args&&... args) {
    T* ptr = parserContainers.addStage<T>(std::forward<Args>(args)...);
    parsers.push_back(ptr);
    
    return ptr;
  }
  
  template <typename ...Args>
  ParserHelper* addParserHelper(Args&&... args) {
    if (ptr != nullptr) throw std::runtime_error("ParserContainer error: ptr is already exist");
    
    ptr = new ParserHelper(std::forward<Args>(args)...);
    
    return ptr;
  }
  
  size_t size() const {
    return parserContainers.size();
  }
private:
  std::vector<ResourceParser*> parsers;
  ParserHelper* ptr;
  StageContainer parserContainers;
};

class PhysicsContainer {
public:
  PhysicsContainer(const size_t &size) 
  : broad(nullptr), 
    narrow(nullptr),
    solver(nullptr),
    sorter(nullptr),
    physics(nullptr),
    container(size) {}
    
  ~PhysicsContainer() {
    if (broad != nullptr) container.destroyStage(broad);
    if (narrow != nullptr) container.destroyStage(narrow);
    if (solver != nullptr) container.destroyStage(solver);
    if (sorter != nullptr) container.destroyStage(sorter);
    if (physics != nullptr) container.destroyStage(physics);
  }
  
  template <typename T, typename ...Args> 
  T* createBroadphase(Args&&... args) {
    if (broad != nullptr) throw std::runtime_error("PhysicsContainer error: broadphase is already exist");
    
    T* ptr = container.addStage<T>(std::forward<Args>(args)...);
    broad = ptr;
    
    return ptr;
  }
  
  template <typename T, typename ...Args> 
  T* createNarrowphase(Args&&... args) {
    if (narrow != nullptr) throw std::runtime_error("PhysicsContainer error: broadphase is already exist");
    
    T* ptr = container.addStage<T>(std::forward<Args>(args)...);
    narrow = ptr;
    
    return ptr;
  }
  
  template <typename T, typename ...Args> 
  T* createSolver(Args&&... args) {
    if (solver != nullptr) throw std::runtime_error("PhysicsContainer error: broadphase is already exist");
    
    T* ptr = container.addStage<T>(std::forward<Args>(args)...);
    solver = ptr;
    
    return ptr;
  }
  
  template <typename T, typename ...Args> 
  T* createPhysicsSorter(Args&&... args) {
    if (sorter != nullptr) throw std::runtime_error("PhysicsContainer error: broadphase is already exist");
    
    T* ptr = container.addStage<T>(std::forward<Args>(args)...);
    sorter = ptr;
    
    return ptr;
  }
  
  template <typename T, typename ...Args> 
  T* createPhysicsEngine(Args&&... args) {
    if (physics != nullptr) throw std::runtime_error("PhysicsContainer error: broadphase is already exist");
    
    T* ptr = container.addStage<T>(std::forward<Args>(args)...);
    physics = ptr;
    
    return ptr;
  }
private:
  Broadphase* broad;
  Narrowphase* narrow;
  Solver* solver;
  PhysicsSorter* sorter;
  PhysicsEngine* physics;
  StageContainer container;
};

class ArrayContainers {
public:
  ArrayContainers(const size_t &size) : container(size) {}
  ~ArrayContainers() {
    for (size_t i = 0; i < destructables.size(); ++i) {
      container.destroyStage(destructables[i]);
    }
  }
    
  template <typename T, typename ...Args>
  T* add(Args&&... args) {
    T* ptr = container.addStage<T>(std::forward<Args>(args)...);
    destructables.push_back(ptr);
    
    return ptr;
  }
private:
  std::vector<Destructable*> destructables;
  StageContainer container;
};

#endif
