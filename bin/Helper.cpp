#include "Helper.h"

#include <algorithm>
// #include <lldb/Utility/Status.h>

// GPUArray<MonsterGPUOptimizer::InstanceData> instanceData;

static const std::vector<const char*> instanceLayers = {
  "VK_LAYER_LUNARG_standard_validation",
  "VK_LAYER_LUNARG_api_dump",
  "VK_LAYER_LUNARG_assistant_layer"
};

void clipbardPaste(nk_handle usr, nk_text_edit *edit) {
    const char *text = glfwGetClipboardString(reinterpret_cast<Window*>(usr.ptr)->handle());

    if (text) nk_textedit_paste(edit, text, nk_strlen(text));
}

void clipbardCopy(nk_handle usr, const char *text, const int len) {
  if (len == 0) return;

  char str[len+1];
  memcpy(str, text, len);
  str[len] = '\0';

  glfwSetClipboardString(reinterpret_cast<Window*>(usr.ptr)->handle(), str);
}

system_container::system_container() :
  container(
    sizeof(GraphicsContainer) +
//     sizeof(PostPhysics) +
    sizeof(systems::sound) +
                       
    sizeof(MonsterGPUOptimizer) +
    sizeof(MonsterGPUOptimizer) +
    sizeof(LightOptimizer) +
    sizeof(MonsterDebugOptimizer) +
    sizeof(GeometryDebugOptimizer) +
                       
    sizeof(CPUOctreeBroadphaseParallel) +
    sizeof(CPUNarrowphaseParallel) +
    sizeof(CPUSolverParallel) +
    sizeof(CPUPhysicsSorter) +
    sizeof(CPUPhysicsParallel) +
    
    sizeof(graph::container) +
    sizeof(systems::pathfinder)
  ),
  graphics_container(nullptr),
//   post_physics(nullptr),
  sound_system(nullptr),
  
  monster_optimiser(nullptr),
  geometry_optimiser(nullptr),
  lights_optimiser(nullptr),
  monster_debug_optimiser(nullptr),
  geometry_debug_optimiser(nullptr),
  
  broad(nullptr),
  narrow(nullptr),
  solver(nullptr),
  sorter(nullptr),
  physics(nullptr),
  
  edge_container(nullptr),
  pathfinder_system(nullptr)
{}

#define DESTROY_CONTAINERS(var) if (var != nullptr) container.destroy(var); var = nullptr;
system_container::~system_container() {
  DESTROY_CONTAINERS(graphics_container)
//   DESTROY_CONTAINERS(post_physics)
  DESTROY_CONTAINERS(sound_system)
  
  DESTROY_CONTAINERS(monster_optimiser)
  DESTROY_CONTAINERS(geometry_optimiser)
  DESTROY_CONTAINERS(lights_optimiser)
  DESTROY_CONTAINERS(monster_debug_optimiser)
  DESTROY_CONTAINERS(geometry_debug_optimiser)
  
  DESTROY_CONTAINERS(broad)
  DESTROY_CONTAINERS(narrow)
  DESTROY_CONTAINERS(solver)
  DESTROY_CONTAINERS(sorter)
  DESTROY_CONTAINERS(physics)
  
  DESTROY_CONTAINERS(edge_container)
  DESTROY_CONTAINERS(pathfinder_system)
}

DataArrays::DataArrays() : 
  container(
    sizeof(GPUContainer<Transform>) +
    sizeof(GPUContainer<InputData>) +
    sizeof(GPUContainer<simd::mat4>) +
    sizeof(GPUBuffer<uint32_t>) +
    sizeof(GPUContainer<RotationData>) +
    sizeof(GPUContainer<ExternalData>) +
    sizeof(GPUContainer<Texture>)
  ), transforms(nullptr), inputs(nullptr), matrices(nullptr), rotationCountBuffer(nullptr), rotations(nullptr), externals(nullptr), textures(nullptr) {}
  
DataArrays::~DataArrays() {
  DESTROY_CONTAINERS(transforms)
  DESTROY_CONTAINERS(inputs)
  DESTROY_CONTAINERS(matrices)
  DESTROY_CONTAINERS(rotations)
  DESTROY_CONTAINERS(externals)
  DESTROY_CONTAINERS(textures)
}

KeyConfiguration::KeyConfiguration(const KeyConfiguration &copy) {
  cont = copy.cont;
}

#ifndef UINT16_WIDTH
  #define UINT16_WIDTH 16
#endif

KeyConfiguration::KeyConfiguration(const uint32_t &key1) {
  cont = (uint64_t(key1) << UINT16_WIDTH*0) | (uint64_t(UINT16_MAX) << UINT16_WIDTH*1) | (uint64_t(UINT16_MAX) << UINT16_WIDTH*2) | (uint64_t(UINT16_MAX) << UINT16_WIDTH*3);
}

KeyConfiguration::KeyConfiguration(const uint32_t &key1, const uint32_t &key2) {
  cont = (uint64_t(key1) << UINT16_WIDTH*0) | (uint64_t(key2) << UINT16_WIDTH*1) | (uint64_t(UINT16_MAX) << UINT16_WIDTH*2) | (uint64_t(UINT16_MAX) << UINT16_WIDTH*3);
}

KeyConfiguration::KeyConfiguration(const uint32_t &key1, const uint32_t &key2, const uint32_t &key3) {
  cont = (uint64_t(key1) << UINT16_WIDTH*0) | (uint64_t(key2) << UINT16_WIDTH*1) | (uint64_t(key3) << UINT16_WIDTH*2) | (uint64_t(UINT16_MAX) << UINT16_WIDTH*3);
}

KeyConfiguration::KeyConfiguration(const uint32_t &key1, const uint32_t &key2, const uint32_t &key3, const uint32_t &key4) {
  cont = (uint64_t(key1) << UINT16_WIDTH*0) | (uint64_t(key2) << UINT16_WIDTH*1) | (uint64_t(key3) << UINT16_WIDTH*2) | (uint64_t(key4) << UINT16_WIDTH*3);
}

uint32_t KeyConfiguration::getFirstKey() const {
  const uint16_t tmp = (cont >> UINT16_WIDTH*0) | UINT16_MAX;
  return tmp == UINT16_MAX ? UINT32_MAX : tmp;
}

uint32_t KeyConfiguration::getSecondKey() const {
  const uint16_t tmp = (cont >> UINT16_WIDTH*1) | UINT16_MAX;
  return tmp == UINT16_MAX ? UINT32_MAX : tmp;
}

uint32_t KeyConfiguration::getThirdKey() const {
  const uint16_t tmp = (cont >> UINT16_WIDTH*2) | UINT16_MAX;
  return tmp == UINT16_MAX ? UINT32_MAX : tmp;
}

uint32_t KeyConfiguration::getForthKey() const {
  const uint16_t tmp = (cont >> UINT16_WIDTH*3) | UINT16_MAX;
  return tmp == UINT16_MAX ? UINT32_MAX : tmp;
}

uint32_t KeyConfiguration::getKeysCount() const {
  uint32_t count = 0;
  for (uint8_t i = 0; i < 4; ++i) {
    const uint16_t tmp = uint16_t(cont >> UINT16_WIDTH*i); // | UINT16_MAX
    count += uint32_t(tmp != UINT16_MAX); //  ? count : count + 1;
  }

  return count;
}

uint32_t KeyConfiguration::operator[] (const uint8_t &index) const {
  const uint16_t tmp = (cont >> UINT16_WIDTH*index); // | UINT16_MAX
  return tmp == UINT16_MAX ? UINT32_MAX : tmp;
}

input_function::input_function() {}
input_function::input_function(const std::string &name, const std::function<void()> &f) : name(name), f(f) {}

key::action::type::type() : m_container(0) {}
key::action::type::type(const std::vector<enum state> &states, const bool notUsedWhileModificator, const modificator &mod) : m_container(0) {
  make_type(states, notUsedWhileModificator, mod);
}

#define ACTION_STATE_PLACE 0
#define NOT_USED_PLACE (static_cast<uint32_t>(key::state::long_click)+ACTION_STATE_PLACE)
#define CHANGED_PLACE (NOT_USED_PLACE+1)
#define MOD_PLACE (CHANGED_PLACE+1)

void key::action::type::make_type(const std::vector<enum state> &states, const bool notUsedWhileModificator, const modificator &mod) {
  for (const auto state : states) {
    if (state == key::state::unknown) continue;
    
    m_container |= (1 << (static_cast<uint32_t>(state)-1));
  }
  
  m_container |= (static_cast<uint32_t>(notUsedWhileModificator) << NOT_USED_PLACE);
  
  m_container |= (static_cast<uint32_t>(mod) << MOD_PLACE);
}

bool key::action::type::is_valid_state(const key::state &state) const {
  if (state == key::state::unknown) return false;
  
  const uint32_t mask = 1 << (static_cast<uint32_t>(state)-1);
  return (m_container & mask) == mask;
}

bool key::action::type::isUsedWithModificators() const {
  const uint32_t mask = 1 << NOT_USED_PLACE;
  return (m_container & mask) == mask;
}

bool key::action::type::changed() const {
  const uint32_t mask = 1 << CHANGED_PLACE;
  return (m_container & mask) == mask;
}

key::modificator key::action::type::key_modificator() const {
  const uint32_t mask = 0x7;
  return static_cast<key::modificator>((m_container >> MOD_PLACE) & mask);
}

void key::action::type::set_changed(const bool value) {
  const uint32_t mask = 1 << CHANGED_PLACE;
  m_container = value ? m_container | mask : m_container & (~mask);
}

key::action::action() : m_time(0), m_key(0) {}
key::action::action(const std::vector<enum state> &states, const bool notUsedWhileModificator, const modificator &mod, const int32_t &key, input_function* func) : m_time(0), m_type(states, notUsedWhileModificator, mod), m_key(key), m_current(key::state::unknown), m_func(func) {}

