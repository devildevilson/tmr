#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <array>

#include <glm/glm.hpp>

#include <gheap.hpp>
#include <MemoryPool.h>

template <typename V, typename E>
struct StandartVertexNeighborCost;
template <typename V>
struct StandartVertexGoalCost;

template <typename V, typename E, typename VertexCost = StandartVertexNeighborCost<V, E>, typename VertexGoal = StandartVertexGoalCost<V>>
class graph_t;

template <typename V, typename E>
struct StandartVertexNeighborCost {
  float operator() (const V &first, const V &second, const E &edge) const {
    return edge;
  }
};

template <typename V>
struct StandartVertexGoalCost {
  float operator() (const V &vertex, const V &goal) const {
    return glm::distance(vertex, goal);
  }
};

template <typename V,
          typename E,
          typename VertexCost,
          typename VertexGoal>
class graph_t {
public:
  class vertex_t;
  class edge_t;

  struct node_t;

  typedef edge_t Edge;
  typedef vertex_t Vertex;
  typedef node_t SearchNode;
  typedef graph_t<V, E, VertexCost, VertexGoal> Graph;

  enum class SearchState {
    NotInitialised,
    Searching,
    Succeeded,
    Failed,
    Invalid
  };

  class edge_t {
    friend Graph;
  public:
    edge_t() {}
    ~edge_t() {}

    Vertex & first() const { return graph->verts[ends[0]]; }
    Vertex & second() const { return graph->verts[ends[1]]; }
    Vertex & operator[] (const uint8_t &index) const { return graph->verts[ends[index]]; }
    void swapSides() { size_t tmp = ends[0]; ends[0] = ends[1]; ends[1] = tmp; }
    constexpr uint8_t size() const { return 2; }
    bool operator==(const Edge &other) const { return this->graph == other.graph && this->graphIndex == other.graphIndex; }

    E& data() { return dataStorage; }

    bool isValid() const { return graph != nullptr && graphIndex != UINT64_MAX; }
    bool isActive() const { return active; }
    void setActive() { active = true; }
    void setInActive() { active = false; }
  private:
    bool active = false;
    Graph* graph = nullptr;
    size_t graphIndex = UINT64_MAX;
    std::array<size_t, 2> ends;

    E dataStorage;
  };

  //template <typename VV>
  class vertex_t {
    friend Graph;
  public:
    vertex_t() {}
    ~vertex_t() {}

    size_t degree() const { return edges.size(); }
    Edge & operator[] (const size_t &index) const { return graph->edgs[edges[index]]; }

    bool hasEdge(const Vertex &other, Edge &edge) const {
      for (const auto &index : edges) {
          const Edge &tmpEdge = graph->edge(index);
          if ((tmpEdge.first() == other) || (tmpEdge.second() == other)) {
              edge = tmpEdge;
              return true;
          }
      }

      return false;
    }

    bool hasEdge(const Vertex &other) const {
      for (const auto &index : edges) {
          const Edge &tmpEdge = graph->edge(index);
          if ((tmpEdge.first() == other) || (tmpEdge.second() == other)) return true;
      }

      return false;
    }

    bool operator==(const Vertex &other) const { return this->graph == other.graph && this->graphIndex == other.graphIndex; }

    V& data() { return dataStorage; }

    bool isValid() const { return graph != nullptr && graphIndex != UINT64_MAX; }
    bool isActive() const { return active; }
    void setActive() { active = true; }
    void setInActive() { active = false; }
  private:
    bool active = false;
    Graph* graph = nullptr;
    size_t graphIndex = UINT64_MAX;
    std::vector<size_t> edges;

    V dataStorage;
  };

  struct node_t {
    // нужны ли нам указатели на соседние элементы (vector по идее быстрее)
    //SearchNode *parent = nullptr; // used during the search to record the parent of successor nodes
    //SearchNode *child = nullptr; // used after the search for the application to view the search in reverse
    //size_t index = UINT64_MAX; // нужен ли нам индекс?
    bool isSolution = false;

