#ifndef YACS_H
#define YACS_H

#include <vector>
#include <unordered_map>
#include <functional>
#include <iostream>
#include <cstdint>
#include <cassert>

#include "MemoryPool.h"

namespace yacs {
  struct YACSType {
    YACSType() {}

    YACSType(const size_t &type) {
      yacsType = type;
    }

    size_t get() {
      return yacsType;
    }

    void set(const size_t &type) {
      yacsType = type;
    }

    size_t yacsType;
  };
}

#ifndef USER_DEFINED_COMPONENT_TYPE
  #define CLASS_TYPE_DECLARE static YACSType yacsType;
  #define CLASS_TYPE_DEFINE(name) YACSType name::yacsType(SIZE_MAX);
#endif

#ifndef YACS_UPDATE_TYPE
#define YACS_UPDATE_TYPE void* system = nullptr, float time = 0.0f
#define YACS_UPDATE_CALL system, time
#define YACS_SYSTEM_UPDATE float time = 0.0f
#define YACS_SYSTEM_UPDATE_CALL time
#endif

#ifndef YACS_DEFAULT_COMPONENTS_COUNT
#define YACS_DEFAULT_COMPONENTS_COUNT 100
#endif

#ifndef YACS_DEFAULT_ENTITY_COUNT
#define YACS_DEFAULT_ENTITY_COUNT 100
#endif

namespace yacs {
  class System;
  class Entity;
  class World;
  template<typename T>
  class ComponentHandle;

  class Component {
    friend Entity;
    friend World;
  public:
    Component() : ent(nullptr), id(SIZE_MAX), containerId(SIZE_MAX), allocatorId(SIZE_MAX) {}
    virtual ~Component() {}

    virtual void update(YACS_UPDATE_TYPE) = 0;
    virtual void init(void* userData) = 0;
    Entity* getEntity() const { return ent; }
  private:
    Entity* ent;
    size_t id;
    size_t containerId;
    size_t allocatorId;
//     size_t typeId;
  };
  
  class TypelessPool {
  public:
    TypelessPool(const size_t &typeId, const size_t &blockSize) : typeId(typeId), blockSize(blockSize), currentSize(0), memory(nullptr), freeSlots(nullptr) {
      allocateBlock();
    }
    
    TypelessPool(TypelessPool &&another) : typeId(another.typeId), blockSize(another.blockSize), currentSize(another.currentSize), memory(another.memory), freeSlots(another.freeSlots) {
      another.currentSize = 0;
      another.memory = 0;
      another.freeSlots = nullptr;
    }
    
    ~TypelessPool() {
      char* tmp = memory;
      while (tmp != nullptr) {
        char** ptr = reinterpret_cast<char**>(tmp);
        char* nextBuffer = ptr[0];
        
        delete [] tmp;
        tmp = nextBuffer;
      }
    }
    
    template<typename T, typename... Args>
    T* newElement(Args&&... args) {
      assert(T::yacsType.get() == typeId && "Wrong object type for typeless pool");
      
      T* ptr;
      
      if (freeSlots != nullptr) {
        ptr = reinterpret_cast<T*>(freeSlots);
        freeSlots = freeSlots->next;
      } else {
        if (currentSize + std::max(sizeof(T), sizeof(_Slot)) > blockSize) allocateBlock();
      
        ptr = reinterpret_cast<T*>(memory+sizeof(char*)+currentSize);
        currentSize += std::max(sizeof(T), sizeof(_Slot));
      }
      
      new (ptr) T(std::forward<Args>(args)...);
      
      return ptr;
    }
    
    template<typename T>
    void deleteElement(T* ptr) {
      if (ptr == nullptr) return;
      assert(T::yacsType.get() == typeId && "Wrong object type for typeless pool");
      
      ptr->~T();
      
      reinterpret_cast<_Slot*>(ptr)->next = freeSlots;
      freeSlots = reinterpret_cast<_Slot*>(ptr);
    }
    
