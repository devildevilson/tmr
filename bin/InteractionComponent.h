#ifndef COMBAT_COMPONENT_H
#define COMBAT_COMPONENT_H

#include "Utility.h"

#include "EntityComponentSystem.h"
#include "Type.h"
#include "Physics.h"

#include "ThreadPool.h"

#include <unordered_set>
#include <unordered_map>
#include <chrono>
#include <mutex>

// class Combat : public yacs::Component {
// public:
//   Combat();
//   ~Combat();
//   
//   void update(const size_t &time = 0) override;
//   void init(void* userData) override;
// private:
//   EventComponent* events;
//   TransformComponent* trans;
// };

// тип самого взаимодействия в принципе там очень много общих данных
// единственное что нужно знать скорее всего это как конкретно меняются характеристики
// так же может поменяться скорость и прочее прочее
// для того чтобы правильно поменять характеристики нам нужна функция составленная игроком которая что то делает с характеристиками
// здесь она и будет храниться наверное
// хотя большинство вещей потребуют очень конкретных действий, тип добавить бонус, сложить число для которых особо ненужен механизм своих функций
// + у нас есть взаимодействие "использовать" которое может двигать объект по какому то принципу
// с использованием связана одна нехорошая вещь, скорее всего придется сделать для нее бехавиор три 
// где тогда хранить вейпоинты (или аналог)? можно на этапе создания вейпоинты раскидать, 
// вообще подобные вещи кажется с помощью joint делаются, но у меня физика простая
// так или иначе нужен способ постепенного обновления положения, то есть мне наверное нужно задать скорость
// и двигаться до чего то (?), до точки (но это очень не точно), до пересечения с плоскостью, перемещаться на расстояние
// зачем мне этот класс в принципе?
class Communication {
public:
  
};

// так короч, у скиллов и атаки есть очень много похожих моментов, например, что атака что скиллы могут заспаунить какие то вещи которые вызывают эвенты у других 
// то есть есть возможность это дело засунуть в один компонент
// проблема: как создавать вещи пересечения, просто как физик компонент, или как энтити
// для проджект тайлов лучше всего конечно создавать отдельные энтити
// а для просто атаки лучше физик компонент использовать
// в общем нужно сделать какие виртуальные функции

// короч у нас может быть 4 типа ситуации: таргет, луч, атака с физ объектом, проджектайл
// причем все 4 ситуации валидны как для скиллов так и для просто атаки
// после получения эвента мы должны каким то образом взять интересующий нас тип и сделать интересующее нас действие
// то есть нам для типа еще потребуется какой то контейнер

// нужно наверное выделить еще один тип - это аура
// т.к. он работает немножко по другому, нам надо запустить эвент входа в ауру и выхода из нее
// либо пускать эвенты каждый кадр (каждое некоторое время), вообще идея неплохая, с другой стороны
// вызовы эвентов каждый кадр, конечно, ударит по производительности, лучший способ это конечно эвенты на вход и выход
// эвенты на вход и выход все наверное

struct PhysUserData;

class Interaction {
public:
  static void setContainers(Container<Transform>* transforms, Container<simd::mat4>* matrices, Container<RotationData>* rotationDatas);
  
  enum class type {
    target,
    ray,
    physics,
    projectile,
    aura
  };
  
  Interaction(const type &t, const Type &eventType, void* userData) : t(t), eventType(eventType), userData(userData), finished(false) {}
  virtual ~Interaction() {}
  
  struct NewData {
    simd::vec4 pos;
    simd::vec4 dir;
  };
  virtual void update_data(const NewData &data) = 0;
  virtual void update(const size_t &time) = 0;
  virtual void cancel() = 0;
  
  // в большинстве случаев, объектов для которых нужно вызвать эвент будет 1
  // и вообще можно сделать примерно также как мы делали для EntityAI то есть 
  // выдавать указатель на PhysicsIndexContainer (спорно) пока не nullptr
  // вообще можно выдвавать сразу юзер дату, но может ли нам что нибудь еще пригодиться?
  virtual PhysUserData* get_next() = 0;
  
