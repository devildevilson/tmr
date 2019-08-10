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
class EventComponent;
class TransformComponent;
class InteractionSystem;
class EntityAI;

class Interaction {
public:
  static void setContainers(Container<Transform>* transforms, Container<simd::mat4>* matrices, Container<RotationData>* rotationDatas);
  
  enum class type {
    target,
    ray,
    physics,
    projectile,
    aura,
    collision
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
    size_t delayTime;
    yacs::Entity* entity;
    
    Type event;
    void* userData;
  };
  TargetInteraction(const CreateInfo &info);
  ~TargetInteraction();
  
  void update_data(const NewData &data) override;
  void update(const size_t &time) override;
  void cancel() override;
  
  PhysUserData* get_next() override;
private:
  // тут по идее только один объект
  // и все, как его сюда добавить? просто передать при создании
  uint32_t index;
  size_t delayTime;
  size_t currentTime;
  yacs::Entity* entity;
};

class RayInteraction : public Interaction {
public:
  struct CreateInfo {
    float pos[4];
    float dir[4];
    float maxDist;
    float minDist;
    uint32_t ignoreObj;
    uint32_t filter;
    size_t delayTime;
    
    Type event;
    void* userData;
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
  // количество лучей?
  uint32_t rayIndex;
  float lastPos[4];
  float lastDir[4];
  float pos[4];
  float dir[4];
  float maxDist;
  float minDist;
  uint32_t ignoreObj;
  uint32_t filter;
  
  size_t currentTime;
  size_t delayTime;
  
  size_t state;
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

// скорее всего существует всегда
// функция вызываемая при коллизии, должна определяться для триггера
// можно хранить ее здесь, запускать в эвенте коллизия, например
// тогда в этой функции мы никак не должны изменять триггер объект
// у нас может быть две ситуации, либо вызывать каждый кадр триггер функцию 
// либо вызывать только при начале колизии и при конце (как я планирую сделать в ауре)
class CollisionInteraction : public Interaction {
public:
  struct CreateInfo {
    
  };
  CollisionInteraction();
  ~CollisionInteraction();
  
  void update_data(const NewData &data) override;
  void update(const size_t &time) override;
  void cancel() override;
  
  PhysUserData* get_next() override;
  
private:
  uint32_t physicsObjectID;
  
  std::function<void(const yacs::Entity*, yacs::Entity*)> func;
};

template <typename T, size_t N>
struct MultithreadingMemoryPool {
  std::mutex creationMutex;
  MemoryPool<T, sizeof(T)*N> pool;
};

// у каждого компонента рекция на эвент атака будет примерно одинаковая, как бы мне сохранить память?
// я так полагаю что это как раз тот случай когда либо удобно, либо мало памяти жрет (возможно)

class InteractionComponent : public yacs::Component {
public:
  CLASS_TYPE_DECLARE
  
  struct CreateInfo {
    InteractionSystem* system;
  };
  InteractionComponent(const CreateInfo &info);
  ~InteractionComponent();
  
  void update(const size_t &time = 0) override;
  void init(void* userData) override;
  
  // этот способ не позволяет использовать например разное оружее (то есть использовать разные данные на один эвент)
  // для оружия в этом случае самым логичным решением будет создать специальный компонент который будет разные ситуации в одном эвенте обрабатывать
  void create(const Type &creationEvent, const TargetInteraction::CreateInfo &info);  // юзер дату мы тоже сюда складываем
  void create(const Type &creationEvent, const RayInteraction::CreateInfo &info);     // так что имеет смысл ее хранить в другом месте
  void create(const Type &creationEvent, const PhysicsInteraction::CreateInfo &info); // 
  
  // нужно по идее еще проверить наличие
  // удалять? не думаю, возможно потребуется найти, а потом провзаимодействовать с отдельным элементом
  // но не факт
  bool has(const enum Interaction::type &type, const Type &event);
  
  void create(const TargetInteraction::CreateInfo &info);
  void create(const RayInteraction::CreateInfo &info);
  void create(const PhysicsInteraction::CreateInfo &info);
  
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
  
  // я тут подумал, что в будущем возможно эффективно выделять память для почти всех массивов
  // для этого нужен очень крупный пул, в котором каждая страница (?) памяти будет выделять с небольшим количеством метаинформации
  // метаинформация будет содержать данные о незанятых байтах (или незанятом кусочке памяти объекта), в этом случае обход свободного места
  // будет достаточно быстр, можно еще сделать дефрагментацию, но для этого нужно будет хранить указатели на все массивы использующие пул
  // об этом потом
  std::vector<Interaction*> interactions;
  
  // мне нужно еще заблокировать/разблокировать возможность некоторых интеракций, например, атака происходит только тогда 
  // когда нет блока, причем блок может быть и от простой анимации (перезарядка), как это предусмотреть?
  // должна быть какая-то обратная связь, можно конечно организовать с помощью тех же эвентов
  // но на мой взгляд это черевато проблемами со скоростью? с другой стороны, нет никакого общего способа решить данную беду
  // наверное буду использовать тогда эвенты
  // вообще у меня будет ряд действий которые потребуют вызова эвента при окончании, нужно просто ввести какой то способ обобщить все эти вещи
  
  static MultithreadingMemoryPool<TargetInteraction, 20> targetInt;
  static MultithreadingMemoryPool<RayInteraction, 20> rayInt;
  static MultithreadingMemoryPool<PhysicsInteraction, 100> physInt;
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

// компонент с оружием как он должен выглядеть?
// мы должны контролировать что создавать, характеристики, оружее котрое сейчас используем

#endif
