#include "CPUAnimationSystem.h"

#include "Globals.h"
#include "Utility.h"
#include "Physics.h"

#include "AnimationComponent.h"

#include <climits>

#define REPEATED_PLACE 0
#define BLOCKING_PLACE (REPEATED_PLACE+1)
#define BLOCKING_MOVEMENT_PLACE (BLOCKING_PLACE+1)
#define FRAME_COUNT_PLACE (BLOCKING_MOVEMENT_PLACE+1)
#define DEPENDANT_PLACE (FRAME_COUNT_PLACE+sizeof(uint8_t)*CHAR_BIT)

AnimType::AnimType() : container(0) {}

AnimType::AnimType(const bool &repeated, const bool &blocking, const bool &blockingMovement, const uint8_t &frameCount, const bool &dependant) : container(0) {
  makeType(repeated, blocking, blockingMovement, frameCount, dependant);
}

void AnimType::makeType(const bool &repeated, const bool &blocking, const bool &blockingMovement, const uint8_t &frameCount, const bool &dependant) {
  container = container | (uint32_t(repeated) << REPEATED_PLACE);
  container = container | (uint32_t(blocking) << BLOCKING_PLACE);
  container = container | (uint32_t(blockingMovement) << BLOCKING_MOVEMENT_PLACE);
  container = container | (         frameCount << FRAME_COUNT_PLACE);
  container = container | (uint32_t(dependant) << DEPENDANT_PLACE);
  
//   assert(frameCount == frameSize());
}

bool AnimType::isRepeated() const {
  const uint32_t mask = 1;
  return bool((container >> REPEATED_PLACE) & mask);
}

bool AnimType::isBlocking() const {
  const uint32_t mask = 1;
  return bool((container >> BLOCKING_PLACE) & mask);
}

bool AnimType::isBlockingMovement() const {
  const uint32_t mask = 1;
  return bool((container >> BLOCKING_MOVEMENT_PLACE) & mask);
}

uint8_t AnimType::frameSize() const {
  const uint32_t mask = 0xFF;
  return uint8_t((container >> FRAME_COUNT_PLACE) & mask);
}

bool AnimType::isDependant() const {
  const uint32_t mask = 1;
  return bool((container >> DEPENDANT_PLACE) & mask);
}

//Animation::Animation();
Animation::Animation(const AnimType &type, const size_t &animTime, const size_t &textureOffset, const size_t &animSize) 
: type(type), 
  switchesTime(0), 
  currentFrameIndex(0), 
  accumulatedTime(0), 
  animationTime(animTime), 
  textureOffset(textureOffset), 
  animSize(animSize) {}

void Animation::update(const uint64_t &time) {
//   assert(type.frameSize() != 0);
//   assert(animSize != 0);
  const uint32_t frameCount = animSize / type.frameSize();
  
  if (accumulatedTime + time >= animationTime) {
    if (type.isRepeated() || switchesTime < frameCount) {
//       assert(frameCount != 0);
      currentFrameIndex = (currentFrameIndex+1) % frameCount;
      ++switchesTime;
    }
  }
  
  accumulatedTime = (accumulatedTime+time) % animationTime;
}

bool Animation::isFinished() const {
  return switchesTime >= animSize / type.frameSize();
}

bool Animation::isRepeated() const {
  return type.isRepeated();
}

bool Animation::isBlocking() const {
  return type.isBlocking();
}

bool Animation::isBlockingMovement() const {
  return type.isBlockingMovement();
}

uint8_t Animation::frameSize() const {
  return type.frameSize();
}

uint32_t Animation::frameCount() const {
  return animSize / type.frameSize();
}

size_t Animation::getCurrentFrameOffset() const {
  return textureOffset + currentFrameIndex*frameSize();
}

void Animation::stop() {
  currentFrameIndex = 0;
  switchesTime = 0;
  accumulatedTime = 0;
}

CPUAnimationSystem::CPUAnimationSystem() : customTimeArray(nullptr), transforms(nullptr), objTexture(nullptr) {}
CPUAnimationSystem::~CPUAnimationSystem() {}

void CPUAnimationSystem::setInputBuffers(const InputBuffers &buffers) {
//   stateArray = buffers.stateArray;
  customTimeArray = buffers.customTimeArray;
  transforms = buffers.transforms;
}

