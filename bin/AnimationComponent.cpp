#include "AnimationComponent.h"

#include "Globals.h"
#include "Components.h"
#include "GraphicComponets.h"
#include "EventComponent.h"

// все это дело очень подвязано на AnimationSystem

CLASS_TYPE_DEFINE_WITH_NAME(AnimationComponent, "AnimationComponent")

void AnimationComponent::setStateContainer(Container<AnimationState>* stateContainer) {
  AnimationComponent::stateContainer = stateContainer;
}

AnimationComponent::AnimationComponent() : animStateCurrentIndex(0) {
  animStateCurrentIndex = stateContainer->insert(0);
}

AnimationComponent::~AnimationComponent() {
  stateContainer->erase(animStateCurrentIndex);
}

void AnimationComponent::update(const uint64_t &time) {
  (void)time;
  // здесь видимо ничего
  // такая система с состояниями как у меня сейчас,
  // требует чтобы я обратно стейт менял на какой-то дефолтный или следующий
  
  // получается что должна быть какая то обратная связь
  // то есть предположим у нас есть опять же анимация удара
  // мы кликаем мышкой, и передаем нужный тип в стейт контроллер
  // контроллер передает по чайлдам тип, который мы вычисляем 
  // и по идее мы должны послать сигнал обратно что мы можем обрадотать стейт
  // в контроллере мы увеличиваем счетчик, а когда у нас анимация (удар или звук)
  // заканчивается мы запускаем обратную связь, которая уменьшает счетчик
  // если счетчик == 0 то мы переходим в состояние по умолчанию, или в следующее по стеку
  // если анимация не имеет конца (например бег), то что тогда? вообще ничего, то есть 
  // я должен понимать какая анимация залупена (повторяется),
  // и следовательно в этом стейте никуда не переключаться
  // тип состояний? мне по идее нужно только узнать луп/не луп
  // и тип мы возвращаемся в предыдущую повторяемую анимацию,
  // с другой стороны вся эта запара может не иметь никакого смысла,
  // так как мы должны передавать информацию о конце состояния в ии, 
  // где мы скорее всего снова запустим бехавиор три, с этим особых проблем нет
  // в принципе у нас и сейчас есть isFinished, вот только не понятно как определить 
  // когда анимация повторяется, а когда она действительно закончилась
  // то есть по идее нужно просто вести список тех состояний которые свободно повторяются
  // то есть в этих состояниях нам пофиг закончилась ли анимация или нет
  
  // прерывания анимаций? неплохая идея сделать механизм прерывания анимаций, 
  // то есть например противник получил большой урон, и должен постоять покричать
}

// static const Type type1 = Type::get("Type1");

void AnimationComponent::init(void* userData) {
  (void)userData;
  
//   throw std::runtime_error("AnimationComponent init");
  
  controller = getEntity()->get<StateController>().get();
  if (controller == nullptr) {
    Global::console()->printE("Initializing animation without state controller component");
    throw std::runtime_error("Initializing animation without state controller component");
  }
  
  localEvents = getEntity()->get<EventComponent>().get();
  if (localEvents == nullptr) {
    Global::console()->printE("Initializing animation without event component");
    throw std::runtime_error("Initializing animation without event component");
  }
  
//   controller->addChild(this);
  
  TransformComponent* trans = getEntity()->get<TransformComponent>().get();
  auto g = getEntity()->get<GraphicComponent>(); // что делать с интерфейсом игрока?
  
  const AnimationSystem::AnimationUnitCreateInfo info{
    animStateCurrentIndex,
    controller->getCustomTimeIndex(), //UINT32_MAX,
    trans == nullptr ? UINT32_MAX : trans->transformIndex,
    g->getTextureContainerIndex()
  };
  
  animationUnitIndex = Global::animations()->registerAnimationUnit(info, this);
  // тут нужно еще добавлять указатель на этот класс
}

// void AnimationComponent::addChild(Controller* c) {
//   (void)c;
//   throw std::runtime_error("Cannot use AnimationComponent's addChild");
// }
// 
// ControllerChildData AnimationComponent::changeState(const Type &state) {
//   auto itr = states.find(state);
//   if (itr == states.end()) return ControllerChildData(false, false, false);
//   
//   stateContainer->at(animStateCurrentIndex) = itr->second;
//   
//   return ControllerChildData(true, isBlocking(), isBlockingMovement());
// }
// 
// void AnimationComponent::reset() {
//   // что здесь?
// }
// 
// void AnimationComponent::finishCallback() {
//   controller->finishCallback();
// }
// 
// bool AnimationComponent::isFinished() const {
//   // тут мне нужно получить анимацию и почекать у нее конец
//   // AnimationUnitData мне скорее всего нахрен не нужен
//   
//   const uint32_t animationId = Global::animations()->getCurrentAnimationId(animationUnitIndex);
//   return Global::animations()->getAnimationById(animationId).isFinished();
// }
// 
// bool AnimationComponent::isBlocking() const {
//   const uint32_t animationId = Global::animations()->getCurrentAnimationId(animationUnitIndex);
//   return Global::animations()->getAnimationById(animationId).isBlocking();
// }
// 
// bool AnimationComponent::isBlockingMovement() const {
//   const uint32_t animationId = Global::animations()->getCurrentAnimationId(animationUnitIndex);
//   return Global::animations()->getAnimationById(animationId).isBlockingMovement();
// }

// void AnimationComponent::precacheStateCount(const uint32_t &count) {
//   Global::animations()->precacheStateCount(animationUnitIndex, count);
// }

void AnimationComponent::setAnimation(const Type &state, const std::string &animName) {
  static const auto changeAnimId = [&] (const Type &type, const EventData &data, const uint32_t &index) {
    (void)data;
    (void)type;
    stateContainer->at(animStateCurrentIndex) = index;
    return success;
  };
  
//   localEvents->registerEvent(type1, [&] (const EventData &data) {
//     stateContainer->at(animStateCurrentIndex) = states[type1];
//     
//     return success;
//   });
  
  localEvents->registerEvent(state, std::bind(changeAnimId, std::placeholders::_1, std::placeholders::_2, Global::animations()->getAnimationId(animName)));
  
//   const uint32_t animIndex = Global::animations()->getAnimationId(animName);
//   states[state] = animIndex;
  
//   auto itr = states.find(state);
//   if (itr == states.end()) {
//     itr = states.insert(std::make_pair(state, animStateCurrentIndex)).first;
//     ++animStateCurrentIndex;
//   }
  
//   Global::animations()->setAnimation(animationUnitIndex, itr->second, animName);
}

Container<AnimationState>* AnimationComponent::stateContainer = nullptr;