    void deleteElement(const size_t &type, Component* ptr) {
      if (ptr == nullptr) return;
      assert(type == typeId && "Wrong object type for typeless pool");
      
      ptr->~Component();
      
      reinterpret_cast<_Slot*>(ptr)->next = freeSlots;
      freeSlots = reinterpret_cast<_Slot*>(ptr);
    }
    
    TypelessPool & operator=(const TypelessPool &pool) = delete;
    TypelessPool & operator=(TypelessPool &&pool) = delete;
  private:
    union _Slot {
      //char obj[typeSize];
      //char* obj;
      _Slot* next;
    };
    
    const size_t typeId;
//     const size_t typeSize;
    const size_t blockSize;
    size_t currentSize;
    char* memory;
    _Slot* freeSlots;
    
    void allocateBlock() {
      // мы создаем буфер размера size+sizeof(char*)
      const size_t newBufferSize = blockSize + sizeof(char*);
      char* newBuffer = new char[newBufferSize];
      
      // в датаСайз кладем sizeof(char*) так как это указатель на следующий массив
      currentSize = 0;
      
      // в первые sizeof(char*) байт кладем указатель
      char** tmp = reinterpret_cast<char**>(newBuffer);
      tmp[0] = memory;
      memory = newBuffer;
    }
  };

  class BaseEventSubscriber {
    friend World;
  public:
    BaseEventSubscriber() : id(0) {}
    virtual ~BaseEventSubscriber() {}
  private:
    size_t id;
  };

  template<typename T>
  class EventSubscriber : public BaseEventSubscriber {
  public:
    virtual ~EventSubscriber() {}

    virtual void receive(World* world, const T& event) = 0;
  };

  // вызывается после создания энтити
  struct OnEntityCreated {
    static YACSType yacsType;

    Entity* entity;
  };

  // вызывается непосредственно перед удалением энтити
  struct OnEntityDestroyed {
    static YACSType yacsType;

    Entity* entity;
  };

  // вызывается после создания компонента
  template<typename T>
  struct OnComponentAssigned {
    static YACSType yacsType;

    Entity* entity;
    ComponentHandle<T> component;
  };
  
  template<typename T>
  YACSType OnComponentAssigned<T>::yacsType(SIZE_MAX);

  class System {
  public:
    virtual ~System() {}

    virtual void init(World* world) = 0;
    virtual void deinit(World* world) = 0;
    virtual void update(World* world, YACS_SYSTEM_UPDATE) = 0;
  };

  template<typename T>
  class ComponentHandle {
  public:
    ComponentHandle(T* ptr) : ptr(ptr) {}
    bool isValid() const { return ptr != nullptr; }
    T* operator->() { return ptr; }
    T* get() { return ptr; }
  private:
    T* ptr;
  };

  class Entity {
    friend class World;
  public:
    Entity() : id(SIZE_MAX), world(nullptr) {}
    ~Entity() { clean(); }

    template<typename T, typename ...Args>
    ComponentHandle<T> assign(Args&&... args);

    template<typename TypeName, typename ParentName, typename ...Args>
    ComponentHandle<TypeName> assign(Args&&... args);

    template<typename T>
    void remove();

    template<typename T>
    void removeFromWorld();

    template<typename... Types>
    bool has() {
      return has({(Types::yacsType.get())...});
    }

    template<typename T>
    bool has() {
      if (components.find(T::yacsType.get()) != components.end()) return true;

      return false;
    }

    template<typename T>
    ComponentHandle<T> get() {
      auto itr = components.find(T::yacsType.get());
      if (itr == components.end()) return ComponentHandle<T>(nullptr);

      return ComponentHandle<T>(reinterpret_cast<T*>(itr->second));
    }

    Component* get(const size_t &type) {
      auto itr = components.find(type);
      if (itr == components.end()) return nullptr;

      return itr->second;
    }

    void init(void* userData) {
      for (auto &comp : components) {
        comp.second->init(userData);
      }
    }
    
    template<typename T>
    void init(void* userData) {
      auto itr = components.find(T::yacsType.get());
      if (itr == components.end()) return;

      itr->second->init(userData);
    }

    void clean();
    void removeComponentsFromWorld();

