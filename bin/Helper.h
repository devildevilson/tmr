#ifndef HELPER_H
#define HELPER_H

#include "Utility.h"
#include "Globals.h"
//#include "Variable.h"
#include "console_variable.h"
#include "Containers.h"
#include "GraphicsContainer.h"
#include "TimeMeter.h"
#include "settings.h"
#include "typeless_container.h"
#include "system.h"
#include "input.h"
#include "camera.h"
#include "random.h"

#include "CPUArray.h"
#include "CPUBuffer.h"
#include "GPUArray.h"
#include "GPUBuffer.h"

#include "Window.h"
#include "VulkanRender.h"
#include "RenderStages.h"
#include "Optimizers.h"
#include "GPUOptimizers.h"
#include "image_container.h"

//#include "CPUAnimationSystemParallel.h"

#include "PhysicsUtils.h"
#include "CPUPhysicsParallel.h"
#include "CPUOctreeBroadphaseParallel.h"
#include "CPUNarrowphaseParallel.h"
#include "CPUSolverParallel.h"
#include "CPUPhysicsSorter.h"
#include "collision_interaction_system.h"

// #include "PostPhysics.h"
#include "post_physics.h"

#include "ThreadPool.h"

#include "TransformComponent.h"
#include "PhysicsComponent.h"
#include "InputComponent.h"
//#include "InfoComponent.h"
#include "type_info_component.h"
//#include "AIInputComponent.h"
// #include "CameraComponent.h"
//#include "GraphicComponets.h"
#include "graphics_component.h"
//#include "SoundComponent.h"
//#include "StateController.h"
#include "states_component.h"
//#include "AnimationComponent.h"
//#include "EffectComponent.h"
#include "effects_component.h"
//#include "AttributesComponent.h"
#include "attributes_component.h"
//#include "Interactions.h"
#include "interaction.h"
//#include "MovementComponent.h"
#include "movement_component.h"
//#include "AbilityComponent.h"
#include "abilities_component.h"
//#include "InventoryComponent.h"
#include "inventory_component.h"
#include "global_components_indicies.h"
#include "vertex_component.h"

//#include "ParticleSystem.h"
//#include "DecalSystem.h"
//#include "SoundSystem.h"
#include "sound_system.h"

//#include "../resources/ResourceManager.h"
#include "resource.h"
#include "resource_container.h"
#include "resource_parser.h"
#include "modification_container.h"
#include "image_loader.h"
#include "state_loader.h"
#include "effects_loader.h"
#include "attributes_loader.h"
#include "abilities_loader.h"
#include "entity_loader.h"
#include "sound_loader.h"
#include "map_loader.h"
// #include "ModificationContainer.h"
// #include "Modification.h"
// #include "ImageLoader.h"
// #include "SoundLoader.h"
// #include "AnimationLoader.h"
// #include "EffectsLoader.h"
// #include "AttributesLoader.h"
// #include "AbilityTypeLoader.h"
// #include "ItemLoader.h"
// #include "EntityLoader.h"
// #include "HardcodedLoaders.h"

#define ABILITIES_CONTAINER
#define EFFECTS_CONTAINER
#define STATES_CONTAINER
#define WEAPONS_CONTAINER
#define ATTRIBUTE_TYPES_CONTAINER
#include "game_resources.h"
#include "sound_data.h"

#include "core_funcs.h"
#include "game_funcs.h"

//#include "DelayedWorkSystem.h"
#include "delayed_work_system.h"

// #include "Menu.h"
// #include "MenuItems.h"
#include "Interface.h"
#include "Interfaces.h"
#include "overlay.h"

//#include "AISystem.h"
//#include "AIComponent.h"
//#include "CPUPathFindingPhaseParallel.h"
//#include "Graph.h"
#include "ai_component.h"
#include "pathfinder_system.h"
#include "graph.h"
#include "vertex_component.h"
#define TINY_BEHAVIOUR_MULTITHREADING
#include "tiny_behaviour/TinyBehavior.h"

//#include <apathy/path.hpp>
#include <cppfs/fs.h>
#include <cppfs/FileHandle.h>
#include <cppfs/FileIterator.h>

