#include "AIComponent.h"

#include "Globals.h"
#include "TinyBehavior/TinyBehavior/TinyBehavior.h"
#include "Components.h"
#include "AISystem.h"

AIBasicComponent::AIBasicComponent(const CreateInfo &info) : internalIndexVal(SIZE_MAX) {
  aiData.r = info.radius;
  aiData.vertex = info.currentVertex;
  aiData.lastVertex = info.currentVertex;
  aiData.timeThreshold = info.timeThreshold;
}

AIBasicComponent::~AIBasicComponent() {
  if (internalIndexVal != SIZE_MAX) {
    Global::ai()->removeBasicComponent(this);
    internalIndexVal = SIZE_MAX;
  }
}

void AIBasicComponent::update(const size_t &time) {
  (void)time;
  // ничего?
}

void AIBasicComponent::init(void* userData) {
  (void)userData;
  
  Global::ai()->registerBasicComponent(this);
  
  // возможно вершина должна приходить через юзер дату
}

size_t & AIBasicComponent::internalIndex() {
  return internalIndexVal;
}

AIComponent::AIComponent(const CreateInfo &info) : AIBasicComponent({info.radius, info.timeThreshold, info.currentVertex}), tree(info.tree) {}
AIComponent::~AIComponent() {
  Global::ai()->removeComponent(this);
  internalIndexVal = SIZE_MAX;
}

void AIComponent::update(const size_t &time) {
  // здесь мы обновляем ии, то есть делаем следующее:
  
  if (hasGroup() && this != group()->ai()) {
    // только выходим?
    return;
  }
  
  // время лучше перенести из аиДаты
  aiData.currentTime += time;
  
  auto runningNode = tree->getRunning();
  
  if (aiData.currentTime < aiData.timeThreshold && runningNode != nullptr) {
    // обновляем только нод
    runningNode->update(this);
    return;
  }
  
  if (aiData.currentTime >= aiData.timeThreshold) {
    // может ли у нас не быть aitree? не думаю
    tree->update(this);
    aiData.currentTime = 0;
  }
  
  // по идее на этом все
}

void AIComponent::init(void* userData) {
  (void)userData;
  
  physics = getEntity()->get<PhysicsComponent2>().get();
  trans = getEntity()->get<TransformComponent>().get();
  
  localEvents = getEntity()->get<EventComponent>().get();
  if (localEvents == nullptr) {
    Global::console()->printE("Initializing AIComponent without EventComponent");
    throw std::runtime_error("Initializing AIComponent without EventComponent");
  }
  
  Global::ai()->registerComponent(this);
  
  localEvents->registerEvent(Type::get("move_to_target"), [&] (const Type & type, const EventData &data) {
    if (target() == nullptr) return failure;
    
    // добавляем в очередь на поиск, нужно как то запомнить что мы добавили уже
    // так же можно поискать среди уже существующих путей
    // нужно ли разделить move_to_target от непосредственно поиска?
    // возможно это будет неплохой идеей
  });
}
 
void AIComponent::updateAIData() {
  // тут обновляем поз, вел, дир
  // и прочие вещи
  
  if (trans != nullptr) {
    pos = trans->pos();
    dir = trans->rot();
  }
  
  if (physics != nullptr) {
    vel = physics->getVelocity();
  }
}
