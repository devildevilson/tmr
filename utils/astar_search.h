#ifndef ASTAR_SEARCH_H
#define ASTAR_SEARCH_H

#include <vector>
#include <functional>
#include <gheap.hpp>
#include "MemoryPool.h"

#define ASTAR_SEARCH_NODE_DEFAULT_SIZE 300

namespace devils_engine {
  namespace components {
    class vertex;
  }
  
  namespace graph {
    struct edge;
  }
  
  namespace utils {
    class astar_search {
    public:
      using vertex_cost_f = std::function<float(const components::vertex*, const components::vertex*, const graph::edge*)>;
      using goal_cost_f = std::function<float(const components::vertex*, const components::vertex*)>;
      using predicate_f = std::function<bool(const components::vertex*, const components::vertex*, const graph::edge*)>;
      
      enum class state {
        not_initialised,
        searching,
        succeeded,
        failed,
        out_of_memory,
        invalid
      };
      
      struct node {
        node* parent; // used during the search to record the parent of successor nodes
        node* child; // used after the search for the application to view the search in reverse
        
        float g; // cost of this node + it's predecessors
        float h; // heuristic estimate of distance to goal
        float f; // sum of cumulative cost of predecessors and self and heuristic
        
        const graph::edge* edge;
        const components::vertex* vertex;
        
        node() : parent(nullptr), child(nullptr), g(0.0f), h(0.0f), f(0.0f), edge(nullptr), vertex(nullptr) {}
      };
      
      astar_search();
      ~astar_search();
      
      void set_vertex_cost_f(const vertex_cost_f &f);
      void set_goal_cost_f(const goal_cost_f &f);
      
      void cancel();
      void set(const components::vertex* startVert, const components::vertex* goalVert, const predicate_f &f);
      state step();
      uint32_t step_count() const;
      
      void add_successor(const components::vertex* vert);
      void free_solution();
      std::vector<node*> solution();
      float solution_cost() const;
      
      node* solution_start() const;
      node* solution_goal() const;
      
      std::vector<node*> & open_list();
      std::vector<node*> & closed_list();
    private:
      typedef gheap<2, 1> binary_heap; // имеет смысл попробовать разную -арность у куч
  
      bool canceled;
      state current_state;
      uint32_t steps;
      
      node* start;
      node* current;
      node* goal;
      
      std::vector<node*> openlist; // heap
      std::vector<node*> closedlist; // vector
    //   std::vector<Node*> solution; // запишем ответ в вектор
      std::vector<node*> successors;
      MemoryPool<node, ASTAR_SEARCH_NODE_DEFAULT_SIZE*sizeof(node)> node_pool;
      
      vertex_cost_f neighbor_cost;
      goal_cost_f goal_cost;
      predicate_f predicate;
      
      void free_all();
      void free_unused();
    };
  }
}

#endif
