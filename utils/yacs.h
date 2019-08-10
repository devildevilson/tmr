#ifndef YACS_H
#define YACS_H

#include <vector>
#include <unordered_map>
#include <functional>
#include <cstdint>

#include "yacs_component.h"
#include "yacs_pool.h"

#ifndef YACS_UPDATE_TYPE
#define YACS_UPDATE_TYPE void* system = nullptr, float time = 0.0f
#define YACS_UPDATE_CALL system, time
#define YACS_SYSTEM_UPDATE float time = 0.0f
#define YACS_SYSTEM_FUNC float time
#define YACS_SYSTEM_UPDATE_CALL time
#endif

#ifndef YACS_DEFAULT_COMPONENTS_COUNT
#define YACS_DEFAULT_COMPONENTS_COUNT 100
#endif

#ifndef YACS_DEFAULT_ENTITY_COUNT
#define YACS_DEFAULT_ENTITY_COUNT 100
#endif

// однопоточная версия

namespace yacs {
  class system {
  public:
    virtual ~system() = default;

    virtual void update(YACS_SYSTEM_UPDATE) = 0;
  };

  class world;

  class subscriber_base {
  public:
    subscriber_base() : id(0) {}
    virtual ~subscriber_base() = default;

    size_t & index() { return id; }
  private:
    size_t id;
  };

  template<typename T>
  class event_subscriber : public subscriber_base {
  public:
    ~event_subscriber() = default;

    virtual void receive(world* world, const T& event) = 0;
  };

  class entity;

  // вызывается после создания энтити
  struct entity_created {
    static size_t type;

    entity* ent;
  };

  // вызывается непосредственно перед удалением энтити
  struct entity_destroyed {
    static size_t type;

    entity* ent;
  };

  // вызывается после создания компонента
  template<typename T>
  struct component_created {
    static size_t type;

    entity* ent;
    component_handle<T> component;
  };

  template<typename T>
  struct component_destroyed {
    static size_t type;

    entity* ent;
    component_handle<T> component;
  };

  template<typename T>
  size_t component_created<T>::type = SIZE_MAX;
  template<typename T>
  size_t component_destroyed<T>::type = SIZE_MAX;

  class entity {
  public:
    static size_t type;

    entity(const size_t &id, world* world);
    ~entity();

    template <typename T, typename ...Args>
    component_handle<T> add(Args&& ...args);

    template <typename T>
    void remove();

    template <typename T>
    bool has() const;

    template <typename ...Types>
    bool has() const;

    template <typename T>
    component_handle<T> get() const;

    size_t id() const;
  private:
    size_t m_id;
    world* m_world;
  protected:
    bool has(const std::initializer_list<size_t> &list) const;

    std::unordered_map<size_t, base_component_storage*> m_components;
  };

  class world {
  public:
    template <typename T>
    static component_storage<T>* get_component_storage(component_handle<T> handle);

    world();
    ~world();

    entity* create_entity();
    void destroy_entity(entity* ent);

    template <typename T>
    void create_allocator(const size_t &size);

    template <typename T, typename ...Args>
    component_handle<T> create_component(Args&& ...args);

    template <typename T>
    void destroy_component(component_handle<T> handle);

    void destroy_component(const size_t &type, base_component_storage* storage);

    void register_system(system* system);
    void remove_system(system* system);

    template <typename T>
    void subscribe(event_subscriber<T>* sub);

    template <typename T>
    void unsubscribe(event_subscriber<T>* sub);

    template <typename T>
    void emit(const T &event);

    void update(YACS_SYSTEM_UPDATE);

    size_t size() const;

    template <typename T>
    size_t count_components() const;

    template <typename ...Types>
    size_t count_entities() const;

    template <typename T>
    component_handle<T> get_component(const size_t &index);
  private:
    template <typename T>
    void allocate_pool();

    typeless_pool entityPool;
    std::vector<entity*> entities;
    std::vector<typeless_pool> componentsPool;
    std::vector<std::vector<base_component_storage*>> components;

    std::vector<system*> systems;
    std::vector<std::vector<subscriber_base*>> subscribers;
  };

