#ifndef AI_SYSTEM_H
#define AI_SYSTEM_H

#include "Engine.h"
#include "Utility.h"
#include "Type.h"
#include "PathFindingPhase.h"
#include "MemoryPool.h"
#include "EntityAI.h"
#include "AIComponent.h"

#include "ThreadPool.h"

#include "StageContainer.h"

#include <atomic>
#include <unordered_map>

class AIComponent;
class AIBasicComponent;
class AIGroup;
class Graph;
namespace tb {
  class BehaviorTree;
}

// я уже достаточно долго думаю на счет ии
// все же я решил что просто переключение состояний недостаточно для меня
// желательно брать именно компоненты и отталкиваясь от них взаимодействовать
// компоненты у энтити нужно немного переделать
// + скорее всего большинство данных должно приходить в формате рид онли
// причем компоненты это: комбат, скилл, аниматион, инвентарь, аттрибуты, может чет еще
// добавить их в отдельный map? так будет удобнее всего
// как сделать блокировку движения? (хотя ее нет и сейчас)
// ну стейт контроллер никуда не девается, нужно только его резетить наверное
// получать компоненты без строгого типа сложнее, так вполне могут получиться проблемы при мультитрединге
// нужно тогда разделить константный ии и обычный
// но и это не гарантирует 100% многопоточности, хотя с другой стороны почему нет?
// здесь вообще не должно быть изменений где-то кроме очень определенных мест
// + изменения должны идти строго от одного потока, то есть не обрабатывать EntityAI больше одного раза за кадр (чтоб обновление не попало в другой поток)
// думаю, что все вышесказанное должно в итоге превратиться в серьезный аи компонент

// можно выделить компонент с группами, для того чтобы все групповые вещи там отрабатывать (движение в группе, построение и прочее)

// так короч, с компонентами трабла основная заключается в том что мне относительно неудобно становится ими пользоваться в cpp коде
// есть вариант сделать ии на основе эвентов, то есть вместо простого переключения состояний использовать эвенты
// эвенты локальны по отношению к энтити, компоненты энтити подключаются к эвентам и при запуске эвента, как то на него реагируют
// например, запускаем эвент атака, на эвент подписаны комбат компонент, анимационный компонент, компонент звука, в нем передаем дополнительные данные
// такие как направление атаки (да и все наверное)
// вообще то очень похоже на то что меня было

// когда вызывать связанные функции? самым легким вариантом будет вызвать сразу на месте какую-нибудь легкую функцию
// другое дело что это скорее всего не очень хорошо с точки зрения кэша
// в ином случае запара наступает с хранением значений между вызовами
// лучше первый вариант

// по хорошему нам нужно вызвать резет перед вызовом очередного эвента
// стейт контроллер должен сохранить свой функционал в том плане что он должен как то сообщать текущее состояние энтити
// или эту функцию может взять на себя эвент компонент
// с эвентами тоже есть проблемы

// короче лучше всего эвенты!
// с эвентами сложнее предугадать конец эвента (например конец атаки)
// предугадывать то может и прям так не нужно
// но вот как указать какое сейчас состояние?
// причем нужно отделить именно состояние от любого другого эвента
// этим вообще может заняться стейт контроллер, он может подписаться на пачку эвентов
// и среди эвентов выбирать состояние, другое дело что мне может потребоваться почистить 
// компоненты после состояния (например вернуть атаку в исходное положение)
// оправлять эвент резет? когда? в общем резет самый более-менее нормальный вариант
// либо подписаться всеми компонентами на все эвенты с состояниями

// эвентами может быть что угодно, например moveToTarget, в этом случае мы пытаемся найти путь до цели
// и дойти до нее
// какие здесь есть подводные камни? передача данных, хоть это и не сложно, но нужно быть аккуратным
// так ли необходимо указывать эвенты состояний? 
// нужно указать чтобы ответить на вопрос какое текущее состояние
// резет вполне может перезапустить работу дерева поведения
// состояние по умочанию? нужно ли что то такое?
// как реагировать на то если эвенты будут приходить каждый кадр?
// если эвенты одинаковые, то никак наверное, но все равно наверное придется обойти слушателей
// через эвент можно передавать актуальную информацию, но лучше так не делать

