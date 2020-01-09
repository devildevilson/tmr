#include "StateController.h"

#include "EventComponent.h"

#include "Globals.h"

#include "InventoryComponent.h"
#include "AnimationComponent.h"
#include "SoundComponent.h"
#include "AttributesComponent.h"
#include "global_components_indicies.h"

static const Type cancel = Type::get("cancel");
static const Type change_weapon = Type::get("change_weapon");
static const Type movement = Type::get("movement");

StateController::StateController(const CreateInfo &info) : ent(info.ent), currentTime(0), lastTime(0), currentState(info.controllerType->defaultState()), controllerType(info.controllerType) {}

// есть несколько специальных состояний: cancel, change_weapon, use_item
// еще нужно учесть скорость атаки, вот тут то лучше передавать помимо
// ресурсов еще и время, либо у каждого компонента хранить указатель на скорость?
// не везде она нужна прямо так скажем

// в openmw при запуске анимации передается еще и модификатор скорости
// передается еще начальный маркер, конечный маркер, начальная позиция
// (тип начать можно с начала, с конца, посередине)

// анимации смены оружия там нет, есть прерывание текущей анимации
// важная загвоздка для меня при прерывании - это прерывание интеракции
// я планирую сделать прерывание вызовом функции у оружия, как по другому?

// можно ли как то увеличить скорость звуковых эффектов?
// как передавать модификатор? то есть нужно его еще где то хранить
// нужно хранить тип в состоянии походу, у нас еще могут быть алиасы
// так как для движения у нас целых 3 типа

// анимацию и звук можно делать здесь, нужно просто как то понять происходит ли нужная анимация уже или еще нет
// по тому же принципу можно делать интеракцию

// здесь же мы можем обновлять компоненты анимации, звука и интеракции
// подавая им верное время, с другой стороны важно чтобы все равботало одинаково при любых состояниях
// проблемы могут возникнуть только при интеракции, их может быть несколько разных,
// по идее все что не физическая интеракция должно никак не реагировать на изменяющееся время
// звукам передавать какое то иное время бесполезно
// с анимациями все проще гораздо

void StateController::update(const size_t &time) {
  if (currentTime >= currentState->time) {
    const size_t diffTime = currentState->time;
    if (currentState->state == change_weapon) {
      // ставим в текущий стейт состояние оружия
      // а в предыдущий этот
      currentTime -= diffTime;
      lastTime = 0;
      computeSpeedMod();
    } else if (currentState->flags.looping()) {
      currentTime -= diffTime;
      lastTime = 0;
      computeSpeedMod();
    } else if (currentState->nextState != nullptr) {
      prevState = currentState;
      currentState = currentState->nextState;
      currentTime -= diffTime;
      lastTime = 0;
      computeSpeedMod();
    }
  }
  
  currentTime += speedMod * time;
  
  if (currentState->animation.valid() && (currentState->animationDelay == 0 ||  (currentTime >= currentState->animationDelay && lastTime < currentState->animationDelay))) {
    const AnimationComponent::PlayInfo info{
      currentState->animation,
      speedMod,
      currentState->time - currentState->animationDelay,
      currentState->flags.looping()
    };
    //animations->play(info);
    ent->at<AnimationComponent>(ANIMATION_COMPONENT_INDEX)->play(info);
  }
  
  if (currentState->sound.valid() && (currentState->soundDelay == 0 || (currentTime >= currentState->soundDelay && lastTime < currentState->soundDelay))) {
    const SoundComponent::PlayInfo info{
      currentState->sound,
      currentState->time - currentState->soundDelay,
      currentState->flags.looping(),
      !currentState->constantSound,
      currentState->relative,
      !currentState->constantSound,
      {0.0f, 0.0f, 0.0f, 0.0f},
      100.0f, // скорее всего всегда константа
      1.0f, // берем рандомно от 0.8 до 1.2 примерно
      1.0f, // пока неясно
      1.0f
    };
    //sounds->play(info);
    ent->at<SoundComponent>(SOUND_COMPONENT_INDEX)->play(info);
  }
  
  if (nextWeaponState != nullptr) {
    // здесь мы должны сменить оружее
    // изображение оружия должно уйти под нижнюю границу экрана
    // меняем стейт со всеми сопутствующими
    // ждем когда оно до конца подымется
    // ставим nullptr
    // для этого нужно контроллировать uv координаты полностью
    
    nextWeaponState = nullptr;
  }
  
  if (currentState->state == change_weapon) {
    // мы можем определить какое состояние у нас дефолтное из текущего оружия
    const UVAnimation uvInfo{
      UVAnimation::type::uv,
      glm::vec2(-0.5f, -0.5f),
      currentState->time
    };
//     animations->apply(uvInfo);
    ent->at<AnimationComponent>(ANIMATION_COMPONENT_INDEX)->apply(uvInfo);
  }
  
  if (prevState->state == change_weapon) {
    const UVAnimation uvInfo{
      UVAnimation::type::uv,
      glm::vec2(0.5f, 0.5f),
      currentState->time
    };
    //animations->apply(uvInfo);
    ent->at<AnimationComponent>(ANIMATION_COMPONENT_INDEX)->apply(uvInfo);
  }

  lastTime = currentTime;
}