void key::action::execute(const std::vector<modificator> &mods, const int32_t &state, const size_t &time) {
  m_time += time;
  
  bool rightModify = false;
  bool additionalModifiers = false;
  for (size_t i = 0; i < mods.size(); ++i) {
    if (mods[i] == m_type.key_modificator()) {
      rightModify = true;
      break;
    } else {
      additionalModifiers = true;
    }
  }
  
  if (additionalModifiers && !m_type.isUsedWithModificators()) return;
  if (m_type.key_modificator() != key::modificator::none && !rightModify) return;
  
  const bool key_pressed = static_cast<bool>(state);
  
  switch (m_current) {
    case key::state::unknown: {
      const key::state old = m_current;
      
      const uint8_t inc = state;
      m_current = static_cast<key::state>(static_cast<uint8_t>(m_current)+inc);
      m_time = time;
      
      m_type.set_changed(m_current != old);
      break;
    }
    
    case key::state::press: {
//       const key::state old = m_current;
      const bool is_long_press = m_time > LONG_PRESS_TIME;
      
      const uint8_t inc = static_cast<uint8_t>(!key_pressed);
      m_current = static_cast<key::state>(static_cast<uint8_t>(m_current)+inc);
      m_current = is_long_press ? key::state::long_press : m_current;
      
      //m_type.set_changed(m_current != old);
      m_type.set_changed(true);
      break;
    }
    
    case key::state::click: {
      const key::state old = m_current;
      const bool is_double_pressed_time = m_time < DOUBLE_PRESS_TIME;
      
      const uint8_t inc = static_cast<uint8_t>(key_pressed && is_double_pressed_time);
      m_current = static_cast<key::state>(static_cast<uint8_t>(m_current)+inc);
      
      const bool just_press = key_pressed && !is_double_pressed_time;
      m_current = just_press ? key::state::press : m_current;
      m_time = just_press ? time : m_time;
      
      m_type.set_changed(m_current != old);
      break;
    }
    
    case key::state::double_press: {
//       const key::state old = m_current;
      const bool is_double_clicked_time = m_time < DOUBLE_CLICK_TIME;
      
      const uint8_t inc = static_cast<uint8_t>(!key_pressed && is_double_clicked_time);
      m_current = static_cast<key::state>(static_cast<uint8_t>(m_current)+inc);
      
      const bool just_press = !key_pressed && !is_double_clicked_time;
      m_current = just_press ? key::state::click : m_current;
      m_time = just_press ? time : m_time;
      
//       m_type.set_changed(m_current != old);
      m_type.set_changed(true);
      break;
    }
    
    case key::state::double_click: {
      const key::state old = m_current;
    
      m_current = key_pressed ? key::state::press : m_current;
      m_time = key_pressed ? time : m_time;
      
      m_type.set_changed(m_current != old);
      break;
    }
    
    case key::state::long_press: {
//       const key::state old = m_current;
      
      m_current = !key_pressed ? key::state::long_click : m_current;
      
//       m_type.set_changed(m_current != old);
      m_type.set_changed(true);
      break;
    }
    
    case key::state::long_click: {
      const key::state old = m_current;
      
      m_current = key_pressed ? key::state::press : m_current;
      m_time = key_pressed ? time : m_time;
      
      m_type.set_changed(m_current != old);
      break;
    }
  }
  
  const bool valid_state = m_type.is_valid_state(m_current);
  
  if (valid_state && m_type.changed()) m_func->f();
  m_type.set_changed(false);
}

int32_t key::action::key() {
  return m_key;
}

enum key::state key::action::current_state() const {
  return m_current;
}

bool key::action::is_valid_state(const enum state &state) const {
  return m_type.is_valid_state(state);
}

bool key::action::isUsedWithModificators() const {
  return m_type.isUsedWithModificators();
}

std::string key::action::name() const {
  return m_func->name;
}

// Reaction::Reaction() {}
// Reaction::Reaction(const std::string &name, const std::function<void()> &f) : name(name), f(f) {}

// ActionKey::ActionKey(const KeyConfiguration &keys, const std::vector<ActionKey*> &keysPtr)
//   : handled(false),
//     keys(keys),
//     keysPtr(keysPtr),
//     commands({nullptr, nullptr, nullptr, nullptr, nullptr, nullptr}),
//     time(0),
//     currentState(KEY_STATE_UNKNOWN) {
// //   std::cout << "keysCount " << keys.getKeysCount() << "\n";
// 
//   const uint32_t count = keys.getKeysCount();
//   if (count == 0) throw std::runtime_error("Key count == 0");
//   if (count != 1 && count != keysPtr.size()) throw std::runtime_error("Wrong key number");
//   //if (count > 1 && count == keysPtr.size())
// }
// 
// void ActionKey::execute(const int32_t &state, const uint64_t &_time) {
//   if (handled) {
//     handled = !handled;
//     return;
//   }
// 
//   time += _time;
//   bool changed = false;
//   if (state == GLFW_PRESS || state == GLFW_REPEAT) {
//     if (currentState == KEY_STATE_CLICK) {
//       if (time < DOUBLE_PRESS_TIME) {
//         currentState = KEY_STATE_DOUBLE_PRESS;
//         changed = true;
//       } else {
//         currentState = KEY_STATE_PRESS;
//         time = 0;
//         changed = true;
//       }
//     } else if (currentState == KEY_STATE_PRESS) {
//       if (time > LONG_PRESS_TIME) {
//         currentState = KEY_STATE_PRESS;
//         changed = true;
//       } else {
//         currentState = KEY_STATE_PRESS;
//         changed = true;
//       }
//     } else if ((currentState == KEY_STATE_DOUBLE_CLICK) || (currentState == KEY_STATE_LONG_CLICK) || (currentState == KEY_STATE_UNKNOWN)) {
//       currentState = KEY_STATE_PRESS;
//       time = 0;
//       changed = true;
//     } else if (currentState == KEY_STATE_LONG_PRESS) {
//       changed = true;
//     } else if (currentState == KEY_STATE_DOUBLE_PRESS) {
//       if (time > DOUBLE_CLICK_TIME) {
//         currentState = KEY_STATE_PRESS;
//         time = 0;
//         changed = true;
//       } else {
//         changed = true;
//       }
//     }
//   } else if (state == GLFW_RELEASE) {
//     if (currentState == KEY_STATE_PRESS) {
//       currentState = KEY_STATE_CLICK;
//       changed = true;
//       //std::cout << "clicked " << glfwGetKeyName(*key, 0) << std::endl;
//     } else if (currentState == KEY_STATE_DOUBLE_PRESS) {
//       if (time < DOUBLE_CLICK_TIME) {
//         currentState = KEY_STATE_DOUBLE_CLICK;
//         changed = true;
//         //std::cout << "double clicked " << glfwGetKeyName(*key, 0) << std::endl;
//       } else {
//         currentState = KEY_STATE_CLICK;
//         time = 0;
//         changed = true;
//       }
//     } else if (currentState == KEY_STATE_LONG_PRESS) { // наверное long click будет происходить сразу же после long press
//       if (time > LONG_CLICK_TIME) {
//         currentState = KEY_STATE_LONG_CLICK;
//         changed = true;
//         //std::cout << "long clicked " << glfwGetKeyName(*key, 0) << std::endl;
//       } else {
//         currentState = KEY_STATE_CLICK;
//         time = 0;
//         changed = true;
//       }
//     }
//   }
// 
//   if (changed && commands[currentState] != nullptr) {
//     commands[currentState]->f();
// 
//     for (uint8_t i = 0; i < keysPtr.size(); ++i) keysPtr[i]->setHandled();
//   }
// }
// 
// void ActionKey::setHandled() {
//   handled = true;
// }
// 
// Reaction* ActionKey::getReaction(uint8_t i) const {
//   return commands[i];
// }
// 
// void ActionKey::setReaction(const uint8_t &index, Reaction* r) {
//   commands[index] = r;
// }
// 
// uint32_t ActionKey::getKey(const uint8_t &index) const {
//   return keys[index];
// }
// 
// uint32_t ActionKey::getKeysCount() const {
//   return keys.getKeysCount();
// }

//KeyContainer::KeyContainer(KeyConfig* config) : config(config) {}
KeyContainer::KeyContainer() {}

KeyContainer::~KeyContainer() {
  for (size_t i = 0; i < config.keys.size(); ++i) {
    keysPool.deleteElement(config.keys[i]);
  }

  for (auto itr = config.reactions.begin(); itr != config.reactions.end(); ++itr) {
    reactionPool.deleteElement(itr->second);
  }
}

// ActionKey* KeyContainer::create(const KeyConfiguration &keys, const std::vector<ActionKey*> &keysPtr) {
//   ActionKey* key = keysPool.newElement(keys, keysPtr);
//   config->keys.push_back(key);
// 
//   return key;
// }

key::action* KeyContainer::create(const std::vector<key::state> &states, const bool notUsedWhileModificator, const key::modificator &mod, const int32_t &key, input_function* func) {
  key::action* action = keysPool.newElement(states, notUsedWhileModificator, mod, key, func);
  config.keys.push_back(action);
  return action;
}

// struct KeyCompare {
//   bool operator() (const ActionKey* first, const ActionKey* second) const {
//     return first->getKeysCount() > second->getKeysCount();
//   }
// };
// 
// void KeyContainer::sort() {
//   std::sort(config->keys.begin(), config->keys.end(), KeyCompare());
// }

// Reaction* KeyContainer::create(const std::string &name, const std::function<void()> &f) {
//   Reaction* react = reactionPool.newElement(name, f);
//   config->reactions[name] = react;
// 
//   return react;
// }

input_function* KeyContainer::create(const std::string &name, const std::function<void()> &f) {
  auto itr = config.reactions.find(name);
  if (itr != config.reactions.end()) throw std::runtime_error("Reaction with name " + name + " is already exist");
  
  input_function* input = reactionPool.newElement(name, f);
  config.reactions[name] = input;
  return input;
}

void initGLFW() {
  if (glfwInit() != GLFW_TRUE) {
    Global::console()->printE("Cannot init glfw!");
    throw std::runtime_error("Cannot init glfw!");
  }

  if (glfwVulkanSupported() != GLFW_TRUE) {
    Global::console()->printE("Vulkan is not supported!");
    throw std::runtime_error("Vulkan is not supported!");
  }

  glfwSetErrorCallback(callback);
}

void deinitGLFW() {
  glfwTerminate();
}

void create_graphics(system_container &container) {
  const size_t stageContainerSize = sizeof(BeginTaskStage) + sizeof(EndTaskStage) + sizeof(MonsterGPUOptimizer) + sizeof(GeometryGPUOptimizer) + 
                                    sizeof(GBufferStage) + sizeof(DefferedLightStage) + sizeof(ToneMappingStage) + sizeof(CopyStage) + sizeof(PostRenderStage);

    GraphicsContainer::CreateInfo info{
      stageContainerSize
      //&systemContainer
    };
    container.graphics_container = container.container.create<GraphicsContainer>();
    container.graphics_container->construct(info);
    Global::get(container.graphics_container);
}

void create_optimizers(system_container &container, DataArrays &arrays) {
  container.lights_optimiser = container.container.create<LightOptimizer>();
  container.monster_debug_optimiser = container.container.create<MonsterDebugOptimizer>();
  container.geometry_debug_optimiser = container.container.create<GeometryDebugOptimizer>();
  
  container.lights_optimiser->setInputBuffers({arrays.transforms});
  container.monster_debug_optimiser->setInputBuffers({arrays.transforms});
  
  Global::render()->addOptimizerToClear(container.lights_optimiser);
  Global::render()->addOptimizerToClear(container.monster_debug_optimiser);
  Global::render()->addOptimizerToClear(container.geometry_debug_optimiser);
  
  Global::get(container.lights_optimiser);
  Global::get(container.monster_debug_optimiser);
  Global::get(container.geometry_debug_optimiser);
}