    size_t getId() { return id; }
  private:
    size_t id;
    World* world;
  protected:
    bool has(const std::initializer_list<size_t> &list) {
      for (const auto &elem : list) {
        if (components.find(elem) == components.end()) return false;
      }

      return true;
    }

    std::unordered_map<size_t, Component*> components;
  };

  class World {
    friend Entity;
  public:
    World() {}
    ~World() {
      clear();
    }

    template<typename T>
    void createAllocator(const size_t &allocatorSize) {
      assert(T::yacsType.get() == SIZE_MAX && "This type is already registered");
      const size_t index = createNewComponentType(allocatorSize);
      T::yacsType.set(index);
    }

    Entity* createEntity() {
      Entity* ent = entitiesPool.newElement();
      entities.push_back(ent);
      ent->world = this;
      ent->id = entities.size()-1;

      emit<OnEntityCreated>({ent});

      return ent;
    }

    void removeEntity(Entity* ent) {
      emit<OnEntityDestroyed>({ent});

//       ent->clean();
      entities.back()->id = ent->id;
      std::swap(entities[ent->id], entities.back());
      entities.pop_back();
      entitiesPool.deleteElement(ent);
    }

    // эти энтити удалятся при world->cleanup();
    void removeLater(Entity* ent) {
      emit<OnEntityDestroyed>({ent});

      ent->removeComponentsFromWorld();
      entities.back()->id = ent->id;
      std::swap(entities[ent->id], entities.back());
      entities.pop_back();
      toDelete.push_back(ent);
    }

    void cleanup() {
      for (auto* ent : toDelete) {
        entitiesPool.deleteElement(ent);
      }

      toDelete.clear();
    }

    template<typename T>
    void init(void* initData) {
      if (T::yacsType.get() >= components.size()) return;

      for (auto &comp : components[T::yacsType.get()]) {
        comp->init(initData);
      }
    }

    void initAll(void* initData) {
      for (auto &compType : components) {
        for (auto &comp : compType) {
          comp->init(initData);
        }
      }
    }

    void registerSystem(System* system) {
      systems.push_back(system);
      system->init(this);
    }

    template<typename T>
    void removeSystem(T* system) {
      for (size_t i = 0; i < systems.size(); ++i) {
        if (systems[i] == system) {
          systems[i]->deinit(this);
          std::swap(systems[i], systems.back());
          systems.pop_back();
          break;
        }
      }
    }

    template<typename T>
    void subscribe(EventSubscriber<T>* sub) {
      if (T::yacsType.get() >= subscribers.size()) {
        subscribers.emplace_back();
        T::yacsType.set(subscribers.size()-1);
      }

      subscribers[T::yacsType.get()].push_back(sub);
      sub->id = subscribers[T::yacsType.get()].size()-1;
    }

    template<typename T>
    void unsubscribe(EventSubscriber<T>* sub) {
      if (T::yacsType.get() >= subscribers.size()) return;
      if (subscribers[T::yacsType.get()].empty()) return;

      std::vector<BaseEventSubscriber*> &eventSubs = subscribers[T::yacsType.get()];
      eventSubs.back()->id = sub->id;
      std::swap(eventSubs[sub->id], eventSubs.back());
      eventSubs.pop_back();
    }

    template<typename T>
    void emit(const T& event) {
      if (T::yacsType.get() >= subscribers.size()) return;
      if (subscribers[T::yacsType.get()].empty()) return;

      std::vector<BaseEventSubscriber*> &eventSubs = subscribers[T::yacsType.get()];
      for (auto* base : eventSubs) {
        auto* sub = reinterpret_cast<EventSubscriber<T>*>(base);
        sub->receive(this, event);
      }
    }

    template<typename T>
    void update(YACS_UPDATE_TYPE) {
      if (T::yacsType.get() >= components.size()) return;

      auto& componentsType = components[T::yacsType.get()];
      for (auto & comp : componentsType) {
        comp->update(YACS_UPDATE_CALL);
      }
    }

