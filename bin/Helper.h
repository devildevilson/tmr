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

#include "Window.h"
#include "VulkanRender.h"
#include "RenderStages.h"
#include "Optimizers.h"

#include "CPUAnimationSystem.h"
#include "AnimationComponent.h"

#include "ResourceManager.h"
#include "TextureLoader.h"
#include "HardcodedLoaders.h"

#include "PhysicsUtils.h"
#include "CPUPhysicsParallel.h"
#include "CPUOctreeBroadphaseParallel.h"
#include "CPUNarrowphaseParallel.h"
#include "CPUSolverParallel.h"
#include "CPUPhysicsSorter.h"

#include "PostPhysics.h"

#include "ThreadPool.h"

#include "Components.h"
#include "GraphicComponets.h"

#include <apathy/path.hpp>

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

// void initCommonSystems(StageContainer &cont);

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
  Container<glm::mat4>* matrices;
  ArrayInterface<uint32_t>* rotationCountBuffer;
  Container<RotationData>* rotations;
  Container<ExternalData>* externals;
  Container<TextureData>* textures;
  Container<AnimationState>* animStates;
  ArrayInterface<BroadphasePair>* broadphasePairs;
};

struct RenderConstructData {
  yavf::Device* device;
  GraphicsContainer* container;
//   yavf::CombinedTask** task;
  VulkanRender* render;
  Window* window;
  HardcodedMapLoader* mapLoader;
  
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
  CLICK        = 0,
  DOUBLE_CLICK = 1,
  LONG_CLICK   = 2,
  PRESS        = 3,
  DOUBLE_PRESS = 4,
  LONG_PRESS   = 5,
  UNKNOWN      = 6
};

struct Reaction {
  Reaction();
  Reaction(const std::string &name, const std::function<void()> &f);
  
  std::string name;
  std::function<void()> f;
};

class ActionKey {
public:
  ActionKey(const KeyConfiguration &keys, const std::vector<ActionKey*> &keysPtr);
  
  void execute(const int32_t &state, const uint64_t &_time);
  void setHandled();
  
  Reaction* getReaction(uint8_t i) const;
  void setReaction(const uint8_t &index, Reaction* r);
  
  uint32_t getKey(const uint8_t &index) const;
  uint32_t getKeysCount() const;
private:
  bool handled;
  KeyConfiguration keys;
  std::vector<ActionKey*> keysPtr;
  std::array<Reaction*, 6> commands;
  uint64_t time;
  KeyState currentState;
};

struct KeyConfig {
  std::vector<ActionKey*> keys;
  std::unordered_map<std::string, Reaction*> reactions;
};

struct KeyContainer {
  MemoryPool<ActionKey, KEY_POOL_SIZE*sizeof(ActionKey)> keysPool;
  MemoryPool<Reaction, REACTION_POOL_SIZE*sizeof(Reaction)> reactionPool;
  KeyConfig* config;
  
  KeyContainer(KeyConfig* config);
  ~KeyContainer();
  
  ActionKey* create(const KeyConfiguration &keys, const std::vector<ActionKey*> &keysPtr);
  void sort();
  
  Reaction* create(const std::string &name, const std::function<void()> &f);
};

// почти в каждой функции еще должны использоваться настройки
void initGLFW();
void deinitGLFW();
void createInstance(yavf::Instance* inst);
void createGLFWwindow(yavf::Instance* inst, WindowData &data);
void createDevice(yavf::Instance* inst, const WindowData &data, yavf::Device** device);
void createWindow(yavf::Instance* inst, yavf::Device* device, const WindowData &data, Window &window);
// void destroyWindow(Window* window);
void createRender(yavf::Instance* inst, yavf::Device* device, const uint32_t &frameCount, const size_t &stageContainerSize, GameSystemContainer &container, VulkanRender** render, yavf::CombinedTask** task);
void createDataArrays(yavf::Device* device, ArrayContainers &arraysContainer, DataArrays &arrays);
// void destroyDataArrays(StageContainer &arraysContainer, DataArrays &arrays);
void createRenderStages(const RenderConstructData &data, std::vector<DynamicPipelineStage*> &dynPipe);
void createPhysics(dt::thread_pool* threadPool, const DataArrays &arrays, PhysicsContainer &physicsContainer, PhysicsEngine** engine); // еще device поди пригодится

void initnk(yavf::Device* device, Window* window, nuklear_data &data);
void deinitnk(nuklear_data &data);

void nextnkFrame(Window* window, nk_context* ctx);

struct SimpleOverlayData {
  glm::vec4 pos;
  glm::vec4 rot;
  size_t frameComputeTime;
  size_t frameSleepTime;
  float fps;
  size_t frustumObjCount;
  size_t rayCollideCount;
  size_t visibleObjCount;
};
void nkOverlay(const SimpleOverlayData &data, nk_context* ctx);

void initGui(yavf::Device* device, Window* window);

void simpleOverlay(const SimpleOverlayData &data);
void nextGuiFrame();
void sync(TimeMeter &tm, const size_t &syncTime); // сюда мы должны передать желаемое время кадра

void guiShutdown(yavf::Device* device);

struct ReactionsCreateInfo {
  KeyContainer* container;
  UserInputComponent* input;
  Window* window;
};
void createReactions(const ReactionsCreateInfo &info);
void setUpKeys(KeyContainer* container);

struct MouseData {
  float xMouseSpeed;
  float yMouseSpeed;
  // что то еще?
};
void mouseInput(UserInputComponent* input, const uint64_t &time);
void keysCallbacks(KeyConfig* config, const uint64_t &time);

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