  enum type type() const { return t; }
  Type event_type() const { return eventType; }
  void* user_data() const { return userData; }
  bool isFinished() const { return finished; }
private:
  enum type t;
  Type eventType;
  void* userData;
  
protected:
  bool finished;
  
  // нам тут нужен доступ к данным о матрицах, поворотах и позиции объекта
  static Container<Transform>* transforms;
  static Container<simd::mat4>* matrices;
  static Container<RotationData>* rotationDatas;
};

// общее у них: время взаимодействия, промежуточный стейт, направление начала, направление конца, функция действия при пересечении/взаимодействии
// вообще у нас длительное по времени взаимодействие только у PhysicsInteraction, остальные скорее всего удалятся после одного раза
// это классы помошники, само взаимодействие у нас будет отдельно
class TargetInteraction : public Interaction {
public:
  struct CreateInfo {
    
  };
  TargetInteraction(const CreateInfo &info);
  ~TargetInteraction();
  
  void update_data(const NewData &data) override;
  void update(const size_t &time) override;
  void cancel() override;
  
  PhysUserData* get_next() override;
private:
  // тут по идее только один объект
  // и все
  // как его сюда добавить?
};

class RayInteraction : public Interaction {
public:
  struct CreateInfo {
    float pos[4];
    float dir[4];
    size_t delayTime;
  };
  RayInteraction(const CreateInfo &info);
  ~RayInteraction();
  
  void update_data(const NewData &data) override;
  void update(const size_t &time) override;
  void cancel() override;
  
  PhysUserData* get_next() override;
private:
  // создаем луч (или несколько лучей? несколько лучей полезно создавать когда мы стреляем из автомата)
  // (и мы тогда интерполируем и несколько лучей создаем по движению камеры, чтобы правильно все сделать, лучи нам нужно создавать в update_data)
  // пока что один луч? скорее всего
  // максимальная дистанция луча еще должна быть
  uint32_t rayIndex;
  float pos[4];
  float dir[4];
  
  size_t delayTime;
};

class PhysicsInteraction : public Interaction {
public:
  enum PhysicsInteractionType {
    INTERACTION_TYPE_SLASHING,
    INTERACTION_TYPE_STABBING, // пока не сделал еще
    INTERACTION_TYPE_AURA // переместится в отдельный класс
  };
  
  struct CreateInfo {
    PhysicsInteractionType type;
    size_t delayTime;
    size_t attackTime;
    size_t tickTime;
    uint32_t tickCount;
    uint32_t ticklessObjectsType;
    float thickness;
    float attackAngle;
    float distance;
    float pos[4];
    float dir[4];
    float plane[4];
    uint32_t transformIndex; // ?
    uint32_t matrixIndex;
    uint32_t rotationIndex;
    
    uint32_t sphereCollisionGroup;
    uint32_t sphereCollisionFilter;
    
    Type eventType;
    void* userData;
  };
  PhysicsInteraction(const CreateInfo &info);
  ~PhysicsInteraction();
  
  void update_data(const NewData &data) override;
  void update(const size_t &time) override;
  void cancel() override;
  
  PhysUserData* get_next() override;
  
  // нам отсюда нужно будет получить данные, чтобы потом вывести точку пересечения
private:
  struct ObjData {
    uint32_t count;
    std::chrono::steady_clock::time_point point;
  };
  
  PhysicsIndexContainer container;
  
  // как правильно отправить данные о взимодействии другому объекту?
  // делэй? с делэем можно тогда создавать сферу позже, но для этого нужна синхронизация
  // мне нужно у сферы еще менять положение время от времени
  size_t delay;
  size_t attackTime;
  size_t lastTime;
  size_t currentTime;
  size_t tickTime;
  
