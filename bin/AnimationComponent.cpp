#include "AnimationComponent.h"

#include "Globals.h"
#include "GraphicComponets.h"
#include "EventComponent.h"
#include "TransformComponent.h"
#include "Physics.h"

void AnimationComponent::setStateContainer(Container<AnimationState>* stateContainer) {
  AnimationComponent::stateContainer = stateContainer;
}

AnimationComponent::AnimationComponent(const CreateInfo &info)
  : currentAnimationIndex(UINT32_MAX),
    oldAnimationIndex(UINT32_MAX),
    accumulatedTime(0),
    localEvents(info.localEvents),
    trans(info.trans),
    graphics(info.graphics) {}

AnimationComponent::~AnimationComponent() {}

void AnimationComponent::update(const uint64_t &time) {
  // вычисляем прибавочное время
  const size_t newTime = time;
  
  const Animation &anim = Global::animations()->getAnimationById(currentAnimationIndex);
  const bool finished = anim.isFinished(accumulatedTime+newTime);
  if (finished) accumulatedTime = 0; // по идее по разному нужно реагировать
  
  accumulatedTime += newTime;
  
  const size_t textureOffset = anim.getCurrentFrameOffset(accumulatedTime);
  const uint8_t frameSize = anim.frameSize();
    
  int a = 0;
  if (trans != nullptr && frameSize > 1) {
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
  
  const size_t finalTextureIndex = textureOffset + a;
  
  graphics->setTexture(Global::animations()->getAnimationTextureData(finalTextureIndex));
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

void AnimationComponent::setAnimation(const Type &state, const ResourceID &id) {
  static const auto changeAnimId = [&] (const Type &type, const EventData &data, const uint32_t &index) {
    (void)data;
    (void)type;
    
    currentAnimationIndex = index;
    if (oldAnimationIndex != currentAnimationIndex) {
      accumulatedTime = 0;
      oldAnimationIndex = currentAnimationIndex;
    }
    
    return success;
  };
  
  localEvents->registerEvent(state, std::bind(changeAnimId, std::placeholders::_1, std::placeholders::_2, Global::animations()->getAnimationId(id)));
}

//size_t & AnimationComponent::getInternalIndex() {
//  return internalIndex;
//}

Container<AnimationState>* AnimationComponent::stateContainer = nullptr;
