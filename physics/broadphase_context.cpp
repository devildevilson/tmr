#include "broadphase_context.h"

namespace devils_engine {
  namespace physics {
    namespace broadphase {
      octree_context::node::node() : free_index(UINT32_MAX), parent_index(UINT32_MAX), next_level_index(UINT32_MAX) {}
      uint32_t octree_context::node::add_proxy(const uint32_t &index) {
        std::unique_lock<std::mutex> lock(mutex);
        
        for (uint32_t i = 0; i < proxy_indices.size(); ++i) {
          if (proxy_indices[i] == UINT32_MAX) {
            proxy_indices[i] = index;
            return i;
          }
        }
        
        proxy_indices.push_back(index);
        return proxy_indices.size()-1;
      }
      
      void octree_context::node::remove_proxy(const uint32_t &array_index) {
        std::unique_lock<std::mutex> lock(mutex);
        
        ASSERT(array_index < proxy_indices.size());
        proxy_indices[array_index] = UINT32_MAX;
      }
      
      octree_context::proxy::flags::flags() : container(0) {}
      octree_context::proxy::flags::flags(const uint32_t &flags) : container(flags) {}
      
      bool octree_context::proxy::flags::static_object() const { return (container & STATIC_BODY) == STATIC_BODY; }
      bool octree_context::proxy::flags::need_updation() const { return (container & NEED_UPDATION) == NEED_UPDATION; }
      bool octree_context::proxy::flags::is_sleeping() const { return (container & SLEEPING_OBJECT) == SLEEPING_OBJECT; }
      
      void octree_context::proxy::flags::static_object(const bool value) { container = value ? container | STATIC_BODY : container & ~STATIC_BODY; }
      void octree_context::proxy::flags::need_updation(const bool value) { container = value ? container | NEED_UPDATION : container & ~NEED_UPDATION; }
      void octree_context::proxy::flags::sleeping(const bool value) { container = value ? container | SLEEPING_OBJECT : container & ~SLEEPING_OBJECT; }
      
      octree_context::proxy::proxy() : 
        object_index(UINT32_MAX),
        collision_group(0),
        collision_filter(0),
        collision_trigger(0),
        node_index(UINT32_MAX),
        node_array_index(UINT32_MAX),
        obj_type(UINT32_MAX)
      {}
      
      octree_context::proxy::proxy(const uint32_t &object_index, const uint32_t &collision_group, const uint32_t &collision_filter, const uint32_t &collision_trigger, const uint32_t &obj_type) :
        object_index(object_index),
        collision_group(collision_group),
        collision_filter(collision_filter),
        collision_trigger(collision_trigger),
        node_index(UINT32_MAX),
        node_array_index(UINT32_MAX),
        obj_type(obj_type)
      {}
      
      octree_context::pair::flags::flags() : container(0) {}
      octree_context::pair::flags::flags(const uint32_t &flags) : container(flags) {}
      
      bool octree_context::pair::flags::static_pair() const { return (container & STATIC_PAIR) == STATIC_PAIR; }
      bool octree_context::pair::flags::trigger_only() const { return (container & ONLY_TRIGGER_PAIR) == ONLY_TRIGGER_PAIR; }
      bool octree_context::pair::flags::not_unique() const { return (container & NOT_UNIQUE_PAIR) == NOT_UNIQUE_PAIR; }
      
      void octree_context::pair::flags::static_pair(const bool value) { container = value ? container | STATIC_PAIR : container & ~STATIC_PAIR; }
      void octree_context::pair::flags::trigger_only(const bool value) { container = value ? container | ONLY_TRIGGER_PAIR : container & ~ONLY_TRIGGER_PAIR; }
      void octree_context::pair::flags::not_unique(const bool value) { container = value ? container | NOT_UNIQUE_PAIR : container & ~NOT_UNIQUE_PAIR; }
      
      octree_context::octree_context(const create_info &info) : context(info.context), free_index(UINT32_MAX), pairs_count(0) {
        recreate(info);
      }
      
      basic_vec4 get_aabb_point(const basic_vec4 &center, const basic_vec4 &extents, const uint32_t &index) {
        basic_vec4 pos;
        pos.arr[0] = index & (1<<0) ? center.arr[0] + extents.arr[0] : center.arr[0] - extents.arr[0];
        pos.arr[1] = index & (1<<2) ? center.arr[1] + extents.arr[1] : center.arr[1] - extents.arr[1];
        pos.arr[2] = index & (1<<1) ? center.arr[2] + extents.arr[2] : center.arr[2] - extents.arr[2];
        pos.arr[3] = 1.0f;
        
        return pos;
      }
      
      void octree_context::recreate(const create_info &info) {
        size_t nodesCount = 0;
        size_t eight = 1;
        for (uint32_t i = 0; i < info.depth; ++i) {
          nodesCount += eight;
          eight *= 8;
        }
        
        {
          std::vector<node> new_nodes(nodesCount);
          std::swap(nodes, new_nodes);
//           nodes.update();
        }
        
        nodes[0].parent_index = UINT32_MAX;
        nodes[0].next_level_index = 1;
        nodes[0].box.center = info.center;
        nodes[0].box.extents = info.extents;
        
        std::vector<size_t> node_idx;
        node_idx.reserve(nodesCount);
        node_idx.push_back(0);
        size_t size = 1;
        size_t offset = 0;
        size_t mul = 1;
        size_t count = 1;
        size_t lastCount = 0;
        
        for (uint32_t i = 1; i < info.depth; ++i) {
          for (uint32_t j = 0; j < size - offset; ++j) {
            const size_t parent_index = node_idx[offset + j];
            nodes[parent_index].next_level_index = count + (parent_index - lastCount) * 8;
            
            for (uint8_t k = 0; k < 8; ++k) {
              const size_t index = count + (parent_index - lastCount) * 8 + k;
              const aabb &parent_box = nodes[parent_index].box;
              
              nodes[index].next_level_index = UINT32_MAX;

              simd::vec4 extent = parent_box.extents.get_simd() / 2.0f;
              
              basic_vec4 next_extents = extent;
              basic_vec4 next_center = get_aabb_point(parent_box.center, next_extents, k);

              nodes[index].box = aabb{next_center, next_extents};

              node_idx.push_back(index);
            }
          }
          
          offset = size;
          size = node_idx.size();
          mul *= 8;
          lastCount = count;
          count += mul;
        }
        
        ASSERT(proxies.size() == 0);
      }
      
      uint32_t octree_context::create_proxy(const uint32_t &object_index, const uint32_t &collision_group, const uint32_t &collision_filter, const uint32_t &collision_trigger, const uint32_t &obj_type) {
        uint32_t index = UINT32_MAX;
        
        if (free_index == UINT32_MAX) {
          index = proxies.size();
          proxies.array().emplace_back(object_index, collision_group, collision_filter, collision_trigger, obj_type);
          proxies.update();
        } else {
          index = free_index;
          free_index = proxies[index].obj_type;
          proxies[index] = proxy(object_index, collision_group, collision_filter, collision_trigger, obj_type);
        }
        
        return index;
      }
      
      void octree_context::destroy_proxy(const uint32_t &proxy_index) {
        ASSERT(proxy_index < proxies.size());
        proxies[proxy_index].object_index = UINT32_MAX;
        proxies[proxy_index].obj_type = free_index;
        free_index = proxy_index;
      }
    }
  }
}
