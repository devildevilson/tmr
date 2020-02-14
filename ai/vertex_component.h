#ifndef VERTEX_COMPONENT_H
#define VERTEX_COMPONENT_H

#include <mutex>
#include <vector>
#include "Utility.h"

// как деактивировать вершины? можно декативировав все ребра
// но это долго, ну и проверка нужна именно на вершину

namespace yacs {
  class entity;
}

namespace devils_engine {
  namespace graph {
    struct edge;
  }
  
  namespace components {
    class vertex {
    public:
      struct create_info {
        yacs::entity* ent;
        simd::vec4 center;
        simd::vec4 normal;
      };
      vertex(const create_info &info);
      
      // по всей видимости этот метод должен использоваться в особом месте
      void add(const yacs::entity* ent);
      void remove(const yacs::entity* ent);
      
      void add_edge(graph::edge* edge); // только при загрузке уровня?
      void remove_edge(graph::edge* edge); // только в редакторе?
      
      size_t degree() const;
      size_t size() const;
      
      const yacs::entity* next_entity(size_t &mem) const;
      const graph::edge* next_edge(size_t &mem) const;
      
      size_t has_edge(const vertex* vert) const;
      
      // нет особой необходимости каждый раз лезть за этими данными в физику
      simd::vec4 center() const;
      simd::vec4 normal() const;
      
      float goal_distance_estimate(const vertex* nodeGoal) const;
      float cost(const vertex* successor) const;
      
      bool is_active() const;
      void set_active(const bool value); // тут тоже нужно гарантировать атомарность (хотя по идее присвоение 4 байта атомарная операция)
      
      yacs::entity* entity() const;
    private:
      yacs::entity* ent;
      float center_var[4];
      float normal_var[4];
      std::mutex mutex;
      std::vector<graph::edge*> edges; // const?
      std::vector<const yacs::entity*> entities;
    };
  }
}

#endif
