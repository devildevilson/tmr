#ifndef GRAPH_H
#define GRAPH_H

#include "Utility.h"
#include "MemoryPool.h"
#include <algorithm>
#include <vector>

namespace devils_engine {
  namespace components {
    class vertex;
  }
  
  namespace utils {
    struct line_segment {
      float p_a[4];
      float dir[3];
      float dist;
      
      line_segment();
      line_segment(const simd::vec4 &point_a, const simd::vec4 &point_b);
      line_segment(const simd::vec4 &point_a, const simd::vec4 &dir, const float &dist);
      simd::vec4 point_a() const;
      simd::vec4 point_b() const;
      simd::vec4 direction() const;
      float distance() const;
      simd::vec4 closest_point(const simd::vec4 &p) const;
      void left_right(const simd::vec4 &point, const simd::vec4 &normal, simd::vec4 &left, simd::vec4 &right) const;
    };
  }
  
  namespace graph {
    struct edge {
      enum f : uint32_t {
        fake = (1 << 0),
        active = (1 << 1)
      };
      
      std::pair<const components::vertex*, const components::vertex*> vertices;
      utils::line_segment seg;
      float length;
      float angle;
      float height;
      uint32_t flags;
      
      edge();
      bool is_fake() const;
      bool is_active() const;
      void set_active(const bool value);
    };
    
    struct container {
      // вершины? вершины в данном случае компоненты
      // создаются и хранятся они как компоненты
      // обход энтити будет производиться по вершинам
      // тут нужно лишь хранить ребра
      // причем скорее всего только хранить
      
      std::vector<edge*> edges;
      MemoryPool<edge, sizeof(edge)*100> edges_pool;
      
      ~container();
      edge* create(const components::vertex* first, const components::vertex* second, const simd::vec4 &point_a, const simd::vec4 &point_b);
      void destroy(edge* e);
    };
  }
}

#endif
