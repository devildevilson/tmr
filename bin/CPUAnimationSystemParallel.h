#ifndef CPU_ANIMATION_SYSTEM_PARALLEL_H
#define CPU_ANIMATION_SYSTEM_PARALLEL_H

#include "AnimationSystem.h"

#include "ThreadPool.h"

class CPUAnimationSystemParallel : public AnimationSystem {
public:
  CPUAnimationSystemParallel(dt::thread_pool* pool);
  ~CPUAnimationSystemParallel();
  
  void update(const uint64_t & time) override;
  
  void registerAnimationUnit(AnimationComponent* component) override;
  void removeAnimationUnit(AnimationComponent* component) override;

  uint32_t createAnimation(const ResourceID &animId, const AnimationCreateInfoNewFrames &info) override;
  uint32_t createAnimation(const ResourceID &animId, const AnimationCreateInfoFromExisting &info) override;
  
  uint32_t getAnimationId(const ResourceID &animId) const override;
  
  Animation & getAnimationById(const uint32_t &id) override;
  const Animation & getAnimationById(const uint32_t &id) const override;
  
  Animation & getAnimationByName(const ResourceID &animId) override;
  const Animation & getAnimationByName(const ResourceID &animId) const override;
  
  TextureData getAnimationTextureData(const size_t &index) const override;
private:
  dt::thread_pool* pool;
  
  std::vector<AnimationComponent*> components;
  std::vector<Animation> animations;
  std::vector<TextureData> textures;
  std::unordered_map<ResourceID, size_t> animationIdx;
};

#endif