bool StateController::blocking() const {
  return nextWeaponState != nullptr || currentState->flags.blocking();
}

bool StateController::blockingMovement() const {
  return currentState->flags.blockingMovement();
}

bool StateController::looping() const {
  return currentState->flags.looping();
}

bool StateController::usedWeapon() const {
  return currentState->flags.useWeapon();
}

bool StateController::usedItem() const {
  return currentState->flags.useItem();
}

bool StateController::inDefaultState() const {
  return currentState == controllerType->defaultState();
}

bool StateController::isFinished() const {
  return currentTime >= currentState->time;
}

Type StateController::state() const {
  return currentState->state;
}

Type StateController::nextState() const {
  if (currentState->nextState != nullptr) return currentState->nextState->state;
  return Type();
}

Type StateController::previousState() const {
  if (prevState != nullptr) return prevState->state;
  return Type();
}

size_t StateController::time() const {
  return currentTime;
}

size_t StateController::stateTime() const {
  return currentState->time;
}

float StateController::speedModificator() const {
  return speedMod;
}

const StateControllerType* StateController::type() const {
  return controllerType;
}

// event StateController::call(const Type &type, const EventData &data, yacs::entity* entity) {
//   (void)data;
//   (void)entity;
//   currentState = controllerType->state(type);
//   if (currentState == nullptr) throw std::runtime_error("Cannot find state "+type.name()); // ??
// 
//   // причем тут нужно еще учесть специальные состояния
//   // зачем как то особо учитывать конкретно это состояние?
//   if (currentState->state == cancel) {
//     auto weapon = inventory->currentWeapon();
// //     weapon->cancel_interaction(interactions);
// 
//     // резкая остановка звука, лучше конечно так не делать
//     // нужно остановить звук постепенно
//     sounds->cancel();
// 
//     // останавливаем звук и анимацию
//     // нужно ли останавливать анимацию? то есть у нас и так и сяк при изменении анимации, вернется в исходное состояние
//     return success;
//   }
// 
//   // присуще только игроку
//   if (currentState->state == change_weapon) {
//     // должна быть специальная анимация опускания оружия
//     // и поднимания оружия
// 
//     // этот тип должен быть продолжительным и в два этапа
// 
//     const UVAnimation uvInfo{
//       UVAnimation::type::uv,
//       glm::vec2(-0.5f, -0.5f),
//       currentState->time
//     };
//     animations->apply(uvInfo);
// 
//     // нужно еще текущее положение текстурки
// 
//     return success;
//   }
// 
//   // мы возвращаем здесь эвент, можем ли мы вернуть фейл если патронов нет
//   // скорее всего я так и думал сделать но забыл
//   event ev = success;
//   if (currentState->flags.useWeapon()) {
//     auto weapon = inventory->currentWeapon();
// //     ev = weapon->interaction(attribs, interactions);
//   } else if (currentState->flags.useItem()) {
//     auto item = inventory->currentItem();
//     inventory->remove(item);
// //     ev = item->interaction(attribs, interactions);
//   }
// 
//   if (ev == failure) return ev;
// 
//   // нужно добавить делэй
//   const AnimationComponent::PlayInfo animInfo{
//     currentState->animation,
//     1.0f, // как лучше всего сделать? модификатор скорости у нас зависит от состояния
//     currentState->time,
//     currentState->flags.looping()
//   };
//   animations->play(animInfo);
// 
//   // для некоторых звуков можно указать фреквенси, но я не уверен что это хороший вариант
//   const SoundComponent::PlayInfo soundInfo{
//     currentState->sound,
//     currentState->time,
//     false,
//     true,
//     false,
//     true,
//     {0.0f, 0.0f, 0.0f, 0.0f},
//     100.0f,
//     1.0f,
//     1.0f,
//     1.0f,
//   };
//   sounds->play(soundInfo);
// 
//   // также мы должны учесть передвижение ИГРОКА
//   if (currentState->state == movement) {
//     const UVAnimation uvInfo{
//       UVAnimation::type::speed,
//       glm::vec2(0.0f, 0.0f),
//       currentState->time
//     };
//     animations->apply(uvInfo);
//   }
// 
//   return success;
// }