    float g = 0.0f; // цена этой вершины + вершин что мы прошли
    float h = 0.0f; // примерная дальность до конечной цели
    float f = 0.0f; // g + h

    size_t edgeIndex = UINT64_MAX;
    Vertex vertex;
  };

  struct NodeMore {
    bool operator() (SearchNode* const& left, SearchNode* const& right) const {
        return left->f > right->f;
    }
  };

  graph_t() {}
  ~graph_t() {}

  bool null() const { return verts.empty(); }
  size_t order() const { return verts.size(); }

  bool empty() const { return edgs.empty(); }
  size_t size() const { return edgs.size(); }

  bool hasEdge(const Vertex &vert1, const Vertex &vert2, Edge &edge) const {
    for (const auto &index : vert1.edges) {
      if ((edgs[index].first() == vert2) || (edgs[index].second() == vert2)) {
        edge = edgs[index];
        return true;
      }
    }

    return false;
  }

  bool hasEdge(const size_t &vert1, const size_t &vert2, Edge &edge) const {
    for (const auto &index : verts[vert1].edges) {
      if ((edgs[index].first() == verts[vert2]) || (edgs[index].second() == verts[vert2])) {
        edge = edgs[index];
        return true;
      }
    }

    return false;
  }

  bool hasEdge(const Vertex &vert1, const Vertex &vert2) const {
    for (const auto &index : vert1.edges) {
      if ((edgs[index].first() == vert2) || (edgs[index].second() == vert2)) return true;
    }

    return false;
  }

  bool hasEdge(const size_t &vert1, const size_t &vert2) const {
    for (const auto &index : verts[vert1].edges) {
      if ((edgs[index].first() == verts[vert2]) || (edgs[index].second() == verts[vert2])) return true;
    }

    return false;
  }

  Edge & edge(const size_t &index) { return edgs[index]; }
  void swapSides(const size_t &index) { edgs[index].swapSides(); }
  constexpr uint8_t edgeSize() const { return 2; }

  Vertex & vertex(const size_t &index) { return verts[index]; }
  Vertex & first(const Edge &edge) { return edge.first(); }
  Vertex & second(const Edge &edge) { return edge.second(); }
  Vertex & first(const size_t &edge) { return edgs[edge].first(); }
  Vertex & second(const size_t &edge) { return edgs[edge].second(); }

  size_t addVertex(V data) {
    if (freeVerts.empty()) {
      verts.emplace_back();
      verts.back().graph = this;
      verts.back().graphIndex = verts.size()-1;
      verts.back().dataStorage = data;
      verts.back().setActive();

      return verts.size()-1;
    }

    size_t index = freeVerts.back();
    freeVerts.pop_back();
    verts[index].graph = this;
    verts[index].graphIndex = index;
    verts[index].dataStorage = data;
    verts[index].setActive();

    return index;
  }

  void deleteVertex(const size_t &index) {
    if (verts.size() <= index) return;

    for (const auto &index : verts[index].edges) deleteEdge(index);
    verts[index].graph = nullptr;
    verts[index].graphIndex = UINT64_MAX;
    //verts[index].dataPtr = nullptr;
    verts[index].edges.clear();
    verts[index].setInActive();

    freeVerts.push_back(index);
  }

  void deleteVertex(Vertex &vertex) {
      if (vertex.graph != this) return;

      size_t index = vertex.graphIndex;
      for (const auto &index : verts[index].edges) deleteEdge(index);
      vertex.graph = nullptr;
      vertex.graphIndex = UINT64_MAX;
      vertex.dataStorage = nullptr;
      vertex.edges.clear();
      vertex.setInActive();
      verts[index].graph = nullptr;
      verts[index].graphIndex = UINT64_MAX;
      verts[index].edges.clear();
      verts[index].setInActive();

      freeVerts.push_back(index);
  }

  void deleteAdjacent(const Edge &edge, const Vertex &vert) {
    if (!(edge.first() == vert) || !(edge.second() == vert)) return;

    size_t index = edge.first() == vert ? edge.ends[0] : edge.ends[1];

    for (const auto &index : verts[index].edges) deleteEdge(index);
    verts[index].graph = nullptr;
    verts[index].graphIndex = UINT64_MAX;
    verts[index].edges.clear();
    verts[index].setInActive();

    freeVerts.push_back(index);
  }

