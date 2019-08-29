#include "AnimationSystem.h"

#include "Globals.h"
#include "Utility.h"
// #include "Physics2.h"

//#include <climits>

//#define REPEATED_PLACE 0
//// #define BLOCKING_PLACE (REPEATED_PLACE+1)
//// #define BLOCKING_MOVEMENT_PLACE (BLOCKING_PLACE+1)
//#define FRAME_COUNT_PLACE (REPEATED_PLACE+1)
//#define DEPENDANT_PLACE (FRAME_COUNT_PLACE+sizeof(uint8_t)*CHAR_BIT)
//
//AnimType::AnimType() : container(0) {}
//
//AnimType::AnimType(const bool &repeated, const uint8_t &frameCount, const bool &dependant) : container(0) {
//  makeType(repeated, frameCount, dependant);
//}
//
//void AnimType::makeType(const bool &repeated, const uint8_t &frameCount, const bool &dependant) {
//  container = container | (uint32_t(repeated) << REPEATED_PLACE);
////   container = container | (uint32_t(blocking) << BLOCKING_PLACE);
////   container = container | (uint32_t(blockingMovement) << BLOCKING_MOVEMENT_PLACE);
//  container = container | (uint32_t(frameCount) << FRAME_COUNT_PLACE);
//  container = container | (uint32_t(dependant) << DEPENDANT_PLACE);
//}
//
//bool AnimType::isRepeated() const {
//  const uint32_t mask = 1;
//  return bool((container >> REPEATED_PLACE) & mask);
//}
//
//uint8_t AnimType::frameSize() const {
//  const uint32_t mask = 0xFF;
//  return uint8_t((container >> FRAME_COUNT_PLACE) & mask);
//}
//
//bool AnimType::isDependant() const {
//  const uint32_t mask = 1;
//  return bool((container >> DEPENDANT_PLACE) & mask);
//}

//Animation::Animation();
Animation::Animation(const size_t &imageOffset, const uint8_t &animFrameSize, const size_t &frameStart, const size_t &frameCount) : animFrameSize(animFrameSize), animFrameStart(frameStart), animFrameCount(frameCount), imageOffset(imageOffset) {}

//bool Animation::isFinished(const size_t &time) const {
//  return time >= frameTime * frameCount();
//}

//bool Animation::isRepeated() const {
//  return type.isRepeated();
//}

uint8_t Animation::frameSize() const {
  return animFrameSize;
}

uint32_t Animation::frameStart() const {
  return animFrameStart;
}

uint32_t Animation::frameCount() const {
  return animFrameCount;
}

size_t Animation::offset() const {
  return imageOffset;
}

size_t Animation::imagesCount() const {
  uint32_t start = frameStart();
  uint32_t end = frameCount();
  if (start > end) std::swap(start, end);

  return (end - start) * frameSize();
}

//size_t Animation::getCurrentFrameOffset(const size_t &time) const {
//  const size_t index = (time / frameTime) % frameCount();
//  // если мы вышли за пределы то начинаем заново, но тем не менее нужно будет это контролировать из компонента
//
//  return textureOffset + index*frameSize();
//}

size_t Animation::getCurrentFrameOffset(const size_t &currentTime, const size_t &animTime) const {
  uint32_t start = frameStart();
  uint32_t end = frameCount();

  // всегда берем последний индекс у анимации
  const double ratio = std::min(double(currentTime)/double(animTime), 1.0);
  const int32_t index = ratio * (int32_t(end) - int32_t(start));

#ifdef _DEBUG
  const uint32_t count = std::max(start, end);
  ASSERT(std::abs(index) < count);
#endif

  return imageOffset + (start + index) * frameSize();
}