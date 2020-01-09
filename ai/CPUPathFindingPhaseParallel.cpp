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

RawPath::RawPath() {}
RawPath::RawPath(const size_t &size) : rawPath(size) {}
RawPath::~RawPath() {}

std::vector<RawPathPiece> & RawPath::graphData() {
  return rawPath;
}

const std::vector<RawPathPiece> & RawPath::graphData() const {
  return rawPath;
}

std::vector<FunnelPath> & RawPath::funnelData() {
  return funnelPath;
}

const std::vector<FunnelPath> & RawPath::funnelData() const {
  return funnelPath;
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
//   size_t i = 0;
//   size_t lastIndex = 0;
  size_t idx = 0;
  
//   std::cout << "path ptr " << this << '\n';
//   std::cout << "rawPath.data() " << rawPath.data() << '\n';
//   if (rawPath.data() == nullptr) {
//     throw std::runtime_error("wtf");
//   }
  
  dist = 10000.0f;
//   simd::vec4 prevPoint = simd::vec4(rawPath[0].funnelPoint);
//   for (size_t i = 1; i < rawPath.size(); ++i) {
//     if (f_eq(rawPath[i].funnelPoint[3], 0.0f)) continue;
//     
//     const simd::vec4 point = simd::vec4(rawPath[i].funnelPoint);
//     const simd::vec4 p = closestPoint(prevPoint, point, pos);
//     const float tmp = simd::distance2(p, pos);
//     if (dist > tmp) {
//       dist = tmp;
//       idx = i;
//       closest = p;
//     }
//     
// //     PRINT_VAR( "index    ", i)
// //     PRINT("")
// //     PRINT_VEC4("prev   ", prevPoint)
// //     PRINT_VEC4("current", point)
// //     PRINT("")
// //     PRINT_VEC4("closest", p)
// //     PRINT_VEC4("pos    ", pos)
// //     PRINT("")
// //     PRINT_VAR( "tmp      ", tmp)
// //     PRINT_VAR( "dist     ", dist)
// //     PRINT_VAR( "idx      ", idx)
// //     PRINT("")
//     
//     prevPoint = point;
//   }
  
  for (size_t i = 1; i < funnelPath.size(); ++i) {
    const simd::vec4 p = closestPoint(funnelPath[i-1].funnelPoint, funnelPath[i].funnelPoint, pos);
    const float tmp = simd::distance2(p, pos);
    if (dist > tmp) {
      dist = tmp;
      idx = i;
      closest = p;
    }
  }
  
//   for (i = 1; i < rawPath.size(); ++i) {
//     if (glm::abs(simd::dot(simd::vec4(rawPath[i].edgeDir), simd::vec4(rawPath[i].edgeDir))) > EPSILON) break;
//   }
//   
//   closest = closestPoint(simd::vec4(rawPath[lastIndex].funnelPoint), simd::vec4(rawPath[i].funnelPoint), pos);
//   dist = simd::distance2(closest, pos);
//   
//   lastIndex = i;
//   ++i;
//   
//   for (; i < rawPath.size()-1; ++i) {
//     if (glm::abs(simd::dot(simd::vec4(rawPath[i].edgeDir), simd::vec4(rawPath[i].edgeDir))) <= EPSILON) continue;
//     
//     const simd::vec4 p = closestPoint(simd::vec4(rawPath[lastIndex].funnelPoint), simd::vec4(rawPath[i].funnelPoint), pos);
//     float tmp = simd::distance2(p, pos);
//     
//     if (dist > tmp) {
//       dist = tmp;
//       idx = lastIndex;
//       closest = p;
//     }
//     
//     lastIndex = i;
//   }
  
  return idx;
}

