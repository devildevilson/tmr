#include "CPUPathFindingPhaseParallel.h"

CPUPathFindingPhaseParallel::CPUPathFindingPhaseParallel(const CreateInfo &info) : pool(info.pool) {
  search = new AStarSearch[pool->size()];
  
  for (size_t i = 0; i < pool->size(); ++i) {
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
  
}

void CPUPathFindingPhaseParallel::update() {
  
}

// может вернуть nullptr, если путь не найден
RawPath* CPUPathFindingPhaseParallel::rawPathPtr(const vertex_t* start, const vertex_t* goal) const {
  
}