void CPUAnimationSystem::setOutputBuffers(const OutputBuffers &buffers) {
  objTexture = buffers.textures;
}

uint32_t CPUAnimationSystem::registerAnimationUnit(const AnimationUnitCreateInfo &info, AnimationComponent* component) {
//   uint32_t dataIndex;
  uint32_t unitIndex;
  
  if (freeUnits.empty()) {
    unitIndex = units.size();
    units.push_back({
      UINT32_MAX,
      info.stateIndex,
      info.timeIndex,
      info.transformIndex,
      info.textureIndex
    });
    
    components.push_back(component);
  } else {
    unitIndex = freeUnits.back();
    freeUnits.pop_back();
    
    units[unitIndex] = {
      UINT32_MAX,
      info.stateIndex,
      info.timeIndex,
      info.transformIndex,
      info.textureIndex
    };
    
    components[unitIndex] = component;
  }
  
  return unitIndex;
}

void CPUAnimationSystem::removeAnimationUnit(const uint32_t &unitIndex) {
  freeUnits.push_back(unitIndex);
  units[unitIndex].textureIndex = UINT32_MAX;
  components[unitIndex] = nullptr;
}

// void CPUAnimationSystem::precacheStateCount(const uint32_t &animationUnit, const uint32_t &stateCount) {
//   units[animationUnit].states.resize(stateCount);
// }
// нужно наверное добавить метод add
// void CPUAnimationSystem::setAnimation(const uint32_t &animationUnit, const AnimationState &state, const uint32_t &animId) {
//   units[animationUnit].states[state] = animId;
// }
// 
// void CPUAnimationSystem::setAnimation(const uint32_t &animationUnit, const AnimationState &state, const std::string &animName) {
//   auto itr = animationNames.find(animName);
//   if (itr == animationNames.end()) throw std::runtime_error("there is no animation with name " + animName);
//   
//   units[animationUnit].states[state] = itr->second;
// }

void CPUAnimationSystem::update(const uint64_t &time) {
  // как будет происходить апдейт
  
  for (uint32_t i = 0; i < units.size(); ++i) {
    // тут должна быть проверка на валидный юнит
    if (units[i].textureIndex == UINT32_MAX) continue;
  
    if (units[i].oldAnimId != units[i].currentAnimId) {
      const uint32_t animIndex = units[i].oldAnimId;
      if (animIndex != UINT32_MAX) animations[animIndex].stop();
      
      units[i].oldAnimId = units[i].currentAnimId;
      
      //datas[units[i].animationUnitDataIndex]->animationId = units[i].states[units[i].oldState];
    }
    
    const uint32_t animIndex = units[i].currentAnimId;
    
    const uint64_t newTime = units[i].currentTimeIndex == UINT32_MAX ? time : customTimeArray->at(units[i].currentTimeIndex);
    animations[animIndex].update(newTime);
    
    const size_t textureOffset = animations[animIndex].getCurrentFrameOffset();
    const uint8_t frameSize = animations[animIndex].frameSize();
    
    int a = 0;
    if (units[i].transformIndex != UINT32_MAX && frameSize > 1) {
      // тут вычисляем поворот относительно игрока
      // для этого нам потребуется еще один буфер с данными игрока
      const glm::vec4 playerPos = Global::getPlayerPos();
      const Transform &trans = transforms->at(units[i].transformIndex);
      
      glm::vec4 dir = playerPos - trans.pos;
      
      // это не особо решает проблему с изменением координат
      // скорее всего мне потребуется умножать на матрицу вектор, чтобы привести его в обатное состояние
      // но теперь мне скорее всего этого будет достаточно
      const glm::vec4 dirOnGround = projectVectorOnPlane(-Global::physics()->getGravityNorm(), trans.pos, dir);
      
      dir = glm::normalize(dirOnGround);
      
      float angle2 = glm::acos(glm::dot(trans.rot, dir));
      // проверим сторону
      const bool side = sideOf(trans.pos, trans.pos+trans.rot, playerPos, -Global::physics()->getGravityNorm()) > 0.0f;
      angle2 = side ? -angle2 : angle2;
      //if (sideOf(trans.pos, trans.pos+trans.rot, playerPos, -Global::physic()->getGravityNorm()) > 0.0f) angle2 = -angle2;
      
      #define PI_FRAME_SIZE (PI_2 / frameSize)
      #define PI_HALF_FRAME_SIZE (PI_FRAME_SIZE / 2)
      //#define PI_FRAME_SIZE PI_Q
      //#define PI_HALF_FRAME_SIZE PI_E
      // поправка на 22.5 градусов (так как 0 принадлежит [-22.5, 22.5))
      angle2 -= PI_HALF_FRAME_SIZE;
      
      //if (angle2 < 0.0f)
      //if (angle2 > PI_2)
      angle2 = angle2 < 0.0f ? angle2 + PI_2 : angle2;
      angle2 = angle2 > PI_2 ? angle2 - PI_2 : angle2;
      a = glm::floor(angle2 / PI_FRAME_SIZE);

      // я не понимаю почему (5 при 8 сторонах)
      a = (a + (frameSize/2 + 1)) % frameSize;
      //a = (a + 5) % 8;
    }
    
    const size_t finalTextureIndex = textureOffset + a;
    
    ASSERT(objTexture->size() > units[i].textureIndex);
    ASSERT(textures.size() > finalTextureIndex);
    objTexture->at(units[i].textureIndex) = textures[finalTextureIndex];
  }
  
//   for (uint32_t i = 0; i < components.size(); ++i) {
//     if (components[i] == nullptr) continue;
//     
//     const uint32_t animIndex = units[i].currentAnimId;
//     if (animations[animIndex].isFinished()) components[i]->finishCallback();
//   }
}

