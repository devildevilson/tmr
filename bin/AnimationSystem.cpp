#include "AnimationSystem.h"

#include "Globals.h"
#include "Utility.h"
#include "Physics2.h"

#include <climits>

#define REPEATED_PLACE 0
#define BLOCKING_PLACE (REPEATED_PLACE+1)
#define BLOCKING_MOVEMENT_PLACE (BLOCKING_PLACE+1)
#define FRAME_COUNT_PLACE (BLOCKING_MOVEMENT_PLACE+1)
#define DEPENDANT_PLACE (FRAME_COUNT_PLACE+sizeof(uint8_t)*CHAR_BIT)

AnimType::AnimType() {}

AnimType::AnimType(const bool &repeated, const bool &blocking, const bool &blockingMovement, const uint8_t &frameCount, const bool &dependant) {
  makeType(repeated, blocking, blockingMovement, frameCount, dependant);
}

void AnimType::makeType(const bool &repeated, const bool &blocking, const bool &blockingMovement, const uint8_t &frameCount, const bool &dependant) {
  container = container | (uint32_t(repeated) << REPEATED_PLACE);
  container = container | (uint32_t(blocking) << BLOCKING_PLACE);
  container = container | (uint32_t(blockingMovement) << BLOCKING_MOVEMENT_PLACE);
  container = container | (uint32_t(frameCount) << FRAME_COUNT_PLACE);
  container = container | (uint32_t(dependant) << DEPENDANT_PLACE);
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
Animation::Animation(const AnimType &type, const size_t &animTime, const size_t &textureOffset, const size_t &animSize) {
  this->type = type;
  this->animationTime = animTime;
  this->textureOffset = textureOffset;
  this->animSize = animSize;
  accumulatedTime = 0;
  switchesTime = 0;
  currentFrameIndex = 0;
}

void Animation::update(const uint64_t &time) {
  const uint32_t frameCount = animSize / type.frameSize();
  
  if (accumulatedTime + time >= animationTime) {
    if (type.isRepeated() || switchesTime < frameCount) {
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

AnimationSystem::AnimationSystem() : freeDataIndex(UINT32_MAX), freeUnitIndex(UINT32_MAX) {}
AnimationSystem::~AnimationSystem() {}

void AnimationSystem::setInputBuffers(const InputBuffers &buffers) {
  stateArray = buffers.stateArray;
  customTimeArray = buffers.customTimeArray;
  transforms = buffers.transforms;
}

void AnimationSystem::setOutputBuffers(const OutputBuffers &buffers) {
  objTexture = buffers.textures;
}

uint32_t AnimationSystem::registerAnimationUnit(const AnimationUnitCreateInfo &info, AnimationUnitData* data) {
  uint32_t dataIndex;
  uint32_t unitIndex;
  
  if (freeDataIndex != UINT32_MAX) {
    dataIndex = freeDataIndex;
    freeDataIndex = reinterpret_cast<size_t>(datas[dataIndex]);
    datas[dataIndex] = data;
  } else {
    dataIndex = datas.size();
    datas.push_back(data);
  }
  
  data->animationId = UINT32_MAX;
  data->internalIndex = dataIndex;
  
  if (freeUnitIndex != UINT32_MAX) {
    unitIndex = freeUnitIndex;
    freeUnitIndex = units[unitIndex].oldState;
  } else {
    unitIndex = units.size();
    units.emplace_back();
  }
  
  units[unitIndex].animationUnitDataIndex = dataIndex;
  units[unitIndex].currentStateIndex = info.stateIndex;
  units[unitIndex].currentTimeIndex = info.timeIndex;
  units[unitIndex].textureIndex = info.textureIndex;
  units[unitIndex].transformIndex = info.transformIndex;
  units[unitIndex].oldState = UINT32_MAX;
  
  return unitIndex;
}

void AnimationSystem::removeAnimationUnit(const uint32_t &unitIndex) {
  const uint32_t dataIndex = units[unitIndex].animationUnitDataIndex;
  
  datas[dataIndex]->animationId = UINT32_MAX;
  datas[dataIndex]->internalIndex = UINT32_MAX;
  
  datas[dataIndex] = reinterpret_cast<AnimationUnitData*>(size_t(freeDataIndex));
  freeDataIndex = dataIndex;
  
  units[unitIndex].transformIndex = UINT32_MAX;
  units[unitIndex].animationUnitDataIndex = UINT32_MAX;
  units[unitIndex].currentStateIndex = UINT32_MAX;
  units[unitIndex].currentTimeIndex = UINT32_MAX;
  units[unitIndex].textureIndex = UINT32_MAX;
  units[unitIndex].states.clear();
  
  units[unitIndex].oldState = freeUnitIndex;
  freeUnitIndex = unitIndex;
}

void AnimationSystem::precacheStateCount(const uint32_t &animationUnit, const uint32_t &stateCount) {
  units[animationUnit].states.resize(stateCount);
}
// нужно наверное добавить метод add
void AnimationSystem::setAnimation(const uint32_t &animationUnit, const AnimationState &state, const uint32_t &animId) {
  units[animationUnit].states[state] = animId;
}

void AnimationSystem::setAnimation(const uint32_t &animationUnit, const AnimationState &state, const std::string &animName) {
  auto itr = animationNames.find(animName);
  if (itr == animationNames.end()) throw std::runtime_error("there is no animation with name " + animName);
  
  units[animationUnit].states[state] = itr->second;
}

void AnimationSystem::update(const uint64_t &time) {
  // как будет происходить апдейт
  
  for (uint32_t i = 0; i < units.size(); ++i) {
    // тут должна быть проверка на валидный юнит
    if (units[i].transformIndex == UINT32_MAX) continue;
    
    if (units[i].oldState != stateArray->at(units[i].currentStateIndex)) {
      const uint32_t animIndex = units[i].states[units[i].oldState];
      animations[animIndex].stop();
      
      units[i].oldState = stateArray->at(units[i].currentStateIndex);
      
      datas[units[i].animationUnitDataIndex]->animationId = units[i].states[units[i].oldState];
    }
    
    const uint32_t animIndex = units[i].states[units[i].oldState];
    
    animations[animIndex].update(time);
    
    const size_t textureOffset = animations[animIndex].getCurrentFrameOffset();
    const uint8_t frameSize = animations[animIndex].frameSize();
    
    // тут вычисляем поворот относительно игрока
    // для этого нам потребуется еще один буфер с данными игрока
    const glm::vec4 playerPos = glm::vec4(Global::getPlayerPos(), 1.0f);
    const Transform &trans = transforms->at(units[i].transformIndex);
    
    glm::vec4 dir = playerPos - trans.pos;
    
    // это не особо решает проблему с изменением координат
    // скорее всего мне потребуется умножать на матрицу вектор, чтобы привести его в обатное состояние
    // но теперь мне скорее всего этого будет достаточно
    glm::vec4 dirOnGround = projectVectorOnPlane(-Global::physic2()->getGravityNorm(), trans.pos, dir);
    
    dir = glm::normalize(dirOnGround);
    
    float angle2 = glm::acos(glm::dot(trans.rot, dir));
    // проверим сторону
    if (sideOf(trans.pos, trans.pos+trans.rot, playerPos, -Global::physic2()->getGravityNorm()) > 0.0f) angle2 = -angle2;
    
    #define PI_FRAME_SIZE (PI / frameSize)
    // поправка на 22.5 градусов (так как 0 принадлежит [-22.5, 22.5))
    angle2 -= PI_FRAME_SIZE;
    
    if (angle2 < 0.0f) angle2 = angle2 + PI_2;
    if (angle2 > PI_2) angle2 = angle2 - PI_2;
    int a = glm::floor(angle2 / PI_FRAME_SIZE);

    // я не понимаю почему (5 при 8 сторонах)
    a = (a + (frameSize/2 + 1)) % frameSize;
    
    const size_t finalTextureIndex = textureOffset + a;
    
    objTexture->at(units[i].textureIndex) = textures[finalTextureIndex];
  }
}

uint32_t AnimationSystem::createAnimation(const AnimationCreateInfoNewFrames &info) {
  const size_t frameSize = info.frames[0].size();
  if (frameSize > 256) throw std::runtime_error("frame size > 256 is not allowed");
  
  for (uint32_t i = 1; i < info.frames.size(); ++i) {
    if (info.frames[i].size() != frameSize) throw std::runtime_error("frame size must be same across animation");
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

uint32_t AnimationSystem::createAnimation(const AnimationCreateInfoFromExisting &info) {
  // тут нужно добавить еще парочку переменных в анимацию, поэтому пока это не работает
  throw std::runtime_error("not implemented yet");
  return 0;
}

uint32_t AnimationSystem::getCurrentAnimationId(const uint32_t &unitIndex) const {
  const auto &unit = units[unitIndex];
  return unit.states[unit.oldState];
}

Animation & AnimationSystem::getAnimationById(const uint32_t &id) {
  return animations[id];
}

const Animation & AnimationSystem::getAnimationById(const uint32_t &id) const {
  return animations[id];
}

Animation & AnimationSystem::getAnimationByName(const std::string &name) {
  auto itr = animationNames.find(name);
  // может не надо кидать эксепшон? хотя едва ли мне может пригодиться ситуация когда я по имени не нашел анимацию
  // точнее это может быть полезным для чеканья ошибок в моде
  if (itr == animationNames.end()) throw std::runtime_error("there is no animation with name " + name);
  
  return animations[itr->second];
}

const Animation & AnimationSystem::getAnimationByName(const std::string &name) const {
  auto itr = animationNames.find(name);
  if (itr == animationNames.end()) throw std::runtime_error("there is no animation with name " + name);
  
  return animations[itr->second];
}
