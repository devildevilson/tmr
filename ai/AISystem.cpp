#include "AISystem.h"

#include "Globals.h"

#define TINY_BEHAVIOUR_MULTITHREADING
#include "tiny_behaviour/TinyBehavior.h"

#include "Graph.h"

Blackboard::Blackboard() {}
Blackboard::~Blackboard() {}

void Blackboard::setData(const Type &name, const size_t &data) {
  board[name] = data;
}

std::atomic<size_t> & Blackboard::data(const Type &name) {
  auto itr = board.find(name);
  if (itr == board.end()) throw std::runtime_error("Need to set data to board before use");
  
  return itr->second;
}

const std::atomic<size_t> & Blackboard::data(const Type &name) const {
  auto itr = board.find(name);
  if (itr == board.end()) throw std::runtime_error("Need to set data to board before use");
  
  return itr->second;
}

CPUAISystem::CPUAISystem(const CreateInfo &info) : updateDelta(info.updateDelta), accumulator(0), pool(info.pool), pathfinding(nullptr), container(info.utilitySystemsSize) {}
CPUAISystem::~CPUAISystem() {
  container.destroy(pathfinding);
  container.destroy(graph);
  
//  for (size_t i = 0; i < groupAI.size(); ++i) {
//    groupAIPool.deleteElement(groupAI[i]);
//  }
  
  for (size_t i = 0; i < groups.size(); ++i) {
    groupPool.deleteElement(groups[i]);
  }
  
  for (auto &pair : treesPtr) {
    delete pair.second;
  }
}

void CPUAISystem::update(const uint64_t &time) {
  // как у нас выглядит типичный апдейт?
  
  // как я и писал ранее, аи будет обрабатывается четко определенными шагами:
  // 1. обновляем данные восприятия, причем мы по идее должны зарегестрировать некоторые триггеры
  //    скорее всего выносить пересечения в эвенты не нужно 
  //    (так как понятное дело что почти в 90% случаях есть пересечение хоть с чем то, большая часть из этого не нужна)
  //    пересечения нужно собирать и так, просто выдать аи доступ к буферам с пересечениями (нужно просто сделать это ридонли)
  //    когда собирать восприятие? понятное дело что вызывать ии в каждом кадре - это накладно, да и не нужно
  //    тогда восприятие должно собираться тоже только в те моменты когда мы обновляем ии? 
  //    мы не знаем когда нам могут пригодиться данные о восприятии, положении и прочем, то есть полное обновление дерева поведения
  //    каждый кадр вызываться естественно не будет, но вот отдельные ее части еще как, следовательно может потребоваться все
  //    видимо обновлять данные придется каждый раз
  // 2. теперь обновляем сам компонент, то есть обновляем дерево поведения
  //    что это означает? мы должны проверить подошло ли время для обновления всего дерева, если нет, то обновить лишь ту часть которая RUNNING
  // 3. после того как мы обновим дерево, у нас будет несколько вызванных эвентов (думаю что обходить подписки о эвентах можено и на месте)
  //    эвенты будут определять дальнейшее поведение энтити (ударить, подойти, сделать скилл и прочее)
  //    на эвенты будут подписаны разные части энтити, которые отреагируют в дальнейшей работе движка
  //    тут меня интересует эвент движения, мы должны из энтитиАИ подписаться на все эвенты поиска пути
  //    в этом эвенте мы должны добавить энтити в очередь на поиск пути, и затем запустить поиск пути 
  
  // вроде бы все, дальше приступаем к обновлению других вещей
  
//   for (size_t i = 0; i < aiObjects.size(); ++i) {
    // нужно ли обновлять объекты этого типа? скорее да чем нет
    // в каком плане обновлять? что мне требуется сделать чтобы обновить декоративный предмет?
    // нет скорее всего делать ничего не нужно
//   }
  
  static const auto updateAIData = [&] (const size_t &start, const size_t &count) {
    for (size_t i = start; i < start+count; ++i) {
      yacs::component_handle<AIComponent> handle = Global::world()->get_component<AIComponent>(i);
      handle->updateAIData();
//      aiEntities[i]->updateAIData();
    }
  };
  
  static const auto updateAI = [&] (const size_t &start, const size_t &count, const size_t &time) {
    for (size_t i = start; i < start+count; ++i) {
      yacs::component_handle<AIComponent> handle = Global::world()->get_component<AIComponent>(i);
      handle->update(time);
//      aiEntities[i]->update(time);
    }
  };
  
//  static const auto updateAIGroup = [&] (const size_t &start, const size_t &count, const size_t &time) {
//    for (size_t i = start; i < start+count; ++i) {
//      groupAI[i]->update(time);
//    }
//  };
  
  // кадр может не успеть сделать всю нагрузку и тогда аккумулятор будет все больше и больше, что будет в итоге приводить к зависанию
  // как ограничивать? вообще можно accumulator = (accumulator + frameTime) % CONST
  // тогда дойдя до определенного уровня мы будем скидывать все накопления и начинать заного
  // наверное вместе с этим надо бы что нибудь в консоль вывести
  // какая константа? я думаю что 
  
  const size_t frameTime = std::min(time, size_t(ACCUMULATOR_MAX_CONSTANT));
  accumulator += frameTime;
  
  if (accumulator > ACCUMULATOR_MAX_CONSTANT) {
    accumulator = accumulator % ACCUMULATOR_MAX_CONSTANT;
    
    Global::console()->printW("AI lags detected. Check your PC suitability for the game minimal requirements or remove some redundant mods");
  }

  const size_t componentCount = Global::world()->count_components<AIComponent>();
  while (accumulator >= updateDelta) {
    // нужно ли здесь что-то интерполировать? пока ничего в голову не приходит
    /*if (!aiEntities.empty())*/ {
      const size_t count = std::ceil(float(componentCount) / float(pool->size()+1));
      size_t start = 0;
      for (uint32_t i = 0; i < pool->size()+1; ++i) {
        const size_t jobCount = std::min(count, componentCount-start);
        if (jobCount == 0) break;

        pool->submitnr(updateAIData, start, jobCount);

        start += jobCount;
      }
      
      pool->compute();
      pool->wait();
    }
    
    // нужно наверное сделать по отдельности
//     if (!aiEntities.empty() || !groupAI.empty()) {
      /*if (!aiEntities.empty())*/ {
        const size_t count = std::ceil(float(componentCount) / float(pool->size()+1));
        size_t start = 0;
        for (uint32_t i = 0; i < pool->size()+1; ++i) {
          const size_t jobCount = std::min(count, componentCount-start);
          if (jobCount == 0) break;

          pool->submitnr(updateAI, start, jobCount, updateDelta);

          start += jobCount;
        }
      }
      
//      /*if (!groupAI.empty())*/ {
//        const size_t count = std::ceil(float(groupAI.size()) / float(pool->size()+1));
//        size_t start = 0;
//        for (uint32_t i = 0; i < pool->size()+1; ++i) {
//          const size_t jobCount = std::min(count, aiEntities.size()-start);
//          if (jobCount == 0) break;
//
//          pool->submitnr(updateAIGroup, start, jobCount, updateDelta);
//
//          start += jobCount;
//        }
//      }
      
      pool->compute();
      pool->wait();
//     }
    
  //   // обновляем данные ии (позиция, скорость и др наравне с данными восприятия)
  //   for (size_t i = 0; i < aiEntities.size(); ++i) {
  //     aiEntities[i]->updateAIData();
  //   }
  //   
  //   // обновляем дерево поведения для ии, ах да
  //   // у нас же еще есть групповой ии
  //   for (size_t i = 0; i < aiEntities.size(); ++i) {
  //     aiEntities[i]->update(time);
  //   }
  //   
  //   // групповой ии в принципе делает примерно то же самое 
  //   // обходит дерево и принимает какие-то решения
  //   // как разделить деревья для группы и обычные?
  //   // по идее это теже деревья просто указатель мы приводим к другому типу
  //   // 
  //   for (size_t i = 0; i < groups.size(); ++i) {
  //     groupAI[i]->update(time);
  //   }
    
    // внури вызовутся какие-нибудь эвенты, например для поиска пути
    // и соотвественно после этого шага мы вызовем поиск пути
    // эвенты для поиска пути портят на самом деле всю малину
    pathfinding->update();
    
    accumulator -= updateDelta;
  }

//   const float alpha = float(accumulator) / float(updateDelta);
  
  // примерно так выглядит метод апдейт
}

