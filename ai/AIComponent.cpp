#include "AIComponent.h"

#include "Globals.h"
#define TINY_BEHAVIOUR_MULTITHREADING
#include "tiny_behaviour/TinyBehavior.h"
#include "Components.h"
#include "AISystem.h"

#include "MoveEventData.h"
#include "Graph.h"

AIBasicComponent::AIBasicComponent(const CreateInfo &info) : internalIndexVal(SIZE_MAX) {
  aiData.r = info.radius;
  aiData.vertex = info.currentVertex;
  aiData.lastVertex = info.currentVertex;
  
  aiData.vertex->addObject(this);
}

AIBasicComponent::~AIBasicComponent() {
//   if (internalIndexVal != SIZE_MAX) {
    Global::ai()->removeBasicComponent(this);
//     internalIndexVal = SIZE_MAX;
//   }
  
  if (aiData.vertex != nullptr) aiData.vertex->removeObject(this);
  else aiData.lastVertex->removeObject(this);
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

AIComponent::AIComponent(const CreateInfo &info) : AIBasicComponent({info.radius, info.currentVertex}), 
                                                   tree(info.tree), 
                                                   runningNode(nullptr), 
                                                   physics(nullptr), 
                                                   trans(nullptr), 
                                                   timeThreshold(info.timeThreshold),
                                                   currentTime(0),
                                                   pathfindingFrame(SIZE_MAX),
                                                   pathFindType(info.pathFindType) {
  states.setOnGround(aiData.vertex != nullptr);
  
  //aiData.vertex->addObject(this);
}
                                                   
AIComponent::~AIComponent() {
//   if (internalIndexVal != SIZE_MAX) {
    Global::ai()->removeComponent(this);
//     internalIndexVal = SIZE_MAX;
//   }
  
//   if (aiData.vertex != nullptr) aiData.vertex->removeObject(this);
//   else aiData.lastVertex->removeObject(this);
}

void AIComponent::update(const size_t &time) {
  // здесь мы обновляем ии, то есть делаем следующее:
  
  input->setMovement(0.0f, 0.0f, 0.0f); // && this != group()->ai()
  if (hasGroup()) {
    // только выходим?
    return;
  }
  
  // время лучше перенести из аиДаты
  currentTime += time;
  
  if (currentTime < timeThreshold && runningNode != nullptr) {
    // обновляем только нод
    auto status = runningNode->update(this);
    
    if (status == tb::Node::status::success || status == tb::Node::status::failure) runningNode = nullptr;
    
    return;
  }
  
  runningNode = nullptr;
  
  if (currentTime >= timeThreshold || runningNode == nullptr) {
    // может ли у нас не быть aitree? не думаю
    tree->update(this, &runningNode);
    currentTime = 0;
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
  
  // move_to_target в таком виде наверное не будет существовать
  localEvents->registerEvent(Type::get("move_to_target"), [&] (const Type & type, const EventData &data) {
    (void)type;
    (void)data;
    
    if (target() == nullptr) return failure;

    // для начала мы должны разделить движение по пути и движение к объекту
    // тут будет просто движение к объекту
    // как его сделать? нам нужено поведение преследования
    // по идее нам нужно просто вызвать функцию в инпуте
    input->seek(target()->position());
    input->setMovement(1.0f, 0.0f, 0.0f);
    
    return running;
  });
  
  localEvents->registerEvent(Type::get("move_path"), [&] (const Type & type, const EventData &data) {
    (void)type;
    (void)data;
    
    if (foundPath.path == nullptr && oldPath.path == nullptr) return failure;

    // тут мы пытаемся идти по пути
    // нужно ли вообще хранить путь в таком виде?
    // здесь мы можем преобразовать путь с помощью фуннела
    // фуннел будем выполнять при поиске
    // теперь по идее мы должны передать путь в инпут компонент и по нему мы должны пройти
    
    // предиктион тайм? вообще наверное по хорошему надо бы объеденить независимость от времени ИИ и физики
    // но в текущем случае сделать это сложно, является ли это большой проблемой?
    // тут определяется какие данные нужно взять для инпута чтобы потом двигаться в нужную сторону
                             
    RawPath* path = foundPath.path == nullptr ? oldPath.path : foundPath.path;
    
    // че с индексом? видимо надо искать индекс последней фуннел точки
    const size_t currentIndex = input->followPath(Global::ai()->getUpdateDelta(), path);
    input->setMovement(1.0f, 0.0f, 0.0f);
    
    simd::vec4 vec;
    float dist;
    const size_t lastIndex = path->getNearPathSegmentIndex(path->data().back().funnelPoint, vec, dist);
    
    if (lastIndex == currentIndex) {
      if (foundPath.path == nullptr) {
        Global::ai()->pathfindingSystem()->releasePath(oldPath.req);
        oldPath.path = nullptr;
      } else {
        Global::ai()->pathfindingSystem()->releasePath(foundPath.req);
        foundPath.path = nullptr;
      }
      
      states.setPathExisting(false);
      
      return success;
    }
    
    return running;
  });
  
  localEvents->registerEvent(Type::get("move"), [&] (const Type & type, const EventData &data) {
    (void)type;
    // не помешает еще сделать пользовательское движение в какие-нибудь стороны
    // какие данные нужны? и нужно ли их хранить? хотя если мы будем вызывать это дело каждый кадр, то может и нет
    // пока будем возвращать раннинг будем делать каждый раз при обновлении компонента
    
    MoveEventData* move = reinterpret_cast<MoveEventData*>(data.userData);
    if (move->type == MoveEvent::point) {
      input->seek(simd::vec4(move->arr));
    } else if (move->type == MoveEvent::direction) {
      input->front() = simd::vec4(move->arr);
    }
    
    input->setMovement(1.0f, 0.0f, 0.0f);
    
    return running;
  });
  
  localEvents->registerEvent(Type::get("find_path"), [&] (const Type & type, const EventData &data) {
    (void)type;
    (void)data;
    
    if (target() == nullptr) return failure;
    
    // возможно нужно сначало проверить не сильно ли устарел путь?
    // мало того нужно чтобы челик не останавливался пока путь ищет
    // а шел хоть куда нибудь
    // значит удалять старый путь нужно не здесь, а видимо когда нашли новый
    // так же нужно запомнить req
    if (foundPath.path != nullptr) {
      oldPath.path = foundPath.path;
      oldPath.req = foundPath.req;
      //Global::ai()->pathfindingSystem()->releasePath(foundPath.req);
      foundPath.path = nullptr;
    }
                             
    // ставим в очередь
    const size_t frameIndex = Global::frameIndex();
    if (pathfindingFrame == SIZE_MAX || frameIndex - pathfindingFrame > 10) { // нужно придумать какой-нибудь коэфициент
      const FindRequest req{
        pathFindType,
        this->vertex(),
        target()->vertex()
      };
      Global::ai()->pathfindingSystem()->queueRequest(req);
      
      // чтобы отделить один реквест от другого, наверное нужно запомнить какой сейчас кадр
      pathfindingFrame = frameIndex;
      foundPath.req = req;
      
      return running;
    }
    
    // пытаемся получить ответ из pathfindingSystem
    if (pathfindingFrame != SIZE_MAX && frameIndex - pathfindingFrame <= 10) {
      const auto &pathData = Global::ai()->pathfindingSystem()->getPath(foundPath.req);
      
      if (pathData.state == path_finding_state::delayed) return running;
      else if (pathData.state == path_finding_state::has_path) {
        if (oldPath.path != nullptr) {
          Global::ai()->pathfindingSystem()->releasePath(oldPath.req);
          oldPath.path = nullptr;
        }
        
        // надо запомнить путь и при каких переменных мы его взяли
        foundPath.path = pathData.path;
//         foundPath.frameIndex = frameIndex;
        
        states.setPathExisting(true);
        
        return success;
      }
    }
    
    return failure;
  });
}
 
void AIComponent::updateAIData() {
  // тут обновляем поз, вел, дир
  // и прочие вещи
  
  if (trans != nullptr) {
    trans->pos().storeu(pos);
    trans->rot().storeu(dir);
  }
  
  if (physics != nullptr) {
    physics->getVelocity().storeu(vel);
    objectIndex = physics->getIndexContainer().objectDataIndex;
    
    auto cont = physics->getGround();
    if (cont != nullptr) {
      PhysUserData* data = reinterpret_cast<PhysUserData*>(cont->userData);
      
      if (aiData.vertex != data->vertex) {
        data->vertex->addObject(this);
        if (aiData.vertex != nullptr) aiData.vertex->removeObject(this);
        else aiData.lastVertex->removeObject(this);
        
        aiData.lastVertex = aiData.vertex != nullptr ? aiData.vertex : aiData.lastVertex;
        aiData.vertex = data->vertex;
      }
    } else {
      if (aiData.vertex != nullptr) {
        aiData.lastVertex = aiData.vertex != nullptr ? aiData.vertex : aiData.lastVertex;
        aiData.vertex = nullptr;
      }
    }
  }
  
  states.setOnGround(aiData.vertex != nullptr);
  
  // тут же мы должны обновить vertex на котором сейчас стоим
  // из физики мы должны вытянуть граунд
  // как это сделать я примерно понимаю, нужен указатель на вершину
  // неплохо было бы сохранить еще инфу о том находимся ли мы сейчас на земле (это скорее всего можно вывести из текущей вершины)
}