// size_t RawPath::getNextSegmentIndex(const size_t &index) const {
//   size_t newIndex = index+1;
//   while (rawPath[newIndex].funnelPoint[3] <= 0.0f) {
//     ++newIndex;
//   }
//   
//   return newIndex;
// }
// 
// size_t RawPath::getPrevSegmentIndex(const size_t &index) const {
//   if (index == 0) return 0;
//   
//   size_t newIndex = index-1;
//   while (newIndex != 0 || rawPath[newIndex].funnelPoint[3] <= 0.0f) {
//     --newIndex;
//   }
//   
//   return newIndex;
// }
// 
// size_t RawPath::getNextSegmentIndex2(const size_t &index) const {
//   size_t newIndex = index+1;
//   simd::vec4 point = rawPath[index].funnelPoint;
//   while (rawPath[newIndex].funnelPoint[3] <= 0.0f && !simd::all(simd::equal(point, rawPath[newIndex].edgePoint, EPSILON))) {
//     ++newIndex;
//   }
//   
//   return newIndex;
// }
// 
// size_t RawPath::getPrevSegmentIndex2(const size_t &index) const {
//   if (index == 0) return 0;
//   
//   size_t newIndex = index-1;
//   simd::vec4 point = rawPath[index].funnelPoint;
//   while (newIndex != 0 || (rawPath[newIndex].funnelPoint[3] <= 0.0f && !simd::all(simd::equal(point, rawPath[newIndex].edgePoint, EPSILON)))) {
//     --newIndex;
//   }
//   
//   return newIndex;
// }

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
void CPUPathFindingPhaseParallel::registerPathType(const Type &type, const std::function<bool(const vertex_t*, const vertex_t*, const edge_t*)> &predicate, const float &offset) {
  auto itr = types.find(type);
  if (itr != types.end()) throw std::runtime_error("Path finding type with name " + type.name() + " is already registered");
  
  types[type].predicate = predicate;
  types[type].offset = offset;
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
  
  static const auto pathFinding = [&] (const Type &type, const std::function<bool(const vertex_t*, const vertex_t*, const edge_t*)> &predicate, const float &funnelOffset, localIterator itr) {
    (void)type;
    
    //if (itr->second.path != nullptr && itr->second.state == path_finding_state::has_path) return;
    // здесь можно поставить условие, чтобы мы не обрабатывали запрос, которыей уже ранее был обработан
    // для того чтобы заного его обработать, его нужно передобавить
    if (itr->second.state != path_finding_state::delayed) return;
    
    AStarSearch* searchPtr = nullptr;
    {
      std::unique_lock<std::mutex> lock(stackMutex);
      
      searchPtr = stack.back();
      stack.pop_back();
    }
    
    searchPtr->setSearch(itr->first.first, itr->first.second, predicate);
    
//     state != AStarSearch::SEARCH_STATE_SUCCEEDED && 
//     state != AStarSearch::SEARCH_STATE_FAILED && 
//     state != AStarSearch::SEARCH_STATE_INVALID && 
//     state != AStarSearch::SEARCH_STATE_NOT_INITIALISED && 
//     state != AStarSearch::SEARCH_STATE_OUT_OF_MEMORY
    
    AStarSearch::SearchState state = AStarSearch::SEARCH_STATE_SEARCHING;
    while (state == AStarSearch::SEARCH_STATE_SEARCHING) {
      state = searchPtr->step();
    }
    
    if (state == AStarSearch::SEARCH_STATE_INVALID || 
        state == AStarSearch::SEARCH_STATE_NOT_INITIALISED || 
        state == AStarSearch::SEARCH_STATE_OUT_OF_MEMORY) {
      throw std::runtime_error("Something wrong with searching");
    }
    
    if (state == AStarSearch::SEARCH_STATE_FAILED) {
      // ошибка
//       searchPtr->freeSolutionNodes();
      
      itr->second.path = nullptr;
      itr->second.state = path_finding_state::path_not_exist;
      
      std::unique_lock<std::mutex> lock(stackMutex);
      stack.push_back(searchPtr);
      return;
    }
    
//     PRINT_VAR("start", itr->first.first)
//     PRINT_VAR("goal ", itr->first.second)
    
    // мы уже можем стоять на месте гоала, нужно вернуть только один нод
    const auto &solution = searchPtr->getSolution();
//     for (size_t i = 0; i < solution.size(); ++i) {
//       PRINT_VAR("index ", i)
//       PRINT_VAR("node  ", solution[i])
//       PRINT_VAR("vertex", solution[i]->vertex)
//     }
//     
//     if (solution.size() < 2) throw std::runtime_error("dl;ma;krnkba';");
    
    // одна из вершин обрабатывается неверно (справа от вершины соединенной с подъемом)
    // мне нужно понять что с ней не так
    // большая часть багов с поиском пути исправлена, осталось только доделать вариант с fake edge
    // + улучшить код отступов (то есть чтобы челики стенку не полировали когда рядом с ней проходят, а пытались чуть чуть от нее отойти)
    
    // из солюшена мы составляем RawPath и где то его сохраняем
    RawPath* path = nullptr;
    {
      std::unique_lock<std::mutex> lock(poolMutex);
      path = pathsPool.newElement();
      std::cout << "path " << path << " created" << "\n";
    }
    
    // нужно поставить первым центр первой вершины
//     const RawPathPiece p{
//       nullptr,
//       nullptr,
//       //{firstCenter.x, firstCenter.y, firstCenter.z, firstCenter.w},
//       {0.0f, 0.0f, 0.0f, 0.0f},
//       {0.0f, 0.0f, 0.0f, 0.0f},
//       {0.0f, 0.0f, 0.0f, 0.0f}
//     };
//     path->graphData().push_back(p);
//     
//     const glm::vec4 firstCenter = solution[0]->vertex->getVertexData()->center;
//     path->graphData()[0].funnelPoint[0] = firstCenter.x;
//     path->graphData()[0].funnelPoint[1] = firstCenter.y;
//     path->graphData()[0].funnelPoint[2] = firstCenter.z;
//     path->graphData()[0].funnelPoint[3] = firstCenter.w;
    
    for (size_t i = 0; i < solution.size(); ++i) {
      const RawPathPiece p{
        solution[i]->vertex,
        i+1 == solution.size() ? nullptr : solution[i]->vertex->edge(solution[i+1]->vertex), // solution[i]->edge
//         {0.0f, 0.0f, 0.0f, 0.0f},
//         {0.0f, 0.0f, 0.0f, 0.0f},
//         {0.0f, 0.0f, 0.0f, 0.0f}
      };
      
      path->graphData().push_back(p);
    }
    
    searchPtr->freeSolutionNodes();
    {
      std::unique_lock<std::mutex> lock(stackMutex);
      stack.push_back(searchPtr);
    }
    searchPtr = nullptr;
    
    computeFunnel(path, funnelOffset);
    
//     size_t count = 0;
//     for (size_t i = 0; i < path->graphData().size(); ++i) {
//       const simd::vec4 funnelPoint = simd::vec4(path->graphData()[i].funnelPoint);
//       const float vecW = path->graphData()[i].funnelPoint[3];
//       if (vecW > 0.0f) {
//         for (size_t j = i-count; j < i; ++j) {
//           const edge_t* edge = path->graphData()[j].toNextVertex;
//           if (edge == nullptr) {
//             funnelPoint.storeu(path->graphData()[j].edgePoint);
//             continue;
//           }
//           
//           const glm::vec4 &a = edge->getSegment().a;
//           const glm::vec4 &b = edge->getSegment().b;
//           
//           const simd::vec4 edgeAS = simd::vec4(&a.x);
//           const simd::vec4 edgeBS = simd::vec4(&b.x);
//           const bool isA = simd::all(simd::equal(funnelPoint, edgeAS, EPSILON));
//           const float distA = simd::distance2(edgeAS, funnelPoint);
//           const float distB = simd::distance2(edgeBS, funnelPoint);
// //           const simd::vec4 dir = simd::normalize(edgeBS - edgeAS);
//           
// //           const simd::vec4 edgeASs = isA ? edgeAS : edgeBS;
// //           const simd::vec4 edgeBSs = isA ? edgeBS : edgeAS;
//           const simd::vec4 edgeASs = distA <  distB ? edgeAS : edgeBS;
//           const simd::vec4 edgeBSs = distA >= distB ? edgeBS : edgeAS;
//           //const simd::vec4 edgeDir = isA ? dir : -dir;
//           const simd::vec4 edgeDir = simd::normalize(edgeBSs - edgeASs);
//           edgeASs.storeu(path->graphData()[j].edgePoint);
//           edgeDir.storeu(path->graphData()[j].edgeDir);
//         }
//         
//         const edge_t* edge = path->graphData()[i].toNextVertex;
//         if (edge == nullptr) {
//           funnelPoint.storeu(path->graphData()[i].edgePoint);
//           continue;
//         }
//         
//         const glm::vec4 &a = edge->getSegment().a;
//         const glm::vec4 &b = edge->getSegment().b;
//         
//         const simd::vec4 edgeAS = simd::vec4(&a.x);
//         const simd::vec4 edgeBS = simd::vec4(&b.x);
//         const bool isA = simd::all(simd::equal(funnelPoint, edgeAS, EPSILON));
// //           const float distA = simd::distance2(edgeAS, funnelPoint);
// //           const float distB = simd::distance2(edgeBS, funnelPoint);
// //           const simd::vec4 dir = simd::normalize(edgeBS - edgeAS);
//         
//         const simd::vec4 edgeASs = isA ? edgeAS : edgeBS;
//         const simd::vec4 edgeBSs = isA ? edgeBS : edgeAS;
// //           const simd::vec4 edgeASs = distA <  distB ? edgeAS : edgeBS;
// //           const simd::vec4 edgeBSs = distA >= distB ? edgeBS : edgeAS;
//         //const simd::vec4 edgeDir = isA ? dir : -dir;
//         const simd::vec4 edgeDir = simd::normalize(edgeBSs - edgeASs);
//         edgeASs.storeu(path->graphData()[i].edgePoint);
//         edgeDir.storeu(path->graphData()[i].edgeDir);
//         
//         count = 0;
//       } else {
//         ++count;
//       }
//     }
    
//     for (size_t i = 0; i < path->graphData().size(); ++i) {
//       const edge_t* edge = path->graphData()[i].toNextVertex;
//       
//       const glm::vec4 &a = edge == nullptr ? glm::vec4(0.0f) : edge->getSegment().a;
//       const glm::vec4 &b = edge == nullptr ? glm::vec4(0.0f) : edge->getSegment().b;
//       const glm::vec4 &center = path->graphData()[i].vertex == nullptr ? glm::vec4(0.0f) : path->graphData()[i].vertex->getVertexData()->center;
//       
//       PRINT_VAR( "index", i)
//       PRINT_VEC3("edge a       ", simd::vec4(&a.x))
//       PRINT_VEC3("edge b       ", simd::vec4(&b.x))
//       PRINT_VEC3("vertex center", simd::vec4(&center.x))
//       PRINT_VEC4("funnelPoint  ", simd::vec4(path->graphData()[i].funnelPoint))
//       PRINT_VEC3("edgeDir      ", simd::vec4(path->graphData()[i].edgeDir))
//       PRINT_VEC3("edgePoint    ", simd::vec4(path->graphData()[i].edgePoint))
//     }
//     
//     throw std::runtime_error("no more");
    
    for (size_t i = 0; i < path->funnelData().size(); ++i) {
      PRINT_VAR("index", i)
      PRINT_VEC3("point1", path->funnelData()[i].funnelPoint)
//       PRINT_VEC3("point2", path->funnelData()[i].another)
      PRINT_VEC3("dir  ", path->funnelData()[i].edgeDir)
    }
    
//     throw std::runtime_error("no more");
    
    {
//       std::unique_lock<std::mutex> lock(types[type].mutex);
      itr->second.path = path;
      itr->second.state = path_finding_state::has_path;
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
      pool->submitnr(pathFinding, type.first, type.second.predicate, type.second.offset, itr);
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
        std::cout << "path " << queueItr->second.path << " deleted" << "\n";
        pathsPool.deleteElement(queueItr->second.path);
        queueItr->second.path = nullptr;
      }
      
      itr->second.queue.erase(queueItr);
    }
  }
}