  size_t addEdge(Vertex &vert1, Vertex &vert2, E data) {
    if (!vert1.isValid() || !vert2.isValid()) return UINT64_MAX;
    if (vert1.graph != this || vert2.graph != this) return UINT64_MAX;

    if (freeEdgs.empty()) {
      edgs.emplace_back();
      edgs.back().graph = this;
      edgs.back().graphIndex = edgs.size()-1;
      edgs.back().ends[0] = vert1.graphIndex;
      edgs.back().ends[1] = vert2.graphIndex;
      size_t tmp1 = verts[vert1.graphIndex].edges.size();
      size_t tmp2 = verts[vert2.graphIndex].edges.size();
      verts[vert1.graphIndex].edges.push_back(edgs.size()-1);
      verts[vert2.graphIndex].edges.push_back(edgs.size()-1);
      if (vert1.edges.size() != tmp1) vert1.edges.push_back(edgs.size()-1);
      if (vert2.edges.size() != tmp2) vert2.edges.push_back(edgs.size()-1);
      edgs.back().dataStorage = data;
      edgs.back().setActive();

      return edgs.size()-1;
    }

    size_t index = freeEdgs.back();
    freeEdgs.pop_back();
    edgs[index].graph = this;
    edgs[index].graphIndex = index;
    edgs[index].ends[0] = vert1.graphIndex;
    edgs[index].ends[1] = vert2.graphIndex;
    size_t tmp1 = verts[vert1.graphIndex].edges.size();
    size_t tmp2 = verts[vert2.graphIndex].edges.size();
    verts[vert1.graphIndex].edges.push_back(index);
    verts[vert2.graphIndex].edges.push_back(index);
    if (vert1.edges.size() != tmp1) vert1.edges.push_back(index);
    if (vert2.edges.size() != tmp2) vert2.edges.push_back(index);
    edgs[index].dataStorage = data;
    edgs[index].setActive();

    return index;
  }

  size_t addEdge(const size_t &vert1, const size_t &vert2, E data) {
    if (verts.size() <= vert1 || verts.size() <= vert2) return UINT64_MAX;

    if (freeEdgs.empty()) {
      edgs.emplace_back();
      edgs.back().graph = this;
      edgs.back().graphIndex = edgs.size()-1;
      edgs.back().ends[0] = vert1;
      edgs.back().ends[1] = vert2;
      verts[vert1].edges.push_back(edgs.size()-1);
      verts[vert2].edges.push_back(edgs.size()-1);
      edgs.back().dataStorage = data;
      edgs.back().setActive();

      return edgs.size()-1;
    }

    size_t index = freeEdgs.back();
    freeEdgs.pop_back();
    edgs[index].graph = this;
    edgs[index].graphIndex = index;
    edgs[index].ends[0] = vert1;
    edgs[index].ends[1] = vert2;
    verts[vert1].edges.push_back(index);
    verts[vert2].edges.push_back(index);
    edgs[index].dataStorage = data;
    edgs[index].setActive();

    return index;
  }

  void deleteEdge(Vertex &vert1, Vertex &vert2) {
    if (!vert1.isValid() || !vert2.isValid()) return;
    if (vert1.graph != this || vert2.graph != this) return;
    Edge tmp;
    if (!hasEdge(vert1.graphIndex, vert2.graphIndex, tmp)) return;

    auto itr = verts[vert1.graphIndex].edges.begin();
    for (; itr != verts[vert1.graphIndex].edges.end(); itr++) if (*itr == tmp.graphIndex) break;
    verts[vert1.graphIndex].edges.erase(itr);

    itr = verts[vert2.graphIndex].edges.begin();
    for (; itr != verts[vert2.graphIndex].edges.end(); itr++) if (*itr == tmp.graphIndex) break;
    verts[vert2.graphIndex].edges.erase(itr);

    edgs[tmp.graphIndex].graph = nullptr;
    edgs[tmp.graphIndex].graphIndex = UINT64_MAX;
    edgs[tmp.graphIndex].ends[0] = UINT64_MAX;
    edgs[tmp.graphIndex].ends[1] = UINT64_MAX;
    edgs[tmp.graphIndex].setInActive();

    freeEdgs.push_back(tmp.graphIndex);
  }

