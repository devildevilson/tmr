#ifndef RESOURCE_CONTAINER_H
#define RESOURCE_CONTAINER_H

#include "MemoryPool.h"
#include <vector>
#include <unordered_map>

namespace devils_engine {
  namespace utils {
    template <typename IDType, typename T, size_t N>
    class resource_container_map {
    public:
      resource_container_map() {}
      ~resource_container_map() {
        for (auto ptr : resources) {
          resourcePool.deleteElement(ptr);
        }
      }
      
      template <typename... Args>
      const T* create(const IDType &id, Args&&... args) {
        auto itr = resources.find(id);
        if (itr != resources.end()) return nullptr;
        
        auto ptr = resourcePool.newElement(std::forward<Args>(args)...);
        resources[id] = ptr;
        return ptr;
      }
      
      bool destroy(const IDType &id) {
        auto itr = resources.find(id);
        if (itr == resources.end()) return false;
        
        resourcePool.deleteElement(itr->second);
        resources.erase(itr);
        return true;
      }
      
      const T* get(const IDType &id) const {
        auto itr = resources.find(id);
        if (itr != resources.end()) return itr->second;
        
        return nullptr;
      }
    private:
      MemoryPool<T, sizeof(T)*N> resourcePool;
      std::unordered_map<IDType, T*> resources;
    };

    template <typename IDType, typename T, size_t N>
    class resource_container_array {
    public:
      resource_container_array() {}
      ~resource_container_array() {
        for (auto & ptr : resources) {
          resourcePool.deleteElement(ptr.second);
        }
      }
      
      template <typename ...Args>
      T* create(const IDType &id, Args&&... args) {
        const size_t index = findResource(id);
        if (index != SIZE_MAX) return nullptr;
        
        auto ptr = resourcePool.newElement(std::forward<Args>(args)...);
        resources.push_back(std::make_pair(id, ptr));
        return ptr;
      }
      
      bool destroy(const IDType &id) {
        const size_t index = findResource(id);
        if (index == SIZE_MAX) return false;
        
        resourcePool.deleteElement(resources[index].second);
        std::swap(resources[index], resources.back());
        resources.pop_back();
        return true;
      }
      
      const T* get(const IDType &id) const {
        const size_t index = findResource(id);
        if (index != SIZE_MAX) return resources[index].second;
        
        return nullptr;
      }
      
      T* get(const IDType &id) {
        const size_t index = findResource(id);
        if (index != SIZE_MAX) return resources[index].second;
        
        return nullptr;
      }
      
      size_t size() const {
        return resources.size();
      }
      
      const T* at(const size_t &index) const {
        return index >= size() ? nullptr : resources[index].second;
      }
      
      T* at(const size_t &index) {
        return index >= size() ? nullptr : resources[index].second;
      }
    private:
      MemoryPool<T, sizeof(T)*N> resourcePool;
      std::vector<std::pair<IDType, T*>> resources;
      
      size_t findResource(const IDType &id) const {
        for (size_t i = 0; i < resources.size(); ++i) {
          if (resources[i].first == id) return i;
        }
        
        return SIZE_MAX;
      }
    };
  }
}

#endif