    void update(YACS_SYSTEM_UPDATE) {
      for (auto &system : systems) {
        system->update(this, YACS_SYSTEM_UPDATE_CALL);
      }
    }

    template<typename T>
    void each(const typename std::common_type<std::function<void(Entity*, ComponentHandle<T>)>>::type &func) {
      std::vector<Component*> &comps = components[T::yacsType.get()];
      for (auto& comp : comps) {
        func(comp->getEntity(), ComponentHandle<T>(comp));
      }
    }

    template<typename ...Types>
    void each(const typename std::common_type<std::function<void(Entity*, ComponentHandle<Types>...)>>::type &func) {
      const std::initializer_list<size_t> list = {(Types::yacsType.get())...};
      size_t index = getMinSizeIndex(list);

      for (auto& comp : components[index]) {
        Entity* const& ent = comp->getEntity();
        if (ent->has(list)) func(ent, ent->get<Types>()...);
      }
    }
    
    size_t count() {
      return entities.size();
    }

    template<typename ...Types>
    size_t count() {
      const std::initializer_list<size_t> list = {(Types::yacsType.get())...};
      size_t index = getMinSizeIndex(list);

      size_t num = 0;
      for (auto& comp : components[index]) {
        Entity* const& ent = comp->getEntity();
        if (ent->has(list)) ++num;
      }

      return num;
    }
    
    void clear() {
//       std::cout << "Destroing world" << "\n";
      
      for (size_t i = 0; i < systems.size(); ++i) {
        systems[i]->deinit(this);
      }
      systems.clear();
      
      subscribers.clear();
      
      for (auto* entity : entities) {
        entitiesPool.deleteElement(entity);
      }
      entities.clear();
      
      components.clear();

      allocators.clear();
    }
  private:
    size_t allocateAllocator(const size_t &poolSize) {
      size_t index = allocators.size();
      allocators.emplace_back(index, poolSize);
      return index;
    }
    
    size_t createNewComponentType(const size_t &poolSize) {
      const size_t index = allocateAllocator(poolSize);
      assert(index == components.size() && "Allocator types count != components types count");
      components.emplace_back();
      return index;
    }

    template<typename T, typename ...Args>
    T* allocateComponent(Args&&... args) {
      if (T::yacsType.get() >= allocators.size()) {
        T::yacsType.set(createNewComponentType(sizeof(T)*YACS_DEFAULT_COMPONENTS_COUNT));
      }
      
      const size_t typeId = T::yacsType.get();
      T* ret = allocators[typeId].newElement<T>(std::forward<Args>(args)...);
      components[typeId].push_back(ret);
      ret->id = components[typeId].size()-1;
      ret->containerId = typeId;
      ret->allocatorId = typeId;

      return ret;
    }

    template<typename T, typename Parent, typename ...Args>
    T* allocateComponent(Args&&... args) {
      if (T::yacsType.get() >= allocators.size()) {
        T::yacsType.set(createNewComponentType(sizeof(T)*YACS_DEFAULT_COMPONENTS_COUNT));
      }
      
      if (Parent::yacsType.get() >= allocators.size()) {
        Parent::yacsType.set(createNewComponentType(sizeof(Parent)*YACS_DEFAULT_COMPONENTS_COUNT));
      }
      
      const size_t typeId = T::yacsType.get();
      const size_t parentTypeId = Parent::yacsType.get();
      T* ret = allocators[typeId].newElement<T>(std::forward<Args>(args)...);
      components[parentTypeId].push_back(ret);
      ret->id = components[parentTypeId].size()-1;
      ret->containerId = parentTypeId;
      ret->allocatorId = typeId;

      return ret;
    }

    template<typename T>
    void deallocateComponent(T* component) {
      if (T::yacsType.get() >= allocators.size()) return;

      if (component->id != SIZE_MAX) {
        std::vector<Component*> &container = components[component->containerId];
        container.back()->id = component->id;
        std::swap(container[component->id], container.back());
        container.pop_back();
      }

      allocators[T::yacsType.get()].deleteElement(component);
    }