  template <typename T, typename ...Args>
  component_handle<T> entity::add(Args&& ...args) {
    auto itr = m_components.find(component_storage<T>::type);
    if (itr != m_components.end()) return component_handle<T>(nullptr);

    component_handle<T> handle = m_world->create_component<T>(std::forward<Args>(args)...);
    ASSERT(component_storage<T>::type != SIZE_MAX);
    ASSERT(handle.valid());
    component_storage<T>* storage = world::get_component_storage(handle);
    m_components[component_storage<T>::type] = storage;
    //World::get_component_storage(handle)->index() = ???
    m_world->emit(component_created<T>{this, handle});

    return handle;
  }

  template <typename T>
  void entity::remove() {
    auto itr = m_components.find(component_storage<T>::type);
    if (itr == m_components.end()) return;

    T* ptr = static_cast<T*>(itr->second);
    m_world->emit(component_destroyed<T>{this, component_handle<T>(ptr)});
    m_world->destroy_component(component_handle<T>(ptr));
  }

  template <typename T>
  bool entity::has() const {
    auto itr = m_components.find(component_storage<T>::type);
    return itr != m_components.end();
  }

  template <typename ...Types>
  bool entity::has() const {
    const std::initializer_list<size_t> list = {component_storage<Types>::type...};
    return has(list);
  }

  template <typename T>
  component_handle<T> entity::get() const {
    auto itr = m_components.find(component_storage<T>::type);
    if (itr == m_components.end()) return component_handle<T>(nullptr);

    T* ptr = static_cast<T*>(itr->second);
    return component_handle<T>(ptr);
  }

  template <typename T>
  component_storage<T>* world::get_component_storage(component_handle<T> handle) {
    const size_t size = sizeof(component_storage<T>) - sizeof(T);
    char* start = reinterpret_cast<char*>(handle.get()) - size;
    return reinterpret_cast<component_storage<T>*>(start);
  }

  template <typename T>
  void world::create_allocator(const size_t &size) {
    if (component_storage<T>::type < componentsPool.size()) return;

    component_storage<T>::type = componentsPool.size();
    componentsPool.emplace_back(component_storage<T>::type, size);
    components.emplace_back();
  }

  template <typename T, typename ...Args>
  component_handle<T> world::create_component(Args&& ...args) {
    if (component_storage<T>::type >= componentsPool.size()) {
      allocate_pool<T>();
      ASSERT(component_storage<T>::type < componentsPool.size());
      ASSERT(component_storage<T>::type < components.size());
    }

    const size_t poolIndex = component_storage<T>::type;
    component_storage<T>* storage = componentsPool[poolIndex].create<component_storage<T>>(std::forward<Args>(args)...);
    storage->index() = components[poolIndex].size();
    components[poolIndex].push_back(storage);

    return component_handle<T>(storage->ptr());
  }

  template <typename T>
  void world::destroy_component(component_handle<T> handle) {
    if (!handle.valid()) return;
    if (component_storage<T>::type >= componentsPool.size()) return;

    const size_t poolIndex = component_storage<T>::type;
    component_storage<T>* storage = world::get_component_storage(handle);
    components[poolIndex].back()->index() = storage->index();
    std::swap(components[poolIndex].back(), components[poolIndex][storage->index()]);
    components[poolIndex].pop_back();
    componentsPool[poolIndex].destroy<T>(storage);
  }

  template <typename T>
  void world::subscribe(event_subscriber<T>* sub) {
    if (T::type >= subscribers.size()) {
      T::type = subscribers.size();
      subscribers.emplace_back();
    }

    sub->index() = subscribers[T::type].size();
    subscribers[T::type].push_back(sub);
  }

  template <typename T>
  void world::unsubscribe(event_subscriber<T>* sub) {
    if (T::type >= subscribers.size()) return;
    if (subscribers[T::type].empty()) return;

    std::vector<subscriber_base*> &eventSubs = subscribers[T::type];
    eventSubs.back()->index() = sub->index();
    std::swap(eventSubs[sub->index()], eventSubs.back());
    eventSubs.pop_back();
  }

