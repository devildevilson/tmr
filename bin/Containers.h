#ifndef CONTAINERS_H
#define CONTAINERS_H

#include "Engine.h"
#include "TypelessContainer.h"
#include "Optimizer.h"
//#include "../resources/Manager.h"
//#include "../resources/ResourceManager.h"
// #include "ResourceParser.h"
// #include "ModificationParser.h"

#include "ArrayInterface.h"

// названия то различаюся, нужно переделать
#include "Physics.h"
#include "BroadphaseInterface.h"
#include "Solver.h"
#include "NarrowphaseInterface.h"
#include "PhysicsSorter.h"

#include <vector>
#include <cstdint>

//class PhysicsEngine;
//class Broadphase;
//class Solver;
//class Narrowphase;
//class PhysicsSorter;

//class ResourceParser;
//class ModificationParser;

// это все что нужно скорее всего
// другое дело что некоторые системы у меня еще не готовы
// и вместо них стоят заглушки написанные от руки

class GameSystemContainer {
public:
  GameSystemContainer(const size_t &gameSize) : container(gameSize) {}
  ~GameSystemContainer() {
    for (size_t i = 0; i < engines.size(); ++i) {
      container.destroy(engines[i]);
    }
  }
  
  template <typename T, typename ...Args>
  T* addSystem(Args&&... args) {
    T* ptr = container.create<T>(std::forward<Args>(args)...);
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
  TypelessContainer container;
};

class OptimiserContainer {
public:
  OptimiserContainer(const size_t &size) : container(size) {}
  ~OptimiserContainer() {
    for (size_t i = 0; i < optimizers.size(); ++i) {
      container.destroy(optimizers[i]);
    }
  }
  
  template <typename T, typename ...Args>
  T* add(Args&&... args) {
    T* ptr = container.create<T>(std::forward<Args>(args)...);
    optimizers.push_back(ptr);
    
    return ptr;
  }
  
  size_t size() const {
    return container.size();
  }
private:
  TypelessContainer container;
  std::vector<Optimizer*> optimizers;
};

// class ParserContainer {
// public:
//   ParserContainer(const size_t &size) : ptr(nullptr), parserContainers(size) {}
//   ~ParserContainer() {
// //    if (ptr != nullptr) {
// //      delete ptr;
// //    }
//     
//     for (auto & parser : parsers) {
//       parserContainers.destroy(parser);
//     }
// 
//     parserContainers.destroy(ptr);
//   }
//   
//   template <typename T, typename ...Args>
//   T* add(Args&&... args) {
//     T* ptr = parserContainers.create<T>(std::forward<Args>(args)...);
//     parsers.push_back(ptr);
//     
//     return ptr;
//   }
//   
//   template <typename T, typename ...Args>
//   T* addModParser(Args&&... args) {
//     if (ptr != nullptr) throw std::runtime_error("ParserContainer error: ptr is already exist");
// 
// //    auto* ret = new T(std::forward<Args>(args)...);
//     auto* ret = parserContainers.create<T>(std::forward<Args>(args)...);
//     ptr = ret;
//     
//     return ret;
//   }
//   
//   size_t size() const {
//     return parserContainers.size();
//   }
// private:
//   std::vector<ResourceParser*> parsers;
//   ModificationParser* ptr;
//   TypelessContainer parserContainers;
// };

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
    if (broad != nullptr) container.destroy(broad);
    if (narrow != nullptr) container.destroy(narrow);
    if (solver != nullptr) container.destroy(solver);
    if (sorter != nullptr) container.destroy(sorter);
    if (physics != nullptr) container.destroy(physics);
  }
  
  template <typename T, typename ...Args> 
  T* createBroadphase(Args&&... args) {
    if (broad != nullptr) throw std::runtime_error("PhysicsContainer error: broadphase is already exist");
    
    T* ptr = container.create<T>(std::forward<Args>(args)...);
    broad = ptr;
    
    return ptr;
  }
  
  template <typename T, typename ...Args> 
  T* createNarrowphase(Args&&... args) {
    if (narrow != nullptr) throw std::runtime_error("PhysicsContainer error: broadphase is already exist");
    
    T* ptr = container.create<T>(std::forward<Args>(args)...);
    narrow = ptr;
    
    return ptr;
  }
  
  template <typename T, typename ...Args> 
  T* createSolver(Args&&... args) {
    if (solver != nullptr) throw std::runtime_error("PhysicsContainer error: broadphase is already exist");
    
    T* ptr = container.create<T>(std::forward<Args>(args)...);
    solver = ptr;
    
    return ptr;
  }
  
  template <typename T, typename ...Args> 
  T* createPhysicsSorter(Args&&... args) {
    if (sorter != nullptr) throw std::runtime_error("PhysicsContainer error: broadphase is already exist");
    
    T* ptr = container.create<T>(std::forward<Args>(args)...);
    sorter = ptr;
    
    return ptr;
  }
  
  template <typename T, typename ...Args> 
  T* createPhysicsEngine(Args&&... args) {
    if (physics != nullptr) throw std::runtime_error("PhysicsContainer error: broadphase is already exist");
    
    T* ptr = container.create<T>(std::forward<Args>(args)...);
    physics = ptr;
    
    return ptr;
  }
private:
  Broadphase* broad;
  Narrowphase* narrow;
  Solver* solver;
  PhysicsSorter* sorter;
  PhysicsEngine* physics;
  TypelessContainer container;
};

class ArrayContainers {
public:
  ArrayContainers(const size_t &size) : container(size) {}
  ~ArrayContainers() {
    for (size_t i = 0; i < destructables.size(); ++i) {
      container.destroy(destructables[i]);
    }
  }
    
  template <typename T, typename ...Args>
  T* add(Args&&... args) {
    T* ptr = container.create<T>(std::forward<Args>(args)...);
    destructables.push_back(ptr);
    
    return ptr;
  }
private:
  std::vector<Destructable*> destructables;
  TypelessContainer container;
};

#endif