void CPUPathFindingPhaseParallel::computeFunnel(RawPath* path, const float &offset) {
  // тут мне нужно заполнить данные всех вершин, то есть точку справа (или все же ближайшую?),
  // направление от этой точки (наверное еще длину ребра), точку в которую я направляюсь
  
  // в незаполненных позициях фуннел поинт заполняется нулями
  simd::vec4 apex, leftP, rightP, left, right;
  size_t apexIndex = 0, leftIndex = 0, rightIndex = 0;
  
  size_t firstIndex = 0;
  const glm::vec4 center = path->graphData()[firstIndex].vertex->getVertexData()->center;
  const glm::vec4 normal = path->graphData()[firstIndex].vertex->getVertexData()->normal;
  apex.loadu(&center.x);
  
  path->funnelData().push_back({apex, simd::vec4(0.0f)});
  
  glm::vec4 firstA;
  glm::vec4 firstB;
  const edge_t* edge = path->graphData()[firstIndex].toNextVertex;
  if (edge->isFake()) {
    float b = 0.5f / glm::tan(edge->getAngle());
    
    if (b > edge->getWidth()) edge->getSegment().leftRight(center, normal, firstA, firstB);
    else {
      LineSegment l(edge->getSegment().a, edge->getSegment().a + edge->getSegment().dir()*b);
      l.leftRight(center, normal, firstA, firstB);
    }
  } else {
    edge->getSegment().leftRight(center, normal, firstA, firstB);
  }
  
  leftP.loadu(&firstA.x);
  rightP.loadu(&firstB.x);
  
  // нужно ко всем точкам добавлять оффсет, как это сделать правильно?
  // причем для fakeEdge нужно наоборот вычитать
  // у нас есть левая и правая точка и вектор направления
  // нам нужно сместить левую и правую точки ближе к центру на небольшой оффсет
  // причем он может быть совсем маленьким, только лишь для того чтобы точки на углах не были похожи 1к1
  const glm::vec4 tmpEdgeDir = edge->getDir();
  correctCornerPoints(simd::vec4(&tmpEdgeDir.x), edge->getWidth(), offset, leftP, rightP);
  
  for (size_t i = 1; i < path->graphData().size(); ++i) {
    const vertex_t* vertex = path->graphData()[i].vertex;
    
    const glm::vec4 c = vertex->getVertexData()->center;
    const simd::vec4 center = simd::vec4(&c.x);
    const glm::vec4 n = vertex->getVertexData()->normal;
    const simd::vec4 normal = simd::vec4(&n.x);
    
    // обработка fake edge, нужно передать правильно высоту + ступеньки, как разные ступеньки организовать
    // по идее мы на стадии поиска должны их выщелкивать, но я не помню работает ли это
    if (i+1 != path->graphData().size()) {
      const edge_t* edge = path->graphData()[i].toNextVertex;
      
      if (edge->isFake()) {
        float b = 0.5f / glm::tan(edge->getAngle());
        
        if (b > edge->getWidth()) edge->getSegment().leftRight(center, normal, left, right);
        else {
          LineSegment l(edge->getSegment().a, edge->getSegment().a + edge->getSegment().dir()*b);
          l.leftRight(center, normal, left, right);
        }
      } else {
        edge->getSegment().leftRight(center, normal, left, right);
      }
      
      const glm::vec4 tmpEdgeDir = edge->getDir();
      correctCornerPoints(simd::vec4(&tmpEdgeDir.x), edge->getWidth(), offset, left, right);
    } else {
      left = center;
      right = center;
    }
    
    // тут наверное нужно использовать нормаль гравитации (?)
    if (sideOf(apex, rightP, right, normal) <= 0.0f) {
      if (simd::all(simd::equal(apex, rightP, EPSILON)) || sideOf(apex, leftP, right, normal) > 0.0f) {
        rightP = right;
        rightIndex = i;
        
        // точка внутри воронки
      } else {
        const edge_t* edge = path->graphData()[leftIndex].toNextVertex;
        if (edge == nullptr) {
          path->funnelData().push_back({leftP, simd::vec4(0.0f)});
        } else {
          const glm::vec4 &a = edge->getSegment().a;
          const glm::vec4 &b = edge->getSegment().b;
          const simd::vec4 &aS = simd::vec4(&a.x);
          const simd::vec4 &bS = simd::vec4(&b.x);
          const bool isLeft = simd::all(simd::equal(leftP, aS, EPSILON));
          const simd::vec4 dir = simd::normalize(bS - aS);
          float arr[4];
          dir.storeu(arr);
          const simd::vec4 finalDir = isLeft ? simd::vec4(arr[0], arr[1], arr[2], glm::uintBitsToFloat(i)) : simd::vec4(-arr[0], -arr[1], -arr[2], glm::uintBitsToFloat(i));
          path->funnelData().push_back({leftP, finalDir});
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
      if (simd::all(simd::equal(apex, leftP, EPSILON)) || sideOf(apex, rightP, left, normal) < 0.0f) {
        leftP = left;
        leftIndex = i;
        
        // точка внутри воронки
      } else {
        const edge_t* edge = path->graphData()[rightIndex].toNextVertex;
        if (edge == nullptr) {
          path->funnelData().push_back({rightP, simd::vec4(0.0f)});
        } else {
          const glm::vec4 &a = edge->getSegment().a;
          const glm::vec4 &b = edge->getSegment().b;
          const simd::vec4 &aS = simd::vec4(&a.x);
          const simd::vec4 &bS = simd::vec4(&b.x);
          const bool isRight = simd::all(simd::equal(rightP, aS, EPSILON));
          const simd::vec4 dir = simd::normalize(bS - aS);
          float arr[4];
          dir.storeu(arr);
          const simd::vec4 finalDir = isRight ? simd::vec4(arr[0], arr[1], arr[2], glm::uintBitsToFloat(i)) : simd::vec4(-arr[0], -arr[1], -arr[2], glm::uintBitsToFloat(i));
          path->funnelData().push_back({rightP, finalDir});
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
  
  const glm::vec4 &lastPoint = path->graphData().back().vertex->getVertexData()->center;
  path->funnelData().push_back({simd::vec4(&lastPoint.x), simd::vec4(0.0f)});
}

void CPUPathFindingPhaseParallel::correctCornerPoints(const simd::vec4 &dir, const float &width, const float &offset, simd::vec4 &left, simd::vec4 &right) {
  // предположительно dir это направление от одной из точек к другой, width это растояние между точками, offset - требуемое отстояние
  const float finalOffset = std::min(offset, width/2.0f);
  if (finalOffset < EPSILON) return;
  
  const simd::vec4 &localDir = right - left;
  const float dot = simd::dot(localDir, dir);
  const simd::vec4 &finalDir = dot > 0.0f ? dir : -dir;
  left = left + finalDir * finalOffset;
  right = right - finalDir * finalOffset;
}

// void CPUPathFindingPhaseParallel::computeFunnel(RawPath* path) {
//   // тут мне нужно заполнить данные всех вершин, то есть точку справа (или все же ближайшую?),
//   // направление от этой точки (наверное еще длину ребра), точку в которую я направляюсь
//   
//   // в незаполненных позициях фуннел поинт заполняется нулями
//   simd::vec4 apex, leftP, rightP, left, right;
//   size_t apexIndex = 0, leftIndex = 0, rightIndex = 0;
//   
// //   PRINT_VAR("path->data().size()", path->data().size())
// //   if (path->graphData().size() < 3) {
// //     // последний фуннел поинт нужно поставить в центр вершины
// //     const glm::vec4 &lastPoint = path->graphData().back().vertex->getVertexData()->center;
// //     path->graphData().back().funnelPoint[0] = lastPoint[0];
// //     path->graphData().back().funnelPoint[1] = lastPoint[1];
// //     path->graphData().back().funnelPoint[2] = lastPoint[2];
// //     path->graphData().back().funnelPoint[3] = lastPoint[3];
// //     return;
// //   }
//   
//   size_t firstIndex = 0;
// //   while (firstIndex < path->graphData().size()-1 && path->graphData()[firstIndex].toNextVertex == nullptr) {
// //     ++firstIndex;
// //   }
//   const glm::vec4 center = path->graphData()[firstIndex].vertex->getVertexData()->center;
//   const glm::vec4 normal = path->graphData()[firstIndex].vertex->getVertexData()->normal;
//   apex.loadu(&center.x);
//   
//   path->funnelData().push_back({apex, simd::vec4(0.0f)});
//   
//   glm::vec4 firstA;
//   glm::vec4 firstB;
//   const edge_t* edge = path->graphData()[firstIndex].toNextVertex;
//   if (edge->isFake()) {
//     float b = 0.5f / glm::tan(edge->getAngle());
//     
//     if (b > edge->getWidth()) edge->getSegment().leftRight(center, normal, firstA, firstB);
//     else {
//       LineSegment l(edge->getSegment().a, edge->getSegment().a + edge->getSegment().dir()*b);
//       l.leftRight(center, normal, firstA, firstB);
//     }
//   } else {
//     edge->getSegment().leftRight(center, normal, firstA, firstB);
//   }
//   
//   leftP.loadu(&firstA.x);
//   rightP.loadu(&firstB.x);
//   
//   for (size_t i = 1; i < path->graphData().size(); ++i) {
//     const vertex_t* vertex = path->graphData()[i].vertex;
//     
//     const glm::vec4 c = vertex->getVertexData()->center;
//     const simd::vec4 center = simd::vec4(&c.x);
//     const glm::vec4 n = vertex->getVertexData()->normal;
//     const simd::vec4 normal = simd::vec4(&n.x);
//     
// //     std::cout << "\n";
// //     
// //     std::cout << "index " << i << "\n";
//     
//     // обработка fake edge, нужно передать правильно высоту + ступеньки, как разные ступеньки организовать
//     // по идее мы на стадии поиска должны их выщелкивать, но я не помню работает ли это
//     if (i+1 != path->graphData().size()) {
//       const edge_t* edge = path->graphData()[i].toNextVertex;
//       
//       if (edge->isFake()) {
//         float b = 0.5f / glm::tan(edge->getAngle());
//         
//         if (b > edge->getWidth()) edge->getSegment().leftRight(center, normal, left, right);
//         else {
//           LineSegment l(edge->getSegment().a, edge->getSegment().a + edge->getSegment().dir()*b);
//           l.leftRight(center, normal, left, right);
//         }
//       } else {
//         edge->getSegment().leftRight(center, normal, left, right);
//       }
//     } else {
//       left = center;
//       right = center;
//     }
//     
//     // тут наверное нужно использовать нормаль гравитации (?)
//     if (sideOf(apex, rightP, right, normal) <= 0.0f) {
//       if (rightIndex != i && simd::all(simd::equal(apex, rightP, EPSILON))) {
// //         const edge_t* edge = path->graphData()[rightIndex].toNextVertex;
// //         if (edge != nullptr) {
// //           const glm::vec4 &a = edge->getSegment().a;
// //           const glm::vec4 &b = edge->getSegment().b;
// //           const simd::vec4 &aS = simd::vec4(&a.x);
// //           const simd::vec4 &bS = simd::vec4(&b.x);
// //           const bool isRight = simd::all(simd::equal(rightP, aS, EPSILON));
// //           const simd::vec4 dir = simd::normalize(bS - aS);
// //           float arr[4];
// //           dir.storeu(arr);
// //           const simd::vec4 finalDir = isRight ? simd::vec4(arr[0], arr[1], arr[2], glm::uintBitsToFloat(i)) : simd::vec4(-arr[0], -arr[1], -arr[2], glm::uintBitsToFloat(i));
// //           path->funnelData().push_back({rightP, isRight ? bS : aS, finalDir});
// //         }
//         
//         rightP = right;
//         rightIndex = i;
//       } else if (sideOf(apex, leftP, right, normal) > 0.0f) {
//         rightP = right;
//         rightIndex = i;
//         
//         // точка внутри воронки
//       } else {
//         //path->data()[i].funnelPoint = leftP;
// //         leftP.storeu(path->graphData()[i].funnelPoint);
//         const edge_t* edge = path->graphData()[leftIndex].toNextVertex;
//         if (edge == nullptr) {
//           path->funnelData().push_back({leftP, simd::vec4(0.0f)});
//         } else {
//           const glm::vec4 &a = edge->getSegment().a;
//           const glm::vec4 &b = edge->getSegment().b;
//           const simd::vec4 &aS = simd::vec4(&a.x);
//           const simd::vec4 &bS = simd::vec4(&b.x);
//           const bool isLeft = simd::all(simd::equal(leftP, aS, EPSILON));
//           const simd::vec4 dir = simd::normalize(bS - aS);
//           float arr[4];
//           dir.storeu(arr);
//           const simd::vec4 finalDir = isLeft ? simd::vec4(arr[0], arr[1], arr[2], glm::uintBitsToFloat(i)) : simd::vec4(-arr[0], -arr[1], -arr[2], glm::uintBitsToFloat(i));
//           path->funnelData().push_back({leftP, finalDir});
//         }
//         
// //         if (i+1 == path->data().size()) {
// //           //path->data()[i].edgeDir = simd::vec4(0.0f);
// //           simd::vec4(0.0f).storeu(path->data()[i].edgeDir);
// //         } else {
// //           const glm::vec4 edgeA = path->data()[i].toNextVertex->getSegment().a;
// //           const glm::vec4 edgeB = path->data()[i].toNextVertex->getSegment().b;
// //           const simd::vec4 edgeAS = simd::vec4(&edgeA.x);
// //           const simd::vec4 edgeBS = simd::vec4(&edgeB.x);
// //           const bool isA = simd::all(simd::equal(leftP, edgeAS, EPSILON));
// //           
// //           const simd::vec4 edgeASs = isA ? edgeAS : edgeBS;
// //           const simd::vec4 edgeDir = isA ? simd::normalize(edgeBS - edgeAS) : -simd::normalize(edgeBS - edgeAS);
// // //           path->data()[i].edgePoint = edgeAS;
// // //           path->data()[i].edgeDir = edgeDir;
// //           edgeASs.storeu(path->data()[i].edgePoint);
// //           edgeDir.storeu(path->data()[i].edgeDir);
// //         }
//         
//         apex = leftP;
//         apexIndex = leftIndex;
//         leftP = apex;
//         rightP = apex;
//         leftIndex = apexIndex;
//         rightIndex = apexIndex;
//         
//         i = apexIndex;
//         continue;
//       }
//     }
//     
//     if (sideOf(apex, leftP, left, normal) >= 0.0f) {
//       if (leftIndex != i && simd::all(simd::equal(apex, leftP, EPSILON))) {
// //         const edge_t* edge = path->graphData()[leftIndex].toNextVertex;
// //         if (edge != nullptr) {
// //           const glm::vec4 &a = edge->getSegment().a;
// //           const glm::vec4 &b = edge->getSegment().b;
// //           const simd::vec4 &aS = simd::vec4(&a.x);
// //           const simd::vec4 &bS = simd::vec4(&b.x);
// //           const bool isLeft = simd::all(simd::equal(leftP, aS, EPSILON));
// //           const simd::vec4 dir = simd::normalize(bS - aS);
// //           float arr[4];
// //           dir.storeu(arr);
// //           const simd::vec4 finalDir = isLeft ? simd::vec4(arr[0], arr[1], arr[2], glm::uintBitsToFloat(i)) : simd::vec4(-arr[0], -arr[1], -arr[2], glm::uintBitsToFloat(i));
// //           path->funnelData().push_back({leftP, isLeft ? bS : aS, finalDir});
// //         }
//         
//         leftP = left;
//         leftIndex = i;
//       } else if (sideOf(apex, rightP, left, normal) < 0.0f) {
//         leftP = left;
//         leftIndex = i;
//         
//         // точка внутри воронки
//       } else {
//         //path->data()[i].funnelPoint = rightP;
// //         rightP.storeu(path->graphData()[i].funnelPoint);
//         const edge_t* edge = path->graphData()[rightIndex].toNextVertex;
//         if (edge == nullptr) {
//           path->funnelData().push_back({rightP, simd::vec4(0.0f)});
//         } else {
//           const glm::vec4 &a = edge->getSegment().a;
//           const glm::vec4 &b = edge->getSegment().b;
//           const simd::vec4 &aS = simd::vec4(&a.x);
//           const simd::vec4 &bS = simd::vec4(&b.x);
//           const bool isRight = simd::all(simd::equal(rightP, aS, EPSILON));
//           const simd::vec4 dir = simd::normalize(bS - aS);
//           float arr[4];
//           dir.storeu(arr);
//           const simd::vec4 finalDir = isRight ? simd::vec4(arr[0], arr[1], arr[2], glm::uintBitsToFloat(i)) : simd::vec4(-arr[0], -arr[1], -arr[2], glm::uintBitsToFloat(i));
//           path->funnelData().push_back({rightP, finalDir});
//         }
//         
// //         if (i+1 == path->data().size()) {
// //           //path->data()[i].edgeDir = simd::vec4(0.0f);
// //           simd::vec4(0.0f).storeu(path->data()[i].edgeDir);
// //         } else {
// //           const glm::vec4 edgeA = path->data()[i].toNextVertex->getSegment().a;
// //           const glm::vec4 edgeB = path->data()[i].toNextVertex->getSegment().b;
// //           const simd::vec4 edgeAS = simd::vec4(&edgeA.x);
// //           const simd::vec4 edgeBS = simd::vec4(&edgeB.x);
// //           const bool isA = simd::all(simd::equal(rightP, edgeAS, EPSILON));
// //           
// //           const simd::vec4 edgeDir = isA ? simd::normalize(edgeBS - edgeAS) : -simd::normalize(edgeBS - edgeAS);
// // //           path->data()[i].edgePoint = edgeAS;
// // //           path->data()[i].edgeDir = edgeDir;
// //           
// //           edgeAS.storeu(path->data()[i].edgePoint);
// //           edgeDir.storeu(path->data()[i].edgeDir);
// //         }
//         
//         apex = rightP;
//         apexIndex = rightIndex;
//         leftP = apex;
//         rightP = apex;
//         leftIndex = apexIndex;
//         rightIndex = apexIndex;
//         
//         i = apexIndex;
//         continue;
//       }
//     }
//   }
//   
//   // последний фуннел поинт нужно поставить в центр вершины
// //   const glm::vec4 &lastPoint = path->graphData().back().vertex->getVertexData()->center;
// //   path->graphData().back().funnelPoint[0] = lastPoint[0];
// //   path->graphData().back().funnelPoint[1] = lastPoint[1];
// //   path->graphData().back().funnelPoint[2] = lastPoint[2];
// //   path->graphData().back().funnelPoint[3] = lastPoint[3];
//   
//   const glm::vec4 &lastPoint = path->graphData().back().vertex->getVertexData()->center;
//   path->funnelData().push_back({simd::vec4(&lastPoint.x), simd::vec4(0.0f)});
//   
//   // хотя может это и не нужно
// //   for (size_t i = 0; i < path->data().size(); ++i) {
// //     if (simd::dot(path->data()[i].edgeDir, path->data()[i].edgeDir) <= EPSILON) {
// //       // по идее это не заполненный участок пути
// //       // нам здесь нужно найти направление и точку ребра
// //       // + точку направления, а это значит нужно посчитать пересечение между двумя отрезками,
// //       // точнее близкую точку между двумя отрезками
// //       
// //       const glm::vec4 edgeA = path->data()[i].toNextVertex->getSegment().a;
// //       const glm::vec4 edgeB = path->data()[i].toNextVertex->getSegment().b;
// //       const simd::vec4 edgeAS = simd::vec4(&edgeA.x);
// //       const simd::vec4 edgeBS = simd::vec4(&edgeB.x);
// //       const bool isA = simd::all(simd::equal(leftP, edgeAS, EPSILON));
// //       
// //       path->data()[i].edgePoint = edgeAS;
// //       path->data()[i].edgeDir = isA ? simd::normalize(edgeBS - edgeAS) : -simd::normalize(edgeBS - edgeAS);
// //       
// //       simd::vec4 nextPoint;
// //       for (size_t j = i; j < path->data().size(); ++j) {
// //         if (j+1 != path->data().size()) {
// //           if (simd::dot(path->data()[j].edgeDir, path->data()[j].edgeDir) <= EPSILON) {
// //             nextPoint = path->data()[j].funnelPoint;
// //           }
// //         } else {
// //           const glm::vec4 c = path->data().back().vertex->getVertexData().center;
// //           const simd::vec4 center = simd::vec4(&c.x);
// //           nextPoint = center;
// //         }
// //       }
// //       
// // //       simd::vec4 prevPoint = i == 0 ? path->data().front().funnelPoint
// //     }
// //   }
// }