void create_physics(system_container &container, dt::thread_pool* pool, DataArrays &arrays, const size_t &updateDelta) {
  TimeLogDestructor physics("Physics system initialization");
  
// const GPUOctreeBroadphase::GPUOctreeBroadphaseCreateInfo octreeInfo{
  //   {simd::vec4(0.0f, 0.0f, 0.0f, 1.0f), simd::vec4(100.0f, 100.0f, 100.0f, 0.0f), 5},
  //   device,
  //   task, // таск
  //   nullptr
  // };
  // GPUOctreeBroadphase broad(octreeInfo);

//   const CPUOctreeBroadphase::OctreeCreateInfo octree{
//     simd::vec4(0.0f, 0.0f, 0.0f, 1.0f),
//     simd::vec4(100.0f, 100.0f, 100.0f, 0.0f),
//     4
//   };
//   CPUOctreeBroadphase broad(octree);

  const CPUOctreeBroadphaseParallel::OctreeCreateInfo octree{
    simd::vec4(0.0f, 0.0f, 0.0f, 1.0f),
    simd::vec4(100.0f, 100.0f, 100.0f, 0.0f),
    5
  };
  container.broad = container.container.create<CPUOctreeBroadphaseParallel>(pool, octree);

  //GPUNarrowphase narrow(device, task);
  //CPUNarrowphase narrow(octree.depth);
  //CPUNarrowphaseParallel narrow(&threadPool, octree.depth);
  container.narrow = container.container.create<CPUNarrowphaseParallel>(pool, octree.depth);

  //GPUSolver solver(device, task);
  //CPUSolver solver;
  //CPUSolverParallel solver(&threadPool);
  container.solver = container.container.create<CPUSolverParallel>(pool);
  
//   if (solver == nullptr) {
//     throw std::runtime_error("wtf");
//   }

  // const GPUPhysicsSorterCreateInfo sorterInfo{
  //   {
  //     "shaders/sorting.spv"
  //   },
  //   {
  //     "shaders/sortingOverlapping1.spv", "shaders/sortingOverlapping2.spv"
  //   }
  // };
  // GPUPhysicsSorter sorter(device, task, sorterInfo);

  //CPUPhysicsSorter sorter;
  container.sorter = container.container.create<CPUPhysicsSorter>();

  const uint32_t staticMatrix = arrays.matrices->insert(simd::mat4(1.0f));
  const uint32_t dynamicMatrix = arrays.matrices->insert(simd::mat4(1.0f));
  const PhysicsExternalBuffers bufferInfo{
    staticMatrix,
    dynamicMatrix,
    arrays.inputs,
    arrays.transforms,
    arrays.matrices,
    arrays.rotationCountBuffer,
    arrays.rotations,
    arrays.externals
  };

//   const GPUPhysicsCreateInfo physInfo {
//     &broad,
//     &narrow,
//     &solver,
//     &sorter,
//     &bufferInfo
//   };
//   GPUPhysics phys(device, task, physInfo);

  const CPUPhysicsParallel::CreateInfo physInfo{
    container.broad,
    container.narrow,
    container.solver,
    container.sorter,

    pool,
    &bufferInfo,
    
    updateDelta
  };
  //CPUPhysicsParallel phys(physInfo);
  container.physics = container.container.create<CPUPhysicsParallel>(physInfo);
  Global::get<PhysicsEngine>(container.physics);
}

void create_ai(system_container &container, dt::thread_pool* pool) {
  TimeLogDestructor AIsystem("AI system initialization");
  
  container.edge_container = container.container.create<graph::container>();
  Global::get(container.edge_container);
  
  const systems::pathfinder::create_info info{
    pool,
    container.edge_container,
    [] (const components::vertex* vert, const components::vertex* neighbor, const graph::edge* edge) -> float {
      (void)vert;
      (void)neighbor;
      return edge->length;
    },
    [] (const components::vertex* vert, const components::vertex* neighbor) -> float {
      return vert->goal_distance_estimate(neighbor);
    }
  };
  
  container.pathfinder_system = container.container.create<systems::pathfinder>(info);
  Global::get(container.pathfinder_system);
}

void create_sound_system(system_container &container) {
  TimeLogDestructor AIsystem("Sound system initialization");
  container.sound_system = container.container.create<systems::sound>();
  Global::get(container.sound_system);
}

