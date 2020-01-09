#ifndef MOVEMENT_COMPONENT_H
#define MOVEMENT_COMPONENT_H

#include "Utility.h"

#include "PathFindingPhase.h"

class EntityAI;
class InputComponent;
class PhysicsComponent;
class TransformComponent;
class vertex_t;
namespace yacs {
  class entity;
}

enum class path_state {
  found,
  finding,
  not_found
};

enum class path_travel_state {
  path_not_exist,
  travel_old_path,
  travel_path,
  end_travel
};

// через этот компонент будем искать путь, следовать по пути, двигаться в необходимую строну, преследовать цель и прочее
class MovementComponent {
public:
  struct CreateInfo {
//     EntityAI* parent;
//     InputComponent* input;
//     PhysicsComponent* physics;
//     TransformComponent* trans;
    yacs::entity* ent;
    Type pathFindType;
  };
  MovementComponent(const CreateInfo &info);
  
  path_state findPath(const EntityAI* target); // по идее мы можем указать любой объект на уровне
  path_travel_state travelPath();
  bool pathExist() const;
  void move(const simd::vec4 &dir); // указываем направление
  
  // нужно ли делать версию с конкретной позицией? как сделать патруль? 
  // можно создать несколько псевдо объектов, и двигаться от одного к другому
  void pursue(const EntityAI* target); // скорее всего просто двигаемся к цели
  void flee(const EntityAI* target); // бежим от цели
  
  // какие-то еще способы движения? запрыгивание?
  // еще нужно сделать что то для дверей
private:
  struct PathData {
    RawPath* path;
    FindRequest req;
    // фуннел? мне нужно как то сделать так чтобы челик мог немного отойти от краев когда идет по пути
    // фуннел будем наверное все же хранить в RawPath, просто отдельно
  };
  
  yacs::entity* ent;
  
//   EntityAI* parent;
//   InputComponent* input;
//   PhysicsComponent* physics;
//   TransformComponent* trans;
//   vertex_t* vertex; // вертексов здесь быть не должно
//   vertex_t* lastVertex;
  
  size_t pathfindingTime;
  size_t currentPathSegment;
  
  Type pathFindType;
  PathData foundPath;
  PathData oldPath;
  
  // функции следования по пути мы поставим сюда
  simd::vec4 predictPos(const size_t &predictionTime) const;
  simd::vec4 seek(const simd::vec4 &target);
  simd::vec4 flee(const simd::vec4 &target);
  simd::vec4 followPath(const size_t &predictionTime, const RawPath* path, const size_t &currentPathSegmentIndex);
  void stayOnPath(const size_t &predictionTime, const RawPath* path);
  
  void modEntityRotation();
  
  void releaseCurrentPath();
  void releaseOldPath();
  void releasePaths();
};

#endif