#include <locale>

using namespace devils_engine;

// микросекунды
// примерно 18 кадров
#define DOUBLE_PRESS_TIME 300000
// примерно 25 кадров
#define DOUBLE_CLICK_TIME 400000
// примерно 18 кадров
#define LONG_PRESS_TIME 300000
// примерно 25 кадров
#define LONG_CLICK_TIME 400000

#define KEY_POOL_SIZE 300
#define ACTION_POOL_SIZE 300
#define REACTION_POOL_SIZE 100

// при этом возникают трудности создавать системы в рантайме?
// хотя может и не возникают, но только нужно собрать сайз до создания контейнера
// с созданием системы мне еще потребуется передать ей какие-то данные
// понятное дело что в случае с системами обобщить создание систем не выйдет
// следовательно потребуются уникальные функции для создания каждой системы

// что делать в случае если нам нужно создать дополнительный класс
// например треад пулл, и передать его каким-нибудь системам?
// треад пулл имеет смысл создать только тогда когда хотя бы одна система его использует

// что с системами которые будут разными в зависимости от состояния приложения?
// например рендер менюшки, рендер самой игры и тд, для этих двух рендеров
// точно нужны разные рендер стейджы (для менюшки достаточно 1 тектура на весь экран + гуи)
// по идее мне нужно просто сменить рендер
// (скорее всего тогда мы просто упакуем эти рендеры в дополнительный класс)

// хотя нет, наверное метода updateSystems не будет
// мне нужен более удобный способ влиять на порядок выполнения систем

// вообще нужно наверное еще дополнительно улучшить систем контейнер
// например ввести метод get, который будет возвращать систему
// это значит что нужно для систем сделать статик переменную
// основная проблема наверное немного в другом,
// мне нужно возвращать класс по его интерфейсу, при этом
// как я уже выяснил, у меня будет несколько рендеров одновременно
// в частности с рендерами все сложнее, так как
// по идее количество мониторов = количеству рендеров
// количетсво аутпут конфигураций (монитор + устройство) = количеству рендеров

// void initCommonSystems(TypelessContainer &cont);

// extern GPUArray<MonsterGPUOptimizer::InstanceData> instanceData;

struct system_container {
  utils::typeless_container container;
  GraphicsContainer* graphics_container;
//   PostPhysics* post_physics;
  systems::sound* sound_system;
  
  MonsterOptimizer* monster_optimiser;
  GeometryOptimizer* geometry_optimiser;
  LightOptimizer* lights_optimiser;
  MonsterDebugOptimizer* monster_debug_optimiser;
  GeometryDebugOptimizer* geometry_debug_optimiser;
  
  CPUOctreeBroadphaseParallel* broad;
  CPUNarrowphaseParallel* narrow;
  CPUSolverParallel* solver;
  CPUPhysicsSorter* sorter;
  CPUPhysicsParallel* physics;
  
  graph::container* edge_container;
  systems::pathfinder* pathfinder_system;
  
  system_container();
  ~system_container();
};

struct WindowData {
  bool fullscreen;
  uint32_t width;
  uint32_t height;
  float fov;
  GLFWmonitor* monitor;
  GLFWwindow* glfwWindow;
  VkSurfaceKHR surface;
};

struct DataArrays {
  utils::typeless_container container;
  Container<Transform>* transforms;
  Container<InputData>* inputs;
  Container<simd::mat4>* matrices;
  ArrayInterface<uint32_t>* rotationCountBuffer;
  Container<RotationData>* rotations;
  Container<ExternalData>* externals;
  Container<Texture>* textures;
  //Container<AnimationState>* animStates;
//   ArrayInterface<BroadphasePair>* broadphasePairs;
  DataArrays();
  ~DataArrays();
};

struct RenderConstructData {
  system_container* systems;
  DataArrays* arrays;
  nuklear_data* data;
  MonsterDebugOptimizer* monDebugOpt;
  GeometryDebugOptimizer* geoDebugOpt;
};

struct KeyConfiguration {
  uint64_t cont;

