#include "StateController.h"

#include "EventComponent.h"

#include "Globals.h"

#include "InventoryComponent.h"
#include "AnimationComponent.h"
#include "SoundComponent.h"


StateController::StateController(const CreateInfo &info) : currentTime(0), currentState(info.controllerType->defaultState()), controllerType(info.controllerType) {}

void StateController::update(const size_t &time) {
  currentTime += time;
  if (currentTime >= currentState->time) {
    if (currentState->flags.looping()) {
      currentTime -= currentState->time;
      return;
    }

    if (currentState->nextState == nullptr) return;
    currentState = currentState->nextState;
    currentTime = 0;

    // нужно раскидать ресурсИД по компонентам

    if (currentState->flags.useWeapon()) {
      auto weapon = inventory->currentWeapon();
//       weapon->interaction(attribs, interactions);
    } else if (currentState->flags.useItem()) {
      auto item = inventory->currentItem();
      inventory->remove(item);
//       item->interaction(attribs, interactions);
    }
  }

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
}

bool StateController::blocking() const {
  return currentState->flags.blocking();
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

Type StateController::state() const {
  return currentState->state;
}

Type StateController::nextState() const {
  if (currentState->nextState != nullptr) return currentState->nextState->state;
  return Type();
}

size_t StateController::time() const {
  return currentTime;
}

size_t StateController::stateTime() const {
  return currentState->time;
}

const StateControllerType* StateController::type() const {
  return controllerType;
}

static const Type cancel = Type::get("cancel");
static const Type change_weapon = Type::get("change_weapon");
static const Type movement = Type::get("movement");

event StateController::call(const Type &type, const EventData &data, yacs::entity* entity) {
  (void)data;
  (void)entity;
  currentState = controllerType->state(type);
  if (currentState == nullptr) throw std::runtime_error("Cannot find state "+type.name()); // ??

  // причем тут нужно еще учесть специальные состояния
  // зачем как то особо учитывать конкретно это состояние?
  if (currentState->state == cancel) {
    auto weapon = inventory->currentWeapon();
//     weapon->cancel_interaction(interactions);

    // резкая остановка звука, лучше конечно так не делать
    // нужно остановить звук постепенно
    sounds->cancel();

    // останавливаем звук и анимацию
    // нужно ли останавливать анимацию? то есть у нас и так и сяк при изменении анимации, вернется в исходное состояние
    return success;
  }

  // присуще только игроку
  if (currentState->state == change_weapon) {
    // должна быть специальная анимация опускания оружия
    // и поднимания оружия

    // этот тип должен быть продолжительным и в два этапа

    const UVAnimation uvInfo{
      UVAnimation::type::uv,
      glm::vec2(-0.5f, -0.5f),
      currentState->time
    };
    animations->apply(uvInfo);

    // нужно еще текущее положение текстурки

    return success;
  }

  // мы возвращаем здесь эвент, можем ли мы вернуть фейл если патронов нет
  // скорее всего я так и думал сделать но забыл
  event ev = success;
  if (currentState->flags.useWeapon()) {
    auto weapon = inventory->currentWeapon();
//     ev = weapon->interaction(attribs, interactions);
  } else if (currentState->flags.useItem()) {
    auto item = inventory->currentItem();
    inventory->remove(item);
//     ev = item->interaction(attribs, interactions);
  }

  if (ev == failure) return ev;

  // нужно добавить делэй
  const AnimationComponent::PlayInfo animInfo{
    currentState->animation,
    1.0f, // как лучше всего сделать? модификатор скорости у нас зависит от состояния
    currentState->time,
    currentState->flags.looping()
  };
  animations->play(animInfo);

  // для некоторых звуков можно указать фреквенси, но я не уверен что это хороший вариант
  const SoundComponent::PlayInfo soundInfo{
    currentState->sound,
    currentState->time,
    false,
    true,
    false,
    true,
    {0.0f, 0.0f, 0.0f, 0.0f},
    100.0f,
    1.0f,
    1.0f,
    1.0f,
  };
  sounds->play(soundInfo);

  // также мы должны учесть передвижение ИГРОКА
  if (currentState->state == movement) {
    const UVAnimation uvInfo{
      UVAnimation::type::speed,
      glm::vec2(0.0f, 0.0f),
      currentState->time
    };
    animations->apply(uvInfo);
  }

  return success;
}

void StateController::callDefaultState() {
  currentState = controllerType->defaultState();
  currentTime = 0;

  if (currentState->flags.useWeapon()) {
    auto weapon = inventory->currentWeapon();
//     weapon->interaction(attribs, interactions);
  } else if (currentState->flags.useItem()) {
    auto item = inventory->currentItem();
    inventory->remove(item);
//     item->interaction(attribs, interactions);
  }
}

StateControllerSystem::StateControllerSystem(const CreateInfo &info) : pool(info.pool) {}

StateControllerSystem::~StateControllerSystem() {
  for (auto &pair : types) {
    typePool.deleteElement(pair.second);
  }
}

void StateControllerSystem::update(const size_t &time) {
  static const auto func = [&] (const size_t &time, const size_t &start, const size_t &count) {
    for (size_t i = start; i < start+count; ++i) {
      auto handle = Global::world()->get_component<StateController>(i);
      handle->update(time);
    }
  };

  const size_t &componentsCount = Global::world()->count_components<StateController>();
  const size_t count = std::ceil(float(componentsCount) / float(pool->size()+1));
  size_t start = 0;
  for (uint32_t i = 0; i < pool->size()+1; ++i) {
    const size_t jobCount = std::min(count, componentsCount-start);
    if (jobCount == 0) break;

    pool->submitnr(func, time, start, jobCount);

    start += jobCount;
  }

  pool->compute();
  pool->wait();
}

void StateControllerSystem::create(const Type &type, const StateControllerType::CreateInfo &info) {
  auto itr = types.find(type);
  if (itr != types.end()) throw std::runtime_error("State controller type with name "+type.name()+" is already exist");

  auto ptr = typePool.newElement(info);
  types[type] = ptr;
}

const StateControllerType* StateControllerSystem::get(const Type &type) const {
  auto itr = types.find(type);
  if (itr == types.end()) throw std::runtime_error("State controller type with name "+type.name()+" is not exist");

  return itr->second;
}
