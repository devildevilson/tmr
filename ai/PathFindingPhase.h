#ifndef PATH_FINDING_PHASE_H
#define PATH_FINDING_PHASE_H

#include <vector>
#include <functional>
#include <unordered_map>

#include "Type.h"

class vertex_t;
class edge_t;

struct RawPathPiece {
  const vertex_t* vertex;
  const edge_t* toNextVertex;
  // что тут может еще добавиться?
};

class RawPath {
public:
  RawPath();
  
  std::vector<RawPathPiece> & data();
  const std::vector<RawPathPiece> & data() const;
private:
  // что то еще потребуется?
  
  std::vector<RawPathPiece> rawPath;
};

struct PathfindingType {
  std::function<bool(const vertex_t*, const vertex_t*, const edge_t*)> predicate;
  
  std::vector<std::pair<vertex_t*, vertex_t*>> buffer;
  
  vertex_t* currentStart;
  vertex_t* currentGoal;
  RawPath currentPath;
  
  std::unordered_map<std::pair<vertex_t*, vertex_t*>, std::pair<vertex_t*, edge_t*>> history;
};

class PathFindingPhase {
public:
  virtual ~PathFindingPhase() {}
  
  virtual void registerPathType(const Type &type, const std::function<bool(const vertex_t*, const vertex_t*, const edge_t*)> &predicate) = 0;
  virtual void queueRequest(const Type &type, vertex_t* start, vertex_t* end);
  
  virtual void update() = 0;
  
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