  KeyConfiguration(const KeyConfiguration &copy);
  KeyConfiguration(const uint32_t &key1);
  KeyConfiguration(const uint32_t &key1, const uint32_t &key2);
  KeyConfiguration(const uint32_t &key1, const uint32_t &key2, const uint32_t &key3);
  KeyConfiguration(const uint32_t &key1, const uint32_t &key2, const uint32_t &key3, const uint32_t &key4);

  uint32_t getFirstKey() const;
  uint32_t getSecondKey() const;
  uint32_t getThirdKey() const;
  uint32_t getForthKey() const;

  uint32_t getKeysCount() const;
  uint32_t operator[] (const uint8_t &index) const;
};

// как хэндлить инпут? мне нужно сделать чтоб у меня можно было задавать несколько клавиш
// можно сделать иерархию, для иерархии нужно наверное виртуальные функции, проблема ли это?

enum KeyState : uint8_t {
  KEY_STATE_CLICK        = 0,
  KEY_STATE_DOUBLE_CLICK = 1,
  KEY_STATE_LONG_CLICK   = 2,
  KEY_STATE_PRESS        = 3,
  KEY_STATE_DOUBLE_PRESS = 4,
  KEY_STATE_LONG_PRESS   = 5,
  KEY_STATE_UNKNOWN      = 6
};

struct input_function {
  input_function();
  input_function(const std::string &name, const std::function<void()> &f);

  std::string name;
  std::function<void()> f;
};

namespace key {
  enum class state : uint8_t {
    unknown,
    press,
    click,
    double_press,
    double_click,
    long_press,
    long_click,
  };
  
  // может ли пригодиться реакция на нажатие двух произвольных клавиш?
  // думаю что вряд ли
  // должна быть какая то иерархия  
  enum class modificator : uint8_t {
    none,
    ctrl,
    alt,
    shift,
    super,
    backspace
  };
  
  class action {
  public:
    struct type {
      uint32_t m_container;
      
      type();
      type(const std::vector<enum state> &states, const bool notUsedWhileModificator, const modificator &mod);
      
      void make_type(const std::vector<enum state> &states, const bool notUsedWhileModificator, const modificator &mod);
      
      bool is_valid_state(const enum state &state) const;
      bool isUsedWithModificators() const;
      bool changed() const;
      modificator key_modificator() const;
      
      void set_changed(const bool value);
    };
    
    action();
    action(const std::vector<enum state> &states, const bool notUsedWhileModificator, const modificator &mod, const int32_t &key, input_function* func);
    
    void execute(const std::vector<modificator> &mods, const int32_t &state, const size_t &time);
    
    int32_t key();
    enum state current_state() const;
    bool is_valid_state(const enum state &state) const;
    bool isUsedWithModificators() const;
    std::string name() const;
  private:
    size_t m_time;
    struct type m_type;
    int32_t m_key;
    enum state m_current;
    input_function* m_func;
  };
}

// нам требуется получить состояние любого действия по кнопке
// то есть "attack" -> click + нужно проверить изменилось ли это состояние
// по сравнению с чем? необходимо правильно обработать длительное нажатие
// (пулемет или очень быстрые атаки) понятно что если я зажму кнопку 
// во время анимации чего либо, то это не должно учитываться как длительное нажатие
// в думе длительное нажатие сделано через bool attackdown
// я могу сделать примерно то же самое + там есть переменная refire
// которая отвечает видимо за лицо персонажа (не могу найти использование)
// у меня не особ много возможностей к изменению како то переменной
// например я могу менять аттрибут или как то поиграться с инвентарем
// игроку так или иначе необходимо задать какой то компонент с различными переменными
// для статистики например. я сделал memory 

// блин я только что понял что статистика собирается в момент смерти энтити
// а у меня смерть отложена до вычислений функций хп
// и более того я удаляю инфу об атакующем раньше чем захожу в вычисление аттрибутов
// мне нужно иначе вычислять аттрибуты (самостоятельно обходить все эффекты?)
// самый очевидный вариант в том чтобы обойти текущие эффекты и проверить есть ли здесь игрок
// при добавлении эффекта сразу закидывать изменения в аттрибуты, обновлять эффекты после 
// обновления аттрибутов, лучше добавить штук 16 переменных для каждого объекта
// да и их просто изменять, возможно стоит сделать атомик переменные мутабл
// статистика будет храниться в int_container в type_info

