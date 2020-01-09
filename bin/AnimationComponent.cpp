#include "AnimationComponent.h"

#include "Globals.h"
#include "GraphicComponets.h"
#include "EventComponent.h"
#include "TransformComponent.h"
#include "Physics.h"
#include "PhysicsComponent.h"
#include "global_components_indicies.h"

// void AnimationComponent::setStateContainer(Container<AnimationState>* stateContainer) {
//   AnimationComponent::stateContainer = stateContainer;
// }

AnimationComponent::AnimationComponent(const CreateInfo &info)
  : looping(false),
    direction(true),
    currentAnimationIndex(UINT32_MAX),
    oldAnimationIndex(UINT32_MAX),
    speedMod(1.0f),
    uvtrans({0.0f, 0.0f,}),
    uvAnim{UVAnimation::type::none,
           {0.0f, 0.0f,},
           0},
    accumulatedTime(0),
    animationTime(0),
//    localEvents(info.localEvents),
    ent(info.ent) {}
//     trans(info.trans),
//     phys(info.phys),
//     graphics(info.graphics) {}

AnimationComponent::~AnimationComponent() {}

void AnimationComponent::update(const uint64_t &time) {
  auto trans = ent->at<TransformComponent>(TRANSFORM_COMPONENT_INDEX);
  auto phys = ent->at<PhysicsComponent>(PHYSICS_COMPONENT_INDEX);
  auto graphics = ent->at<GraphicComponent>(GRAPHICS_COMPONENT_INDEX);
  
  // вычисляем прибавочное время
  const size_t newTime = speedMod * time;

  Animation::Image image = {graphics->getTexture().image, false, false};
  if (currentAnimationIndex != UINT32_MAX) {
    const Animation &anim = Global::animations()->getAnimationById(currentAnimationIndex);
//  const bool finished = anim.isFinished(accumulatedTime+newTime);
//  if (finished) accumulatedTime = 0; // по идее по разному нужно реагировать

    accumulatedTime += newTime;
    if (looping) {
      accumulatedTime %= animationTime;
    }

    const size_t textureOffset = anim.getCurrentFrameOffset(accumulatedTime, animationTime);
    const uint8_t frameSize = anim.frameSize();

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
  }

  Texture texture{
    image.image,
    0, // сэмплер здесь меняться не будет
    0.0f,
    0.0f,
  };

  // должна быть только для игрока
  glm::vec2 finalUV = glm::vec2(0.0f, 0.0f);
  if (uvAnim.type == UVAnimation::type::speed) {
    const float speed = phys->getSpeed();
    const float maxSpeed = phys->getMaxSpeed();
    const float ratio = speed/maxSpeed;

    const float timeRatio = float(newTime) / float(uvAnim.time);

    uvtrans.x += direction ? timeRatio : -timeRatio;
    if (uvtrans.x >= PI_H) {
      direction = !direction;
      uvtrans.x = PI_H;
    } else if (uvtrans.x <= -PI_H) {
      direction = !direction;
      uvtrans.x = -PI_H;
    }

    uvtrans.y = -fast_fabsf(std::cos(uvtrans.x));
    // trans.x В РАДИАНАХ!!!

    finalUV = ratio * glm::vec2(fast_fabsf(uvtrans.x) / PI_H, uvtrans.y);
    texture.movementU = finalUV.x;
    texture.movementV = finalUV.y;
  }

  if (uvAnim.type == UVAnimation::type::uv) {
    const float timeRatio = float(newTime) / float(uvAnim.time);

    // хотя тут наверное uvAnim.uvtransition = конечному положению, а может и нет

    uvtrans += timeRatio * uvAnim.uvtransition;
    uvtrans = glm::fract(uvtrans);

    texture.movementU = uvtrans.x;
    texture.movementV = uvtrans.y;
  }

  // u: [-1.0f, 0.0f] = mirror X
  // v: [-1.0f, 0.0f] = mirror Y
  texture.movementU = image.flipU ? -texture.movementU : texture.movementU;
  texture.movementV = image.flipV ? -texture.movementV : texture.movementV;
  
  graphics->setTexture(texture);
}

//void AnimationComponent::init(void* userData) {
//  (void)userData;
//
//  localEvents = getEntity()->get<EventComponent>().get();
//  if (localEvents == nullptr) {
//    Global::console()->printE("Initializing animation without event component");
//    throw std::runtime_error("Initializing animation without event component");
//  }
//
//  trans = getEntity()->get<TransformComponent>().get();
//
//  graphics = getEntity()->get<GraphicComponent>().get();
//  if (graphics == nullptr) {
//    Global::console()->printE("Initializing animation without graphics");
//    throw std::runtime_error("Initializing animation without graphics");
//  }
//}

//void AnimationComponent::setAnimation(const Type &state, const ResourceID &id) {
//  static const auto changeAnimId = [&] (const Type &type, const EventData &data, const uint32_t &index) {
//    (void)data;
//    (void)type;
//
//    currentAnimationIndex = index;
//    if (oldAnimationIndex != currentAnimationIndex) {
//      accumulatedTime = 0;
//      oldAnimationIndex = currentAnimationIndex;
//    }
//
//    return success;
//  };
//
//  localEvents->registerEvent(state, std::bind(changeAnimId, std::placeholders::_1, std::placeholders::_2, Global::animations()->getAnimationId(id)));
//}

void AnimationComponent::play(const PlayInfo &info) {
  oldAnimationIndex = currentAnimationIndex;
  currentAnimationIndex = Global::animations()->getAnimationId(info.id);

  speedMod = info.speedMod;
  animationTime = info.animationTime;
  looping = info.looping;
  uvAnim.type = UVAnimation::type::none;
}

void AnimationComponent::apply(const UVAnimation &uvAnim) {
  this->uvAnim = uvAnim;
}

//size_t & AnimationComponent::getInternalIndex() {
//  return internalIndex;
//}

// Container<AnimationState>* AnimationComponent::stateContainer = nullptr;
