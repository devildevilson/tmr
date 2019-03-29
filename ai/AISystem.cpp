#include "AISystem.h"

CPUAISystem::CPUAISystem(const CreateInfo &info) : pathfinding(info.pathfinding) {}
CPUAISystem::~CPUAISystem() {
  for (size_t i = 0; i < groupAI.size(); ++i) {
    groupAIPool.deleteElement(groupAI[i]);
  }
  
  for (size_t i = 0; i < groups.size(); ++i) {
    groupPool.deleteElement(groups[i]);
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
  
  // обновляем данные ии (позиция, скорость и др наравне с данными восприятия)
  for (size_t i = 0; i < aiEntities.size(); ++i) {
    aiEntities[i]->updateAIData();
  }
  
  // обновляем дерево поведения для ии, ах да
  // у нас же еще есть групповой ии
  for (size_t i = 0; i < aiEntities.size(); ++i) {
    aiEntities[i]->update(time);
  }
  
  // групповой ии в принципе делает примерно то же самое 
  // обходит дерево и принимает какие-то решения
  // как разделить деревья для группы и обычные?
  // по идее это теже деревья просто указатель мы приводим к другому типу
  // 
  for (size_t i = 0; i < groups.size(); ++i) {
    groupAI[i]->update(time);
  }
  
  // внури вызовутся какие-нибудь эвенты, например для поиска пути
  // и соотвественно после этого шага мы вызовем поиск пути
  // эвенты для поиска пути портят на самом деле всю малину
  pathfinding->update();
  
  // примерно так выглядит метод апдейт
}

Blackboard & CPUAISystem::blackboard() {
  return board;
}

const Blackboard & CPUAISystem::blackboard() const {
  return board;
}

void CPUAISystem::registerComponent(AIComponent* component) {
  component->internalIndex() = aiEntities.size();
  aiEntities.push_back(component);
}

void CPUAISystem::registerBasicComponent(AIBasicComponent* component) {
  component->internalIndex() = aiObjects.size();
  aiObjects.push_back(component);
}

void CPUAISystem::removeComponent(AIComponent* component) {
  aiEntities.back()->internalIndex() = component->internalIndex();
  std::swap(aiEntities[component->internalIndex()], aiEntities.back());
  aiEntities.pop_back();
}

void CPUAISystem::removeBasicComponent(AIBasicComponent* component) {
  aiObjects.back()->internalIndex() = component->internalIndex();
  std::swap(aiObjects[component->internalIndex()], aiObjects.back());
  aiObjects.pop_back();
}