// шум при использовании оружия должны "слышать" монстры
// как это сделать? должна быть условная сила шума
// и скорее всего в вершине нужно запомнить объект который этот шум издал
// (ну и силу шума), мне нужно обойти последовательно вершины 
// ну и задать источник шума, в думе задается 2 переменных validcount и soundtraversed
// (первая - глобальная переменная, не знаю откуда она берется, вторая - то сколько звук прошел)

// class ActionKey {
// public:
//   ActionKey(const KeyConfiguration &keys, const std::vector<ActionKey*> &keysPtr);
// 
//   void execute(const int32_t &state, const uint64_t &_time);
//   void setHandled();
// 
//   Reaction* getReaction(uint8_t i) const;
//   void setReaction(const uint8_t &index, Reaction* r);
// 
//   uint32_t getKey(const uint8_t &index) const;
//   uint32_t getKeysCount() const;
// private:
//   bool handled;
//   KeyConfiguration keys;
//   std::vector<ActionKey*> keysPtr;
//   std::array<Reaction*, 6> commands;
//   uint64_t time;
//   KeyState currentState;
// };

struct KeyConfig {
  std::vector<key::action*> keys;
  std::unordered_map<std::string, input_function*> reactions;
};

struct KeyContainer {
//   MemoryPool<ActionKey, KEY_POOL_SIZE*sizeof(ActionKey)> keysPool;
  MemoryPool<key::action, ACTION_POOL_SIZE*sizeof(key::action)> keysPool;
  MemoryPool<input_function, REACTION_POOL_SIZE*sizeof(input_function)> reactionPool;
  KeyConfig config;

  KeyContainer();
  ~KeyContainer();

//   ActionKey* create(const KeyConfiguration &keys, const std::vector<ActionKey*> &keysPtr);
  key::action* create(const std::vector<key::state> &states, const bool notUsedWhileModificator, const key::modificator &mod, const int32_t &key, input_function* func);
//   void sort();

  input_function* create(const std::string &name, const std::function<void()> &f);
};

// почти в каждой функции еще должны использоваться настройки
void initGLFW();
void deinitGLFW();

void create_graphics(system_container &container);
void create_optimizers(system_container &container, DataArrays &arrays);
void create_physics(system_container &container, dt::thread_pool* pool, DataArrays &arrays, const size_t &updateDelta);
void create_ai(system_container &container, dt::thread_pool* pool);
void create_sound_system(system_container &container);

// void createInstance(yavf::Instance* inst);
// void createGLFWwindow(yavf::Instance* inst, WindowData &data);
// void createKHRdisplay(yavf::Instance* inst, WindowData &data);
// void createDevice(yavf::Instance* inst, const WindowData &data, yavf::Device** device);
// void createWindow(yavf::Instance* inst, yavf::Device* device, const WindowData &data, Window &window);
// // void destroyWindow(Window* window);
// void createRender(yavf::Instance* inst, yavf::Device* device, const uint32_t &frameCount, const size_t &stageContainerSize, GameSystemContainer &container, VulkanRender** render, yavf::CombinedTask** task);
void createDataArrays(yavf::Device* device, DataArrays &arrays);
// void destroyDataArrays(TypelessContainer &arraysContainer, DataArrays &arrays);
void createRenderStages(const RenderConstructData &data, std::vector<DynamicPipelineStage*> &dynPipe);
// void createPhysics(dt::thread_pool* threadPool, const DataArrays &arrays, const size_t &updateDelta, PhysicsContainer &physicsContainer, PhysicsEngine** engine); // еще device поди пригодится
// void createAI(dt::thread_pool* threadPool, const size_t &updateDelta, GameSystemContainer &container);
// это должно происходить рядом с createLoaders, должен быть лоадер разных луа вещей
// который по идее должен заполнить эти контейнеры, все функции и деревья здесь используются только при непосредственной загрузке 
// а значит мы по идее на стадии парсинга можем заполнить эти данные, но пока у нас нет луа это не нужно
std::unordered_map<utils::id, tb::BehaviorTree*> createBehaviourTrees();
std::unordered_map<std::string, core::attribute_t<core::float_type>::type::func_type> create_float_attribs_funcs();
std::unordered_map<std::string, core::attribute_t<core::int_type>::type::func_type> create_int_attribs_funcs();
std::unordered_map<std::string, core::state_t::action_func> create_states_funcs();
std::unordered_map<std::string, core::entity_creator::collision_func_t> create_collision_funcs();

