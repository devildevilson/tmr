#include "graph.h"

#include <cstring>

namespace devils_engine {
  namespace utils {
    line_segment::line_segment() {}
    
    line_segment::line_segment(const simd::vec4 &point_a, const simd::vec4 &point_b) {
      const auto direction = point_b - point_a;
      const auto ndir = simd::normalize(direction);
      float arr[4];
      ndir.storeu(arr);
      point_a.storeu(p_a);
      memcpy(dir, arr, 3*sizeof(float));
      dist = simd::length(direction);
    }
    
    line_segment::line_segment(const simd::vec4 &point_a, const simd::vec4 &dir, const float &dist) : dist(dist) {
      const auto ndir = simd::normalize(dir);
      float arr[4];
      ndir.storeu(arr);
      point_a.storeu(p_a);
      memcpy(this->dir, arr, 3*sizeof(float));
    }
    
    simd::vec4 line_segment::point_a() const {
      return simd::vec4(p_a[0], p_a[1], p_a[2], p_a[3]);
    }
    
    simd::vec4 line_segment::point_b() const {
      const auto A = point_a();
      const auto D = direction();
      return A + D*distance();
    }
    
    simd::vec4 line_segment::direction() const {
      return simd::vec4(dir[0], dir[1], dir[2], 0.0f);
    }
    
    float line_segment::distance() const {
      return dist;
    }
    
    simd::vec4 line_segment::closest_point(const simd::vec4 &p) const {
      const auto A = point_a();
      const auto dir = direction() * distance();
      float t = simd::dot(p - A, dir) / simd::dot(dir, dir);
      t = glm::clamp(t, 0.0f, 1.0f);
      return A + t * dir;
    }
    
    void line_segment::left_right(const simd::vec4 &point, const simd::vec4 &normal, simd::vec4 &left, simd::vec4 &right) const {
      simd::vec4 simdVec = closest_point(point) - point;
      simdVec = simd::cross(simdVec, normal); //simd::normalize(
      
      //const simd::vec4 sA = point_a();
      //const simd::vec4 sB = point_b();
      const auto vec = direction() * distance();
      //const simd::vec4 simdDiff = sB - sA; // simd::normalize(
      float simdSign = simd::dot(simdVec, vec);
      
      if (simdSign >= 0.0f) {
        left = point_a();
        right = point_b();
      } else {
        left = point_b();
        right = point_a();
      }
    }
  }
  
  namespace graph {
    edge::edge() : length(0.0f), angle(0.0f), height(0.0f), flags(0) {}
    bool edge::is_fake() const { return (flags & fake) == fake; }
    bool edge::is_active() const { return (flags & active) == active; }
    void edge::set_active(const bool value) {
      flags = value ? flags | active : flags & ~active;
    }
    
    container::~container() {
      for (auto e : edges) {
        edges_pool.deleteElement(e);
      }
    }
    
    edge* container::create(const components::vertex* first, const components::vertex* second, const simd::vec4 &point_a, const simd::vec4 &point_b) {
      edge* e = edges_pool.newElement();
      edges.push_back(e);
      e->set_active(true);
      e->vertices = std::make_pair(first, second);
      e->seg = utils::line_segment(point_a, point_b);
      return e;
    }
    
    void container::destroy(edge* e) {
      for (size_t i = 0; i < edges.size(); ++i) {
        if (edges[i] == e) {
          edges_pool.deleteElement(edges[i]);
          std::swap(edges[i], edges.back());
          edges.pop_back();
          return;
        }
      }
    }
  }
}
