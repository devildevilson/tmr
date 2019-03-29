#include "AStarSearch.h"

#include "Graph.h"

#ifdef _DEBUG
  #include <cassert>
  #define ASSERT(expr) assert(expr)
#else
  #define ASSERT(expr)
#endif

AStarSearch::AStarSearch() 
  : cancel(false), 
    state(SEARCH_STATE_NOT_INITIALISED), 
    steps(0), 
    start(nullptr), 
    current(nullptr),
    goal(nullptr) {}

AStarSearch::~AStarSearch() {
  freeAll();
//   freeUnused();
}

void AStarSearch::setVertexCostFunction(const std::function<float(const vertex_t*, const vertex_t*, const edge_t*)> &f) {
  neighborCost = f;
}

void AStarSearch::setGoalCostFunction(const std::function<float(const vertex_t*, const vertex_t*)> &f) {
  goalCost = f;
}

void AStarSearch::cancelSearch() {
  cancel = true;
}

void AStarSearch::setSearch(const vertex_t* startVert, const vertex_t* goalVert, const std::function<bool(const vertex_t*, const vertex_t*, const edge_t*)> &f) {
  cancel = false;
  
  predicate = f;
  
  start = nodePool.newElement();
  goal = nodePool.newElement();
  
  start->vertex = startVert;
  goal->vertex = goalVert;
  
  state = SEARCH_STATE_SEARCHING;
  
  start->g = 0.0f;
  start->h = start->vertex->goalDistanceEstimate(goal->vertex);
  start->f = start->g + start->h;
  
  openList.push_back(start);
  binary_heap::push_heap(openList.begin(), openList.end(), NodeCompare());
  
  steps = 0;
}

AStarSearch::SearchState AStarSearch::step() {
  ASSERT(state > SEARCH_STATE_NOT_INITIALISED && state < SEARCH_STATE_INVALID && "Search is not initialized");
  
  // Next I want it to be safe to do a searchstep once the search has succeeded...
  if (state == SEARCH_STATE_SUCCEEDED || state == SEARCH_STATE_FAILED) return state;

  // Failure is defined as emptying the open list as there is nothing left to 
  // search...
  // New: Allow user abort
  if (openList.empty() || cancel) {
    freeAll();
    state = SEARCH_STATE_FAILED;
    return state;
  }
  
  // Incremement step count
  ++steps;

  // Pop the best node (the one with the lowest f) 
  Node *n = openList.front(); // get pointer to the node
  binary_heap::pop_heap(openList.begin(), openList.end(), NodeCompare());
  openList.pop_back();

  // Check for the goal, once we pop that we're done
  //if (n->vertex->isGoal(goal->vertex)) {
  if (n->vertex == goal->vertex) {
    // The user is going to use the Goal Node he passed in 
    // so copy the parent pointer of n 
    goal->parent = n->parent;
    goal->g = n->g;

    // A special case is that the goal was passed in as the start state
    // so handle that here
    //if (!n->vertex->isSameState(start->vertex)) {
    if (n->vertex != start->vertex) {
      nodePool.deleteElement(n);

      // set the child pointers in each node (except Goal which has no child)
      Node *nodeChild = goal;
      Node *nodeParent = goal->parent;

      do {
        nodeParent->child = nodeChild;

        nodeChild = nodeParent;
        nodeParent = nodeParent->parent;
      
      } while (nodeChild != start); // Start is always the first node by definition
    }

    // delete nodes that aren't needed for the solution
    freeUnused();

    state = SEARCH_STATE_SUCCEEDED;

    return state;
  } else { // not goal

    // We now need to generate the successors of this node
    // The user helps us to do this, and we keep the new nodes in
    // m_Successors ...

    successors.clear(); // empty vector of successor nodes to n

    // User provides this functions and uses AddSuccessor to add each successor of
    // node 'n' to m_Successors
    for (size_t i = 0; i < n->vertex->degree(); ++i) {
      edge_t* edge = n->vertex->edge(i);
      vertex_t* vertex = edge->first() == n->vertex ? edge->second() : edge->first();
      
      if (!n->vertex[i].isValid() || !n->vertex[i].isActive()) continue;
      if (!predicate(n->vertex, vertex, edge)) continue;
      
      Node* tmp = nodePool.newElement();
      
      tmp->edge = edge;
      tmp->vertex = vertex;
      
      successors.push_back(tmp);
    }

    if (n->vertex->degree() == 0) {
      // free the nodes that may previously have been added 
      for (auto successor = successors.begin(); successor != successors.end(); ++successor) {
        nodePool.deleteElement((*successor));
      }

      successors.clear(); // empty vector of successor nodes to n

      // free up everything else we allocated
      nodePool.deleteElement(n);
      freeAll();

      state = SEARCH_STATE_OUT_OF_MEMORY;
      return state;
    }
    
    // Now handle each successor to the current node ...
    for (auto successor = successors.begin(); successor != successors.end(); ++successor){
      // The g value for this successor ...
      const float newg = n->g + n->vertex->getCost((*successor)->vertex);

      // Now we need to find whether the node is on the open or closed lists
      // If it is but the node that is already on them is better (lower g)
      // then we can forget about this successor

      // First linear search of open list to find node
      auto openlist_result = openList.begin();
      for (; openlist_result != openList.end(); ++openlist_result) {
        //if ((*openlist_result)->vertex->isSameState((*successor)->vertex)) break;
        if ((*openlist_result)->vertex == (*successor)->vertex) break;
      }

      if (openlist_result != openList.end()) {
        // we found this state on open

        if ((*openlist_result)->g <= newg) {
          nodePool.deleteElement(*successor);

          // the one on Open is cheaper than this one
          continue;
        }
      }

      auto closedlist_result = closedList.begin();
      for (; closedlist_result != closedList.end(); ++closedlist_result) {
        //if ((*closedlist_result)->vertex->isSameState((*successor)->vertex)) break;
        if ((*closedlist_result)->vertex == (*successor)->vertex) break;
      }

      if (closedlist_result != closedList.end()) {
        // we found this state on closed

        if ((*closedlist_result)->g <= newg) {
          // the one on Closed is cheaper than this one
          nodePool.deleteElement(*successor);

          continue;
        }
      }

      // This node is the best node so far with this particular state
      // so lets keep it and set up its AStar specific data ...

      (*successor)->parent = n;
      (*successor)->g = newg;
      (*successor)->h = (*successor)->vertex->goalDistanceEstimate(goal->vertex);
      (*successor)->f = (*successor)->g + (*successor)->h;

      // Successor in closed list
      // 1 - Update old version of this node in closed list
      // 2 - Move it from closed to open list
      // 3 - Sort heap again in open list

      if (closedlist_result != closedList.end()) {
        // Update closed node with successor node AStar data
        //*(*closedlist_result) = *(*successor);
        
        (*closedlist_result)->parent = (*successor)->parent;
        (*closedlist_result)->g      = (*successor)->g;
        (*closedlist_result)->h      = (*successor)->h;
        (*closedlist_result)->f      = (*successor)->f;

        // Free successor node
        nodePool.deleteElement(*successor);

        // Push closed node into open list 
        openList.push_back(*closedlist_result);

        // Remove closed node from closed list
        //closedList.erase(closedlist_result);
        std::swap(*closedlist_result, closedList.back());
        closedList.pop_back();

        // Sort back element into heap
        binary_heap::push_heap(openList.begin(), openList.end(), NodeCompare());

        // Fix thanks to ...
        // Greg Douglas <gregdouglasmail@gmail.com>
        // who noticed that this code path was incorrect
        // Here we have found a new state which is already CLOSED
      } else if (openlist_result != openList.end()) {
        // Successor in open list
        // 1 - Update old version of this node in open list
        // 2 - sort heap again in open list
        
        // Update open node with successor node AStar data
        //*(*openlist_result) = *(*successor);
        (*openlist_result)->parent = (*successor)->parent;
        (*openlist_result)->g      = (*successor)->g;
        (*openlist_result)->h      = (*successor)->h;
        (*openlist_result)->f      = (*successor)->f;

        // Free successor node
        nodePool.deleteElement(*successor);

        // re-make the heap 
        // make_heap rather than sort_heap is an essential bug fix
        // thanks to Mike Ryynanen for pointing this out and then explaining
        // it in detail. sort_heap called on an invalid heap does not work
        binary_heap::make_heap(openList.begin(), openList.end(), NodeCompare());
      } else {
        // New successor
        // 1 - Move it from successors to open list
        // 2 - sort heap again in open list
        
        // Push successor node into open list
        openList.push_back(*successor);

        // Sort back element into heap
        binary_heap::push_heap(openList.begin(), openList.end(), NodeCompare());
      }
    }

    // push n onto Closed, as we have expanded it now
    closedList.push_back(n);

  } // end else (not goal so expand)

  return state; // Succeeded bool is false at this point.
}

