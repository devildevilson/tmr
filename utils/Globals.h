#ifndef GLOBALS_H
#define GLOBALS_H

#include <chrono>

#include "Utility.h"

#include "Console.h"
#include "CmdMgr.h"

#define PRINT_VEC3(name, vec) std::cout << name << " x: " << vec.x << " y: " << vec.y << " z: " << vec.z << "\n";
#define PRINT_VEC4(name, vec) std::cout << name << " x: " << vec.x << " y: " << vec.y << " z: " << vec.z << " w: " << vec.w << "\n";
#define PRINT_VAR(name, var) std::cout << name << ": " << var << "\n";

// как сделать то же самое в рантайме?
// из типа получить поинтер нужно, синглтон?

class VulkanRender;
class Window;

namespace yacs {
  class world;
  class entity;
}

namespace yavf {
  class Buffer;
}

#define KEYS_COUNT 350
#define TEXT_MEMORY_MAX 256

// struct GlobalData {
//   bool focusOnInterface;
//   
//   //Keys keys;
//   bool keys[KEYS_COUNT];
//   
//   float mouseWheel;
//   //bool mousePressed[8]; // ???
//   
//   uint32_t currentText;
//   uint32_t text[TEXT_MEMORY_MAX];
//   // мы сразу принимаем на вход юникод
//   
//   std::chrono::steady_clock::time_point doubleClickPoint;
//   glm::uvec2 clickPos;
//   float fbScaleX;
//   float fbScaleY;
// };

class Global {
  //friend Game;
public:
  static void init();
  static void clear();
  
  static Console* console();
  static CmdMgr* commands();
  
  static VulkanRender* render();
  static Window* window();
  static size_t frameIndex();
  static size_t mcsSinceEpoch();
  
//   static GlobalData* data();
  
  static std::string getGameDir();
  static simd::vec4 getPlayerPos(); // если у нас здесь есть указатель на игрока, то отдельно хранить позицию игрока не нужно
  static glm::vec4 getPlayerRot();
  
  static yacs::entity* player();
  static size_t level_time();
  
  // вот собственно и решение
  template <typename T>
  static T* get(T* ptr = nullptr) {
    static T* container = nullptr;
    if (ptr != nullptr) container = ptr;
    return container;
  }
  
  void setRender(VulkanRender* render);
  void setWindow(Window* window);
  void incrementFrameIndex();
  void set_player(yacs::entity* player);
  void increase_level_time(const size_t &time);
  void reset_level_time();
  
  void setGameDir(const std::string &dir);
  
  void setPlayerPos(const simd::vec4 &pos);
  void setPlayerRot(const glm::vec4 &rot);
private:
  static Console globalConsole;
  static CmdMgr globalCommandMgr;
  static VulkanRender* renderPtr;
  static Window* windowPtr;
  
  static yacs::entity* player_ptr;
  static size_t currentFrameIndex;
  static size_t current_level_time;
  
//   static GlobalData globalData;
  
  static std::string appDir;
  static simd::vec4 playerCoords;
  static glm::vec4 playerView;
};

extern float angle;
extern int angleInt;
extern glm::vec3 playerRotation;
// extern glm::vec3 frontP;
// extern glm::vec3 frontP2;

const uint32_t queriesCount = 5100;

#endif // GLOBALS_H
