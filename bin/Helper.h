#ifndef HELPER_H
#define HELPER_H

#include "Utility.h"
#include "Globals.h"
#include "Variable.h"
#include "Containers.h"
#include "GraphicsContainer.h"
#include "TimeMeter.h"
#include "Settings.h"

#include "CPUArray.h"
#include "CPUBuffer.h"
#include "GPUArray.h"
#include "GPUBuffer.h"

#include "Window.h"
#include "VulkanRender.h"
#include "RenderStages.h"
#include "Optimizers.h"
#include "GPUOptimizers.h"

#include "CPUAnimationSystemParallel.h"
#include "AnimationComponent.h"

#include "PhysicsUtils.h"
#include "CPUPhysicsParallel.h"
#include "CPUOctreeBroadphaseParallel.h"
#include "CPUNarrowphaseParallel.h"
#include "CPUSolverParallel.h"
#include "CPUPhysicsSorter.h"

#include "PostPhysics.h"

#include "ThreadPool.h"

#include "TransformComponent.h"
#include "PhysicsComponent.h"
#include "InputComponent.h"
#include "InfoComponent.h"
#include "AIInputComponent.h"
#include "CameraComponent.h"
#include "GraphicComponets.h"
#include "SoundComponent.h"

//#include "ParticleSystem.h"
//#include "DecalSystem.h"
#include "SoundSystem.h"

//#include "../resources/ResourceManager.h"
#include "ModificationContainer.h"
#include "Modification.h"
#include "ImageLoader.h"
#include "SoundLoader.h"
#include "HardcodedLoaders.h"

#include "DelayedWorkSystem.h"

#include "Menu.h"
#include "MenuItems.h"

#include "AISystem.h"
#include "AIComponent.h"
#include "CPUPathFindingPhaseParallel.h"
#include "Graph.h"
#define TINY_BEHAVIOUR_MULTITHREADING
#include "tiny_behaviour/TinyBehavior.h"

//#include <apathy/path.hpp>
#include <cppfs/fs.h>
#include <cppfs/FileHandle.h>
#include <cppfs/FileIterator.h>

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
  Container<Transform>* transforms;
  Container<InputData>* inputs;
  Container<simd::mat4>* matrices;
  ArrayInterface<uint32_t>* rotationCountBuffer;
  Container<RotationData>* rotations;
  Container<ExternalData>* externals;
  Container<Texture>* textures;
  Container<AnimationState>* animStates;
//   ArrayInterface<BroadphasePair>* broadphasePairs;
};

struct RenderConstructData {
  yavf::Device* device;
  GraphicsContainer* container;
  VulkanRender* render;
  Window* window;
  HardcodedMapLoader* mapLoader;
  DataArrays* arrays;

  MonsterOptimizer* mon;
  GeometryOptimizer* geo;
  LightOptimizer* lights;
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
void createInstance(yavf::Instance* inst);
void createGLFWwindow(yavf::Instance* inst, WindowData &data);
void createKHRdisplay(yavf::Instance* inst, WindowData &data);
void createDevice(yavf::Instance* inst, const WindowData &data, yavf::Device** device);
void createWindow(yavf::Instance* inst, yavf::Device* device, const WindowData &data, Window &window);
// void destroyWindow(Window* window);
void createRender(yavf::Instance* inst, yavf::Device* device, const uint32_t &frameCount, const size_t &stageContainerSize, GameSystemContainer &container, VulkanRender** render, yavf::CombinedTask** task);
void createDataArrays(yavf::Device* device, ArrayContainers &arraysContainer, DataArrays &arrays);
// void destroyDataArrays(TypelessContainer &arraysContainer, DataArrays &arrays);
void createRenderStages(const RenderConstructData &data, std::vector<DynamicPipelineStage*> &dynPipe);
void createPhysics(dt::thread_pool* threadPool, const DataArrays &arrays, const size_t &updateDelta, PhysicsContainer &physicsContainer, PhysicsEngine** engine); // еще device поди пригодится
void createAI(dt::thread_pool* threadPool, const size_t &updateDelta, GameSystemContainer &container);
void createBehaviourTrees();

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
  UserInputComponent* input;
  Window* window;
  MenuStateMachine* menuContainer;
  EntityAI* brain;
};
void createReactions(const ReactionsCreateInfo &info);
void setUpKeys(KeyContainer* container);

struct MouseData {
  float xMouseSpeed;
  float yMouseSpeed;
  // что то еще?
};
void mouseInput(UserInputComponent* input, const uint64_t &time);
void keysCallbacks(KeyContainer* container, const uint64_t &time);

void menuKeysCallback(MenuStateMachine* menu);

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
