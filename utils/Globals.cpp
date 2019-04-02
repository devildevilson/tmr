#include "Globals.h"

#include <cstdint>
#include <climits>

const uint32_t mask = 0x1;

// индекс - это номер клавиши!
//bool Keys::get(const uint32_t &index) const {
//  const uint32_t dataIndex = index / UINT32_WIDTH;
//  const uint32_t bitIndex = index % UINT32_WIDTH;
//  
//  return bool(boolData[dataIndex] & (mask << bitIndex));
//}

//void Keys::set(const uint32_t &index, bool data) {
//  const uint32_t dataIndex = index / UINT32_WIDTH;
//  const uint32_t bitIndex = index % UINT32_WIDTH;
//  
//  boolData[dataIndex] = data ? boolData[dataIndex] | (mask << bitIndex) : boolData[dataIndex] & (~(mask << bitIndex));
//}

// Console* Global::globalConsole = nullptr;
// CmdMgr* Global::globalCommandMgr = nullptr;
// VariableC* Global::globalVarMgr = nullptr;
// EventsContainer Global::eventsMgr;
// VulkanRender* Global::renderPtr = nullptr;
// Game* Global::gamePtr = nullptr;
// Physics* Global::phys = nullptr;

Console Global::globalConsole;
CmdMgr Global::globalCommandMgr;
// VariableC Global::globalVarMgr;
EventsContainer Global::eventsMgr;
VulkanRender* Global::renderPtr = nullptr;
Game* Global::gamePtr = nullptr;
// Physics* Global::phys = nullptr;
PhysicsEngine* Global::phys2 = nullptr;
AISystem* Global::aiS = nullptr;
JobSystem* Global::jobSystem = nullptr;
Random* Global::randomPtr = nullptr;
Settings* Global::settingsPtr = nullptr;
AnimationSystem* Global::anim = nullptr;
Window* Global::windowPtr = nullptr;

GlobalData Global::globalData = {
  false,
  {false},
  0.0f,
  0,
  {0},
  std::chrono::steady_clock::now(),
  {0, 0},
  0.0f,
  0.0f
};

std::string Global::appDir;
simd::vec4 Global::playerCoords;
glm::vec4 Global::playerView;

// Buffer Global::meshVertex;
// Buffer Global::meshIndex;
// Buffer Global::aabb;
// Buffer Global::aabbIdx;
// Buffer Global::sphereVertex;
// Buffer Global::sphereIndex;
// Buffer Global::plane1;
// Buffer Global::plane2;
// Buffer Global::mapVertex;
// Buffer Global::mapIndex;
// yavf::Buffer Global::mapVertex = yavf::Buffer(nullptr);
// yavf::Buffer Global::mapIndex = yavf::Buffer(nullptr);
// yavf::Buffer Global::indirect = yavf::Buffer(nullptr);

void Global::init() {
  //globalConsole = new Console();
  
  //globalCommandMgr = new CmdMgr();
  globalCommandMgr.init();
  
  //globalVarMgr = new VariableC();
//   globalVarMgr.init();
  
  Global::data()->focusOnInterface = false;
  //Global::data()->focusOnInterface = true;
  
  memset(Global::data()->keys, 0, KEYS_COUNT*sizeof(bool));
  
  Global::data()->mouseWheel = 0.0f;
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

// VariableC* Global::variables() {
//   return &globalVarMgr;
// }

EventsContainer* Global::events() {
  return &eventsMgr;
}

VulkanRender* Global::render() {
  return renderPtr;
}

Game* Global::game() {
  return gamePtr;
}

// Physics* Global::physic() {
//   return phys;
// }

PhysicsEngine* Global::physics() {
  return phys2;
}

AISystem* Global::ai() {
  return aiS;
}

JobSystem* Global::job() {
  return jobSystem;
}

Random* Global::random() {
  return randomPtr;
}

Settings* Global::settings() {
  return settingsPtr;
}

AnimationSystem* Global::animations() {
  return anim;
}

Window* Global::window() {
  return windowPtr;
}

GlobalData* Global::data() {
  return &globalData;
}

std::string Global::getGameDir() {
  return appDir;
}

simd::vec4 Global::getPlayerPos() {
  return playerCoords;
}

glm::vec4 Global::getPlayerRot() {
  return playerView;
}

// yavf::Buffer Global::getMapVertex() {
//   return mapVertex;
// }
// 
// yavf::Buffer Global::getMapIndex() {
//   return mapIndex;
// }
// 
// yavf::Buffer Global::getIndirectBuffer() {
//   return indirect;
// }

void Global::setRender(VulkanRender* render) {
  this->renderPtr = render;
}

void Global::setPhysics(PhysicsEngine* phys) {
  this->phys2 = phys;
}

void Global::setAISystem(AISystem* ai) {
  this->aiS = ai;
}

void Global::setRandom(Random* random) {
  this->randomPtr = random;
}

void Global::setSettings(Settings* settings) {
  this->settingsPtr = settings;
}

void Global::setAnimations(AnimationSystem* anims) {
  this->anim = anims;
}

void Global::setWindow(Window* window) {
  this->windowPtr = window;
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

// Buffer Global::getMeshVertex() {
//   return meshVertex;
// }

// Buffer Global::getMeshIndex() {
//   return meshIndex;
// }

// Buffer Global::getAABB() {
//   return aabb;
// }

// Buffer Global::getAABBIdx() {
//   return aabbIdx;
// }

// Buffer Global::getSphereVertex() {
//   return sphereVertex;
// }

// Buffer Global::getSphereIndex() {
//   return sphereIndex;
// }

// Buffer Global::getPlane1() {
//   return plane1;
// }

// Buffer Global::getPlane2() {
//   return plane2;
// }

// Buffer Global::getMapVertex() {
//   return mapVertex;
// }

// Buffer Global::getMapIndex() {
//   return mapIndex;
// }

float angle = 0.0f;
int angleInt = 0;
glm::vec3 playerRotation;

// glm::vec3 frontP;
// glm::vec3 frontP2;

//Game* gamePtr = nullptr;
//Application* appPtr = nullptr;
