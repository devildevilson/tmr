#ifndef PATH_FINDING_PHASE_H
#define PATH_FINDING_PHASE_H

#include <vector>
#include <functional>
#include <unordered_map>

#include "Type.h"
#include "UniqueID.h"
#include "Utility.h"

class vertex_t;
class edge_t;

struct RawPathPiece {
  const vertex_t* vertex;
  const edge_t* toNextVertex;
  // что тут может еще добавиться?
  // тут можно расположить данные от фуннел алгоритма
  // нам потребуется: точка "начала" ребра, направление ребра ОТ точки, точка в котороую мы идем
  simd::vec4 edgePoint;
  simd::vec4 edgeDir;
  simd::vec4 funnelPoint;
};

class RawPath {
public:
  RawPath() = default;
  RawPath(const size_t &size);
  ~RawPath() = default;
  
  std::vector<RawPathPiece> & data();
  const std::vector<RawPathPiece> & data() const;
  
  const vertex_t* start() const;
  const vertex_t* goal() const;
  
  size_t getNearPathSegmentIndex(const simd::vec4 &pos, simd::vec4 &closest, float &dist) const;
  
  RawPath & operator=(const RawPath &path) = default;
private:
  // что то еще потребуется?
  
  std::vector<RawPathPiece> rawPath;
};

// struct PathfindingType {
//   std::function<bool(const vertex_t*, const vertex_t*, const edge_t*)> predicate;
//   
//   std::vector<std::pair<vertex_t*, vertex_t*>> buffer;
//   
//   vertex_t* currentStart;
//   vertex_t* currentGoal;
//   RawPath currentPath;
//   
//   std::unordered_map<std::pair<vertex_t*, vertex_t*>, std::pair<vertex_t*, edge_t*>> history;
// };

struct FindRequest {
  Type type; 
  const vertex_t* start; 
  const vertex_t* end;
//   uint32_t intellegence;
};

enum class path_finding_state {
  has_path,
  path_not_exist,
  delayed
};

struct PathFindingReturn {
  path_finding_state state;
  RawPath* path;
  size_t pointerCount;
};

class PathFindingPhase {
public:
  virtual ~PathFindingPhase() {}
  
  virtual void registerPathType(const Type &type, const std::function<bool(const vertex_t*, const vertex_t*, const edge_t*)> &predicate) = 0;
  virtual void queueRequest(const FindRequest &request) = 0;
//   virtual size_t requestCount() const = 0;
  
  virtual void update() = 0;
  
  virtual PathFindingReturn getPath(const FindRequest &req) = 0;
  virtual void releasePath(const FindRequest &req) = 0;
  
  // здесь целиком и полностью будет поиск пути, нужно продумать 
  // как искать как можно меньше (объекты в группы в зависимости от стартовой и конечной позиций)
  // как запоминать результат (у меня уже было какое-то решение)
  // как и когда чистить то что мы заполнили (может потребоваться на больших картах)
  // как представить путь который мы нашли
  // как распараллелить весь поиск (те группы которые я упоминал ранее могут в этом помочь)
  // поиск должен проводится с учетом некоторых характеристик
  // что-то еще?
  
  // тут наверное помимо поиска пути должна быть еще работа с графом
  // получить вершину, получить все объекты в радиусе, и прочее
  
  // работа с графом будет тут опосредованная
};

#endif
