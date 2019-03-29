#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <array>
#include <functional>

#include "Utility.h"
#include "AStarSearch.h"

#include <gheap.hpp>
#include <MemoryPool.h>

#include <unordered_map>

#define GRAPH_VERTEX_POOL_DEFAULT_SIZE 1000
#define GRAPH_EDGE_POOL_DEFAULT_SIZE 3000

class Graph;
class vertex_t;

namespace std {
  template<>
  struct hash<std::pair<uint64_t, uint64_t>> {
    size_t operator() (const std::pair<uint64_t, uint64_t> &pair) const {
      return (pair.first + pair.second) * (pair.first + pair.second + 1) / 2 + pair.second;
    }
  };
}

struct LineSegment {
  glm::vec4 a;
  glm::vec4 b;
  
  LineSegment();
  LineSegment(const glm::vec4 &a, const glm::vec4 &b);
  
  float length() const;
  glm::vec4 dir() const;
  
};

struct EdgeData {
  bool isFakeEdge;
  bool pointA;
  float angle;
  float distance;
  float width;
  float height; // for fake edges only
  glm::vec4 segDir; // необходимо обязательно чекнуть на знак
  LineSegment segment;
};

struct Vertex {
  // тут по идее нужно указатель на физический компонент
  // для того чтобы быстро получать разные данные
  // но с другой стороны что мне реально может потребоваться?
  
  // 100% нормаль
  glm::vec4 normal;
  
  // здесь должна быть какая-то точка, по которой мы будем определять дистанцию
  // проблема в том что это скорее всего будет центральная точка плоскости
  // а это не даст нам 100% качественный результат поиска пути
  // можно ли с этим что-то сделать? нужно наверное будет сменить подход
  // пока что это работает сносно
  glm::vec4 center;
  
  // массив всех объектов которые находятся на этой вершине
  // можно ли, а главное нужно ли тут делать так чтобы 
  // объекты находились в уже созданной памяти? (оптимизация тут нужна?)
  // думаю что не особо
  
  // сюда будут входить только ИИ компоненты? видимо да
  std::vector<void*> objects;
};

// что делать с декоративными объектами, которые будут мешать пройти?
// нужно как-то верно на них среагировать движущемуся объекту
// в принципе, такие объекты должны по идее быть добавлены с минимальным набором данных
// для того чтобы верно определить как их обойти
// видимо для деревьев придется создавать AIBasicComponent

class edge_t {
  friend Graph;
public:
  edge_t(Graph* graph, size_t graphIndex, const EdgeData &data);
  ~edge_t();
  
  vertex_t* first() const;
  vertex_t* second() const;
  vertex_t* operator[] (const uint8_t &index) const;
  void swapSides();
  constexpr uint8_t size() const { return 2; }
  
  void setFirst(vertex_t* ptr);
  void setSecond(vertex_t* ptr);
  void set(const uint8_t &index, vertex_t* ptr);
  
  bool isFake() const;
  float getDistance() const;
  float getWidth() const;
  float getAngle() const;
  float getHeight() const;
  glm::vec4 getDir() const;
  LineSegment getSegment() const;

  bool isValid() const;
  bool isActive() const;
  void setActive(const bool active);
  
  size_t internalIndex() const;
private:
  bool active;
  Graph* graph;
  size_t graphIndex;
  std::array<vertex_t*, 2> ends;
  
  //void* dataPtr = nullptr;
  EdgeData data;
};

class vertex_t : public AStarState {
  friend Graph;
public:
  vertex_t(Graph* graph, size_t graphIndex, const Vertex &data);
  ~vertex_t();
  
  size_t degree() const;
  edge_t* operator[] (const size_t &index) const;
  edge_t* edge(const size_t &index) const;
  bool hasEdge(const vertex_t* other, edge_t** edge) const;
  bool hasEdge(const vertex_t* other) const;
  vertex_t* neighbor(const size_t &index) const;
  
  void addEdge(edge_t* edge);
  void removeEdge(edge_t* edge);
  
  Vertex getVertexData() const;
  float getAngle(const vertex_t* other) const;
  
  float goalDistanceEstimate(const vertex_t* nodeGoal) const override;
  //bool isGoal(const vertex_t* nodeGoal) const override;
  //bool getSuccessors(AStarSearch* astarSearch, const vertex_t* parentNode) const override;
  float getCost(const vertex_t* successor) const override;
  //bool isSameState(const vertex_t* rhs) const override;

  bool isValid() const;
  bool isActive() const;
  void setActive(const bool active);
  
  size_t internalIndex() const;
private:
  bool active;
  Graph* graph;
  size_t graphIndex;
  std::vector<edge_t*> edges;
  
  Vertex vertex;
};

class Graph {
public:
  Graph();
  ~Graph();

  bool null() const;
  size_t order() const;

  bool empty() const;
  size_t size() const;
  
  edge_t* edge(const size_t &index);
  const edge_t* edge(const size_t &index) const;

  vertex_t* vertex(const size_t &index);
  const vertex_t* vertex(const size_t &index) const;
  size_t getIndex(const vertex_t* vert) const;

  vertex_t* addVertex(const Vertex &data);
  void deleteVertex(vertex_t* vertex);
  void deleteAdjacent(const edge_t* edge, const vertex_t* vert);

  edge_t* addEdge(vertex_t* vert1, vertex_t* vert2, const EdgeData &data);
  void deleteEdge(vertex_t* vert1, vertex_t* vert2);
  void deleteEdge(edge_t* edge);
private:
  std::vector<vertex_t*> verts;
  std::vector<edge_t*> edgs;
  
  MemoryPool<vertex_t, GRAPH_VERTEX_POOL_DEFAULT_SIZE*sizeof(vertex_t)> vertexPool;
  MemoryPool<edge_t, GRAPH_EDGE_POOL_DEFAULT_SIZE*sizeof(edge_t)> edgePool;
};

#endif

