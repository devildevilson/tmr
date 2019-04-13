#include "CPUPathFindingPhaseParallel.h"

#include "Graph.h"
#include "AStarSearch.h"

namespace std {
  size_t hash<std::pair<const vertex_t*, const vertex_t*>>::operator()(const std::pair<const vertex_t*, const vertex_t*> &element) const {
    size_t hash = 17;
    hash = hash * 31 + std::hash<const vertex_t*>()(element.first);
    hash = hash * 31 + std::hash<const vertex_t*>()(element.second);
    return hash;
  }
}

CPUPathFindingPhaseParallel::CPUPathFindingPhaseParallel(const CreateInfo &info) : pool(info.pool), graph(info.graph) {
  search = new AStarSearch[pool->size()+1];
  
  for (size_t i = 0; i < pool->size()+1; ++i) {
    search[i].setGoalCostFunction(info.goalCost);
    search[i].setVertexCostFunction(info.neighborCost);
  }
}

CPUPathFindingPhaseParallel::~CPUPathFindingPhaseParallel() {
  delete [] search;
}

// создадим тип
// добавим запрос на путь
void CPUPathFindingPhaseParallel::registerPathType(const Type &type, const std::function<bool(const vertex_t*, const vertex_t*, const edge_t*)> &predicate) {
  auto itr = types.find(type);
  if (itr == types.end()) return;
  
  types[type].predicate = predicate;
}

void CPUPathFindingPhaseParallel::queueRequest(const Type &type, vertex_t* start, vertex_t* end) {
  auto itr = types.find(type);
  if (itr == types.end()) throw std::runtime_error("Could not find path findinf type");
  
  {
    std::unique_lock<std::mutex> lock(itr->second.mutex);
    itr->second.queue.insert(std::make_pair(start, end));
  }
}

void CPUPathFindingPhaseParallel::update() {
  static const auto pathFinding = [&] (const Type &type, const std::function<bool(const vertex_t*, const vertex_t*, const edge_t*)> &predicate, vertex_t* start, vertex_t* end) {
    // индекс потока
    size_t index;
    
    search[index].setSearch(start, end, predicate);
    
    AStarSearch::SearchState state = AStarSearch::SEARCH_STATE_SEARCHING;
    while (state != AStarSearch::SEARCH_STATE_SUCCEEDED || state != AStarSearch::SEARCH_STATE_FAILED) {
      state = search[index].step();
    }
    
    if (state == AStarSearch::SEARCH_STATE_FAILED) {
      // ошибка
      
      return;
    }
    
    auto solution = search[index].getSolution();
    
    // из солюшена мы составляем RawPath и где то его сохраняем
    RawPath* path = nullptr;
    {
      std::unique_lock<std::mutex> lock(poolMutex);
      path = pathsPool.newElement();
    }
    
    for (size_t i = 0; i < solution.size(); ++i) {
      const RawPathPiece p{
        solution[i]->vertex,
        solution[i]->edge
      };
      
      path->data().push_back(p);
    }
    
    search[index].freeSolutionNodes();
    
    {
      std::unique_lock<std::mutex> lock(types[type].mutex);
      
      types[type].paths.insert(std::make_pair(std::make_pair(start, end), path));
    }
    
    // выглядит это как то так
  };
  
  for (auto &type : types) {
    for (auto &pair : type.second.queue) {
      pool->submitnr(pathFinding, type.first, type.second.predicate, pair.first, pair.second);
    }
  }
  
  pool->compute();
  pool->wait();
}

// может вернуть nullptr, если путь не найден
RawPath* CPUPathFindingPhaseParallel::rawPathPtr(const Type &type, const vertex_t* start, const vertex_t* goal) const {
  auto itr = types.find(type);
  if (itr == types.end()) return nullptr;
  
  // тут по правилам тоже нужно мьютекс присобачить, но с учетом того что конкретно этот участок будет отрабатывать позже всех
  // он не будет пересекаться точно, когда мне заполнять историю?
  auto pathItr = itr->second.paths.find(std::make_pair(start, goal));
  if (pathItr == itr->second.paths.end()) return nullptr;
  
  return pathItr->second;
}