uint32_t CPUAnimationSystem::createAnimation(const AnimationCreateInfoNewFrames &info) {
  if (info.frames.empty()) throw std::runtime_error("empty animation data");
  
  const size_t frameSize = info.frames[0].size();
  if (frameSize > 256) throw std::runtime_error("frame size > 256 is not allowed");
  if (frameSize == 0) throw std::runtime_error("frames size == 0");
  
  for (uint32_t i = 1; i < info.frames.size(); ++i) {
    if (info.frames[i].size() != frameSize) throw std::runtime_error("frame size must be the same across animation");
  }
  
  auto itr = animationNames.find(info.name);
  if (itr != animationNames.end()) throw std::runtime_error("Animation with " + info.name + " already exist");
  
  const size_t size = info.frames.size() * frameSize;
  const AnimType type(info.repeated, info.blocking, info.blockingMovement, frameSize, false);
  const Animation a(type, info.animationTime, textures.size(), size);
  
  uint32_t id = animations.size();
  animations.push_back(a);
  animationNames[info.name] = id;
  
  for (uint32_t i = 0; i < info.frames.size(); ++i) {
    for (uint32_t j = 0; j < info.frames[i].size(); ++j) {
      textures.push_back(info.frames[i][j]);
    }
  }
  
  return id;
}

uint32_t CPUAnimationSystem::createAnimation(const AnimationCreateInfoFromExisting &info) {
  // тут нужно добавить еще парочку переменных в анимацию, поэтому пока это не работает
  throw std::runtime_error("not implemented yet");
  return 0;
}

uint32_t CPUAnimationSystem::getCurrentAnimationId(const uint32_t &unitIndex) const {
  const auto &unit = units[unitIndex];
  return unit.currentAnimId;
}

uint32_t CPUAnimationSystem::getAnimationId(const std::string &name) const {
  auto itr = animationNames.find(name);
  if (itr == animationNames.end()) return UINT32_MAX;
  
  return itr->second;
}

Animation & CPUAnimationSystem::getAnimationById(const uint32_t &id) {
  return animations[id];
}

const Animation & CPUAnimationSystem::getAnimationById(const uint32_t &id) const {
  return animations[id];
}

Animation & CPUAnimationSystem::getAnimationByName(const std::string &name) {
  auto itr = animationNames.find(name);
  // может не надо кидать эксепшон? хотя едва ли мне может пригодиться ситуация когда я по имени не нашел анимацию
  // точнее это может быть полезным для чеканья ошибок в моде
  if (itr == animationNames.end()) throw std::runtime_error("there is no animation with name " + name);
  
  return animations[itr->second];
}

const Animation & CPUAnimationSystem::getAnimationByName(const std::string &name) const {
  auto itr = animationNames.find(name);
  if (itr == animationNames.end()) throw std::runtime_error("there is no animation with name " + name);
  
  return animations[itr->second];
}