// так же мы должны иметь возможность получить какой-нибудь фидбек
// то есть например, у нас может быть цель, но мы скорее всего не сможем до нее добраться
// но проверим мы это только позже, как быть?
// короч это убивает практически все что я напридумывал
// здесь по идее нужно возвращать некий объект, который сообщит о состоянии эвента
// а может и не убивает, ответ может быть в форме фаил, ран, сусек,
// где фаил - означает что действие не может быть выполнено, ран - выясняем возможно ли действие,
// сусек - мы выполнили или сможем успешно выполнить действие
// может ли мне потребоваться какая то дополнительная информация о том почему мы не выполнили действие?
// вообще в случае с магией действительно может потребоваться, но всю информацию мы можем взять из аттрибутов
// иные случаи? щас сходу я не могу толком ничего придумать, 
// думаю что в единичных случаях можно придумать какой-нибудь костыль

// продумать на счет взятия данных из компонентов
// что если у нас между двумя вызовами pushEvent есть очень большой разрыв?
// по идее это не проблема для некоторых действий, а остальные мы можем проверить использовав номер фрейма
// система сенсоров (на земле ли?, с кем столкнулись, кто атаковал и прочее) где должно быть?
// эти данные по идее нам нужны каждый кадр, следовательно должны быть доступны как можно ближе
// то есть какое то хранилище, заполняется по ходу дела, например с помощью эвентов (онКоллиде)
// вызывать он коллайд каждый раз - это тупо, лучше пусть просто держит в себе указатели на массивы

// то есть использование должно быть примерно таким:
// нод: стейт = пушЭвент(мувТоТаргет), ретерн стейт

// в общем как то так

// нужно ли мне заполнять данными сам EntityAI, или можно брать из уже существующих источников?
// при заполнении - локальность данных, но нужно предобновить каждый объект
// из существующих - легче, но медленее

struct MinEntityData {
  glm::vec4 pos;
  glm::vec4 velocity;
  uint32_t vertexId;
  // тут наверное еще добавиться предыдущий vertexId
  // какие еще данные?
  // радиус, для боундинг сферы
};

class Blackboard {
public:
  Blackboard();
  ~Blackboard();
  
  void setData(const Type &name, const size_t &data);
  
  std::atomic<size_t> & data(const Type &name);
  const std::atomic<size_t> & data(const Type &name) const;
private:
  std::unordered_map<Type, std::atomic<size_t>> board;
};

// если в компоненте уже будет указатель на дерево, то зачем мне это?
// это внутри компонента
class BehaviourTreePhase {
public:
  virtual ~BehaviourTreePhase() {}
  
  virtual void update() = 0;
  
  // что у нас здесь?
  // у нас тут список AIComponent, в которых необходимо обойти деревья
  // + здесь они будут храниться, создаваться и удаляться
};

class BehaviourResolvePhase {
public:
  virtual ~BehaviourResolvePhase() {}
  
  virtual void update() = 0;
  
  // здесь я планировал делать всякие дополнительные вещи требующиеся для ии, например, фуннел алгоритм
  // вообще здесь планировался более обширный функционал
  // но в связи с переделками я полагаю мне может не потребоваться этот класс вовсе
};

class AISystem : public Engine {
public:
  virtual ~AISystem() {}
  
  // из чего у нас состоит ии?
  // у нас есть несколько этапов которые проходит энтити
  // они располагаются всегда в одном порядке, но не факт что энтити проходит эти этапы в каждом кадре
  // перво наперво это бехавиор три, в нем энтити принимает решение что делать дальше
  // затем идет поиск пути, тут мы пытаемся возпользоваться А* (когда именно нужно вызывать поиск пути?)
  // и теперь у нас идут вычисления непосредственно поведения + каких то специальных случаев (атака, использование скилла и проч)
  // все пункты поддаются распараллеливанию, причем вычисление поведения можно засунуть даже на гпу (остальное не выйдет)
  
  // структура данных? это прежде всего граф, его нужно переписать, сейчас он выглядит плохо
  // по графу мы должны: искать путь, перебирать соседние вершины, искать всех энтити в округе
  // минимальный набор данных который нам нужен для энтити: это позиция, скорость, индексы где находится
  
  // для бехавиор три также нужен блэкбоард, то есть контейнер в котором мы должны хранить некие данные
  // скорее всего это будут только целочисленные значения, так как я хочу распараллелить деревья
  
  // на этапе деревьев у нас должен быть хорошо задизайнен энтити аи (скорее всего это компонент)
  // можно ли обойтись без виртуального наследования? ну то есть для всех энтити максимально общий компонент
  
  // возможно будет полезным представить игровую карту в виде плоскости с нормалью по гравитации
  // не уверен как это мне поможет, но я надеюсь что так я смогу быстро определить энтити к какой нибудь поверхности
  // и возможно так у меня получится сделать тени примерно так как они были в думе 95 года + еще кое-какие полезные фишки
  // тогда что делать когда у меня изменится вектор гравитации? по идее пересобрать все (но это скрее всего будет капец долго)
  // с другой стороны, если энити будет всегда принадлежать хоть какой то вершине, то данные о затенении можно взять из вершины
  // здесь только одна проблема: если челик в полете то определить к чему он там принадлежит гораздо сложнее
  