    void deallocateComponent(const size_t &yacsType, Component* component) {
      (void)yacsType;
      //if (yacsType >= allocators.size()) return;

      if (component->id != SIZE_MAX) {
          std::vector<Component*> &container = components[component->containerId];
          container.back()->id = component->id;
          std::swap(container[component->id], container.back());
          container.pop_back();
      }

      const size_t allocType = component->allocatorId;
      allocators[allocType].deleteElement(allocType, component);
    }

    void removeComponent(const size_t &yacsType, Component* component) {
      if (yacsType >= components.size()) return;

      std::vector<Component*> &container = components[component->containerId];
      container.back()->id = component->id;
      std::swap(container[component->id], container.back());
      container.pop_back();

      component->id = SIZE_MAX;
    }

    size_t getMinSizeIndex(const std::initializer_list<size_t> &list) {
      size_t size = SIZE_MAX;
      size_t index = 0;
      for (const auto &elem : list) {
        if (size > this->components[elem].size()) {
          size = this->components[elem].size();
          index = elem;
        }
      }

      return index;
    }

    //std::vector<ComponentAlloc*> allocators;
    std::vector<TypelessPool> allocators;
    std::vector<std::vector<Component*>> components;
    
    MemoryPool<Entity, sizeof(Entity)*YACS_DEFAULT_ENTITY_COUNT> entitiesPool;
    std::vector<Entity*> entities;
    std::vector<Entity*> toDelete;
    
    std::vector<System*> systems;
    std::vector<std::vector<BaseEventSubscriber*>> subscribers;
  };
  
//     template<typename T>
//     void print(T arg) {
//       std::cout << " " << arg;
//     }
//     
//     template<typename T, typename ...Args>
//     void print(T arg, Args&&... args) {
//       std::cout << " " << arg;
//       
//       print(args...);
//     }
//     
//     template<> void print() {
//       std::cout << " ";
//     }

  template<typename T, typename ...Args>
  ComponentHandle<T> Entity::assign(Args&&... args) {
//       std::cout << "Allocating " << T::yacsType.name << "\n";
//       std::cout << "Args count: " << sizeof...(Args) << "\n";
    
    T* comp = world->allocateComponent<T>(std::forward<Args>(args)...);
    comp->ent = this;

    components[T::yacsType.get()] = comp;
    ComponentHandle<T> handle(comp);

//       std::cout << "before emit" << "\n";
    world->emit<OnComponentAssigned<T>>({this, handle});
//       std::cout << "after emit" << "\n";
    
//       std::cout << "Successfuly allocated " << T::yacsType.name << "\n";

    return handle;
  }

  template<typename TypeName, typename ParentName, typename ...Args>
  ComponentHandle<TypeName> Entity::assign(Args&&... args) {
    TypeName* comp = world->allocateComponent<TypeName, ParentName>(std::forward<Args>(args)...);
    comp->ent = this;

    components[ParentName::yacsType.get()] = comp;
    ComponentHandle<TypeName> handle(comp);
    
    world->emit<OnComponentAssigned<TypeName>>({this, handle});

    return handle;
  }

  template<typename T>
  void Entity::remove() {
    auto itr = components.find(T::yacsType.get());
    if (itr == components.end()) return;

    world->deallocateComponent<T>(itr->second);
    components.erase(itr);
  }

  template<typename T>
  void Entity::removeFromWorld() {
    auto itr = components.find(T::yacsType.get());
    if (itr == components.end()) return;

    world->removeComponent(itr->first, itr->second);
  }

  inline void Entity::clean() {
    for (auto &elem : components) {
      world->deallocateComponent(elem.first, elem.second);
    }

    components.clear();
  }

  inline void Entity::removeComponentsFromWorld() {
    for (auto &elem : components) {
      world->removeComponent(elem.first, elem.second);
    }
  }
}

#endif

#ifdef YACS_DEFINE_EVENT_TYPE    
  yacs::YACSType yacs::OnEntityCreated::yacsType(SIZE_MAX);
  yacs::YACSType yacs::OnEntityDestroyed::yacsType(SIZE_MAX);
#endif
