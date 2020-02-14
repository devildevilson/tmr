#include "CPUAnimationSystemParallel.h"

#include "AnimationComponent.h"
#include "Globals.h"

CPUAnimationSystemParallel::CPUAnimationSystemParallel(dt::thread_pool* pool) : pool(pool) {}
CPUAnimationSystemParallel::~CPUAnimationSystemParallel() {}

void CPUAnimationSystemParallel::update(const uint64_t & time) {
//  static const auto func = [&] (const size_t &start, const size_t &count) {
//    for (size_t i = start; i < start+count; ++i) {
//      components[i]->update(time);
//    }
//  };
//
//  const size_t count = std::ceil(float(components.size()) / float(pool->size()+1));
//  size_t start = 0;
//  for (uint32_t i = 0; i < pool->size()+1; ++i) {
//    const size_t jobCount = std::min(count, components.size()-start);
//    if (jobCount == 0) break;
//
//    pool->submitnr(func, start, jobCount);
//
//    start += jobCount;
//  }
//
//  pool->compute();
//  pool->wait();

//   static const auto func = [&] (const size_t &start, const size_t &count) {
//     for (size_t i = start; i < start+count; ++i) {
//       // надеюсь так ничего не сломается
//       yacs::component_handle<AnimationComponent> handle = Global::world()->get_component<AnimationComponent>(i);
//       handle->update(time);
// //      components[i]->update(time);
//     }
//   };

  const size_t componentsCount = Global::world()->count_components<AnimationComponent>();
  const size_t count = std::ceil(float(componentsCount) / float(pool->size() + 1));
  size_t start = 0;
  for (uint32_t i = 0; i < pool->size()+1; ++i) {
    const size_t jobCount = std::min(count, componentsCount-start);
    if (jobCount == 0) break;

    //pool->submitnr(func, start, jobCount);
    pool->submitbase([start, jobCount, time] () {
      for (size_t i = start; i < start+jobCount; ++i) {
        // надеюсь так ничего не сломается
        yacs::component_handle<AnimationComponent> handle = Global::world()->get_component<AnimationComponent>(i);
        handle->update(time);
  //      components[i]->update(time);
      }
    });

    start += jobCount;
  }

  pool->compute();
  pool->wait();
}

void CPUAnimationSystemParallel::registerAnimationUnit(AnimationComponent* component) {
//  component->getInternalIndex() = components.size();
//  components.push_back(component);
}

void CPUAnimationSystemParallel::removeAnimationUnit(AnimationComponent* component) {
//  components.back()->getInternalIndex() = component->getInternalIndex();
//  std::swap(components.back(), components[component->getInternalIndex()]);
//  components.pop_back();
}

uint32_t CPUAnimationSystemParallel::createAnimation(const ResourceID &animId, const Animation::CreateInfo &info) {
  if (info.frames.empty()) throw std::runtime_error("Empty animation data");
  
  const size_t frameSize = info.frames[0].size();
  if (frameSize > 256) throw std::runtime_error("Frame size > 256 is not allowed");
  if (frameSize == 0) throw std::runtime_error("Frames size == 0");
  
  for (uint32_t i = 1; i < info.frames.size(); ++i) {
    if (info.frames[i].size() != frameSize) throw std::runtime_error("Frame size must be the same across animation");
  }
  
  auto itr = animationIdx.find(animId);
  if (itr != animationIdx.end()) throw std::runtime_error("Animation with name " + animId.name() + " is already exist");
  
//  const size_t size = info.frames.size() * frameSize;
//  const AnimType type(info.repeated, frameSize, false);
//  const Animation a(type, info.animationTime, textures.size(), size);
  
  uint32_t id = animations.size();
  //animations.push_back(a);
  animations.emplace_back(textures.size(), frameSize, 0, info.frames.size());
  animationIdx[animId] = id;
  
  for (uint32_t i = 0; i < info.frames.size(); ++i) {
    for (uint32_t j = 0; j < info.frames[i].size(); ++j) {
      textures.push_back(info.frames[i][j]);
    }
  }
  
  return id;
}

uint32_t CPUAnimationSystemParallel::createAnimation(const ResourceID &animId, const Animation::DependantInfo &info) {
  auto itr = animationIdx.find(info.existingId);
  if (itr == animationIdx.end()) throw std::runtime_error("Animation with name " + info.existingId.name() + " is not exist");

  const uint32_t animIndex = itr->second;
  const uint32_t count = std::max(info.animStart, info.animEnd);

  const Animation &anim = animations[animIndex];
  if (count > anim.frameCount()) throw std::runtime_error("Could not create animation " + animId.name() +
                                                          " from " + info.existingId.name() +
                                                          " cause info frame count" + std::to_string(count) +
                                                          " > existing animation frame count " + std::to_string(anim.frameCount()));

  uint32_t id = animations.size();
  animations.emplace_back(anim.offset(), anim.frameSize(), info.animStart, info.animEnd);
  animationIdx[animId] = id;

  return id;
}

uint32_t CPUAnimationSystemParallel::getAnimationId(const ResourceID &animId) const {
  auto itr = animationIdx.find(animId);
  if (itr == animationIdx.end()) return UINT32_MAX;
  
  return itr->second;
}

Animation & CPUAnimationSystemParallel::getAnimationById(const uint32_t &id) {
  return animations[id];
}

const Animation & CPUAnimationSystemParallel::getAnimationById(const uint32_t &id) const {
  return animations[id];
}

Animation & CPUAnimationSystemParallel::getAnimationByName(const ResourceID &animId) {
  auto itr = animationIdx.find(animId);
  // может не надо кидать эксепшон? хотя едва ли мне может пригодиться ситуация когда я по имени не нашел анимацию
  // точнее это может быть полезным для чеканья ошибок в моде
  if (itr == animationIdx.end()) throw std::runtime_error("Animation with name " + animId.name() + " is not exist");
  
  return animations[itr->second];
}

const Animation & CPUAnimationSystemParallel::getAnimationByName(const ResourceID &animId) const {
  auto itr = animationIdx.find(animId);
  if (itr == animationIdx.end()) throw std::runtime_error("Animation with name " + animId.name() + " is not exist");
  
  return animations[itr->second];
}

size_t CPUAnimationSystemParallel::addAnimationTextureData(const std::vector<std::vector<Animation::Image>> &frames) {
  size_t index = textures.size();
  for (size_t i = 0; i < frames.size(); ++i) {
    for (size_t j = 0; j < frames[i].size(); ++j) {
      textures.push_back(frames[i][j]);
    }
  }
  
  return index;
}

Animation::Image CPUAnimationSystemParallel::getAnimationTextureData(const size_t &index) const {
  ASSERT(textures.size() > index);
  return textures[index];
}
