#ifndef ANIMATION_COMPONENT_H
#define ANIMATION_COMPONENT_H

#include "AnimationSystem.h"
#include "Type.h"

class PhysicsComponent;
class TransformComponent;
class GraphicComponent;
namespace yacs {
  class entity;
}

// нужны данные об uv анимации
struct UVAnimation {
  enum class type {
    speed,
    uv,
    none
  };

  enum type type;
  glm::vec2 uvtransition; // смещение
  size_t time; // время применения смещения
};

class AnimationComponent {
public:
  struct CreateInfo {
    yacs::entity* ent;
  };
  AnimationComponent(const CreateInfo &info);
  ~AnimationComponent();
  
  void update(const size_t &time);

  struct PlayInfo {
    Type id;
    bool looping;
    float speedMod;
  };
  void play(const PlayInfo &info);
  
  struct PlayInfoPtr {
    const Animation* animation;
    bool looping;
    float speedMod;
  };
  void play(const PlayInfoPtr &info);
  
//   void apply(const UVAnimation &uvAnim);
  
  size_t time() const;
  size_t current_time() const;
private:
  float speedMod;
  uint32_t bool_container;
  const Animation* currentAnimation;
  const Animation* oldAnimation;

  size_t accumulatedTime;
  
  yacs::entity* ent;
};

#endif
