#ifndef CPU_PATH_FINDING_PHASE_PARALLEL
#define CPU_PATH_FINDING_PHASE_PARALLEL

#include "PathFindingPhase.h"

#include "ThreadPool.h"
#include "MemoryPool.h"

class vertex_t;
class edge_t;
class Graph;
class AStarSearch;

// #include "Graph.h"
// #include "AStarSearch.h"

#include <atomic>
#include <unordered_map>
#include <unordered_set>

// тут нам нужно продумать какие-то типы поиска пути
// типы - это прежде всего разные функции
// добавление в очередь происходит по этим типам
// в типах мы ищем путь для уникальных пар вершин

namespace std {
  template<> 
  struct hash<std::pair<const vertex_t*, const vertex_t*>> {
    size_t operator()(const std::pair<const vertex_t*, const vertex_t*> &element) const;
  };
}

struct PathFindingType {
  float offset;
  std::function<bool(const vertex_t*, const vertex_t*, const edge_t*)> predicate;
  std::mutex mutex;
  //std::vector<RawPath> foundPaths;
  
  std::unordered_map<std::pair<const vertex_t*, const vertex_t*>, PathFindingReturn> queue;
};

class CPUPathFindingPhaseParallel : public PathFindingPhase {
public:
  struct CreateInfo {
    dt::thread_pool* pool;
    Graph* graph;
    std::function<float(const vertex_t*, const vertex_t*, const edge_t*)> neighborCost;
    std::function<float(const vertex_t*, const vertex_t*)> goalCost;
  };
  CPUPathFindingPhaseParallel(const CreateInfo &info);
  ~CPUPathFindingPhaseParallel();
  
  // создадим тип
  // добавим запрос на путь
  void registerPathType(const Type &type, const std::function<bool(const vertex_t*, const vertex_t*, const edge_t*)> &predicate, const float &offset) override;
  void queueRequest(const FindRequest &request) override;
  
  void update() override;
  
  PathFindingReturn getPath(const FindRequest &req) override;
  void releasePath(const FindRequest &req) override;
  
  // может вернуть nullptr, если путь не найден
//   RawPath* rawPathPtr(const Type &type, const vertex_t* start, const vertex_t* goal) const;
  
//   RawPath findPath(const Type &type, const vertex_t* start, const vertex_t* goal);
  
  // в принципе и так и так мне нужно проверить историю или найти путь
private:
  dt::thread_pool* pool;
  
  Graph* graph;
  AStarSearch* search;
  std::vector<AStarSearch*> stack;
  std::mutex stackMutex;
  
//   std::mutex requestMutex;
//   std::atomic<size_t> requestsCount;
//   std::vector<FindRequest> requests;
  
  // нужна еще история (не нужна)
  std::unordered_map<Type, PathFindingType> types;
  
//   std::unordered_map<size_t, RawPath*> requestedPaths;
  
  std::mutex poolMutex;
  MemoryPool<RawPath, sizeof(RawPath)*50> pathsPool;
  
  void computeFunnel(RawPath* path, const float &offset);
  void correctCornerPoints(const simd::vec4 &dir, const float &width, const float &offset, simd::vec4 &left, simd::vec4 &right);
  
  static size_t newUniqueId;
};

// КАК УДАЛЯТЬ СТАРЫЕ ВЕЩИ?
// если подойти с другой стороны то может вызывать не здесь?

// нужно вызывать поиск пути отдельно от передвижения
// поиск пути СКОРЕЕ ВСЕГО будет происходить в момент вызова
// в конце кадра поиск пути раскидаем в историю, когда чистить?
// чистить скорее всего будем по времени, то каждых например 30 секунд

// путь видимо придется много копировать

// короче чет опять очень медленное продвижение
// но главное что я понял, что сделать то как я раньше хотел сложно
// и наверное нужно искать почти сразу как возникает необходимость
// теперь осталось понять как все устроить
// тип когда мы обращаемся к поиску, мы отправляемся в метод findPath
// мы должны запомнить где то уже найденный путь, и мне хочется чтобы мы запоминали все же указатели

// мне нужно сформировать массив
// блен использовать голый new не хочется
// но с другой стороны в векторе он тоже используется

// надо наверное с другой стороны зайти
// зашел с другой стороны, но ясности не прибавило ничего
// короче нужно совместить текущий способ и предыдущий

// У МЕНЯ НЕ РАБОТАЕТ ИСТОРИЯ В СЛУЧАЕ КОГДА БУДЕТ МЕНЯТЬСЯ ГРАВИТАЦИЯ!
// (в смысле что путь иногда может стать невалидным при изменении гравитации)
// но все равно нужно научиться распределять нагрузку на поиск пути

// у меня может быть 3 состояние: путь найден, пути нет, еще не приступали/не закончен поиск

#endif
