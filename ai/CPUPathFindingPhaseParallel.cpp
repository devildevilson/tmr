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

RawPath::RawPath(const size_t &size) : rawPath(size) {}

std::vector<RawPathPiece> & RawPath::data() {
  return rawPath;
}

const std::vector<RawPathPiece> & RawPath::data() const {
  return rawPath;
}

const vertex_t* RawPath::start() const {
  if (rawPath.empty()) return nullptr;
  
  return rawPath.front().vertex;
}

const vertex_t* RawPath::goal() const {
  if (rawPath.empty()) return nullptr;
  
  return rawPath.back().vertex;
}

simd::vec4 closestPoint(const simd::vec4 &a, const simd::vec4 &b, const simd::vec4 &point) {
  const simd::vec4 vec = b - a;
  
  float t = simd::dot(point - a, vec) / simd::dot(vec, vec);
  t = glm::clamp(t, 0.0f, 1.0f);
  
  return a + t * vec;
}

size_t RawPath::getNearPathSegmentIndex(const simd::vec4 &pos, simd::vec4 &closest, float &dist) const {
  size_t i = 0;
  size_t lastIndex = 0;
  size_t idx = 0;
  
  for (i = 1; i < rawPath.size(); ++i) {
    if (glm::abs(simd::dot(rawPath[i].edgeDir, rawPath[i].edgeDir)) > EPSILON) break;
  }
  
  closest = closestPoint(rawPath[lastIndex].funnelPoint, rawPath[i].funnelPoint, pos);
  dist = simd::distance2(closest, pos);
  
  lastIndex = i;
  ++i;
  
  for (; i < rawPath.size()-1; ++i) {
    if (glm::abs(simd::dot(rawPath[i].edgeDir, rawPath[i].edgeDir)) <= EPSILON) continue;
    
    const simd::vec4 p = closestPoint(rawPath[lastIndex].funnelPoint, rawPath[i].funnelPoint, pos);
    float tmp = simd::distance2(p, pos);
    
    if (dist > tmp) {
      dist = tmp;
      idx = lastIndex;
      closest = p;
    }
    
    lastIndex = i;
  }
  
  return idx;
}