Blackboard & CPUAISystem::blackboard() {
  return board;
}

const Blackboard & CPUAISystem::blackboard() const {
  return board;
}

PathFindingPhase* CPUAISystem::pathfindingSystem() const {
  return pathfinding;
}

//void CPUAISystem::registerComponent(AIComponent* component) {
//  component->internalIndex() = aiEntities.size();
//  aiEntities.push_back(component);
//}
//
//void CPUAISystem::registerBasicComponent(AIBasicComponent* component) {
//  component->internalIndex() = aiObjects.size();
//  aiObjects.push_back(component);
//}
//
//void CPUAISystem::removeComponent(AIComponent* component) {
//  if (aiEntities[component->internalIndex()] != component) return;
//
//  aiEntities.back()->internalIndex() = component->internalIndex();
//  std::swap(aiEntities[component->internalIndex()], aiEntities.back());
//  aiEntities.pop_back();
//}
//
//void CPUAISystem::removeBasicComponent(AIBasicComponent* component) {
//  if (aiObjects[component->internalIndex()] != component) return;
//
//  aiObjects.back()->internalIndex() = component->internalIndex();
//  std::swap(aiObjects[component->internalIndex()], aiObjects.back());
//  aiObjects.pop_back();
//}

size_t CPUAISystem::getUpdateDelta() const {
  return updateDelta;
}

void CPUAISystem::setBehaviourTreePointer(const Type &name, tb::BehaviorTree* tree) {
  auto itr = treesPtr.find(name);
  if (itr != treesPtr.end()) throw std::runtime_error("Behaviour tree with name " + name.name() + " is already exist");
  
  treesPtr[name] = tree;
}

tb::BehaviorTree* CPUAISystem::getBehaviourTreePointer(const Type &name) const {
  auto itr = treesPtr.find(name);
  if (itr == treesPtr.end()) throw std::runtime_error("Behaviour tree with name " + name.name() + " is not exist");
  
  return itr->second;
}

Graph* CPUAISystem::getGraph() {
  return graph;
}