  // так же скорее всего мне потребуется сделать более легкое выталкивание объектов из друг друга
  // то есть чтоб объекты чуть чуть могли входить в друг друга при определенных условиях
  // хотя это может и не потребоваться
  
  // апдейт будет состоять из строго определенных шагов (возможно добавится что то еще)
  // 1. обновление данных в TargetAI (позиция, скорость и прочее)
  // 2. обход дерева поведения (во время него составляются списки дальнейших задач)
  // 3. на 2-ом шаге для каждого объекта определяется нужен ли ему поиск пути
  //    на этом мы вычисляем путь (способы разные, например просто тупо вперед)
  // 4. на 2-ом шаге выясняем что сейчас нам нужно сделать (скилл, атака, почекать видимость)
  //    добавляем это дело в очереди, которые решим далее по ходу обновления фрейма
  // 5. после 3 нам еще нужно будет вычилсить фуннел алгоритм
  // 6. после 4 нам ужно будет раздать комманды остальным системам и компонентам
  //    теперь раздавать команды мы будем на 2 шаге, с помощью эвентов
  // на этом все, далее идет работа других систем (например, физики)
  //void update(const uint64_t &time) override;
  
  // тут еще нужны стандарные методы создания деревьев, получения блэкбоарда
  // инициализация и прочее
  // неплохо было бы чтобы и этот класс был интерфейсом для разных реализаций
  
  virtual PathFindingPhase* pathfindingSystem() const = 0;
  
  virtual void registerComponent(AIComponent* component) = 0;
  virtual void registerBasicComponent(AIBasicComponent* component) = 0;
  virtual void removeComponent(AIComponent* component) = 0;
  virtual void removeBasicComponent(AIBasicComponent* component) = 0;
  
  virtual size_t getUpdateDelta() const = 0;
  
  virtual void setBehaviourTreePointer(const Type &name, tb::BehaviorTree* tree) = 0;
  virtual tb::BehaviorTree* getBehaviourTreePointer(const Type &name) const = 0;
};


// нам бы тут не помешал бы еще выход в граф
// да и вообще нужно посильнее граф задействовать
class CPUAISystem : public AISystem {
public:
  struct CreateInfo {
    dt::thread_pool* pool;
//     PathFindingPhase* pathfinding;
    size_t updateDelta;
    size_t utilitySystemsSize;
  };
  CPUAISystem(const CreateInfo &info);
  ~CPUAISystem();
  
  void update(const uint64_t &time) override;
  
  Blackboard & blackboard();
  const Blackboard & blackboard() const;
  
  PathFindingPhase* pathfindingSystem() const override;
  
  void registerComponent(AIComponent* component) override;
  void registerBasicComponent(AIBasicComponent* component) override;
  void removeComponent(AIComponent* component) override;
  void removeBasicComponent(AIBasicComponent* component) override;
  
  size_t getUpdateDelta() const override;
  
  void setBehaviourTreePointer(const Type &name, tb::BehaviorTree* tree) override;
  tb::BehaviorTree* getBehaviourTreePointer(const Type &name) const override;
  
  template <typename T, typename... Args>
  T* createPathfindingSystem(Args&&... args) {
    T* ptr = container.addStage<T>(std::forward<Args>(args)...);
    pathfinding = ptr;
    return ptr;
  }
  
  template <typename T, typename... Args>
  T* createGraph(Args&&... args) {
    T* ptr = container.addStage<T>(std::forward<Args>(args)...);
    graph = ptr;
    return ptr;
  }
private:
  size_t updateDelta;
  size_t accumulator;
  
  dt::thread_pool* pool;
  Blackboard board;
  
  std::vector<AIComponent*> aiEntities; // здесь мы обновляем дерево поведения
  std::vector<AIBasicComponent*> aiObjects; // с этим аи может взаимодействовать
  std::vector<AIComponent*> groupAI;
  std::vector<AIGroup*> groups; // с группами не все очевидно
  
  std::unordered_map<Type, tb::BehaviorTree*> treesPtr;
  
  // + здесь же фаза поиска пути, которая наступает после основного обновления
  PathFindingPhase* pathfinding;
  Graph* graph;
  
  StageContainer container;
  
  MemoryPool<AIGroup, sizeof(AIGroup)*20> groupPool;
  MemoryPool<AIComponent, sizeof(AIComponent)*20> groupAIPool; // поторопился
};

// нам нужно еще сделать граф и А* алгоритм

#endif