CPUPathFindingPhaseParallel::CPUPathFindingPhaseParallel(const CreateInfo &info) : pool(info.pool), graph(info.graph), search(nullptr), stack(info.pool->size()+1) {
  search = new AStarSearch[pool->size()+1];
  
  for (size_t i = 0; i < pool->size()+1; ++i) {
    search[i].setGoalCostFunction(info.goalCost);
    search[i].setVertexCostFunction(info.neighborCost);
  }
  
  for (size_t i = 0; i < pool->size()+1; ++i) {
    stack[i] = &search[i];
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

void CPUPathFindingPhaseParallel::queueRequest(const FindRequest &request) {
  auto itr = types.find(request.type);
  if (itr == types.end()) throw std::runtime_error("Could not find path finding type");
  
  const PathFindingReturn ret{
    path_finding_state::delayed,
    nullptr,
    1
  };
  
  {
    std::unique_lock<std::mutex> lock(itr->second.mutex);
    
    const auto pair = std::make_pair(request.start, request.end);
    auto queueItr = itr->second.queue.find(pair);
    if (queueItr == itr->second.queue.end()) {
      itr->second.queue.insert(std::make_pair(std::make_pair(request.start, request.end), ret));
    } else {
      ++queueItr->second.pointerCount;
    }
  }
  
  // не знаю что делать пока с интеллектом, он может быть разным, а ну хотя
  // сравнивать найденный путь с интеллектом
}

void CPUPathFindingPhaseParallel::update() {
  typedef std::unordered_map<std::pair<const vertex_t*, const vertex_t*>, PathFindingReturn>::iterator localIterator;
  
  static const auto pathFinding = [&] (const Type &type, const std::function<bool(const vertex_t*, const vertex_t*, const edge_t*)> &predicate, localIterator itr) {
    (void)type;
    
    AStarSearch* searchPtr = nullptr;
    {
      std::unique_lock<std::mutex> lock(stackMutex);
      
      searchPtr = stack.back();
      stack.pop_back();
    }
    
    searchPtr->setSearch(itr->first.first, itr->first.second, predicate);
    
    AStarSearch::SearchState state = AStarSearch::SEARCH_STATE_SEARCHING;
    while (state != AStarSearch::SEARCH_STATE_SUCCEEDED && 
           state != AStarSearch::SEARCH_STATE_FAILED && 
           state != AStarSearch::SEARCH_STATE_INVALID && 
           state != AStarSearch::SEARCH_STATE_NOT_INITIALISED && 
           state != AStarSearch::SEARCH_STATE_OUT_OF_MEMORY) {
      state = searchPtr->step();
    }
    
    if (state == AStarSearch::SEARCH_STATE_INVALID || 
        state == AStarSearch::SEARCH_STATE_NOT_INITIALISED || 
        state == AStarSearch::SEARCH_STATE_OUT_OF_MEMORY) {
      throw std::runtime_error("Something wrong with searching");
    }
    
    if (state == AStarSearch::SEARCH_STATE_FAILED) {
      // ошибка
      searchPtr->freeSolutionNodes();
      
      std::unique_lock<std::mutex> lock(stackMutex);
      stack.push_back(searchPtr);
      return;
    }
    
    const auto &solution = searchPtr->getSolution();
    
    // из солюшена мы составляем RawPath и где то его сохраняем
    RawPath* path = nullptr;
    {
      std::unique_lock<std::mutex> lock(poolMutex);
      path = pathsPool.newElement();
    }
    
    for (size_t i = 0; i < solution.size(); ++i) {
      const RawPathPiece p{
        solution[i]->vertex,
        solution[i]->edge,
        simd::vec4(),
        simd::vec4(),
        simd::vec4()
      };
      
      path->data().push_back(p);
    }
    
    searchPtr->freeSolutionNodes();
    {
      std::unique_lock<std::mutex> lock(stackMutex);
      stack.push_back(searchPtr);
    }
    searchPtr = nullptr;
    
    computeFunnel(path);
    
    {
//       std::unique_lock<std::mutex> lock(types[type].mutex);
      itr->second.path = path;
    }
    
    // выглядит это как то так
  };
  
  // тут нам еще нужно добавить балансирование нагрузки
  // как оно должно выглядеть? я должен часть работы спихивать на другой кадр
  // скорее всего мне нужно задать какой нибудь предел выполнения задач на этом кадре
  // то есть число после которого я перестаю запихивать команды в очередь, 
  // ситуация когда у меня каждый раз добавляется в очередь больше задач чем предел в кадре
  // ТЕОРЕТИЧЕСКИ возможна, но на практике скорее всего нет, главное чтобы число было как можно больше
  
  size_t taskCount = 0;
  for (auto &type : types) {
    for (auto itr = type.second.queue.begin(); itr != type.second.queue.end(); ++itr) {
      pool->submitnr(pathFinding, type.first, type.second.predicate, itr);
      ++taskCount;
      // макс размер
    }
  }
  
  pool->compute();
  pool->wait();
}

PathFindingReturn CPUPathFindingPhaseParallel::getPath(const FindRequest &req) {
  auto itr = types.find(req.type);
  if (itr == types.end()) throw std::runtime_error("Could not find path finding type");
  
  std::unique_lock<std::mutex> lock(itr->second.mutex);
  
  const auto pair = std::make_pair(req.start, req.end);
  auto queueItr = itr->second.queue.find(pair);
  if (queueItr != itr->second.queue.end()) return queueItr->second;
  
  return {
    path_finding_state::path_not_exist,
    nullptr,
    0
  };
}

void CPUPathFindingPhaseParallel::releasePath(const FindRequest &req) {
  auto itr = types.find(req.type);
  if (itr == types.end()) throw std::runtime_error("Could not find path finding type");
  
  std::unique_lock<std::mutex> lock(itr->second.mutex);
  
  const auto pair = std::make_pair(req.start, req.end);
  auto queueItr = itr->second.queue.find(pair);
  if (queueItr != itr->second.queue.end()) {
    --queueItr->second.pointerCount;
    
    if (queueItr->second.pointerCount == 0) {
      if (queueItr->second.path != nullptr) {
        std::unique_lock<std::mutex> lock(poolMutex);
        pathsPool.deleteElement(queueItr->second.path);
      }
      
      itr->second.queue.erase(queueItr);
    }
  }
}

void CPUPathFindingPhaseParallel::computeFunnel(RawPath* path) {
  // тут мне нужно заполнить данные всех вершин, то есть точку справа (или все же ближайшую?),
  // направление от этой точки (наверное еще длину ребра), точку в которую я направляюсь
  simd::vec4 apex, leftP, rightP, left, right;
  size_t apexIndex = 0, leftIndex = 0, rightIndex = 0;
  
  const glm::vec4 center = path->data()[0].vertex->getVertexData()->center;
  apex.loadu(&center.x);
  leftP.loadu(&center.x);
  rightP.loadu(&center.x);
  
  for (size_t i = 0; i < path->data().size(); ++i) {
    const vertex_t* vertex = path->data()[i].vertex;
    
    const glm::vec4 c = vertex->getVertexData()->center;
    const simd::vec4 center = simd::vec4(&c.x);
    const glm::vec4 n = vertex->getVertexData()->normal;
    const simd::vec4 normal = simd::vec4(&n.x);
    
    if (i+1 != path->data().size()) {
      const edge_t* edge = path->data()[i].toNextVertex;
      
      if (edge->isFake()) {
        float b = 1.0f / glm::tan(edge->getAngle());
        
        if (b > edge->getWidth()) edge->getSegment().leftRight(center, normal, left, right);
        else {
          LineSegment l(edge->getSegment().a, edge->getSegment().a + edge->getSegment().dir()*b);
          l.leftRight(center, normal, left, right);
        }
      } else {
        edge->getSegment().leftRight(center, normal, left, right);
      }
    } else {
      left = center;
      right = center;
    }
    
    // тут наверное нужно использовать нормаль гравитации (?)
    if (sideOf(apex, rightP, right, normal) <= 0.0f) {
      if (sideOf(apex, leftP, right, normal) > 0.0f) {
        rightP = right;
        rightIndex = i;
      } else {
        path->data()[i].funnelPoint = leftP;
        
        if (i+1 == path->data().size()) {
          path->data()[i].edgeDir = simd::vec4(0.0f);
        } else {
          const glm::vec4 edgeA = path->data()[i].toNextVertex->getSegment().a;
          const glm::vec4 edgeB = path->data()[i].toNextVertex->getSegment().b;
          const simd::vec4 edgeAS = simd::vec4(&edgeA.x);
          const simd::vec4 edgeBS = simd::vec4(&edgeB.x);
          const bool isA = simd::all(simd::equal(leftP, edgeAS, EPSILON));
          
          path->data()[i].edgePoint = edgeAS;
          path->data()[i].edgeDir = isA ? simd::normalize(edgeBS - edgeAS) : -simd::normalize(edgeBS - edgeAS);
        }
        
        apex = leftP;
        apexIndex = leftIndex;
        leftP = apex;
        rightP = apex;
        leftIndex = apexIndex;
        rightIndex = apexIndex;
        
        i = apexIndex;
        continue;
      }
    }
    
    if (sideOf(apex, leftP, left, normal) >= 0.0f) {
      if (sideOf(apex, rightP, left, normal) < 0.0f) {
        leftP = left;
        leftIndex = i;
      } else {
        path->data()[i].funnelPoint = rightP;
        
        if (i+1 == path->data().size()) {
          path->data()[i].edgeDir = simd::vec4(0.0f);
        } else {
          const glm::vec4 edgeA = path->data()[i].toNextVertex->getSegment().a;
          const glm::vec4 edgeB = path->data()[i].toNextVertex->getSegment().b;
          const simd::vec4 edgeAS = simd::vec4(&edgeA.x);
          const simd::vec4 edgeBS = simd::vec4(&edgeB.x);
          const bool isA = simd::all(simd::equal(leftP, edgeAS, EPSILON));
          
          path->data()[i].edgePoint = edgeAS;
          path->data()[i].edgeDir = isA ? simd::normalize(edgeBS - edgeAS) : -simd::normalize(edgeBS - edgeAS);
        }
        
        apex = rightP;
        apexIndex = rightIndex;
        leftP = apex;
        rightP = apex;
        leftIndex = apexIndex;
        rightIndex = apexIndex;
        
        i = apexIndex;
        continue;
      }
    }
  }
  
  // хотя может это и не нужно
//   for (size_t i = 0; i < path->data().size(); ++i) {
//     if (simd::dot(path->data()[i].edgeDir, path->data()[i].edgeDir) <= EPSILON) {
//       // по идее это не заполненный участок пути
//       // нам здесь нужно найти направление и точку ребра
//       // + точку направления, а это значит нужно посчитать пересечение между двумя отрезками,
//       // точнее близкую точку между двумя отрезками
//       
//       const glm::vec4 edgeA = path->data()[i].toNextVertex->getSegment().a;
//       const glm::vec4 edgeB = path->data()[i].toNextVertex->getSegment().b;
//       const simd::vec4 edgeAS = simd::vec4(&edgeA.x);
//       const simd::vec4 edgeBS = simd::vec4(&edgeB.x);
//       const bool isA = simd::all(simd::equal(leftP, edgeAS, EPSILON));
//       
//       path->data()[i].edgePoint = edgeAS;
//       path->data()[i].edgeDir = isA ? simd::normalize(edgeBS - edgeAS) : -simd::normalize(edgeBS - edgeAS);
//       
//       simd::vec4 nextPoint;
//       for (size_t j = i; j < path->data().size(); ++j) {
//         if (j+1 != path->data().size()) {
//           if (simd::dot(path->data()[j].edgeDir, path->data()[j].edgeDir) <= EPSILON) {
//             nextPoint = path->data()[j].funnelPoint;
//           }
//         } else {
//           const glm::vec4 c = path->data().back().vertex->getVertexData().center;
//           const simd::vec4 center = simd::vec4(&c.x);
//           nextPoint = center;
//         }
//       }
//       
// //       simd::vec4 prevPoint = i == 0 ? path->data().front().funnelPoint
//     }
//   }
}

// может вернуть nullptr, если путь не найден
// RawPath* CPUPathFindingPhaseParallel::rawPathPtr(const Type &type, const vertex_t* start, const vertex_t* goal) const {
//   auto itr = types.find(type);
//   if (itr == types.end()) return nullptr;
//   
//   // тут по правилам тоже нужно мьютекс присобачить, но с учетом того что конкретно этот участок будет отрабатывать позже всех
//   // он не будет пересекаться точно, когда мне заполнять историю?
//   auto pathItr = itr->second.paths.find(std::make_pair(start, goal));
//   if (pathItr == itr->second.paths.end()) return nullptr;
//   
//   return pathItr->second;
// }
// 
// RawPath CPUPathFindingPhaseParallel::findPath(const Type &type, const vertex_t* start, const vertex_t* goal) {
//   auto itr = types.find(type);
//   if (itr == types.end()) return RawPath();
//   
//   auto historyItr = itr->second.history.find(std::make_pair(start, goal));
//   if (historyItr != itr->second.history.end()) {
//     RawPath path;
//     
//     const vertex_t* next = historyItr->second.first;
//     path.data().push_back(RawPathPiece{start, historyItr->second.second});
//     while (next != goal) {
//       auto tmp = itr->second.history.find(std::make_pair(next, goal));
//       if (tmp == itr->second.history.end()) throw std::runtime_error("Bad history");
//       
//       path.data().push_back(RawPathPiece{next, tmp->second.second});
//       next = tmp->second.first;
//     }
//     
//     path.data().push_back(RawPathPiece{goal, nullptr});
//     
//     return path;
//   }
//   
//   // это может быть очень накладно из-за синхронизаций
//   {
//     std::unique_lock<std::mutex> lock(itr->second.mutex);
//     
//     for (size_t i = 0; i < itr->second.foundPaths.size(); ++i) {
//       const RawPath &tmp = itr->second.foundPaths[i];
//       if (tmp.start() == start && tmp.goal() == goal) return itr->second.foundPaths[i];
//     }
//   }
//   
//   // тут нужно взять поисковик для конкретного потока
//   // как это наиболее безболезненно сделать?
//   // по идее можно в стек положить, а потом оттуда брать
//   
//   AStarSearch* s;
//   {
//     std::unique_lock<std::mutex> lock(requestMutex);
//     
//     // берем из стека
//   }
//   
//   // ищем путь
//   s->setSearch(start, goal, itr->second.predicate);
//   
//   size_t intel = 0;
//   AStarSearch::SearchState state = AStarSearch::SEARCH_STATE_SEARCHING;
//   while (state != AStarSearch::SEARCH_STATE_FAILED && 
//          state != AStarSearch::SEARCH_STATE_INVALID && 
//          state != AStarSearch::SEARCH_STATE_NOT_INITIALISED && 
//          state != AStarSearch::SEARCH_STATE_OUT_OF_MEMORY) {
//     state = s->step();
//     ++intel;
//   }
//   
//   RawPath path;
//   
//   if (state == AStarSearch::SEARCH_STATE_INVALID || state == AStarSearch::SEARCH_STATE_NOT_INITIALISED || state == AStarSearch::SEARCH_STATE_OUT_OF_MEMORY) {
//     throw std::runtime_error("something wrong");
//   }
//   
//   if (state == AStarSearch::SEARCH_STATE_SUCCEEDED) {
//     const std::vector<AStarSearch::Node*> &nodes = s->getSolution();
//     for (size_t i = 0; i < nodes.size(); ++i) {
//       path.data()[i] = RawPathPiece{nodes[i]->vertex, nodes[i]->edge};
//     }
//     
//     s->freeSolutionNodes();
//     
//     std::unique_lock<std::mutex> lock(itr->second.mutex);
//     itr->second.foundPaths.push_back(path);
//   }
//   
//   {
//     std::unique_lock<std::mutex> lock(requestMutex);
//     
//     // кладем обратно в стек
//   }
//   
//   return path;
// }