void StateController::setDefaultState() {
  prevState = currentState;
  currentState = controllerType->defaultState();
  currentTime = 0;
  lastTime = 0;

//   if (currentState->flags.useWeapon()) {
//     auto weapon = inventory->currentWeapon();
// //     weapon->interaction(attribs, interactions);
//   } else if (currentState->flags.useItem()) {
//     auto item = inventory->currentItem();
//     inventory->remove(item);
// //     item->interaction(attribs, interactions);
//   }
}

bool StateController::setState(const Type &state) {
  if (this->state() == state) return true;
  
  auto statePtr = controllerType->state(state);
  if (statePtr == nullptr) return false;
  
  prevState = currentState;
  currentState = statePtr;
  currentTime = 0;
  lastTime = 0;
  computeSpeedMod();
  ASSERT(currentState != nullptr);
  
  return true;
}

bool StateController::changeWeaponState(const Type &state) {
  if (this->state() == state) return true;
  
  auto statePtr = controllerType->state(state);
  if (statePtr == nullptr) return false;
  
  nextWeaponState = statePtr;
  
  return true;
}

void StateController::computeSpeedMod() {
  speedMod = 1.0f;
//   if (currentState->speedModificator != nullptr && attribs != nullptr) {
//     auto attrib = attribs->get<FLOAT_ATTRIBUTE_TYPE>(currentState->speedModificator);
//     speedMod = attrib->value();
//   }
  
  auto attribs = ent->at<AttributeComponent>(ATTRIBUTE_COMPONENT_INDEX);
  if (currentState->speedModificator != nullptr && attribs.valid()) {
    auto attrib = attribs->get<FLOAT_ATTRIBUTE_TYPE>(currentState->speedModificator);
    speedMod = attrib->value();
  }
}

StateControllerSystem::StateControllerSystem(const CreateInfo &info) : pool(info.pool) {}

StateControllerSystem::~StateControllerSystem() {
//   for (auto &pair : types) {
//     typePool.deleteElement(pair.second);
//   }
}

void StateControllerSystem::update(const size_t &time) {
//   static const auto func = [&] (const size_t &time, const size_t &start, const size_t &count) {
//     for (size_t i = start; i < start+count; ++i) {
//       auto handle = Global::world()->get_component<StateController>(i);
//       handle->update(time);
//     }
//   };

  const size_t &componentsCount = Global::world()->count_components<StateController>();
  const size_t count = std::ceil(float(componentsCount) / float(pool->size()+1));
  size_t start = 0;
  for (uint32_t i = 0; i < pool->size()+1; ++i) {
    const size_t jobCount = std::min(count, componentsCount-start);
    if (jobCount == 0) break;

    pool->submitbase([time, start, jobCount] () {
      for (size_t i = start; i < start+jobCount; ++i) {
        auto handle = Global::world()->get_component<StateController>(i);
        handle->update(time);
      }
    });

    start += jobCount;
  }

  pool->compute();
  pool->wait();
}

// void StateControllerSystem::create(const Type &type, const StateControllerType::CreateInfo &info) {
//   auto itr = types.find(type);
//   if (itr != types.end()) throw std::runtime_error("State controller type with name "+type.name()+" is already exist");
// 
//   auto ptr = typePool.newElement(info);
//   types[type] = ptr;
// }
// 
// const StateControllerType* StateControllerSystem::get(const Type &type) const {
//   auto itr = types.find(type);
//   if (itr == types.end()) throw std::runtime_error("State controller type with name "+type.name()+" is not exist");
// 
//   return itr->second;
// }
