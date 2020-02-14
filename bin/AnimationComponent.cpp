#include "AnimationComponent.h"

#include "Globals.h"
#include "GraphicComponets.h"
#include "EventComponent.h"
#include "TransformComponent.h"
#include "Physics.h"
#include "PhysicsComponent.h"
#include "SoundComponent.h"
#include "global_components_indicies.h"
#include "AnimationLoader.h"

// удаление объектов требуется совсем немногим энтити
// в основном только проджекттайлам
// можно ли это привязать к интеракции? наверное
// что еще необходимо удалить?

enum {
  ANIMATION_LOOPING = (1<<0),
  ANIMATION_DIRECTION = (1<<1),
  ANIMATION_LOOPING_RESET = (1<<2)
};

AnimationComponent::AnimationComponent(const CreateInfo &info)
  : speedMod(1.0f),
    bool_container(0),
    currentAnimation(nullptr),
    oldAnimation(nullptr),
    accumulatedTime(0),
    ent(info.ent) {}

AnimationComponent::~AnimationComponent() {}

void AnimationComponent::update(const uint64_t &time) {
  auto trans = ent->at<TransformComponent>(TRANSFORM_COMPONENT_INDEX);
  auto phys = ent->at<PhysicsComponent>(PHYSICS_COMPONENT_INDEX);
  auto graphics = ent->at<GraphicComponent>(GRAPHICS_COMPONENT_INDEX);
  auto sound = ent->at<SoundComponent>(SOUND_COMPONENT_INDEX);
  
  // вычисляем прибавочное время
  const size_t newTime = speedMod * time;

  Animation::Image image = {graphics->getTexture().image, false, false};
//   if (currentAnimationIndex != UINT32_MAX) {
//     const Animation &anim = Global::animations()->getAnimationById(currentAnimationIndex);
//  const bool finished = anim.isFinished(accumulatedTime+newTime);
//  if (finished) accumulatedTime = 0; // по идее по разному нужно реагировать

    accumulatedTime += newTime;
    if ((bool_container & ANIMATION_LOOPING) == ANIMATION_LOOPING) {
      accumulatedTime %= currentAnimation->time();
      bool_container &= ~ANIMATION_LOOPING_RESET;
    }
    
    // делэй + если looping
    if (currentAnimation->sound()->sound != nullptr && accumulatedTime >= currentAnimation->sound()->delay && ((bool_container & ANIMATION_LOOPING_RESET) == ANIMATION_LOOPING_RESET)) {
      bool_container |= ANIMATION_LOOPING_RESET;
      const SoundComponent::PlayInfo info{
        currentAnimation->sound()->sound,
        false,
        !(currentAnimation->sound()->static_sound || currentAnimation->sound()->relative_pos),
        false, //currentAnimation->sound()->relative_pos,
        !currentAnimation->sound()->static_sound,
        currentAnimation->sound()->scalar,
        100.0f,
        1.0f,
        1.0f,
        1.0f
      };
      sound->play(info);
    }

    const size_t textureOffset = currentAnimation->getCurrentFrameOffset(accumulatedTime);
    const uint8_t frameSize = currentAnimation->frameSize();

    int a = 0;
    if (trans.valid() && frameSize > 1) { // trans != nullptr
      // тут вычисляем поворот относительно игрока
      // для этого нам потребуется еще один буфер с данными игрока
      const simd::vec4 playerPos = Global::getPlayerPos();

      simd::vec4 dir = playerPos - trans->pos();

      const simd::vec4 dirOnGround = projectVectorOnPlane(-PhysicsEngine::getGravityNorm(), trans->pos(), dir);

      dir = simd::normalize(dirOnGround);

      float angle2 = glm::acos(simd::dot(trans->rot(), dir));
      // проверим сторону
      const bool side = sideOf(trans->pos(), trans->pos()+trans->rot(), playerPos, -PhysicsEngine::getGravityNorm()) > 0.0f;
      angle2 = side ? -angle2 : angle2;

#define PI_FRAME_SIZE (PI_2 / frameSize)
#define PI_HALF_FRAME_SIZE (PI_FRAME_SIZE / 2)
      // поправка на 22.5 градусов (так как 0 принадлежит [-22.5, 22.5))
      angle2 -= PI_HALF_FRAME_SIZE;

      angle2 = angle2 < 0.0f ? angle2 + PI_2 : angle2;
      angle2 = angle2 > PI_2 ? angle2 - PI_2 : angle2;
      a = glm::floor(angle2 / PI_FRAME_SIZE);

      // я не понимаю почему (5 при 8 сторонах)
      a = (a + (frameSize/2 + 1)) % frameSize;
    }

    // во первых нужно разделить ИЗОБРАЖЕНИЕ от всей остальной информации
    // то есть image: arrayIndex, layer
    // texture: image, sampler, (u,v) information
    // текстуру собирать нужно отдельно, много разных вещей нужно учесть

    const size_t finalTextureIndex = textureOffset + a;
    image = Global::animations()->getAnimationTextureData(finalTextureIndex);
//   }

  Texture texture{
    image.image,
    0, // сэмплер здесь меняться не будет
    1.0f,
    1.0f,
  };

//   // должна быть только для игрока
//   glm::vec2 finalUV = glm::vec2(0.0f, 0.0f);
//   if (uvAnim.type == UVAnimation::type::speed) {
//     const float speed = phys->getSpeed();
//     const float maxSpeed = phys->getMaxSpeed();
//     const float ratio = speed/maxSpeed;
// 
//     const float timeRatio = float(newTime) / float(uvAnim.time);
// 
//     uvtrans.x += direction ? timeRatio : -timeRatio;
//     if (uvtrans.x >= PI_H) {
//       direction = !direction;
//       uvtrans.x = PI_H;
//     } else if (uvtrans.x <= -PI_H) {
//       direction = !direction;
//       uvtrans.x = -PI_H;
//     }
// 
//     uvtrans.y = -fast_fabsf(std::cos(uvtrans.x));
//     // trans.x В РАДИАНАХ!!!
// 
//     finalUV = ratio * glm::vec2(fast_fabsf(uvtrans.x) / PI_H, uvtrans.y);
//     texture.movementU = finalUV.x;
//     texture.movementV = finalUV.y;
//   }
// 
//   if (uvAnim.type == UVAnimation::type::uv) {
//     const float timeRatio = float(newTime) / float(uvAnim.time);
// 
//     // хотя тут наверное uvAnim.uvtransition = конечному положению, а может и нет
// 
//     uvtrans += timeRatio * uvAnim.uvtransition;
//     uvtrans = glm::fract(uvtrans);
// 
//     texture.movementU = uvtrans.x;
//     texture.movementV = uvtrans.y;
//   }

  // u: [-1.0f, 0.0f] = mirror X
  // v: [-1.0f, 0.0f] = mirror Y
  texture.movementU = image.flipU ? -texture.movementU : texture.movementU;
  texture.movementV = image.flipV ? -texture.movementV : texture.movementV;
  
  graphics->setTexture(texture);
}