  void deleteEdge(const size_t &vert1, const size_t &vert2) {
    Edge tmp;
    if (!hasEdge(vert1, vert2, tmp)) return;

    auto itr = verts[vert1].edges.begin();
    for (; itr != verts[vert1].edges.end(); itr++) if (*itr == tmp.graphIndex) break;
    verts[vert1].edges.erase(itr);

    itr = verts[vert2].edges.begin();
    for (; itr != verts[vert2].edges.end(); itr++) if (*itr == tmp.graphIndex) break;
    verts[vert2].edges.erase(itr);

    edgs[tmp.graphIndex].graph = nullptr;
    edgs[tmp.graphIndex].graphIndex = UINT64_MAX;
    edgs[tmp.graphIndex].ends[0] = UINT64_MAX;
    edgs[tmp.graphIndex].ends[1] = UINT64_MAX;
    edgs[tmp.graphIndex].setInActive();

    freeEdgs.push_back(tmp.graphIndex);
  }

  void deleteEdge(Edge &edge) {
    if (!edge.isValid()) return;
    if (edge.graph != this) return;

    auto itr = verts[edge.ends[0]].edges.begin();
    for (; itr != verts[edge.ends[0]].edges.end(); itr++) if (*itr == edge.graphIndex) break;
    verts[edge.ends[0]].edges.erase(itr);

    itr = verts[edge.ends[1]].edges.begin();
    for (; itr != verts[edge.ends[1]].edges.end(); itr++) if (*itr == edge.graphIndex) break;
    verts[edge.ends[1]].edges.erase(itr);

    edge.graph = nullptr;
    edge.graphIndex = UINT64_MAX;
    edge.ends[0] = UINT64_MAX;
    edge.ends[1] = UINT64_MAX;
    edge.dataStorage = nullptr;
    edge.setInActive();
    edgs[edge.graphIndex].graph = nullptr;
    edgs[edge.graphIndex].graphIndex = UINT64_MAX;
    edgs[edge.graphIndex].ends[0] = UINT64_MAX;
    edgs[edge.graphIndex].ends[1] = UINT64_MAX;
    edgs[edge.graphIndex].setInActive();

    freeEdgs.push_back(edge.graphIndex);
  }

  void deleteEdge(const size_t &edge) {
    if (edgs.size() <= edge) return;

    auto itr = verts[edgs[edge].ends[0]].edges.begin();
    for (; itr != verts[edgs[edge].ends[0]].edges.end(); itr++) if (*itr == edge) break;
    verts[edgs[edge].ends[0]].edges.erase(itr);

    itr = verts[edgs[edge].ends[1]].edges.begin();
    for (; itr != verts[edgs[edge].ends[1]].edges.end(); itr++) if (*itr == edge) break;
    verts[edgs[edge].ends[1]].edges.erase(itr);

    edgs[edge].graph = nullptr;
    edgs[edge].graphIndex = UINT64_MAX;
    edgs[edge].ends[0] = UINT64_MAX;
    edgs[edge].ends[1] = UINT64_MAX;
    edgs[edge].setInActive();

    freeEdgs.push_back(edge);
  }

  void setSearch(const size_t &startVert, const size_t &goalVert) {
    if (verts.size() <= startVert || verts.size() <= goalVert) return;
    if (state == SearchState::SEARCHING) clearSearch();

    cancel = false;

    state = SearchState::SEARCHING;

    start = pool.newElement();
    goal = pool.newElement();

    start->vertex = verts[startVert];
    goal->vertex = verts[goalVert];

    start->g = 0.0f;
    start->h = VertexGoal()(start->vertex.data(), goal->vertex.data());
    start->f = start->g + start->h;

    openList.push_back(start);
    // здесь должна быть сортировка, но я не понимаю зачем она нужна (?)
    // это не сортировка, а функция, превращающая vector в heap
    binary_heap::push_heap(openList.begin(), openList.end(), NodeMore());

    steps = 0;
    solution.clear();
  }

