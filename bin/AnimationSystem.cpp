#include "AnimationSystem.h"

#include "Globals.h"
#include "Utility.h"

Animation::Animation(const ConstructorInfo &info) : animFrameSize(info.animFrameSize), animFrameStart(info.frameStart), animFrameCount(info.frameCount), imageOffset(info.imageOffset), animTime(info.time), sound_data(info.sound_data) {}

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

size_t Animation::time() const {
  return animTime;
}

size_t Animation::getCurrentFrameOffset(const size_t &currentTime) const {
  uint32_t start = frameStart();
//   uint32_t end = frameCount();
  
  uint32_t count = frameCount();
  size_t time_one = animTime / count;
  size_t index = currentTime / time_one;

  // всегда берем последний индекс у анимации
//   const double ratio = std::min(double(currentTime)/double(animTime), 1.0);
//   const int32_t index = ratio * (int32_t(end) - int32_t(start));

#ifdef _DEBUG
//   const uint32_t count = std::max(start, end);
  ASSERT(index < count);
#endif

  return imageOffset + (start + index) * frameSize();
}

const struct Animation::sound* Animation::sound() const {
  return &sound_data;
}
