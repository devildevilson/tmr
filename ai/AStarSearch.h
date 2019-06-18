#ifndef A_STAR_SEARCH_H
#define A_STAR_SEARCH_H

#include <functional>
#include <vector>
#include <gheap.hpp>
#include "MemoryPool.h"

#define A_STAR_SEARCH_NODE_DEFAULT_SIZE 1000

class vertex_t;
class edge_t;

// короче мне нужно сделать поиск на несколько тредов
// для этого мне нужно создать несколько инстансов AStarSearch
class AStarSearch {
public:
  enum SearchState {
    SEARCH_STATE_NOT_INITIALISED,
    SEARCH_STATE_SEARCHING,
    SEARCH_STATE_SUCCEEDED,
    SEARCH_STATE_FAILED,
    SEARCH_STATE_OUT_OF_MEMORY,
    SEARCH_STATE_INVALID
  };
  
  struct Node {
    Node* parent; // used during the search to record the parent of successor nodes
    Node* child; // used after the search for the application to view the search in reverse
    
    float g; // cost of this node + it's predecessors
    float h; // heuristic estimate of distance to goal
    float f; // sum of cumulative cost of predecessors and self and heuristic
    
    const edge_t* edge;
    const vertex_t* vertex;
    
    Node() : parent(nullptr), child(nullptr), g(0.0f), h(0.0f), f(0.0f), edge(nullptr), vertex(nullptr) {}
    
    bool operator>(const Node* another) const {
      return this->f > another->f;
    }
  };
  
  struct NodeCompare {
    bool operator() (const Node* right, const Node* left) const {
      return right->f > left->f;
    }
  };
  
  AStarSearch();
  ~AStarSearch();
  
  void setVertexCostFunction(const std::function<float(const vertex_t*, const vertex_t*, const edge_t*)> &f);
  void setGoalCostFunction(const std::function<float(const vertex_t*, const vertex_t*)> &f);
  
  void cancelSearch();
  void setSearch(const vertex_t* startVert, const vertex_t* goalVert, const std::function<bool(const vertex_t*, const vertex_t*, const edge_t*)> &f);
  SearchState step();
  uint32_t getStepCount() const;
  
  void addSuccessor(const vertex_t* vert);
  void freeSolutionNodes();
  std::vector<Node*> getSolution();
  float getSolutionCost() const;
  
  Node* getSolutionStart() const;
  Node* getSolutionGoal() const;
  
  std::vector<Node*> & getOpenList();
  std::vector<Node*> & getClosedList();
private:
  typedef gheap<2, 1> binary_heap; // имеет смысл попробовать разную -арность у куч
  
  bool cancel;
  SearchState state;
  uint32_t steps;
  
  Node* start;
  Node* current;
  Node* goal;
  
  std::vector<Node*> openList; // heap
  std::vector<Node*> closedList; // vector
//   std::vector<Node*> solution; // запишем ответ в вектор
  std::vector<Node*> successors;
  MemoryPool<Node, A_STAR_SEARCH_NODE_DEFAULT_SIZE*sizeof(Node)> nodePool;
  
  std::function<float(const vertex_t*, const vertex_t*, const edge_t*)> neighborCost;
  std::function<float(const vertex_t*, const vertex_t*)> goalCost;
  std::function<bool(const vertex_t*, const vertex_t*, const edge_t*)> predicate;
  
  void freeAll();
  void freeUnused();
};

class AStarState {
public:
  virtual ~AStarState() {}
  
  virtual float goalDistanceEstimate(const vertex_t* nodeGoal) const = 0; // Heuristic function which computes the estimated cost to the goal node
//   virtual bool isGoal(const vertex_t* nodeGoal) const = 0; // Returns true if this node is the goal node
//   virtual bool getSuccessors(AStarSearch* astarSearch, const vertex_t* parentNode) const = 0; // Retrieves all successors to this node and adds them via astarsearch.addSuccessor()
  virtual float getCost(const vertex_t* successor) const = 0; // Computes the cost of travelling from this node to the successor node
//   virtual bool isSameState(const vertex_t* rhs) const = 0; // Returns true if this node is the same as the rhs node
};

#endif
