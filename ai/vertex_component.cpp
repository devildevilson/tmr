#include "vertex_component.h"

#include "graph.h"

namespace devils_engine {
  namespace components {
    vertex::vertex(const create_info &info) : ent(info.ent) {
      info.center.storeu(center_var);
      info.normal.storeu(normal_var);
      normal_var[3] = 1.0f;
    }
    
    void vertex::add(const yacs::entity* ent) {
      std::unique_lock<std::mutex> lock(mutex);
      entities.push_back(ent);
    }
    
    void vertex::remove(const yacs::entity* ent) {
      std::unique_lock<std::mutex> lock(mutex);
      for (size_t i = 0; i < entities.size(); ++i) {
        if (entities[i] == ent) {
          std::swap(entities[i], entities.back());
          entities.pop_back();
          return;
        }
      }
    }
    
    void vertex::add_edge(graph::edge* edge) {
      edges.push_back(edge);
    }
    
    void vertex::remove_edge(graph::edge* edge) {
      for (size_t i = 0; i < edges.size(); ++i) {
        if (edges[i] == edge) {
          std::swap(edges[i], edges.back());
          edges.pop_back();
          return;
        }
      }
    }
    
    size_t vertex::degree() const {
      return edges.size();
    }
    
    size_t vertex::size() const {
      return entities.size();
    }
    
    const yacs::entity* vertex::next_entity(size_t &mem) const {
      if (mem >= entities.size()) return nullptr;
      const size_t index = mem++;
      return entities[index];
    }
    
    const graph::edge* vertex::next_edge(size_t &mem) const {
      if (mem >= edges.size()) return nullptr;
      const size_t index = mem++;
      return edges[index];
    }
    
    size_t vertex::has_edge(const vertex* vert) const {
      for (size_t i = 0; i < edges.size(); ++i) {
        if (edges[i]->vertices.first == vert || edges[i]->vertices.second == vert) return i;
      }
      
      return SIZE_MAX;
    }
    
    // нет особой необходимости каждый раз лезть за этими данными в физику
    simd::vec4 vertex::center() const {
      return simd::vec4(center_var[0], center_var[1], center_var[2], 1.0f);
    }
    
    simd::vec4 vertex::normal() const {
      return simd::vec4(normal_var[0], normal_var[1], normal_var[2], 0.0f);
    }
    
    float vertex::goal_distance_estimate(const vertex* goal) const {
      return simd::distance(center(), goal->center());
    }
    
    float vertex::cost(const vertex* successor) const {
      return simd::distance(center(), successor->center());
    }
    
    bool vertex::is_active() const {
      return normal_var[3] != 0.0f;
    }
    
    void vertex::set_active(const bool value) {
      normal_var[3] = value ? 1.0f : 0.0f;
    }
    
    yacs::entity* vertex::entity() const {
      return ent;
    }
  }
}