  void setSearch(const Vertex &startVert, const Vertex &goalVert) {
    if (!startVert.isValid() || !goalVert.isValid()) return;
    if (startVert.graph != this || goalVert.graph != this) return;
    if (state == SearchState::SEARCHING) clearSearch();

    cancel = false;

    state = SearchState::SEARCHING;

    start = pool.newElement();
    goal = pool.newElement();

    start->vertex = verts[startVert.graphIndex];
    goal->vertex = verts[goalVert.graphIndex];

    start->g = 0.0f;
    start->h = VertexGoal()(start->vertex.data(), goal->vertex.data());
    start->f = start->g + start->h;
    start->isSolution = true;

    openList.push_back(start);
    // здесь должна быть сортировка, но я не понимаю зачем она нужна (?)
    // это не сортировка, а функция, превращающая vector в heap
    binary_heap::push_heap(openList.begin(), openList.end(), NodeMore());

    steps = 0;
    solution.clear();
  }

  SearchState searchStep() {
    if (state == SearchState::NOT_INITIALISED || state == SearchState::INVALID) return state;
    if (state == SearchState::SUCCEEDED || state == SearchState::FAILED) return state;

    if (openList.empty() || cancel) {
      clearSearch();
      state = SearchState::FAILED;
      return state;
    }

    steps++;

    SearchNode* n = openList.front();
    // pop_heap - обычно элементы в векторе удаляются с конца,
    // pop_heap двигает удаляемый (самый большой/маленький по приоритету) в конец
    binary_heap::pop_heap(openList.begin(), openList.end(), NodeMore());
    openList.pop_back();

    if (n->vertex == goal->vertex) {
      //goal->g = n->g;
      //n->index = solution.size()-1;
      solution.push_back(n);

      if (!(n->vertex == start->vertex)) {
        // если n не равно start, то мы должны удалить n (?)
        // в той реализации обход начинается со старта
        // затем проходим весь ответ для заполнения указателей на детей
        // заполняю bool что это ответ

        for (size_t i = 0; i < solution.size(); i++) {
          solution[i]->isSolution = true;
        }
      }

      // удаляем неиспользованные ноды
      clearUnused();

      state = SearchState::SUCCEEDED;
      return state;
    } else {
      // теперь получаем все вершины соседние с текущей
      successors.clear();

      SearchNode* tmp;
      for (size_t i = 0; i < n->vertex.degree(); i++) {
        tmp = pool.newElement();
        if (!n->vertex[i].isValid() || !n->vertex[i].isActive()) continue;
        if (n->vertex[i].first() == n->vertex) tmp->vertex = n->vertex[i].second();
        else tmp->vertex = n->vertex[i].first();
        tmp->edgeIndex = n->vertex[i].graphIndex;

        successors.push_back(tmp);
      }

      for (size_t i = 0; i < successors.size(); i++) {
        float newg = n->g + VertexCost()(n->vertex.data(), successors[i]->vertex.data(), edge(successors[i]->edgeIndex).data());

        auto olItr = openList.begin();
        for (; olItr != openList.end(); olItr++) {
          if ((*olItr)->vertex == successors[i]->vertex) break;
        }

        if (olItr != openList.end()) {
          if ((*olItr)->g <= newg) {
            // удаляем текущий successors
            pool.deleteElement(successors[i]);
            continue;
          }
        }

        auto clItr = closedList.begin();
        for (; clItr != closedList.end(); clItr++) {
          if ((*clItr)->vertex == successors[i]->vertex) break;
        }

        if (clItr != closedList.end()) {
          if ((*clItr)->g <= newg) {
            // удаляем текущий successors
            pool.deleteElement(successors[i]);
            continue;
          }
        }

        successors[i]->g = newg;
        successors[i]->h = VertexGoal()(successors[i]->vertex.data(), goal->vertex.data());
        successors[i]->f = successors[i]->g + successors[i]->h;
        successors[i]->isSolution = true;
        solution.push_back(successors[i]);

        if (clItr != closedList.end()) {
          // удаляем нод на который указывает clItr
          pool.deleteElement(*clItr);
          closedList.erase(clItr); // также удаляем его из вектора
        }

        if (olItr != openList.end()) {
          // удаляем нод на который указывает olItr
          pool.deleteElement(*olItr);
          openList.erase(olItr); // также удаляем его из вектора

          // теперь ясно что и в std и gheap есть функции pop_heap, push_heap и make_heap
          // которые располагают объекты так как как они располагаются в куче (структура данных)
          // после удаления из openList нужно заново сделать кучу
          binary_heap::make_heap(openList.begin(), openList.end(), NodeMore());
        }

        openList.push_back(successors[i]);

        // push_heap - в вектор обычно добавляются элементы с конца,
        // поэтому push_heap берет последний элемент и работает с ним уже в куче
        binary_heap::push_heap(openList.begin(), openList.end(), NodeMore());
      }

      // обработали n можем запихать его в closedList
      closedList.push_back(n);
    }

    return state;
  }

