#ifndef CPU_ANIMATION_SYSTEM_H
#define CPU_ANIMATION_SYSTEM_H

#include "AnimationSystem.h"

class AnimationComponent;

class CPUAnimationSystem : public AnimationSystem {
public:
  // что такое состояние? по идее это просто индекс
  // беда в том что у меня может не быть анимации на это состояние, че делать?
  // забивать болт и оставлять дырки? идея такая себе
  // состояние по идее нужно делать для каждой системы разное (ну то есть для тех систем для которых это вообще нужно)
  struct AnimationUnit {
//     uint32_t oldState;
//     uint32_t currentStateIndex;
    uint32_t oldAnimId;
    uint32_t currentAnimId;
    uint32_t currentTimeIndex; // тип если это не UINT32_MAX то берем время по этому индексу
    uint32_t transformIndex;
    uint32_t textureIndex;
//     uint32_t animationUnitDataIndex;
    
//     std::vector<uint32_t> states; // тут по идее индекс анимации
  };
  
  CPUAnimationSystem();
  ~CPUAnimationSystem();
  
  void setInputBuffers(const InputBuffers &buffers) override;
  void setOutputBuffers(const OutputBuffers &buffers) override;
  
  // здесь я должен передать индексы для инпут и аутпут буферов
  uint32_t registerAnimationUnit(const AnimationUnitCreateInfo &info, AnimationComponent* component) override;
  void removeAnimationUnit(const uint32_t &unitIndex) override;
  
//   void precacheStateCount(const uint32_t &animationUnit, const uint32_t &stateCount) override;
//   void setAnimation(const uint32_t &animationUnit, const AnimationState &state, const uint32_t &animId) override;
//   void setAnimation(const uint32_t &animationUnit, const AnimationState &state, const std::string &animName) override;
  
  void update(const uint64_t &time) override; // у меня время то будет отличаться для каждой анимации
  
  // вообще мне кажется что такие вещи как анимации текстурки и прочее (то есть ресурсы игры с диска... как то так)
  // лучше бы создавать и получать по какому-нибудь интерфесу
  // их неплохо было бы удалять
  uint32_t createAnimation(const AnimationCreateInfoNewFrames &info) override;
  uint32_t createAnimation(const AnimationCreateInfoFromExisting &info) override;
  
  uint32_t getCurrentAnimationId(const uint32_t &unitIndex) const override;
  
  uint32_t getAnimationId(const std::string &name) const override;
  
  Animation & getAnimationById(const uint32_t &id) override;
  const Animation & getAnimationById(const uint32_t &id) const override;
  
  Animation & getAnimationByName(const std::string &name) override;
  const Animation & getAnimationByName(const std::string &name) const override;
private:
//   ArrayInterface<AnimationState>* stateArray = nullptr;
  ArrayInterface<uint32_t>* customTimeArray;
  ArrayInterface<Transform>* transforms;
  
  //ArrayInterface<Texture>* objTexture = nullptr;
  ArrayInterface<TextureData>* objTexture;
  
  //std::vector<Texture> textures; // текстурки иногда будут дублироваться скорее всего
  std::vector<TextureData> textures; // текстурки иногда будут дублироваться скорее всего
  
//   uint32_t freeDataIndex;
//   uint32_t freeUnitIndex;
//   std::vector<AnimationUnitData*> datas;
  std::vector<size_t> freeUnits;
  std::vector<AnimationUnit> units;
  std::vector<AnimationComponent*> components;
  
  std::vector<Animation> animations;
  std::unordered_map<std::string, size_t> animationNames;
};

#endif
