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

  // существует 2 способа избавиться вообще от контейнера для контейнеров
  // причем они не сказать чтобы очень плохие
  // 1) константный энтити - определяем размер энтити (размер всех компонентов) при создании, 
  // в энтити держим указатель на часть памяти под компоненты, в начале памяти держим тип + начало компонента
  // 2) я думал насчет динамичекого выделения памяти, но чет это выглядит плохо
  // еще вполне рабочий вариант использовать статический энтити
  // то есть мы должны при создании энтити указать какие у него типы компонентов
  // единственная проблема здесь в том, что нам нужно будет инициализировать все компоненты
  // ну и обход всех энтити череват большими кэш миссами
  
  // мы должны заменить хранилище компонентов на массив 
  // (будет ли прирост скорости? по идее должен быть)
  // (по крайней мере можено избежать дефрагментации возникающей при использовании map)
  // массив также позволит небезопасно брать указатель по индексу 
  // (точнее мы можем по индексу возвращать nullptr, если там лежит какой то друго тип)
  // к сожалению видимо придется еще дополнительно заполнять хранилище ничем
  // для того чтобы сохранить порядок индексов, в этом нам поможет метод set
  class entity {
  public:
    static size_t type;

    entity(const size_t &id, world* world);
    ~entity();

    template <typename T, typename ...Args>
    component_handle<T> add(Args&& ...args);

    template <typename T>
    bool remove();

    template <typename T>
    bool has() const;

    template <typename ...Types>
    bool has() const;

    template <typename T>
    component_handle<T> get();

    template <typename T>
    const_component_handle<T> get() const;
    
    template <typename T>
    component_handle<T> at(const size_t &index);
    
    template <typename T>
    const_component_handle<T> at(const size_t &index) const;

    template <typename T>
    bool set(const component_handle<T> &comp);

    template <typename T>
    bool unset();

    size_t id() const;
    size_t components_count() const;
  private:
    size_t m_id;
    world* m_world;
  protected:
    bool has(const std::initializer_list<size_t> &list) const;
    template <typename T>
    size_t find() const;
    size_t find(const size_t &type) const;
    
    template <typename T>
    void add_to_array(const component_handle<T> &comp);
    template <typename T>
    void remove_from_array(const size_t &index);

    //std::unordered_map<size_t, base_component_storage*> m_components;
    std::vector<std::pair<size_t, base_component_storage*>> m_components;
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
//     auto itr = m_components.find(component_storage<T>::type);
//     if (itr != m_components.end()) return component_handle<T>(nullptr);
// 
//     component_handle<T> handle = m_world->create_component<T>(std::forward<Args>(args)...);
//     ASSERT(component_storage<T>::type != SIZE_MAX);
//     ASSERT(handle.valid());
//     component_storage<T>* storage = world::get_component_storage(handle);
//     m_components[component_storage<T>::type] = storage;
//     //World::get_component_storage(handle)->index() = ???
//     m_world->emit(component_created<T>{this, handle});
// 
//     return handle;
    
    if (component_storage<T>::type != SIZE_MAX) {
      const size_t index = find<T>();
      if (index != SIZE_MAX) return component_handle<T>(nullptr);
    }

    component_handle<T> handle = m_world->create_component<T>(std::forward<Args>(args)...);
    ASSERT(component_storage<T>::type != SIZE_MAX);
    ASSERT(handle.valid());
    add_to_array(handle);
    m_world->emit(component_created<T>{this, handle});

    return handle;
  }

  template <typename T>
  bool entity::remove() {
//     auto itr = m_components.find(component_storage<T>::type);
//     if (itr == m_components.end()) return;
// 
//     auto* ptr = static_cast<component_storage<T>*>(itr->second);
//     m_world->emit(component_destroyed<T>{this, component_handle<T>(ptr->ptr())});
//     m_world->destroy_component(component_handle<T>(ptr->ptr()));
    
    if (component_storage<T>::type == SIZE_MAX) return false;
    
    const size_t index = find<T>();
    if (index == SIZE_MAX) return false;

    auto* ptr = static_cast<component_storage<T>*>(m_components[index].second);
    m_world->emit(component_destroyed<T>{this, component_handle<T>(ptr->ptr())});
    m_world->destroy_component(component_handle<T>(ptr->ptr()));
    remove_from_array<T>(index);
    return true;
  }

  template <typename T>
  bool entity::has() const {
//     auto itr = m_components.find(component_storage<T>::type);
//     return itr != m_components.end();
    const size_t index = find<T>();
    return index != SIZE_MAX;
  }

  template <typename ...Types>
  bool entity::has() const {
    const std::initializer_list<size_t> list = {component_storage<Types>::type...};
    return has(list);
  }

  template <typename T>
  component_handle<T> entity::get() {
//     auto itr = m_components.find(component_storage<T>::type);
//     if (itr == m_components.end()) return component_handle<T>(nullptr);
// 
//     auto* ptr = static_cast<component_storage<T>*>(itr->second);
//     return component_handle<T>(ptr->ptr());
    
    const size_t index = find<T>();
    if (index == SIZE_MAX) return component_handle<T>(nullptr);

    auto* ptr = static_cast<component_storage<T>*>(m_components[index].second);
    return component_handle<T>(ptr->ptr());
  }

  template <typename T>
  const_component_handle<T> entity::get() const {
//     auto itr = m_components.find(component_storage<T>::type);
//     if (itr == m_components.end()) return const_component_handle<T>(nullptr);
// 
//     const auto* ptr = static_cast<const component_storage<T>*>(itr->second);
//     return const_component_handle<T>(ptr->ptr());
    
    const size_t index = find<T>();
    if (index == SIZE_MAX) return const_component_handle<T>(nullptr);

    const auto* ptr = static_cast<const component_storage<T>*>(m_components[index].second);
    return const_component_handle<T>(ptr->ptr());
  }
  
  template <typename T>
  component_handle<T> entity::at(const size_t &index) {
    if (index >= m_components.size()) return component_handle<T>(nullptr);
    if (m_components[index].first != component_storage<T>::type) return component_handle<T>(nullptr);
    auto* ptr = static_cast<component_storage<T>*>(m_components[index].second);
    return component_handle<T>(ptr->ptr());
  }
  
  template <typename T>
  const_component_handle<T> entity::at(const size_t &index) const {
    if (index >= m_components.size()) return const_component_handle<T>(nullptr);
    if (m_components[index].first != component_storage<T>::type) return const_component_handle<T>(nullptr);
    const auto* ptr = static_cast<const component_storage<T>*>(m_components[index].second);
    return const_component_handle<T>(ptr->ptr());
  }

  template <typename T>
  bool entity::set(const component_handle<T> &comp) {
    // по идее нужно либо создать аллокатор
    // либо как то иначе добавить компонент
    // скорее всего мне нужно добавить компонент по своему индексу аллокатора
    // да и вообще нужно следить чтобы они именно так располагались
    // в этом случае массив у каждого энтити расширяется и так то new используется
    if (component_storage<T>::type == SIZE_MAX && !comp.valid()) {
      add_to_array(comp);
      return true;
    } else if (component_storage<T>::type != SIZE_MAX && !comp.valid()) return false;
    
    const size_t index = find<T>();
    if (index != SIZE_MAX) return false;
    add_to_array(comp);
    return true;
  }

  template <typename T>
  bool entity::unset() {
    const size_t index = find<T>();
    if (index == SIZE_MAX) return false;
    remove_from_array<T>(index);
    return true;
  }
  
  template <typename T>
  size_t entity::find() const {
    for (size_t i = 0; i < m_components.size(); ++i) {
      if (m_components[i].first == component_storage<T>::type) return i;
    }
    
    return SIZE_MAX;
  }
  
  template <typename T>
  void entity::add_to_array(const component_handle<T> &comp) {
    component_storage<T>* ptr = world::get_component_storage(comp);
    m_components.push_back(std::make_pair(component_storage<T>::type, ptr));
  }
  
  template <typename T>
  void entity::remove_from_array(const size_t &index) {
    std::swap(m_components[index], m_components.back());
    m_components.pop_back();
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
  
  // в многопоточной версии нужно убедиться что components не используется во время того как мы добавляем 
  // созданный компонент, это можно гарантировать создав пул для каждого ThreadsafeArray
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
      if (pair.second == nullptr) continue;
      m_world->destroy_component(pair.first, pair.second);
    }
  }
  
  size_t entity::find(const size_t &type) const {
    for (size_t i = 0; i < m_components.size(); ++i) {
      if (m_components[i].first == type) return i;
    }
    
    return SIZE_MAX;
  }

  bool entity::has(const std::initializer_list<size_t> &list) const {
//     for (auto type : list) {
//       if (m_components.find(type) == m_components.end()) return false;
//     }
    
    for (const auto &type : list) {
      if (find(type) == SIZE_MAX) return false;
    }

    return true;
  }

  size_t entity::id() const { return m_id; }
  size_t entity::components_count() const { return m_components.size(); }

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
