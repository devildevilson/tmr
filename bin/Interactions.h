#ifndef INTERACTIONS_H
#define INTERACTIONS_H

#include "Interaction.h"
#include "Physics.h"
#include <unordered_map>
#include <unordered_set>

namespace yacs {
  class entity;
}

class Effect;
class TransformComponent;
class PhysicsComponent;

void interaction(const Type &type, const yacs::entity* ent1, yacs::entity* ent2, const Interaction* inter);

class TargetInteraction : public Interaction {
public:
  struct CreateInfo {
    size_t delayTime;
    yacs::entity* obj;
    yacs::entity* target;

    Type event;
  };
  TargetInteraction(const CreateInfo &info);
  ~TargetInteraction();

  void update(const size_t &time) override;
private:
  // тут по идее только один объект
  // и все, как его сюда добавить? просто передать при создании
//  uint32_t index;
  size_t delayTime;
  size_t currentTime;
  yacs::entity* obj;
  yacs::entity* target;
};

class RayInteraction : public Interaction {
public:
  enum class type {
    first_min,
    first_max,
    any
  };

  struct CreateInfo {
    enum type interaction_type;

//    float pos[4];
//    float dir[4];
    float maxDist;
    float minDist;
    uint32_t ignoreObj;
    uint32_t filter;
    size_t delayTime;
    yacs::entity* obj;
    TransformComponent* transform;

    Type event;
  };
  RayInteraction(const CreateInfo &info);
  ~RayInteraction();

  void update(const size_t &time) override;
private:
  // создаем луч (или несколько лучей? несколько лучей полезно создавать когда мы стреляем из автомата)
  // (и мы тогда интерполируем и несколько лучей создаем по движению камеры, чтобы правильно все сделать, лучи нам нужно создавать в update_data)
  // количество лучей?
  enum type interaction_type;

  uint32_t rayIndex; // должно быть несколько
  float lastPos[4];
  float lastDir[4];
//  float pos[4];
//  float dir[4];
  float maxDist;
  float minDist;
  uint32_t ignoreObj;
  uint32_t filter;
  yacs::entity* obj;
  TransformComponent* transform;

  size_t currentTime;
  size_t delayTime;

  size_t state; // ???
};

class SlashingInteraction : public Interaction {
public:
  struct CreateInfo {
    size_t delayTime;
    size_t attackTime;
    size_t tickTime;
    uint32_t tickCount;
    uint32_t ticklessObjectsType;
    float thickness;
    float attackAngle;
    float distance;
    float attackSpeed;
//    float pos[4];
//    float dir[4];
    float plane[4];
//    uint32_t transformIndex; // ?
//    uint32_t matrixIndex;
//    uint32_t rotationIndex;

//    uint32_t sphereCollisionGroup;
//    uint32_t sphereCollisionFilter;

    yacs::entity* obj;
    PhysicsComponent* phys;
    TransformComponent* transform;

    Type eventType;
  };
  SlashingInteraction(const CreateInfo &info);
  ~SlashingInteraction();

  void update(const size_t &time) override;

  // нам отсюда нужно будет получить данные, чтобы потом вывести точку пересечения
private:
  struct ObjData {
    uint32_t count;
    std::chrono::steady_clock::time_point point;
  };

//  PhysicsIndexContainer container;

  size_t delay;
  size_t attackTime;
  size_t lastTime;
  size_t currentTime;
  size_t tickTime;

//  uint32_t transformIndex;
  uint32_t tickCount;
  uint32_t ticklessObjectsType;
  float thickness;
  float attackAngle;
  float distance;
  float attackSpeed; // просто добавить коэффициент? кажется в других сферах все именно так
  yacs::entity* obj;
  PhysicsComponent* phys;
  TransformComponent* transform;

  float pos[4];
  float dir[4];

  float lastPos[4];
  float lastDir[4];
  float plane[4]; // нужно умножать на матрицу поворота (по идее составляется из вектора вверх объекта, хотя может из вектора вперед)

  size_t state;
  std::unordered_map<uint32_t, ObjData> objects;
};

// все тоже самое, только с течением времени должен изменяться радиус
class StabbingInteraction : public Interaction {
public:
  struct CreateInfo {
    size_t delayTime;
    size_t attackTime;
    size_t tickTime;

    uint32_t tickCount;
    uint32_t ticklessObjectsType;
    float thickness;
    float minDistance;
    float maxDistance;
    float attackSpeed;
    float stabAngle;

    uint32_t sphereCollisionGroup;
    uint32_t sphereCollisionFilter;

    yacs::entity* obj;
    PhysicsComponent* phys;
    TransformComponent* transform;

    Type eventType;
  };
  StabbingInteraction(const CreateInfo &info);
  ~StabbingInteraction();

  void update(const size_t &time) override;
private:
  struct ObjData {
    uint32_t count;
    std::chrono::steady_clock::time_point point;
  };

//  PhysicsIndexContainer container;

  size_t delayTime;
  size_t attackTime;
  size_t currentTime;
  size_t lastTime;
  size_t tickTime;

  uint32_t tickCount;
  uint32_t ticklessObjectsType;
  float thickness;
  float minDistance;
  float maxDistance;
  float attackSpeed;
  float stabAngle; // скорее всего будет константой

  yacs::entity* obj;
  PhysicsComponent* phys;
  TransformComponent* transform;

  std::unordered_map<uint32_t, ObjData> objects;
};

// тут все несколько иначе, вход - добавляем эффект, выход убираем эффект
// ничего не иначе, просто нужно специальные эффекты давать + держать интеракцию подольше
//class AuraInteraction : public Interaction {
//public:
//  struct CreateInfo {
//    float radius;
//
//    uint32_t sphereCollisionGroup;
//    uint32_t sphereCollisionFilter;
//
//    yacs::entity* obj;
//    TransformComponent* transform;
//    Type eventType;
//  };
//  AuraInteraction(const CreateInfo &info);
//  ~AuraInteraction();
//
//  void update(const size_t &time) override;
//private:
//  // в случе с аурой у нас будет вход в ауру и выход из нее, как сделать?
//  // теперь мы все обрабатываем на месте, вход выход изи
//  // аура не обычная интеракция, нам нужно просто добавить эффект при входе и наоборот
//  PhysicsIndexContainer container;
//  yacs::entity* obj;
//  TransformComponent* transform;
//
//  const Effect* effect;
//
//  std::unordered_set<yacs::entity*> uniqueEntities;
//};

// с помощью этого можно изи смоделить ауру, нужно только у эффектов обновлять время действия
class ImpactInteraction : public Interaction {
public:
  struct CreateInfo {
//    float radius;

//    uint32_t sphereCollisionGroup;
//    uint32_t sphereCollisionFilter;

    size_t delayTime;

    yacs::entity* obj;
    PhysicsComponent* phys;
    TransformComponent* transform;
    Type eventType;
  };
  ImpactInteraction(const CreateInfo &info);
  ~ImpactInteraction();

  void update(const size_t &time) override;
private:
//  PhysicsIndexContainer container;

  size_t delayTime;
  size_t currentTime;

  yacs::entity* obj;
  PhysicsComponent* phys;
  TransformComponent* transform;

  // перебираем все объекты с которыми пересекаемся
  // наиболее простой вариант наверное
};

#endif //INTERACTIONS_H
