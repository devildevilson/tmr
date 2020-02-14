#ifndef PATHFINDING_SYSTEM_H
#define PATHFINDING_SYSTEM_H

#include <functional>
#include <vector>
#include <mutex>
#include "id.h"
#include "Utility.h"
#include "MemoryPool.h"

namespace dt {
  class thread_pool;
}

namespace devils_engine {
  namespace utils {
    class astar_search;
  }
  
  namespace graph {
    struct edge;
    struct container;
  }
  
  namespace components {
    class vertex;
  }
  
  namespace path {
    enum class find_state {
      exist,
      not_exist,
      delayed
    };
    
    struct container {
      // этого достаточно для того чтобы описать путь
      // думаю что тут может еще пригодиться способ узнать вершину по которой мы сейчас проходим
      struct data {
        simd::vec4 pos;
        simd::vec4 dir;
      };
      
      std::vector<data> array;
    };
    
    struct raw {
      const components::vertex* vertex;
      const graph::edge* edge;
    };
    
    struct request {
      utils::id id;
      const components::vertex* start;
      const components::vertex* end;
    };
    
    struct response {
      find_state state;
      container* path;
      size_t counter;
    };
  }
  
  namespace systems {
    class pathfinder {
    public:
      struct type {
        struct queue_data {
          const components::vertex* start;
          const components::vertex* end;
          path::response responce;
        };
        
        utils::id id;
        float offset;
        std::function<bool(const components::vertex*, const components::vertex*, const graph::edge*)> predicate;
        std::mutex mutex;
        //std::vector<RawPath> foundPaths;
        
        std::vector<queue_data> queue;
      };
      
      struct create_info {
        dt::thread_pool* pool;
        graph::container* graph;
        std::function<float(const components::vertex*, const components::vertex*, const graph::edge*)> neighbor_cost;
        std::function<float(const components::vertex*, const components::vertex*)> goal_cost;
      };
      pathfinder(const create_info &info);
      ~pathfinder();
      
      void register_type(const utils::id &type, const std::function<bool(const components::vertex*, const components::vertex*, const graph::edge*)> &predicate, const float &offset);
      void queue_request(const path::request &request);
      
      void update();
      
      path::response get_path(const path::request &req);
      void release_path(const path::request &req);
    private:
      dt::thread_pool* pool;
  
      graph::container* graph;
      utils::astar_search* search;
      std::vector<utils::astar_search*> stack;
      std::mutex stack_mutex;
      
      std::vector<type> types;
      
      std::mutex pool_mutex;
      MemoryPool<path::container, sizeof(path::container)*50> paths_pool;
      
      void compute_funnel(const std::vector<path::raw> &raw_path, path::container* path, const float &offset);
      void correct_corner_points(const simd::vec4 &dir, const float &width, const float &offset, simd::vec4 &left, simd::vec4 &right);
      size_t find_type(const utils::id &id) const;
      size_t find_req(const std::vector<type::queue_data> &queue, const components::vertex* start, const components::vertex* end) const;
    };
  }
}

#endif