  template <typename T>
  void world::emit(const T &event) {
    if (T::type >= subscribers.size()) return;
    if (subscribers[T::type].empty()) return;

    std::vector<subscriber_base*> &eventSubs = subscribers[T::type];
    for (auto* base : eventSubs) {
      auto* sub = static_cast<event_subscriber<T>*>(base);
      sub->receive(this, event);
    }
  }

  template <typename T>
  size_t world::count_components() const {
    if (component_storage<T>::type >= componentsPool.size()) return 0;

    return components[component_storage<T>::type].size();
  }

  template <typename ...Types>
  size_t world::count_entities() const {
    size_t c = 0;
    for (auto entity : entities) {
      if (entity->has<Types...>()) ++c;
    }

    return c;
  }

  template <typename T>
  component_handle<T> world::get_component(const size_t &index) {
    if (component_storage<T>::type >= componentsPool.size()) return component_handle<T>(nullptr);
    const size_t poolIndex = component_storage<T>::type;
    if (index >= components[poolIndex].size()) return component_handle<T>(nullptr);

    auto storage = static_cast<component_storage<T>*>(components[poolIndex][index]);
    return component_handle<T>(storage->ptr());
  }

  template <typename T>
  void world::allocate_pool() {
    component_storage<T>::type = componentsPool.size();
    componentsPool.emplace_back(component_storage<T>::type, sizeof(component_storage<T>) * YACS_DEFAULT_COMPONENTS_COUNT);
    components.emplace_back();
  }
}

#endif //YACS_H

#ifdef YACS_IMPLEMENTATION
namespace yacs {
  size_t entity_created::type = SIZE_MAX;
  size_t entity_destroyed::type = SIZE_MAX;

  entity::entity(const size_t &id, world* world) : m_id(id), m_world(world) {}
  entity::~entity() {
    for (const auto &pair : m_components) {
      m_world->destroy_component(pair.first, pair.second);
    }
  }

  bool entity::has(const std::initializer_list<size_t> &list) const {
    for (auto type : list) {
      if (m_components.find(type) == m_components.end()) return false;
    }

    return true;
  }

  size_t entity::id() const { return m_id; }

  size_t entity::type = SIZE_MAX;

  world::world() : entityPool(entity::type, sizeof(entity) * YACS_DEFAULT_ENTITY_COUNT) {}
  world::~world() {
    for (auto entity : entities) {
      entityPool.destroy<class entity>(entity);
    }

    for (size_t i = 0; i < components.size(); ++i) {
      for (auto comp : components[i]) {
        componentsPool[i].destroy(i, comp);
      }
    }
  }

  entity* world::create_entity() {
    entity* ent = entityPool.create<entity>(entities.size(), this);
    entities.push_back(ent);
    emit(entity_created{ent});

    return ent;
  }

  void world::destroy_entity(entity* ent) {
    emit(entity_destroyed{ent});

    for (size_t i = 0; i < entities.size(); ++i) {
      if (entities[i] == ent) {
        std::swap(entities[i], entities.back());
        entities.pop_back();
        break;
      }
    }

    entityPool.destroy<entity>(ent);
  }

  void world::destroy_component(const size_t &type, base_component_storage* storage) {
    if (type >= componentsPool.size()) return;

    const size_t poolIndex = type;
    components[poolIndex].back()->index() = storage->index();
    std::swap(components[poolIndex].back(), components[poolIndex][storage->index()]);
    components[poolIndex].pop_back();
    componentsPool[poolIndex].destroy(type, storage);
  }

  void world::register_system(system* system) {
    systems.push_back(system);
  }

  void world::remove_system(system* system) {
    for (size_t i = 0; i < systems.size(); ++i) {
      if (systems[i] == system) {
        std::swap(systems[i], systems.back());
        systems.pop_back();
        break;
      }
    }
  }

  void world::update(YACS_SYSTEM_FUNC) {
    for (auto system : systems) {
      system->update(YACS_SYSTEM_UPDATE_CALL);
    }
  }

  size_t world::size() const {
    return entities.size();
  }
}
#endif