void AnimationComponent::play(const PlayInfo &info) {
  oldAnimation = currentAnimation;
  currentAnimation = Global::get<AnimationLoader>()->getAnim(info.id);
  ASSERT(currentAnimation != nullptr);
  
//   oldAnimationIndex = currentAnimationIndex;
//   currentAnimationIndex = Global::animations()->getAnimationId(info.id);
//   ASSERT(currentAnimationIndex != UINT32_MAX);

  speedMod = info.speedMod;
  //animationTime = info.animationTime;
  bool_container = info.looping ? bool_container | ANIMATION_LOOPING : bool_container & ~ANIMATION_LOOPING;
  bool_container &= ~ANIMATION_LOOPING_RESET;
//   uvAnim.type = UVAnimation::type::none;
}

void AnimationComponent::play(const PlayInfoPtr &info) {
  oldAnimation = currentAnimation;
  currentAnimation = info.animation;// get
  ASSERT(currentAnimation != nullptr);
  
  speedMod = info.speedMod;
  bool_container = info.looping ? bool_container | ANIMATION_LOOPING : bool_container & ~ANIMATION_LOOPING;
  bool_container &= ~ANIMATION_LOOPING_RESET;
}

// void AnimationComponent::apply(const UVAnimation &uvAnim) {
//   this->uvAnim = uvAnim;
// }
