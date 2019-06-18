#ifndef AI_COMPONENT_H
#define AI_COMPONENT_H

#include "EntityAI.h"
#include "EntityComponentSystem.h"
#include "PathFindingPhase.h"

namespace tb {
  class BehaviorTree;
  class Node;
}

class vertex_t;
class edge_t;

class PhysicsComponent2;
class TransformComponent;
class AIInputComponent;

// тут нужно почекать верные деструкторы 
class AIBasicComponent : public EntityAI, public yacs::Component {
  friend class AISystem;
public:
  struct CreateInfo {
    // тут у нас сразу должна быть указана вершина
    // к которой принадлежит объект
    
    float radius;
    vertex_t* currentVertex;
  };
  
  AIBasicComponent() = delete;
  AIBasicComponent(const CreateInfo &info);
  ~AIBasicComponent();
  
  // update и другие компонентные фичи
  // по идее это класс для разных активируемых объектов
  
  void update(const size_t &time) override;
  void init(void* userData) override;
  
//   void setPosition(const glm::vec4 &value);
//   void direction(const glm::vec4 &value);
//   void velocity(const glm::vec4 &value);
  //void raduis(const float &value) const;
  
  size_t & internalIndex();
protected:
  // тут будет что нибудь?
  
  size_t internalIndexVal;
  
  // нужно тут сохранить позицию (центр) и направление (нормаль)
  // 
};

// так ли необходимо наследоваться от AIBasicComponent?
// мне могут потребоваться какие-то дополнительные функции этого класса
// можно ли их сделать как-нибудь по другому? я пока еще не знаю что ме может потребоваться лол
// нужно сделать еще какое то специальное хранилище у компонента для промежуточных данных
class AIComponent : public AIBasicComponent {
  friend class AISystem;
public:
  // это полноценный ии класс
  // здесь же наверное будет метод апдейтТри
  // скорее всего
  
  struct CreateInfo {
    float radius;
    size_t timeThreshold;
    vertex_t* currentVertex;
    tb::BehaviorTree* tree;
    Type pathFindType;
  };
  AIComponent(const CreateInfo &info);
  ~AIComponent();
  
  void update(const size_t &time = 0) override;
  void init(void* userData) override;
  
  // тут мы должны заполнить такие вещи как позиция, направление, скорость
  // хотя так ли нам это нужно? так ли нам нужно выделять дополнительную память для этого?
  // 
  void updateAIData();
protected:
  struct PathData {
    RawPath* path;
    FindRequest req;
//     size_t frameIndex;
  };
  
  // основная задача это сделать деревья и нод чтоб могли выполняться на разных потоках
  // зависит это в основном не от самого дерева, а скорее от действий которое это дерево запустит
  // дерево я кажется изменил так чтобы можно было изи мультитрединг использовать (НЕ ВСЕ НОДЫ)
  tb::BehaviorTree* tree;
  tb::Node* runningNode;
  PhysicsComponent2* physics;
  TransformComponent* trans;
  AIInputComponent* input;
  
  size_t timeThreshold;
  size_t currentTime;
  size_t pathfindingFrame;
  
  // тип поиска пути
  Type pathFindType;
  PathData foundPath;
  PathData oldPath;
  
  // нужно применить funnel alg на путь и где то сохранить это дело
  // то есть у меня еще один массив будет, где его хранить?
  // можно вычислять сразу после нахождения + добавить пачку дополнительных данных
};

#endif
