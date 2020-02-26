#include "Globals.h"

#include <cstdint>
#include <climits>

const uint32_t mask = 0x1;

Console Global::globalConsole;
CmdMgr Global::globalCommandMgr;
VulkanRender* Global::renderPtr = nullptr;
Window* Global::windowPtr = nullptr;
yacs::entity* Global::player_ptr = nullptr;
size_t Global::currentFrameIndex = 0;
size_t Global::current_level_time = 0;

// GlobalData Global::globalData = {
//   false,
//   {false},
//   0.0f,
//   0,
//   {0},
//   std::chrono::steady_clock::now(),
//   {0, 0},
//   0.0f,
//   0.0f
// };

std::string Global::appDir;
simd::vec4 Global::playerCoords;
glm::vec4 Global::playerView;

void Global::init() {
  //globalConsole = new Console();
  
  //globalCommandMgr = new CmdMgr();
  globalCommandMgr.init();
  
  //globalVarMgr = new VariableC();
//   globalVarMgr.init();
  
//   Global::data()->focusOnInterface = false;
//   //Global::data()->focusOnInterface = true;
//   
//   memset(Global::data()->keys, 0, KEYS_COUNT*sizeof(bool));
//   
//   Global::data()->mouseWheel = 0.0f;
}

void Global::clear() {
//   delete globalConsole;
//   delete globalVarMgr;
//   delete globalCommandMgr;
}

Console* Global::console() {
  return &globalConsole;
}

CmdMgr* Global::commands() {
  return &globalCommandMgr;
}

VulkanRender* Global::render() {
  return renderPtr;
}

Window* Global::window() {
  return windowPtr;
}

size_t Global::frameIndex() {
  return currentFrameIndex;
}

size_t Global::mcsSinceEpoch() {
  const auto now = std::chrono::steady_clock::now();
  const auto now_mcs = std::chrono::time_point_cast<CHRONO_TIME_TYPE>(now);

  const auto value = now_mcs.time_since_epoch();
  return value.count();
}

// GlobalData* Global::data() {
//   return &globalData;
// }

std::string Global::getGameDir() {
  return appDir;
}

simd::vec4 Global::getPlayerPos() {
  return playerCoords;
}

glm::vec4 Global::getPlayerRot() {
  return playerView;
}

yacs::entity* Global::player() {
  return player_ptr;
}

size_t Global::level_time() {
  return current_level_time;
}

void Global::setRender(VulkanRender* render) {
  this->renderPtr = render;
}

void Global::setWindow(Window* window) {
  this->windowPtr = window;
}

void Global::incrementFrameIndex() {
  ++currentFrameIndex;
  if (currentFrameIndex == SIZE_MAX) currentFrameIndex = 0;
}

void Global::set_player(yacs::entity* player) {
  player_ptr = player;
}

void Global::increase_level_time(const size_t &time) {
  current_level_time += time;
}

void Global::reset_level_time() {
  current_level_time = 0;
}

void Global::setGameDir(const std::string &dir) {
  this->appDir = dir;
}

void Global::setPlayerPos(const simd::vec4 &pos) {
  playerCoords = pos;
}

void Global::setPlayerRot(const glm::vec4 &rot) {
  playerView = rot;
}

float angle = 0.0f;
int angleInt = 0;
glm::vec3 playerRotation;