void initnk(yavf::Device* device, Window* window, nuklear_data &data) {
  glfwSetKeyCallback(window->handle(), keyCallback);
  glfwSetCharCallback(window->handle(), charCallback);
  glfwSetMouseButtonCallback(window->handle(), mouseButtonCallback);
  glfwSetScrollCallback(window->handle(), scrollCallback);
  glfwSetWindowFocusCallback(window->handle(), focusCallback);
  glfwSetWindowIconifyCallback(window->handle(), iconifyCallback);

  nk_buffer_init_default(&data.cmds);

  {
    const void *image;
    int w, h;
    nk_font_atlas_init_default(&data.atlas);
    nk_font_atlas_begin(&data.atlas);
    data.font = nk_font_atlas_add_default(&data.atlas, 13.0f, NULL);
    image = nk_font_atlas_bake(&data.atlas, &w, &h, NK_FONT_ATLAS_RGBA32);
//     device_upload_atlas(&device, image, w, h); // загрузить текстуру
    yavf::Image* img = nullptr;
    yavf::ImageView* view = nullptr;
    {
      auto staging = device->create(yavf::ImageCreateInfo::texture2DStaging({static_cast<uint32_t>(w), static_cast<uint32_t>(h)}), VMA_MEMORY_USAGE_CPU_ONLY);

      const size_t imageSize = w * h * 4;
      memcpy(staging->ptr(), image, imageSize);

      img = device->create(yavf::ImageCreateInfo::texture2D({static_cast<uint32_t>(w), static_cast<uint32_t>(h)},
                                                            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT),
                           VMA_MEMORY_USAGE_GPU_ONLY);

      yavf::TransferTask* task = device->allocateTransferTask();

      task->begin();
      task->setBarrier(staging, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
      task->setBarrier(img, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
      task->copy(staging, img);
      task->setBarrier(img, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
      task->end();

      task->start();
      task->wait();

      device->deallocate(task);
      device->destroy(staging);

      view = img->createView(VK_IMAGE_VIEW_TYPE_2D, {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

      // тут нужен еще сэмплер и дескриптор
      yavf::Sampler sampler;
      {
        yavf::SamplerMaker sm(device);

        sampler = sm.addressMode(VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT)
                    .anisotropy(0.0f)
                    .borderColor(VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK)
                    .compareOp(VK_FALSE, VK_COMPARE_OP_GREATER)
                    .filter(VK_FILTER_NEAREST, VK_FILTER_NEAREST)
                    .lod(0.0f, 1.0f)
                    .mipmapMode(VK_SAMPLER_MIPMAP_MODE_NEAREST)
                    .unnormalizedCoordinates(VK_FALSE)
                    .create("default_nuklear_sampler");

//         img->setSampler(sampler);
      }

      {
        yavf::DescriptorPool pool = device->descriptorPool(DEFAULT_DESCRIPTOR_POOL_NAME);
        yavf::DescriptorSetLayout sampled_image_layout = device->setLayout(SAMPLED_IMAGE_LAYOUT_NAME);
        {
          yavf::DescriptorLayoutMaker dlm(device);

          if (sampled_image_layout == VK_NULL_HANDLE) {
            sampled_image_layout = dlm.binding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT).create(SAMPLED_IMAGE_LAYOUT_NAME);
          }
        }

        yavf::DescriptorMaker dm(device);

        auto d = dm.layout(sampled_image_layout).create(pool)[0];

        const size_t i = d->add({sampler, view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER});
        view->setDescriptor(d, i);
      }
    }

    // сюда мы по всей видимости передаем указатель на картинку
    nk_font_atlas_end(&data.atlas, nk_handle_ptr(view), &data.null);
  }

  nk_init_default(&data.ctx, &data.font->handle);

  data.ctx.clip.copy = clipbardCopy;
  data.ctx.clip.paste = clipbardPaste;
  data.ctx.clip.userdata = nk_handle_ptr(window);
  Global::get<input::data>()->current_text = 0;
}

void deinitnk(nuklear_data &data) {
  nk_font_atlas_clear(&data.atlas);
  nk_buffer_free(&data.cmds);
  nk_free(&data.ctx);
}

void nextnkFrame(Window* window, nk_context* ctx) {
  double x, y;
  int widht, height, display_width, display_height;
  glfwGetWindowSize(window->handle(), &widht, &height);
  glfwGetFramebufferSize(window->handle(), &display_width, &display_height);
  Global::get<input::data>()->fb_scale.x = float(display_width / widht);
  Global::get<input::data>()->fb_scale.y = float(display_height / height);

  if (Global::get<input::data>()->interface_focus) {
    nk_input_begin(ctx);
    // nk_input_unicode нужен для того чтобы собирать набранный текст
    // можем ли мы воспользоваться им сразу из коллбека?
    // по идее можем, там не оч сложные вычисления
    //nk_input_unicode(ctx, 'f');

    for (uint32_t i = 0; i < Global::get<input::data>()->current_text; ++i) {
      nk_input_unicode(ctx, Global::get<input::data>()->text[i]);
    }

    const uint32_t rel_event = static_cast<uint32_t>(input::type::release);
    //const bool* keys = Global::data()->keys;
    const auto keys = Global::get<input::data>()->key_events.container;
    nk_input_key(ctx, NK_KEY_DEL, keys[GLFW_KEY_DELETE].event != rel_event);
    nk_input_key(ctx, NK_KEY_ENTER, keys[GLFW_KEY_ENTER].event != rel_event);
    nk_input_key(ctx, NK_KEY_TAB, keys[GLFW_KEY_TAB].event != rel_event);
    nk_input_key(ctx, NK_KEY_BACKSPACE, keys[GLFW_KEY_BACKSPACE].event != rel_event);
    nk_input_key(ctx, NK_KEY_UP, keys[GLFW_KEY_UP].event != rel_event);
    nk_input_key(ctx, NK_KEY_DOWN, keys[GLFW_KEY_DOWN].event != rel_event);
    nk_input_key(ctx, NK_KEY_SHIFT, keys[GLFW_KEY_LEFT_SHIFT].event != rel_event ||
                                    keys[GLFW_KEY_RIGHT_SHIFT].event != rel_event);

    if (keys[GLFW_KEY_LEFT_CONTROL].event != rel_event ||
      keys[GLFW_KEY_RIGHT_CONTROL].event != rel_event) {
      nk_input_key(ctx, NK_KEY_COPY, keys[GLFW_KEY_C].event != rel_event);
      nk_input_key(ctx, NK_KEY_PASTE, keys[GLFW_KEY_V].event != rel_event);
      nk_input_key(ctx, NK_KEY_CUT, keys[GLFW_KEY_X].event != rel_event);
      nk_input_key(ctx, NK_KEY_TEXT_UNDO, keys[GLFW_KEY_Z].event != rel_event);
      nk_input_key(ctx, NK_KEY_TEXT_REDO, keys[GLFW_KEY_R].event != rel_event);
      nk_input_key(ctx, NK_KEY_TEXT_WORD_LEFT, keys[GLFW_KEY_LEFT].event != rel_event);
      nk_input_key(ctx, NK_KEY_TEXT_WORD_RIGHT, keys[GLFW_KEY_RIGHT].event != rel_event);
      nk_input_key(ctx, NK_KEY_TEXT_SELECT_ALL, keys[GLFW_KEY_A].event != rel_event);

      nk_input_key(ctx, NK_KEY_SCROLL_START, keys[GLFW_KEY_PAGE_DOWN].event != rel_event);
      nk_input_key(ctx, NK_KEY_SCROLL_END, keys[GLFW_KEY_PAGE_UP].event != rel_event);
      nk_input_key(ctx, NK_KEY_TEXT_START, keys[GLFW_KEY_HOME].event != rel_event);
      nk_input_key(ctx, NK_KEY_TEXT_END, keys[GLFW_KEY_END].event != rel_event);
    } else {
      nk_input_key(ctx, NK_KEY_LEFT, keys[GLFW_KEY_LEFT].event != rel_event);
      nk_input_key(ctx, NK_KEY_RIGHT, keys[GLFW_KEY_RIGHT].event != rel_event);
      nk_input_key(ctx, NK_KEY_COPY, 0);
      nk_input_key(ctx, NK_KEY_PASTE, 0);
      nk_input_key(ctx, NK_KEY_CUT, 0);
//       nk_input_key(ctx, NK_KEY_SHIFT, 0);

      nk_input_key(ctx, NK_KEY_SCROLL_DOWN, keys[GLFW_KEY_PAGE_DOWN].event != rel_event);
      nk_input_key(ctx, NK_KEY_SCROLL_UP, keys[GLFW_KEY_PAGE_UP].event != rel_event);
      nk_input_key(ctx, NK_KEY_TEXT_LINE_START, keys[GLFW_KEY_HOME].event != rel_event);
      nk_input_key(ctx, NK_KEY_TEXT_LINE_END, keys[GLFW_KEY_END].event != rel_event);
    }
    
//     nk_input_key(ctx, NK_KEY_ENTER, keys[GLFW_KEY_ENTER]);
//     nk_input_key(ctx, NK_KEY_TAB, keys[GLFW_KEY_TAB]);
//     nk_input_key(ctx, NK_KEY_BACKSPACE, keys[GLFW_KEY_BACKSPACE]);
//     nk_input_key(ctx, NK_KEY_UP, keys[GLFW_KEY_UP]);
//     nk_input_key(ctx, NK_KEY_DOWN, keys[GLFW_KEY_DOWN]);
//     nk_input_key(ctx, NK_KEY_SHIFT, keys[GLFW_KEY_LEFT_SHIFT] ||
//                                     keys[GLFW_KEY_RIGHT_SHIFT]);
// 
//     if (keys[GLFW_KEY_LEFT_CONTROL] ||
//       keys[GLFW_KEY_RIGHT_CONTROL]) {
//       nk_input_key(ctx, NK_KEY_COPY, keys[GLFW_KEY_C]);
//       nk_input_key(ctx, NK_KEY_PASTE, keys[GLFW_KEY_V]);
//       nk_input_key(ctx, NK_KEY_CUT, keys[GLFW_KEY_X]);
//       nk_input_key(ctx, NK_KEY_TEXT_UNDO, keys[GLFW_KEY_Z]);
//       nk_input_key(ctx, NK_KEY_TEXT_REDO, keys[GLFW_KEY_R]);
//       nk_input_key(ctx, NK_KEY_TEXT_WORD_LEFT, keys[GLFW_KEY_LEFT]);
//       nk_input_key(ctx, NK_KEY_TEXT_WORD_RIGHT, keys[GLFW_KEY_RIGHT]);
//       nk_input_key(ctx, NK_KEY_TEXT_SELECT_ALL, keys[GLFW_KEY_A]);
// 
//       nk_input_key(ctx, NK_KEY_SCROLL_START, keys[GLFW_KEY_PAGE_DOWN]);
//       nk_input_key(ctx, NK_KEY_SCROLL_END, keys[GLFW_KEY_PAGE_UP]);
//       nk_input_key(ctx, NK_KEY_TEXT_START, keys[GLFW_KEY_HOME]);
//       nk_input_key(ctx, NK_KEY_TEXT_END, keys[GLFW_KEY_END]);
//     } else {
//       nk_input_key(ctx, NK_KEY_LEFT, keys[GLFW_KEY_LEFT]);
//       nk_input_key(ctx, NK_KEY_RIGHT, keys[GLFW_KEY_RIGHT]);
//       nk_input_key(ctx, NK_KEY_COPY, 0);
//       nk_input_key(ctx, NK_KEY_PASTE, 0);
//       nk_input_key(ctx, NK_KEY_CUT, 0);
// //       nk_input_key(ctx, NK_KEY_SHIFT, 0);
// 
//       nk_input_key(ctx, NK_KEY_SCROLL_DOWN, keys[GLFW_KEY_PAGE_DOWN]);
//       nk_input_key(ctx, NK_KEY_SCROLL_UP, keys[GLFW_KEY_PAGE_UP]);
//       nk_input_key(ctx, NK_KEY_TEXT_LINE_START, keys[GLFW_KEY_HOME]);
//       nk_input_key(ctx, NK_KEY_TEXT_LINE_END, keys[GLFW_KEY_END]);
//     }

    glfwGetCursorPos(Global::window()->handle(), &x, &y);
    if (Global::window()->isFocused() && Global::get<input::data>()->interface_focus) {
      // Mouse position in screen coordinates (set to -1,-1 if no mouse / on another screen, etc.)
      nk_input_motion(ctx, (int)x, (int)y);
    } else {
      nk_input_motion(ctx, -1, -1);
    }

    // тоже заменить, также наклир дает возможность обработать даблклик, как это сделать верно?
    nk_input_button(ctx, NK_BUTTON_LEFT, (int)x, (int)y, keys[GLFW_MOUSE_BUTTON_LEFT].event != rel_event);
    nk_input_button(ctx, NK_BUTTON_MIDDLE, (int)x, (int)y, keys[GLFW_MOUSE_BUTTON_MIDDLE].event != rel_event);
    nk_input_button(ctx, NK_BUTTON_RIGHT, (int)x, (int)y, keys[GLFW_MOUSE_BUTTON_RIGHT].event != rel_event);

    bool doubleClick = false;
    if (keys[GLFW_MOUSE_BUTTON_LEFT].event != rel_event) {
      auto p = std::chrono::steady_clock::now();
      auto diff = p - Global::get<input::data>()->double_click_time_point;
      auto mcs = std::chrono::duration_cast<std::chrono::microseconds>(diff).count();
      doubleClick = mcs < DOUBLE_CLICK_TIME;
      if (!doubleClick) {
        Global::get<input::data>()->double_click_time_point = p;
      }
    }

    nk_input_button(ctx, NK_BUTTON_DOUBLE, int(Global::get<input::data>()->click_pos.x), int(Global::get<input::data>()->click_pos.y), doubleClick);
    nk_input_scroll(ctx, nk_vec2(Global::get<input::data>()->mouse_wheel, 0.0f));
    nk_input_end(ctx);
    // обнуляем
    Global::get<input::data>()->click_pos = glm::uvec2(x, y);
    Global::get<input::data>()->current_text = 0;
    Global::get<input::data>()->mouse_wheel = 0.0f;

    glfwSetInputMode(window->handle(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  } else {
    glfwSetInputMode(window->handle(), GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
  }

  // короче отрисовка наклира выглядит очень похоже на imgui
}

void nkOverlay(const SimpleOverlayData &data, nk_context* ctx) {
  nk_style* s = &ctx->style;
  nk_color* oldColor = &s->window.background;
  nk_style_item* oldStyleItem = &s->window.fixed_background;
  nk_style_push_color(ctx, oldColor, nk_rgba(oldColor->r, oldColor->g, oldColor->b, int(0.5f*255)));
  nk_style_push_style_item(ctx, oldStyleItem, nk_style_item_color(nk_rgba(oldStyleItem->data.color.r, oldStyleItem->data.color.g, oldStyleItem->data.color.b, int(0.5f*255))));

  if (nk_begin(ctx, "Basic window", nk_rect(10, 10, 300, 240),
        NK_WINDOW_NO_SCROLLBAR)) {
    {
      const simd::vec4 &pos = data.pos;
      float arr[4];
      pos.store(arr);

      const auto &str = fmt::sprintf("Camera pos: (%.2f,%.2f,%.2f,%.2f)", arr[0], arr[1], arr[2], arr[3]);

      nk_layout_row_static(ctx, 10.0f, 300, 1); // ряд высотой 30, каждый элемент шириной 300, 1 столбец
      nk_label(ctx, str.c_str(), NK_TEXT_LEFT); // nk_layout_row_static скорее всего нужно указывать каждый раз
    }

    {
      const simd::vec4 &dir = data.rot;
      float arr[4];
      dir.store(arr);

      const auto &str = fmt::sprintf("Camera dir: (%.2f,%.2f,%.2f)", arr[0], arr[1], arr[2]);

//       nk_layout_row_static(ctx, 30.0f, 300, 1);
      nk_label(ctx, str.c_str(), NK_TEXT_LEFT);
    }

    {
      const size_t average_frame_time = float(data.frameComputeTime) / float(data.frameCount);
      const auto &str = fmt::sprintf("Frame rendered in %lu mcs (%.2f fps)", average_frame_time, float(data.frameCount * TIME_PRECISION) / float(data.frameComputeTime));
      // в скором сремени так уже будет нельзя считать фпс, во время отрисовки добавятся вычисления
      // последний интервал тоже очень сильно изменился лол, что не так?
      // это может быть связано с тем что у меня добавились вычисления ии, но чтоб на 1-2 мс странно
      //  last interval frame time %lu mcs data.lastFrameComputeTime

//       nk_layout_row_static(ctx, 30.0f, 300, 1);
      nk_label(ctx, str.c_str(), NK_TEXT_LEFT);
    }

    {
      const size_t average_sleep_time = float(data.frameSleepTime) / float(data.frameCount);
      const auto &str = fmt::sprintf("Sleep between frames equals %lu mcs", average_sleep_time);

//       nk_layout_row_static(ctx, 30.0f, 300, 1);
      nk_label(ctx, str.c_str(), NK_TEXT_LEFT);
    }

    {
      const auto &str = fmt::sprintf("Final fps is %.2f", data.fps);

//       nk_layout_row_static(ctx, 30.0f, 300, 1);
      nk_label(ctx, str.c_str(), NK_TEXT_LEFT);
    }

    {
      const auto &str = fmt::sprintf("In frustum %zu objects", data.frustumObjCount);

//       nk_layout_row_static(ctx, 30.0f, 300, 1);
      nk_label(ctx, str.c_str(), NK_TEXT_LEFT);
    }

    {
      const auto &str = fmt::sprintf("Ray collide %zu objects", data.rayCollideCount);

//       nk_layout_row_static(ctx, 30.0f, 300, 1);
      nk_label(ctx, str.c_str(), NK_TEXT_LEFT);
    }

    {
      const auto &str = fmt::sprintf("Player see %zu objects", data.visibleObjCount);

//       nk_layout_row_static(ctx, 30.0f, 300, 1);
      nk_label(ctx, str.c_str(), NK_TEXT_LEFT);
    }
  }
  nk_end(ctx);

  nk_style_pop_color(ctx);
  nk_style_pop_style_item(ctx);
}

void setDescriptor(yavf::Buffer* buffer, yavf::DescriptorSet* set) {
  const size_t i = set->add({buffer, 0, buffer->info().size, 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
  buffer->setDescriptor(set, i);
}

void createDataArrays(yavf::Device* device, DataArrays &arrays) {
  GPUContainer<ExternalData>* externals = arrays.container.create<GPUContainer<ExternalData>>(device);
  arrays.externals = externals;
  
  GPUContainer<InputData>* inputs = arrays.container.create<GPUContainer<InputData>>(device);
  arrays.inputs = inputs;
  
  GPUContainer<simd::mat4>* matrices = arrays.container.create<GPUContainer<simd::mat4>>(device);
  arrays.matrices = matrices;
  
  GPUBuffer<uint32_t>* rotationCountBuffer = arrays.container.create<GPUBuffer<uint32_t>>(device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
  arrays.rotationCountBuffer = rotationCountBuffer;
  
  GPUContainer<RotationData>* rotations = arrays.container.create<GPUContainer<RotationData>>(device);
  arrays.rotations = rotations;
  
  GPUContainer<Transform>* transforms = arrays.container.create<GPUContainer<Transform>>(device);
  arrays.transforms = transforms;
  
  GPUContainer<Texture>* textures = arrays.container.create<GPUContainer<Texture>>(device);
  arrays.textures = textures;
  
//   arrays.animStates = arraysContainer.add<CPUContainer<AnimationState>>();
//   arrays.broadphasePairs = arraysContainer.add<GPUArray<BroadphasePair>>(device);

  TransformComponent::setContainer(arrays.transforms);
  InputComponent::setContainer(arrays.inputs);
//   GraphicComponent::setContainer(arrays.matrices);
//   GraphicComponent::setContainer(arrays.rotations);
//   GraphicComponent::setContainer(arrays.textures);
  PhysicsComponent::setContainer(arrays.externals);
//   AnimationComponent::setStateContainer(arrays.animStates);
  
  yavf::DescriptorPool pool = device->descriptorPool(DEFAULT_DESCRIPTOR_POOL_NAME);
  yavf::DescriptorSetLayout storage_layout = device->setLayout(STORAGE_BUFFER_LAYOUT_NAME);
  {
    yavf::DescriptorMaker dm(device);
    
//     for (uint32_t i = 0; i < sizeof(DataArrays)/sizeof(char*)-1; ++i) {
//       dm.layout(storage_layout);
//     }
    
    auto descs = dm.layout(storage_layout).create(pool);
    setDescriptor(externals->vector().handle(), descs[0]);
    
    descs = dm.layout(storage_layout).create(pool);
    setDescriptor(inputs->vector().handle(), descs[0]);
    
    descs = dm.layout(storage_layout).create(pool);
    setDescriptor(matrices->vector().handle(), descs[0]);
    
    descs = dm.layout(storage_layout).create(pool);
    setDescriptor(rotationCountBuffer->buffer(), descs[0]);
    
    descs = dm.layout(storage_layout).create(pool);
    setDescriptor(rotations->vector().handle(), descs[0]);
    
    descs = dm.layout(storage_layout).create(pool);
    setDescriptor(textures->vector().handle(), descs[0]);
    
    descs = dm.layout(storage_layout).create(pool);
    setDescriptor(transforms->vector().handle(), descs[0]);
  }
  
  core::interaction::matrices = matrices;
  core::interaction::transforms = transforms;
  
//   std::cout << "transforms desc " << transforms->vector().descriptorSet()->handle() << '\n';
}

// void destroyDataArrays(TypelessContainer &arraysContainer, DataArrays &arrays) {
//   arraysContainer.destroy(arrays.externals);
//   arraysContainer.destroy(arrays.inputs);
//   arraysContainer.destroy(arrays.matrices);
//   arraysContainer.destroy(arrays.rotationCountBuffer);
//   arraysContainer.destroy(arrays.rotations);
//   arraysContainer.destroy(arrays.transforms);
// }

void createRenderStages(const RenderConstructData &data, std::vector<DynamicPipelineStage*> &dynPipe) {
  Global::render()->addStage<BeginTaskStage>(); //data.container->tasks3()
  
  MonsterGPUOptimizer* monopt = nullptr;
  {
    const MonsterGPUOptimizer::CreateInfo info{
      data.systems->graphics_container->device(),
//       data.container->tasks1(),
      Global::render()->getCameraDataBuffer()
    };
    monopt = Global::render()->addStage<MonsterGPUOptimizer>(info);
    
    monopt->setInputBuffers({data.arrays->transforms, data.arrays->matrices, data.arrays->textures});
  }
  
  GeometryGPUOptimizer* geoopt = nullptr;
  {
    const GeometryGPUOptimizer::CreateInfo info{
      data.systems->graphics_container->device(),
//       data.container->tasks1(),
      Global::render()->getCameraDataBuffer()
    };
    geoopt = Global::render()->addStage<GeometryGPUOptimizer>(info);
    
    geoopt->setInputBuffers({data.arrays->transforms, data.arrays->matrices, data.arrays->rotationCountBuffer, data.arrays->rotations, data.arrays->textures});
  }
  
//   GraphicComponent::setOptimizer(monopt);
//   GraphicComponentIndexes::setOptimizer(geoopt);

  Global::render()->addOptimizerToClear(monopt);
  Global::render()->addOptimizerToClear(geoopt);
  Global::get(monopt);
  Global::get(geoopt);

  // короч для того чтобы перенести оптимизеры на гпу
  // мне нужно добавить много новых стейджев, может быть немного пересмотреть концепцию?
  // отдельно вытащить оптимизеры... создать отдельный стейдж с оптимизацией?
  //

//   yavf::GraphicTask* graphicsTasks = data.task[0];
//   yavf::ComputeTask* computeTasks = data.task[0];

  const size_t gBufferStageContainerSize = sizeof(MonsterGBufferStage) + sizeof(GeometryGBufferStage);
  const GBufferStage::CreateInfo info{
    data.systems->graphics_container->device(),
    Global::render()->getCameraDataBuffer(),
//     data.container->tasks2(), //reinterpret_cast<yavf::GraphicTask**>(data.task),// &graphicsTasks,
    Global::window()->size().extent.width,
    Global::window()->size().extent.height

//       textureLoader->imageDescriptor(),
//       textureLoader->samplerDescriptor()
  };
  GBufferStage* gBuffer = Global::render()->addStage<GBufferStage>(gBufferStageContainerSize, info);
  dynPipe.push_back(gBuffer);

  auto buffer = Global::get<game::map_data_container>()->vertices;
  auto monGbuffer = gBuffer->addPart<MonsterGBufferStage>(monopt);
  auto geoGbuffer = gBuffer->addPart<GeometryGBufferStage>(buffer, geoopt);
  
  (void)monGbuffer;
  (void)geoGbuffer;

//   data.monDebugOpt->setInputBuffers({monGbuffer->getInstanceData()});
//   data.geoDebugOpt->setInputBuffers({geoGbuffer->getInstanceData()});

//     gBuffer->recreatePipelines(textureLoader); // не тут это должно быть

  // нужно получить из деферед дескриптор
  const DefferedLightStage::CreateInfo dInfo{
    data.systems->graphics_container->device(),
    Global::render()->getCameraDataBuffer(),
    Global::render()->getMatrixesBuffer(),
//     data.container->tasks1(), //reinterpret_cast<yavf::ComputeTask**>(data.task), //&computeTasks,

    Global::get<LightOptimizer>(),

    Global::window()->size().extent.width,
    Global::window()->size().extent.height,

    gBuffer->getDeferredRenderTargetDescriptor(),
    gBuffer->getDeferredRenderTargetLayoutDescriptor()
  };
  DefferedLightStage* lightStage = Global::render()->addStage<DefferedLightStage>(dInfo);

  const ToneMappingStage::CreateInfo tInfo{
    data.systems->graphics_container->device(),
//     data.container->tasks1(), //reinterpret_cast<yavf::ComputeTask**>(data.task), //&computeTasks,

    Global::window()->size().extent.width,
    Global::window()->size().extent.height,

    lightStage->getOutputDescriptor()
  };
  ToneMappingStage* tone = Global::render()->addStage<ToneMappingStage>(tInfo);

  const CopyStage::CreateInfo cInfo{
    data.systems->graphics_container->device(),
//     data.container->tasks2(), //reinterpret_cast<yavf::GraphicTask**>(data.task), //&graphicsTasks,

    tone->getOutputImage(),
    gBuffer->getDepthBuffer(),

    Global::window()->getFamily(),
    Global::window()
  };
  CopyStage* copy = Global::render()->addStage<CopyStage>(cInfo);
  (void)copy;

  // и отрисовка гуи
  // до гуи у нас еще должна быть закраска какой-нибудь текстурой если у нас ничего не отрисовалось
  // может быть это скайбокс? вполне возможно

  const size_t postRenderStageContainerSize = sizeof(GuiStage) + sizeof(MonsterDebugStage) + sizeof(GeometryDebugStage);
  const PostRenderStage::CreateInfo pInfo{
    data.systems->graphics_container->device(),
//     data.container->tasks2(), //reinterpret_cast<yavf::GraphicTask**>(data.task), //&graphicsTasks,
    Global::window()
  };
  PostRenderStage* postRender = Global::render()->addStage<PostRenderStage>(postRenderStageContainerSize, pInfo);
  dynPipe.push_back(postRender);

//     const GuiStage::CreateInfo gInfo{
//       device,
//       task,
//       window
//     };
  //GuiStage* gui = render->create<GuiStage>(gInfo);
  GuiStage* gui = postRender->addPart<GuiStage>(data.data);
  (void)gui;

//   const MonsterDebugStage::CreateInfo mdInfo{
//     data.monDebugOpt,
//     data.mon,
//     data.render->getCameraDataBuffer(),
//     monGbuffer->getInstanceData()
//   };
//   MonsterDebugStage* monDebug = postRender->addPart<MonsterDebugStage>(mdInfo);
// 
//   const GeometryDebugStage::CreateInfo gdInfo{
//     data.geoDebugOpt,
//     data.geo,
// 
//     data.render->getCameraDataBuffer(),
// 
//     geoGbuffer->getIndicesArray(),
//     data.mapLoader->mapVertices()
//   };
//   GeometryDebugStage* geoDebug = postRender->addPart<GeometryDebugStage>(gdInfo);

  //data.render->create<EndTaskStage>(reinterpret_cast<yavf::TaskInterface**>(data.task));
  Global::render()->addStage<EndTaskStage>(); // data.container->tasks3()
}

// void createAI(dt::thread_pool* threadPool, const size_t &updateDelta, GameSystemContainer &container) {
//   TimeLogDestructor AIsystem("AI system initialization");
//   
//   const systems::pathfinder::create_info info{
//     threadPool,
//     graph,
//     [] (const components::vertex* vert, const components::vertex* neighbor, const graph::edge* edge) -> float {
//       (void)vert;
//       (void)neighbor;
//       return edge->length;
//     },
//     [] (const components::vertex* vert, const components::vertex* neighbor) -> float {
//       return vert->goal_distance_estimate(neighbor);
//     }
//   };
//   
//   const CPUAISystem::CreateInfo info {
//     threadPool,
//     updateDelta,
//     sizeof(CPUPathFindingPhaseParallel) + sizeof(Graph)
//   };
//   CPUAISystem* system = container.addSystem<CPUAISystem>(info);
//   
//   Graph* graph = system->createGraph<Graph>();
//   
//   const CPUPathFindingPhaseParallel::CreateInfo pathInfo {
//     threadPool,
//     graph,
//     [] (const vertex_t* vert, const vertex_t* neighbor, const edge_t* edge) -> float {
//       (void)vert;
//       (void)neighbor;
//       return edge->getDistance();
//     },
//     [] (const vertex_t* vert, const vertex_t* neighbor) -> float {
//       return vert->goalDistanceEstimate(neighbor);
//     }
//   };
//   
//   CPUPathFindingPhaseParallel* path = system->createPathfindingSystem<CPUPathFindingPhaseParallel>(pathInfo);
//   path->registerPathType(Type::get("default"), [] (const vertex_t*, const vertex_t* neighbor, const edge_t* edge) -> bool {
//     if (edge->getAngle() > PASSABLE_ANGLE) return false;
//     const glm::vec4 norm = neighbor->getVertexData()->normal;
//     if (getAngle(-Global::physics()->getGravityNorm(), simd::vec4(&norm.x)) > PASSABLE_ANGLE) return false;
//     if (edge->getWidth() < 1.0f) return false;
//     if (edge->isFake() && edge->getHeight() > 1.0f) return false;
//     //if (edge.isFake()) return false;
//     
//     return true;
//   }, 0.3f);
//   
//   Global::get<AISystem>(system);
//   Global g;
//   g.setAISystem(system);
// }

std::unordered_map<utils::id, tb::BehaviorTree*> createBehaviourTrees() {
  // 200% нужно проработать мультитрединг (создать контейнер для необходимых данных)
  // в будущем мы обязательно должны воспользоваться как можно большим количеством предсозданных деревьев
  // эти деревья будут брать на себя какие то во первых типовые участки во вторых сложные в плане вычислений участки
//   // например chaseAndAttack или смерть
  
  std::unordered_map<utils::id, tb::BehaviorTree*> trees;
  
  tb::BehaviorTreeBuilder builder;
  {
    tb::BehaviorTree* tree;
    tree = builder.sequence()
                    .action([] (tb::Node* const& node, void* const& ptr) -> tb::Node::status {
                      (void)node;
                      yacs::entity* ent = reinterpret_cast<yacs::entity*>(ptr);
                      
  //                     std::cout << "Start tree" << "\n";
  //                     std::cout << "target " << ai->target() << "\n";
                      auto ai = ent->at<components::tree_ai>(game::monster::ai);
                      auto target = ai->data[0].second;
                      
                      if (target != nullptr) return tb::Node::status::success;
                            
                      return tb::Node::status::failure;
                    })
                    .action([] (tb::Node* const& node, void* const& ptr) {
                      (void)node;
                      yacs::entity* ent = reinterpret_cast<yacs::entity*>(ptr);
                      auto movement = ent->at<components::movement>(game::monster::movement);
                      auto ai = ent->at<components::tree_ai>(game::monster::ai);
                      auto target = ai->data[0].second;
                      auto s = movement->find_path(target);
                      
                      std::cout << "Finding path" << "\n";
                      if (s == components::movement::path_state::finding) return tb::Node::status::running;
                      if (s == components::movement::path_state::found) return tb::Node::status::success;
                      
                      return tb::Node::status::failure;
                    })
                    .action([] (tb::Node* const& node, void* const& ptr) {
                      (void)node;
                      yacs::entity* ent = reinterpret_cast<yacs::entity*>(ptr);
                      auto movement = ent->at<components::movement>(game::monster::movement);
                      auto s = movement->travel_path();
                      
                      std::cout << "Move path" << "\n";
                      
                      if (s == components::movement::state::end_travel) return tb::Node::status::success;
                      if (s == components::movement::state::path_not_exist) return tb::Node::status::failure;
                      
                      return tb::Node::status::running;
                    })
                    .action([] (tb::Node* const& node, void* const& ptr) {
                      (void)node;
                      yacs::entity* ent = reinterpret_cast<yacs::entity*>(ptr);
                      
                      std::cout << "This is last node" << "\n";
                      auto ai = ent->at<components::tree_ai>(game::monster::ai);
                      ai->data[0].second = nullptr;
                      return tb::Node::status::success;
                    })
                  .end()
                .build();
                
    trees[utils::id::get("simple_tree")] = tree;
  }
  
  return trees;
}

std::unordered_map<std::string, core::attribute_t<core::float_type>::type::func_type> create_float_attribs_funcs() {
  std::unordered_map<std::string, core::attribute_t<core::float_type>::type::func_type> attribs_func;
  
  attribs_func["default"] = [] (yacs::entity* ent, const struct core::attribute_t<core::float_type>::type* type, const core::float_type &base, const core::float_type &rawAdd, const float &rawMul, const core::float_type &finalAdd, const float &finalMul) -> core::float_type {
    (void)ent;
    (void)type;
    core::float_type value = base;
    
    value *= rawMul;
    value += rawAdd;
    
    value *= finalMul;
    value += finalAdd;
    
    return value;
  };
  
  return attribs_func;
}

std::unordered_map<std::string, core::attribute_t<core::int_type>::type::func_type> create_int_attribs_funcs() {
  std::unordered_map<std::string, core::attribute_t<core::int_type>::type::func_type> attribs_func;
  
  attribs_func["default"] = [] (yacs::entity* ent, const struct core::attribute_t<core::int_type>::type* type, const core::int_type &base, const core::int_type &rawAdd, const float &rawMul, const core::int_type &finalAdd, const float &finalMul) -> core::int_type {
    (void)ent;
    (void)type;
    core::int_type value = base;
    
    value *= rawMul;
    value += rawAdd;
    
    value *= finalMul;
    value += finalAdd;
    
    return value;
  };
  
  attribs_func["compute_health"] = game::health_func;
  
  return attribs_func;
}

std::unordered_map<std::string, core::state_t::action_func> create_states_funcs() {
  std::unordered_map<std::string, core::state_t::action_func> states_funcs;
  
  return states_funcs;
}

std::unordered_map<std::string, core::entity_creator::collision_func_t> create_collision_funcs() {
  std::unordered_map<std::string, core::entity_creator::collision_func_t> collision_funcs;
  
  return collision_funcs;
}

resources_ptr::resources_ptr(yavf::Device* device) : 
    resources_containers(
      sizeof(game::image_data_container_load) +
      sizeof(game::image_resources_load) +
      sizeof(game::sounds_container_load) +
      sizeof(game::abilities_container_load) + 
      sizeof(game::effects_container_load) + 
      sizeof(game::states_container_load) + 
      sizeof(game::weapons_container_load) + 
      sizeof(game::float_attribute_types_container_load) + 
      sizeof(game::int_attribute_types_container_load) +
      sizeof(game::entity_creators_container_load) + 
      sizeof(game::map_data_container_load)
    ), images(nullptr), image_res(nullptr), sounds(nullptr), abilities(nullptr), effects(nullptr), states(nullptr), weapons(nullptr), float_attribs(nullptr), int_attribs(nullptr), entities(nullptr), map_data(nullptr),
    float_funcs(create_float_attribs_funcs()), int_funcs(create_int_attribs_funcs()), states_funcs(create_states_funcs()), collision_funcs(create_collision_funcs()), trees(createBehaviourTrees())
  {
//     std::cout << "resources_ptr size " << sizeof(game::image_data_container_load) +
//       sizeof(game::image_resources_load) +
//       sizeof(game::sounds_container_load) +
//       sizeof(game::abilities_container_load) + 
//       sizeof(game::effects_container_load) + 
//       sizeof(game::states_container_load) + 
//       sizeof(game::weapons_container_load) + 
//       sizeof(game::float_attribute_types_container_load) + 
//       sizeof(game::int_attribute_types_container_load) +
//       sizeof(game::entity_creators_container_load) + 
//       sizeof(game::map_data_container_load) << "\n";
    
    {
      images = resources_containers.create<game::image_data_container_load>();
      Global::get<game::image_data_container>(images);
    }
    
    {
      image_res = resources_containers.create<game::image_resources_load>(device);
      Global::get<game::image_resources>(image_res);
    }
    
    {
      sounds = resources_containers.create<game::sounds_container_load>();
      Global::get<game::sounds_container>(sounds);
    }
    
    {
      abilities = resources_containers.create<game::abilities_container_load>();
      Global::get<game::abilities_container>(abilities);
    }
    
    {
      effects = resources_containers.create<game::effects_container_load>();
      Global::get<game::effects_container>(effects);
    }
    
    {
      states = resources_containers.create<game::states_container_load>();
      Global::get<game::states_container>(states);
    }
    
    {
      weapons = resources_containers.create<game::weapons_container_load>();
      Global::get<game::weapons_container>(weapons);
    }
    
    {
      float_attribs = resources_containers.create<game::float_attribute_types_container_load>();
      Global::get<game::float_attribute_types_container>(float_attribs);
    }
    
    {
      int_attribs = resources_containers.create<game::int_attribute_types_container_load>();
      Global::get<game::int_attribute_types_container>(int_attribs);
    }
    
    {
      entities = resources_containers.create<game::entity_creators_container_load>();
      Global::get<game::entity_creators_container>(entities);
    }
    
    {
      map_data = resources_containers.create<game::map_data_container_load>();
      Global::get<game::map_data_container>(map_data);
      map_data->vertices = nullptr;
    }
  }
  
resources_ptr::~resources_ptr() {
  if (images != nullptr) resources_containers.destroy(images);
  if (image_res != nullptr) resources_containers.destroy(image_res);
  if (sounds != nullptr) resources_containers.destroy(sounds);
  if (abilities != nullptr) resources_containers.destroy(abilities);
  if (effects != nullptr) resources_containers.destroy(effects);
  if (states != nullptr) resources_containers.destroy(states);
  if (weapons != nullptr) resources_containers.destroy(weapons);
  if (float_attribs != nullptr) resources_containers.destroy(float_attribs);
  if (int_attribs != nullptr) resources_containers.destroy(int_attribs);
  if (entities != nullptr) resources_containers.destroy(entities);
  if (map_data != nullptr) resources_containers.destroy(map_data);
  for (auto tree : trees) {
    delete tree.second;
  }
}

void createLoaders(resources::modification_container &mods, GraphicsContainer* graphicsContainer, const DataArrays &data_arrays, render::image_container* images, resources_ptr &res, resources::map_loader** mapLoader) {
  resources::image_loader* texture_loader = nullptr;
  resources::state_loader* state_loader = nullptr;
  resources::entity_loader* entity_loader = nullptr;
  resources::abilities_loader* abilities_loader = nullptr;
  resources::attributes_loader* attributes_loader = nullptr;
  resources::effects_loader* effects_loader = nullptr;
  resources::sound_loader* sound_loader = nullptr;
  
  ASSERT(graphicsContainer->device());
  
  {
    const resources::image_loader::create_info info{
      graphicsContainer->device(),
      images,
      res.images,
      res.image_res
    };
    texture_loader = mods.add_loader<resources::image_loader>(info);
  }

  {
    const resources::sound_loader::create_info info{
      res.sounds
    };
    sound_loader = mods.add_loader<resources::sound_loader>(info);
  }
  
  {
    const resources::attributes_loader::create_info info{
      res.float_attribs,
      res.int_attribs,
      res.float_funcs,
      res.int_funcs
    };
    attributes_loader = mods.add_loader<resources::attributes_loader>(info);
  }
  
  {
    const resources::effects_loader::create_info info{
      attributes_loader,
      res.effects
    };
    effects_loader = mods.add_loader<resources::effects_loader>(info);
  }
  
  {
    const resources::state_loader::create_info info{
      res.states,
      data_arrays.textures,
      texture_loader,
      res.states_funcs
    };
    state_loader = mods.add_loader<resources::state_loader>(info);
  }
  
  {
    const resources::abilities_loader::create_info info{
      res.abilities,
      state_loader,
      effects_loader
    };
    abilities_loader = mods.add_loader<resources::abilities_loader>(info);
  }
  
  {
    const resources::entity_loader::create_info info{
      abilities_loader,
      attributes_loader,
      state_loader,
      res.entities,
      res.trees,
      res.collision_funcs
    };
    entity_loader = mods.add_loader<resources::entity_loader>(info);
  }
  
  {
    const resources::map_loader::create_info info{
      Global::get<yacs::world>(),
      entity_loader,
      state_loader,
      texture_loader,
      graphicsContainer->device(),
      res.map_data
    };
    *mapLoader = mods.add_loader<resources::map_loader>(info);
  }

//   const HardcodedMapLoader::CreateInfo mInfo{
//     graphicsContainer->device(),
//     entity_loader,
//     texture_loader
//   };
//   *mapLoader = new HardcodedMapLoader(mInfo);

//   const ModificationContainer::CreateInfo pInfo{
//     0
//   };
//   *mods = loaderContainer.addModParser<ModificationContainer>(pInfo);
//   
//   ModificationContainer* cont = *mods;
//   cont->addParser(textureLoader);
//   cont->addParser(soundLoader);
//   cont->addParser(animationLoader);
//   cont->addParser(attributesLoader);
//   cont->addParser(effectsLoader);
//   cont->addParser(abilityTypeLoader);
//   cont->addParser(itemLoader);
//   cont->addParser(entityLoader);
//   cont->addParser(*mapLoader);
}

void createSoundSystem(dt::thread_pool* threadPool, GameSystemContainer &container) {
  TimeLogDestructor soundSystemLog("Sound system initialization");
  //SoundLoader* soundLoader = static_cast<SoundLoader*>(loaders[1]);
  
  (void)threadPool;
  
//   const SoundSystem::CreateInfo sInfo {
//     threadPool,
//     soundLoader, // должен быть контейнер
//     delaySoundWork
//   };
  
  auto soundSystem = container.addSystem<systems::sound>();
  Global::get(soundSystem);

//     const SoundLoader::LoadData sound{
//       "default_sound",
//       Global::getGameDir() + "tmrdata/sounds/Curio feat. Lucy - Ten Feet (Daxten Remix).mp3",
//       false,
//       false
//     };
//     soundLoader->load(sound);
}

void nextGuiFrame() {
//   static double timef = 0.0;
//   ImGuiIO &io = ImGui::GetIO();
//
//   // Setup display size (every frame to accommodate for window resizing)
//   int w, h;
//   int display_w, display_h;
//   glfwGetWindowSize(Global::window()->handle(), &w, &h);
//   glfwGetFramebufferSize(Global::window()->handle(), &display_w, &display_h);
//   io.DisplaySize = ImVec2(float(w), float(h));
//   io.DisplayFramebufferScale = ImVec2(w > 0 ? (float(display_w) / w) : 0, h > 0 ? (float(display_h) / h) : 0);
//
//   // Setup time step
//   double current_time = glfwGetTime();
//   io.DeltaTime = timef > 0.0 ? float(current_time - timef) : float(1.0f/60.0f);
//   timef = current_time;
//
//   if (Global::data()->focusOnInterface) {
//     // Setup inputs
//     // (we already got mouse wheel, keyboard keys & characters from glfw callbacks polled in glfwPollEvents())
//     if (Global::window()->isFocused()) {
//       double mouse_x, mouse_y;
//       glfwGetCursorPos(Global::window()->handle(), &mouse_x, &mouse_y);
//       // Mouse position in screen coordinates (set to -1,-1 if no mouse / on another screen, etc.)
//       io.MousePos = ImVec2(float(mouse_x), float(mouse_y));
//     } else {
//       io.MousePos = ImVec2(-1,-1);
//     }
//
// //     for (int i = 0; i < 3; i++) {
// //       // If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
// //       io.MouseDown[i] = mousePressed[i] || glfwGetMouseButton(window->handle(), i) != 0;
// //       mousePressed[i] = false;
// //     }
//
//     // If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
//     // клик меньше чем за один фрейм? может быть когда фпс мелкий? может пригодится
//     for (uint32_t i = 0; i < 5; ++i) {
//       io.MouseDown[i] = Global::data()->keys[i];
//     }
//
//     io.MouseWheel = Global::data()->mouseWheel;
//     Global::data()->mouseWheel = 0.0f;
//
//     // Hide OS mouse cursor if ImGui is drawing it
//     glfwSetInputMode(Global::window()->handle(), GLFW_CURSOR, io.MouseDrawCursor ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);
//   } else {
//     glfwSetInputMode(Global::window()->handle(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
//   }
//
//   // Start the frame
//   ImGui::NewFrame();
}

void sync(TimeMeter &tm, const size_t &syncTime) {
  tm.stop();

  {
//     RegionLog rl("Global::render()->wait()");
    Global::render()->wait();
  }

  size_t accumulatedTime = 0;
  while (accumulatedTime < syncTime) {
    std::this_thread::sleep_for(std::chrono::microseconds(1));
    const auto point = std::chrono::steady_clock::now() - tm.getStart();
    accumulatedTime = std::chrono::duration_cast<std::chrono::microseconds>(point).count();
  }
}

void createReactions(const ReactionsCreateInfo &info) {
//   auto input = info.input;
//   info.container->create("Step forward", [input] () {
//     input->forward();
//   });
// 
//   info.container->create("Step backward", [input] () {
//     input->backward();
//   });
// 
//   info.container->create("Step left", [input] () {
//     input->left();
//   });
// 
//   info.container->create("Step right", [input] () {
//     input->right();
//   });
// 
//   info.container->create("Jump", [input] () {
//     input->jump();
//   });
  
  auto menu = info.menuContainer;
  info.container->create("Menu", [menu] {
    menu->open();
    Global::get<input::data>()->interface_focus = true;
  });
  
//   info.container->create("menu next", [menu] () {
//     menu->next();
//   });
//   
//   info.container->create("menu prev", [menu] () {
//     menu->prev();
//   });
//   
//   info.container->create("menu increase", [menu] () {
//     menu->increase();
//   });
//   
//   info.container->create("menu decrease", [menu] () {
//     menu->decrease();
//   });
//   
//   info.container->create("menu choose", [menu] () {
//     menu->choose();
//   });
  
  info.container->create("menu escape", [menu] () {
    menu->escape();
  });

  auto window = info.window;
  info.container->create("Interface focus", [window] () {
    static bool lastFocus = false;
    Global::get<input::data>()->interface_focus = !Global::get<input::data>()->interface_focus;

    if (lastFocus && Global::get<input::data>()->interface_focus != lastFocus) {
      int width, height;
      glfwGetWindowSize(window->handle(), &width, &height);
      double centerX = double(width) / 2.0, centerY = double(height) / 2.0;
      glfwSetCursorPos(window->handle(), centerX, centerY);
    }

    lastFocus = Global::get<input::data>()->interface_focus;
  });
  
//   auto brain = info.brain;
//   info.container->create("Set target", [brain, input] () {
//     const AIComponent* comp = static_cast<const AIComponent*>(brain);
// 
//     //auto* phys = input->getEntity()->get<PhysicsComponent2>().get();
//     auto phys = comp->components()->entity->at<PhysicsComponent>(PHYSICS_COMPONENT_INDEX);
//     if (phys->getGround() == nullptr) {
//       const Object obj = Global::physics()->getObjectData(&phys->getIndexContainer());
//       std::cout << "obj index " << obj.objectId << "\n";
//       std::cout << "obj ground index " << obj.groundObjIndex << "\n";
//       throw std::runtime_error("phys->getGround() return nullptr");
//     }
//     auto data = reinterpret_cast<UserDataComponent*>(phys->getGround()->userData);
//     
//     if (data->aiComponent == nullptr) {
//       const Object obj = Global::physics()->getObjectData(&phys->getIndexContainer());
//       std::cout << "obj index " << obj.objectId << "\n";
//       std::cout << "obj ground index " << obj.groundObjIndex << "\n";
//       throw std::runtime_error("data->aiComponent return nullptr");
//     }
//     
//     std::cout << "setting target " << data->aiComponent << "\n";
//     
//     brain->target(data->aiComponent);
//   });

  // а также use, attack, spells (1-9?), item use, hide weapon
  // и прочее
  // где располагать доступ ко всему этому?
  // может ли пользователь добавить свои функции к вызову?
}

const utils::id move_forward = utils::id::get("move_forward");
const utils::id move_backward = utils::id::get("move_backward");
const utils::id move_right = utils::id::get("move_right");
const utils::id move_left = utils::id::get("move_left");
const utils::id jump = utils::id::get("jump");
const utils::id escape = utils::id::get("escape");
const utils::id interface_focus = utils::id::get("interface_focus");

const utils::id menu_next = utils::id::get("menu_next");
const utils::id menu_prev = utils::id::get("menu_prev");
const utils::id menu_increase = utils::id::get("menu_increase");
const utils::id menu_decrease = utils::id::get("menu_decrease");
const utils::id menu_choose = utils::id::get("menu_choose");

void setUpKeys(KeyContainer* container) {
//   {
//     container->create({key::state::press, key::state::double_press, key::state::long_press}, false, key::modificator::none, GLFW_KEY_W, container->config.reactions["Step forward"]);
// //     key->setReaction(KEY_STATE_PRESS,        container->config->reactions["Step forward"]);
// //     key->setReaction(KEY_STATE_DOUBLE_PRESS, container->config->reactions["Step forward"]);
// //     key->setReaction(KEY_STATE_LONG_PRESS,   container->config->reactions["Step forward"]);
//   }
// 
//   container->create({key::state::press, key::state::double_press, key::state::long_press}, false, key::modificator::none, GLFW_KEY_S, container->config.reactions["Step backward"]);
//   container->create({key::state::press, key::state::double_press, key::state::long_press}, false, key::modificator::none, GLFW_KEY_A, container->config.reactions["Step left"]);
//   container->create({key::state::press, key::state::double_press, key::state::long_press}, false, key::modificator::none, GLFW_KEY_D, container->config.reactions["Step right"]);
// 
//   container->create({key::state::press, key::state::long_press}, false, key::modificator::none, GLFW_KEY_SPACE, container->config.reactions["Jump"]);
//   
//   container->create({key::state::click}, true, key::modificator::none, GLFW_KEY_LEFT_ALT, container->config.reactions["Interface focus"]);
//   
//   container->create({key::state::click}, false, key::modificator::none, GLFW_KEY_ESCAPE, container->config.reactions["Menu"]);
  
  // нужно ли искать эвент который уже был привязан к клавише?
  // нужно сделать две вещи: посмотреть чтобы эвент был привязан максимум к двум клавишам
  // и показать сообщение если вдруг мы попробуем перезаписать клавишу
  // кстати у клавиш действительно может быть несколько эвентов
  // например в игре эвенты, эвенты в меню, эвенты в транспорте и проч
  input::set_key(GLFW_KEY_W, move_forward);
  input::set_key(GLFW_KEY_S, move_backward);
  input::set_key(GLFW_KEY_A, move_left);
  input::set_key(GLFW_KEY_D, move_right);
  input::set_key(GLFW_KEY_SPACE, jump);
  input::set_key(GLFW_KEY_ESCAPE, escape);
  input::set_key(GLFW_KEY_LEFT_ALT, interface_focus);
  
  // здесь появляется потребность сделать несколько эвентов на одну кнопку
  input::set_key(GLFW_KEY_UP, menu_prev);
  input::set_key(GLFW_KEY_DOWN, menu_next);
  input::set_key(GLFW_KEY_RIGHT, menu_increase);
  input::set_key(GLFW_KEY_LEFT, menu_decrease);
  input::set_key(GLFW_KEY_ENTER, menu_choose);
  
//   container->create({key::state::click}, true, key::modificator::none, GLFW_KEY_M, container->config.reactions["Set target"]);
}

void mouse_input(yacs::entity* player, const size_t &time) {
  if (Global::get<input::data>()->interface_focus) return;
  
  auto input = player->at<UserInputComponent>(game::entity::input);

  double xpos, ypos;
  int32_t width, height;
  {
    // RegionLog rl("glfwGetCursorPos");
    glfwGetCursorPos(Global::window()->handle(), &xpos, &ypos);

  }

  {
    // RegionLog rl("glfwGetFramebufferSize");
    glfwGetFramebufferSize(Global::window()->handle(), &width, &height);
  }

  {
    // RegionLog rl("glfwSetCursorPos");
    double centerX = double(width) / 2.0, centerY = double(height) / 2.0;
    glfwSetCursorPos(Global::window()->handle(), centerX, centerY);
  }

  // играя с +, - можно делать такие штуки как инверися по осям
  // чувствительность мыши это mouseSpeed

  static float horisontalAngle = 0.0f, verticalAngle = 0.0f;

  static const float mouseSpeed = 5.0f;

  horisontalAngle = mouseSpeed * MCS_TO_SEC(time) * (float(width)  / 2.0f - float(xpos));
  verticalAngle   = mouseSpeed * MCS_TO_SEC(time) * (float(height) / 2.0f - float(ypos));

//   std::cout << "width: " << width << "\n";
//   std::cout << "height: " << height << "\n";
//   std::cout << "xpos: " << xpos << "\n";
//   std::cout << "ypos: " << ypos << "\n";

  input->mouseMove(horisontalAngle, verticalAngle, PhysicsEngine::getOrientation());
}

void keysCallbacks(KeyContainer* container, const uint64_t &time) {
//   const bool shift_mod = Global::data()->keys[GLFW_KEY_LEFT_SHIFT] || 
//                          Global::data()->keys[GLFW_KEY_RIGHT_SHIFT];
//   const bool control_mod = Global::data()->keys[GLFW_KEY_LEFT_CONTROL] || 
//                            Global::data()->keys[GLFW_KEY_RIGHT_CONTROL];
//   const bool super_mod = Global::data()->keys[GLFW_KEY_LEFT_SUPER] || 
//                          Global::data()->keys[GLFW_KEY_RIGHT_SUPER];
//   const bool alt_mod = Global::data()->keys[GLFW_KEY_LEFT_ALT] || 
//                        Global::data()->keys[GLFW_KEY_RIGHT_ALT];
//   const bool backspace_mod = Global::data()->keys[GLFW_KEY_BACKSPACE];
//                        
// //   const bool modificators = shift_mod || control_mod || super_mod || alt_mod || backspace_mod;
//   
//   std::vector<key::modificator> mods;
//   if (control_mod) mods.push_back(key::modificator::ctrl);
//   if (alt_mod) mods.push_back(key::modificator::alt);
//   if (shift_mod) mods.push_back(key::modificator::shift);
//   if (super_mod) mods.push_back(key::modificator::super);
//   if (backspace_mod) mods.push_back(key::modificator::backspace);
//   
//   for (size_t i = 0; i < container->config.keys.size(); ++i) {
// //     const uint32_t keyCount = config->keys[i]->getKeysCount();
// // 
// // //     std::cout << "keyCount " << keyCount << "\n";
// // 
// //     bool state = true;
// //     for (uint32_t j = 0; j < keyCount; ++j) {
// //       const uint32_t key = config->keys[i]->getKey(j);
// // 
// // //       std::cout << "key " << key << "\n";
// // 
// //       state = state && Global::data()->keys[key];
// //     }
//     
//     const bool state = Global::data()->keys[container->config.keys[i]->key()];
//     container->config.keys[i]->execute(mods, state ? GLFW_PRESS : GLFW_RELEASE, time);
//   }
  
  // нам возможно потребуется дабл клик для интерфейса
  
  
}

void keys_callback(yacs::entity* player, interface::container* menu) {
  auto input = player->at<UserInputComponent>(game::entity::input);
  static bool lastFocus = false;
  
  {
    size_t mem = 0;
    auto change = input::next_input_event(mem, 1);
    while (change.id.valid()) {
      //if (jump == change.id && change.event == input::press) input->jump();
      
      if (!menu->is_opened() && escape == change.id && change.event == input::press) {
        menu->open();
        Global::get<input::data>()->interface_focus = menu->is_opened();
        Global::get<systems::sound>()->pause_sounds();
      }
      
      if (menu->is_opened() && escape == change.id && change.event == input::press) {
        menu->escape();
        Global::get<input::data>()->interface_focus = menu->is_opened();
        Global::get<systems::sound>()->resume_sounds();
      }
      
      if (interface_focus == change.id && change.event == input::press) {
        Global::get<input::data>()->interface_focus = !Global::get<input::data>()->interface_focus;
      }
      
      change = input::next_input_event(mem, 1);
    }
    
    if (lastFocus && Global::get<input::data>()->interface_focus != lastFocus) {
      int width, height;
      glfwGetWindowSize(Global::window()->handle(), &width, &height);
      double centerX = double(width) / 2.0, centerY = double(height) / 2.0;
      glfwSetCursorPos(Global::window()->handle(), centerX, centerY);
    }

    lastFocus = Global::get<input::data>()->interface_focus;
  }
  
  {
    size_t mem = 0;
    auto pair = input::pressed_event(mem);
    while (pair.first.valid()) {
      // только движение?
      if (!menu->is_opened()) {
        if (move_forward == pair.first) input->forward();
        else if (move_backward == pair.first) input->backward();
        else if (move_right == pair.first) input->right();
        else if (move_left == pair.first) input->left();
        else if (jump == pair.first) input->jump();
      } else {
//         if (menu_next == id) menu->next(); // туда можно отправлять эвенты сразу
//         else if (menu_prev == id) menu->prev();
//         else if (menu_increase == id) menu->increase();
//         else if (menu_decrease == id) menu->decrease();
        menu->send_event(pair.first, pair.second);
      }
      
      pair = input::pressed_event(mem);
    }
  }
}


void menuKeysCallback(interface::container* menu) {
//   for (size_t i = 0; i < KEYS_COUNT; ++i) {
//     if (Global::data()->keys[i]) {
//       menu->feedback(PressingData{static_cast<uint32_t>(i), 0});
//     }
//   }
}

void callback(int error, const char* description) {
  std::cout << "Error code: " << error << std::endl;
  std::cout << "Error: " << description << std::endl;
}

void scrollCallback(GLFWwindow*, double xoffset, double yoffset) {
  if (!Global::window()->isFocused()) return;
  if (!Global::get<input::data>()->interface_focus) return;

  (void)xoffset;
  Global::get<input::data>()->mouse_wheel += float(yoffset);
}

void charCallback(GLFWwindow*, unsigned int c) {
  if (!Global::window()->isFocused()) return;
  if (!Global::get<input::data>()->interface_focus) return;

  Global::get<input::data>()->text[Global::get<input::data>()->current_text] = c;
  ++Global::get<input::data>()->current_text;
//   ImGuiIO& io = ImGui::GetIO();
//   if (c > 0 && c < 0x10000) io.AddInputCharacter((unsigned short)c);
}

void mouseButtonCallback(GLFWwindow*, int button, int action, int mods) {
  if (!Global::window()->isFocused()) return;

  (void)mods;
//   if (action == GLFW_PRESS) mousePressed[button] = true;
//   if (action == GLFW_RELEASE) mousePressed[button] = false;

  //Global::data()->keys[button] = !(action == GLFW_RELEASE);
  const auto old = Global::get<input::data>()->key_events.container[button].event;
  Global::get<input::data>()->key_events.container[button].event = static_cast<input::type>(action);
  auto data = Global::get<input::data>()->key_events.container[button].data;
  if (data != nullptr && old != static_cast<input::type>(action)) data->time = 0;
}

void keyCallback(GLFWwindow*, int key, int scancode, int action, int mods) {
  (void)mods;
  
  if (!Global::window()->isFocused()) return;
  (void)scancode;

//   ImGuiIO& io = ImGui::GetIO();

  //Global::data()->keys[key] = !(action == GLFW_RELEASE);
  const auto old = Global::get<input::data>()->key_events.container[key].event;
  Global::get<input::data>()->key_events.container[key].event = static_cast<input::type>(action);
  auto data = Global::get<input::data>()->key_events.container[key].data;
  if (data != nullptr && old != static_cast<input::type>(action)) data->time = 0;

//   if (!Global::data()->focusOnInterface) return;
//
//   io.KeysDown[key] = !(action == GLFW_RELEASE);
//
//   (void)mods; // Modifiers are not reliable across systems
//   io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
//   io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
//   io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
//   io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
}

void iconifyCallback(GLFWwindow*, int iconified) {
  Global::window()->setIconify(iconified);
}

void focusCallback(GLFWwindow*, int focused) {
  Global::window()->setFocus(focused);
}

const char* getClipboard(void* user_data) {
  return glfwGetClipboardString(((Window*)user_data)->handle());
}

void setClipboard(void* user_data, const char* text) {
  glfwSetClipboardString(((Window*)user_data)->handle(), text);
}
