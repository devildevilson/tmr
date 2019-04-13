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
  std::function<bool(const vertex_t*, const vertex_t*, const edge_t*)> predicate;
  
  //std::vector<std::pair<vertex_t*, vertex_t*>> queue;
//   std::atomic<size_t> queueSize;
//   std::pair<vertex_t*, vertex_t*>* array;
  std::mutex mutex;
  
  std::unordered_set<std::pair<const vertex_t*, const vertex_t*>> queue;
  
  std::unordered_map<std::pair<const vertex_t*, const vertex_t*>, std::pair<const vertex_t*, const edge_t*>> history;
  
  std::unordered_map<std::pair<const vertex_t*, const vertex_t*>, RawPath*> paths;
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
  void registerPathType(const Type &type, const std::function<bool(const vertex_t*, const vertex_t*, const edge_t*)> &predicate) override;
  void queueRequest(const Type &type, vertex_t* start, vertex_t* end) override;
  
  void update() override;
  
  // может вернуть nullptr, если путь не найден
  RawPath* rawPathPtr(const Type &type, const vertex_t* start, const vertex_t* goal) const;
  
  // в принципе и так и так мне нужно проверить историю или найти путь
private:
  dt::thread_pool* pool;
  
  Graph* graph;
  AStarSearch* search;
  
  // нужна еще история
  std::unordered_map<Type, PathFindingType> types;
  
  std::mutex poolMutex;
  MemoryPool<RawPath, sizeof(RawPath)*50> pathsPool;
};

#endif
