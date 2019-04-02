#ifndef GLOBALS_H
#define GLOBALS_H

#include <chrono>

#include "Utility.h"

#include "Console.h"
#include "CmdMgr.h"
// #include "Variable.h"
#include "Event.h"

#define PRINT_VEC3(name, vec) std::cout << name << " x: " << vec.x << " y: " << vec.y << " z: " << vec.z << "\n";
#define PRINT_VEC4(name, vec) std::cout << name << " x: " << vec.x << " y: " << vec.y << " z: " << vec.z << " w: " << vec.w << "\n";
#define PRINT_VAR(name, var) std::cout << name << ": " << var << "\n";

class Game;
class Application;
class VulkanRender;
// class Physics;
class Actor;
class GameObject;
class AISystem;
class JobSystem;
class Random;
class Settings;
class PhysicsEngine;
class AnimationSystem;
class Window;

namespace yavf {
  class Buffer;
}

struct Keys {
  //   bool keys[350];
  uint32_t boolData[33];

  bool get(const uint32_t &index) const;
  void set(const uint32_t &index, bool data);
};

#define KEYS_COUNT 350
#define TEXT_MEMORY_MAX 256

struct GlobalData {
  bool focusOnInterface;
  
  //Keys keys;
  bool keys[KEYS_COUNT];
  
  float mouseWheel;
  //bool mousePressed[8]; // ???
  
  uint32_t currentText;
  uint32_t text[TEXT_MEMORY_MAX];
  // мы сразу принимаем на вход юникод
  
  std::chrono::steady_clock::time_point doubleClickPoint;
  glm::uvec2 clickPos;
  float fbScaleX;
  float fbScaleY;
};

class Global {
  friend Game;
public:
  static void init();
  static void clear();
  
  static Console* console();
  static CmdMgr* commands();
//   static VariableC* variables();
  static EventsContainer* events();
  
  static VulkanRender* render();
  static Game* game();
//   static Physics* physic();
  static PhysicsEngine* physics();
  static AISystem* ai();
  static JobSystem* job();
  static Random* random();
  static Settings* settings();
  static AnimationSystem* animations();
  static Window* window();
  
  static GlobalData* data();
  
  static std::string getGameDir();
  static simd::vec4 getPlayerPos();
  static glm::vec4 getPlayerRot();

//   static yavf::Buffer getMapVertex();
//   static yavf::Buffer getMapIndex();
//   static yavf::Buffer getIndirectBuffer();
  
  void setRender(VulkanRender* render);
  void setPhysics(PhysicsEngine* phys);
  void setAISystem(AISystem* ai);
  void setRandom(Random* random);
  void setSettings(Settings* settings);
  void setAnimations(AnimationSystem* anims);
  void setWindow(Window* window);
  
  void setGameDir(const std::string &dir);
  
  void setPlayerPos(const simd::vec4 &pos);
  void setPlayerRot(const glm::vec4 &rot);
private:
  static Console globalConsole;
  static CmdMgr globalCommandMgr;
//   static VariableC globalVarMgr;
  static VulkanRender* renderPtr;
  static Game* gamePtr;
//   static Physics* phys;
  static PhysicsEngine* phys2;
  static EventsContainer eventsMgr;
  static AISystem* aiS;
  static JobSystem* jobSystem;
  static Random* randomPtr;
  static Settings* settingsPtr;
  static AnimationSystem* anim;
  static Window* windowPtr;
  
  static GlobalData globalData;
  
  static std::string appDir;
  static simd::vec4 playerCoords;
  static glm::vec4 playerView;

//   static yavf::Buffer mapVertex;
//   static yavf::Buffer mapIndex;
//   static yavf::Buffer indirect;
};

extern float angle;
extern int angleInt;
extern glm::vec3 playerRotation;
// extern glm::vec3 frontP;
// extern glm::vec3 frontP2;

const uint32_t queriesCount = 5100;

#endif // GLOBALS_H