  uint32_t transformIndex;
  uint32_t tickCount;
  uint32_t ticklessObjectsType;
  float thickness;
  float attackAngle;
  float distance;
  PhysicsInteractionType interactionType;
  
  float pos[4];
  float dir[4];
  
  float lastPos[4];
  float lastDir[4];
  float plane[4]; // нужно умножать на матрицу поворота (по идее составляется из вектора вверх объекта, хотя может из вектора вперед)
  
  size_t state;
  std::unordered_map<uint32_t, ObjData> objects;
};

class ProjectileInteraction : public Interaction {
public:
  struct CreateInfo {
    
  };
  ProjectileInteraction();
  ~ProjectileInteraction();
  
  void update_data(const NewData &data) override;
  void update(const size_t &time) override;
  void cancel() override;
  
  PhysUserData* get_next() override;
private:
  // пока что самый непонятный класс
  // get_next скорее всего всегда будет возвращать нулл
  // создаваться проджектайлы наверное будут в update
  // создаваться проджект тайлы должны из какой-нибудь фабрики
  // фабрики будут создаваться в лоадерах и описываться в json я так понимаю
};

class AuraInteraction : public Interaction {
public:
  
private:
  // в случе с аурой у нас будет вход в ауру и выход из нее, как сделать?
};

class EventComponent;
class TransformComponent;
class InteractionSystem;

// у каждого компонента рекция на эвент атака будет примерно одинаковая, как бы мне сохранить память?
// я так полагаю что это как раз тот случай когда либо удобно, либо мало памяти жрет (возможно)

class InteractionComponent : public yacs::Component {
public:
  struct CreateInfo {
    InteractionSystem* system;
  };
  InteractionComponent(const CreateInfo &info);
  ~InteractionComponent();
  
  void update(const size_t &time = 0) override;
  void init(void* userData) override;
  
  size_t & index();
private:
  void deleteInteraction(Interaction* inter);
  
  size_t systemIndex;
  
  InteractionSystem* system;
  EventComponent* events;
  TransformComponent* trans;
  
  // впринципе не так много всего что мы можем передать в качестве юзер_даты: характеристики, список эффектов, от кого пришло
  // я тут подумал неплохо было бы характеристики и эффекты вместе где нибудь держать
  // что такое эффект? перманентное или временное изменение характеристики + возможность накладывать эффекты на других
  // короче нужно сделать обработку уникальных пар объектов для интерактион компонента
  // в этом случае мы можем как угодно планировать свои взаимодействия в паре
  // + не придется вызывать save вариант функции
  
  std::vector<Interaction*> interactions;
  
  static std::mutex creationMutex;
  static MemoryPool<PhysicsInteraction, sizeof(PhysicsInteraction)*20> physInt;
};

// система для InteractionComponent и многопоточность
class InteractionSystem : public Engine {
public:
  struct CreateInfo {
    dt::thread_pool* pool;
  };
  InteractionSystem(const CreateInfo &info);
  ~InteractionSystem();
  
  void update(const uint64_t &time) override;
  
  // этого здесь не будет
//   struct PairData {
//     std::pair<uint32_t, uint32_t> pair;
//     Interaction* inter;
//     InteractionComponent* comp;
//     PhysUserData* secondObj;
//     uint32_t batchID;
//     // характеристики? буду брать из компонента наверное
//   };
//   void addInteractionPair(const PairData &data);
  
  void addInteractionComponent(InteractionComponent* comp);
  void removeInteractionComponent(InteractionComponent* comp);
private:
//   struct UniquePairData {
//     uint32_t start;
//     uint32_t count;
//   };
//   void uniquePairs();
  
  dt::thread_pool* pool;
  
//   std::mutex mutex;
  
  std::vector<InteractionComponent*> components;
//   std::vector<PairData> pairs;
//   std::vector<UniquePairData> uniqueData;
};

#endif