  void cancelSearch() { cancel = true; }

  std::vector<SearchNode*> & getSolution() { return solution; }
  SearchNode* getStart() const { return start; }
  SearchNode* getGoal() const { return goal; }
  SearchNode* getCurrent() const { return current; }

  float getSolutionCost() const {
    if (solution.empty()) return -1.0f;

    return solution.back()->f;
  }

  std::vector<SearchNode*> & getOpenList() { return openList; }
  std::vector<SearchNode*> & getClosedList() { return closedList; }

  uint32_t getStepCount() const { return steps; }

  void clearCurrentSolution() {
    for (size_t i = 0; i < solution.size(); i++) {
      if (!solution[i]->isSolution) pool.deleteElement(solution[i]);
    }
    solution.clear();
  }

  void clearSolution(std::vector<SearchNode*> &solution) {
    for (size_t i = 0; i < solution.size(); i++) {
      if (!solution[i]->isSolution) pool.deleteElement(solution[i]);
    }
    solution.clear();
  }
private:
  typedef gheap<2, 1> binary_heap; // имеет смысл попробовать разную -арность у куч

  bool cancel = false;
  uint32_t steps = 0;
  SearchState state;

  SearchNode* start;
  SearchNode* current;
  SearchNode* goal;

  std::vector<Vertex> verts;
  std::vector<size_t> freeVerts;
  std::vector<Edge> edgs;
  std::vector<size_t> freeEdgs;

  std::vector<SearchNode*> openList; // heap
  std::vector<SearchNode*> closedList; // vector
  std::vector<SearchNode*> solution; // запишем ответ в вектор
  std::vector<SearchNode*> successors;

  MemoryPool<SearchNode> pool; // быстрое создание SearchNode

  void clearSearch() {
    // удаляем все ноды в openList
    for (size_t i = 0; i < openList.size(); i++) {
      pool.deleteElement(openList[i]);
    }
    openList.clear();

    // удаляем все ноды в closedList
    for (size_t i = 0; i < closedList.size(); i++) {
      pool.deleteElement(closedList[i]);
    }
    closedList.clear();

    // удаляем конечный нод
    pool.deleteElement(goal);
  }

  void clearUnused() {
    // удаляем все, кроме тех что входят в ответ, ноды в openList
    for (size_t i = 0; i < openList.size(); i++) {
      if (!openList[i]->isSolution) pool.deleteElement(openList[i]);
    }
    openList.clear();

    // удаляем все, кроме тех что входят в ответ, ноды в closedList
    for (size_t i = 0; i < closedList.size(); i++) {
      if (!closedList[i]->isSolution) pool.deleteElement(closedList[i]);
    }
    closedList.clear();

    // удаляем конечный нод, конкретный нод не спользуется нигде
    pool.deleteElement(goal);
  }
};

#endif