uint32_t AStarSearch::getStepCount() const {
  return steps;
}

void AStarSearch::addSuccessor(const vertex_t* vert) {
  Node* node = nodePool.newElement();
  node->vertex = vert;
  successors.push_back(node);
}

void AStarSearch::freeSolutionNodes() {
  Node* n = start;
  
  if (start->child != nullptr) {
    do {
      Node *del = n;
      n = n->child;
      nodePool.deleteElement(del);
      
      del = nullptr;
    } while (n != goal);

    nodePool.deleteElement(n); // Delete the goal
  } else {
    nodePool.deleteElement(start);
    nodePool.deleteElement(goal);
  }
  
  start = nullptr;
  goal = nullptr;
}

std::vector<AStarSearch::Node*> AStarSearch::getSolution() {
  std::vector<AStarSearch::Node*> sol;
  
  Node* n = start;
  sol.push_back(n);
  while (n != goal) {
    n = n->child;
    sol.push_back(n);
  }
  
  return sol;
}

float AStarSearch::getSolutionCost() const {
  if (goal != nullptr && state == SEARCH_STATE_SUCCEEDED) {
    return goal->g;
  } else {
    return -1.0f;
  }
}

AStarSearch::Node* AStarSearch::getSolutionStart() const {
  return start;
}

AStarSearch::Node* AStarSearch::getSolutionGoal() const {
  return goal;
}

std::vector<AStarSearch::Node*> & AStarSearch::getOpenList() {
  return openList;
}

std::vector<AStarSearch::Node*> & AStarSearch::getClosedList() {
  return closedList;
}

void AStarSearch::freeAll() {
  for (size_t i = 0; i < openList.size(); ++i) {
    nodePool.deleteElement(openList[i]);
  }
  openList.clear();
  
  for (size_t i = 0; i < closedList.size(); ++i) {
    nodePool.deleteElement(closedList[i]);
  }
  closedList.clear();
  
  nodePool.deleteElement(start);
  
  start = nullptr;
  goal = nullptr;
}

void AStarSearch::freeUnused() {
  for (size_t i = 0; i < openList.size(); ++i) {
    if (openList[i]->child == nullptr) nodePool.deleteElement(openList[i]);
  }
  openList.clear();
  
  for (size_t i = 0; i < closedList.size(); ++i) {
    if (closedList[i]->child == nullptr) nodePool.deleteElement(closedList[i]);
  }
  closedList.clear();
}