// мне нужно это аккуратно удалить в конце
struct resources_ptr {
  resources_ptr(yavf::Device* device);
  ~resources_ptr();
  
  utils::typeless_container resources_containers;
  game::image_data_container_load* images;
  game::image_resources_load* image_res;
  game::sounds_container_load* sounds;
  game::abilities_container_load* abilities;
  game::effects_container_load* effects;
  game::states_container_load* states;
  game::weapons_container_load* weapons;
  game::float_attribute_types_container_load* float_attribs;
  game::int_attribute_types_container_load* int_attribs;
  game::entity_creators_container_load* entities;
  game::map_data_container_load* map_data;
  std::unordered_map<std::string, core::attribute_t<core::float_type>::type::func_type> float_funcs;
  std::unordered_map<std::string, core::attribute_t<core::int_type>::type::func_type> int_funcs;
  std::unordered_map<std::string, core::state_t::action_func> states_funcs;
  std::unordered_map<std::string, core::entity_creator::collision_func_t> collision_funcs;
  std::unordered_map<utils::id, tb::BehaviorTree*> trees;
};
void createLoaders(resources::modification_container &mods, GraphicsContainer* graphicsContainer, const DataArrays &data_arrays, render::image_container* images, resources_ptr &res, resources::map_loader** mapLoader);
void createSoundSystem(dt::thread_pool* threadPool, GameSystemContainer &container);

void initnk(yavf::Device* device, Window* window, nuklear_data &data);
void deinitnk(nuklear_data &data);

void nextnkFrame(Window* window, nk_context* ctx);

struct SimpleOverlayData {
  simd::vec4 pos;
  simd::vec4 rot;
  size_t frameComputeTime;
  size_t frameSleepTime;
  size_t lastFrameComputeTime;
  size_t lastFrameSleepTime;
  uint32_t frameCount;
  float fps;
  size_t frustumObjCount;
  size_t rayCollideCount;
  size_t visibleObjCount;
};
void nkOverlay(const SimpleOverlayData &data, nk_context* ctx);

void sync(TimeMeter &tm, const size_t &syncTime); // сюда мы должны передать желаемое время кадра

struct ReactionsCreateInfo {
  KeyContainer* container;
//   UserInputComponent* input;
  Window* window;
  interface::container* menuContainer;
  yacs::entity* brain;
};
void createReactions(const ReactionsCreateInfo &info);
void setUpKeys(KeyContainer* container);

struct MouseData {
  float xMouseSpeed;
  float yMouseSpeed;
  // что то еще?
};
void mouse_input(yacs::entity* player, const size_t &time);
void keys_callback(yacs::entity* player, interface::container* menu);

// void keysCallbacks(KeyContainer* container, const uint64_t &time);
// void menuKeysCallback(interface::container* menu);

void callback(int error, const char* description);
void scrollCallback(GLFWwindow*, double xoffset, double yoffset);
void charCallback(GLFWwindow*, unsigned int c);
void mouseButtonCallback(GLFWwindow*, int button, int action, int mods);
void keyCallback(GLFWwindow*, int key, int scancode, int action, int mods);
void iconifyCallback(GLFWwindow*, int iconified);
void focusCallback(GLFWwindow*, int focused);
const char* getClipboard(void* user_data);
void setClipboard(void* user_data, const char* text);

#endif
