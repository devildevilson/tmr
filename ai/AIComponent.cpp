#include "AIComponent.h"

#include "Globals.h"
#define TINY_BEHAVIOUR_MULTITHREADING
#include "tiny_behaviour/TinyBehavior.h"
//#include "Components.h"
#include "TransformComponent.h"
#include "UserDataComponent.h"
#include "AIInputComponent.h"
#include "AISystem.h"

#include "MoveEventData.h"
#include "Graph.h"

#include "global_components_indicies.h"

AIBasicComponent::AIBasicComponent(const CreateInfo &info) : EntityAI(EntityAI::CreateInfo{info.entType, info.radius, info.currentVertex, info.ent}) {
//   aiData.r = info.radius;
//   aiData.vertex = info.currentVertex;
//   aiData.lastVertex = info.currentVertex;
  
  if (m_vertex != nullptr) m_vertex->addObject(this); // для базового компонента по идее это обязательно
}

AIBasicComponent::~AIBasicComponent() {
//   if (internalIndexVal != SIZE_MAX) {
//    Global::ai()->removeBasicComponent(this);
//     internalIndexVal = SIZE_MAX;
//   }
  
  if (m_vertex != nullptr) m_vertex->removeObject(this);
  else if (lastVertex != nullptr) lastVertex->removeObject(this);
}

void AIBasicComponent::update(const size_t &time) {
  (void)time;
  // ничего?
}

//void AIBasicComponent::init(void* userData) {
//  (void)userData;
//
//  Global::ai()->registerBasicComponent(this);
//
//  // возможно вершина должна приходить через юзер дату
//}

// size_t & AIBasicComponent::internalIndex() {
//   return internalIndexVal;
// }

UserDataComponent* AIBasicComponent::components() const {
  return ent->at<UserDataComponent>(USER_DATA_COMPONENT_INDEX).get();
}

AIComponent::AIComponent(const CreateInfo &info)
  : AIBasicComponent({info.entType, info.radius, info.currentVertex, info.ent}),
    tree(info.tree),
    runningNode(nullptr),
//    physics(info.physics),
//    trans(info.trans),
//     input(info.input),
    timeThreshold(info.timeThreshold),
    currentTime(0) {
//     pathfindingTime(0),
//     currentPathSegment(1),
//     pathFindType(info.pathFindType) {
//   states.setOnGround(aiData.vertex != nullptr);
//  localEvents = info.events;

//   usrData->events->registerEvent(Type(), this);

  // move_to_target в таком виде наверное не будет существовать
//  usrData->events->registerEvent(Type::get("move_to_target"), [&] (const Type & type, const EventData &data) {
//    (void)type;
//    (void)data;
//
//    if (target() == nullptr) return failure;
//
//    // для начала мы должны разделить движение по пути и движение к объекту
//    // тут будет просто движение к объекту
//    // как его сделать? нам нужено поведение преследования
//    // по идее нам нужно просто вызвать функцию в инпуте
//    input->seek(target()->position());
//    input->setMovement(1.0f, 0.0f, 0.0f);
//    const simd::vec4 velocity = usrData->phys->getVelocity();
//    const float dot = simd::dot(velocity, velocity);
//    if (dot > EPSILON) {
//      usrData->trans->rot() = velocity / std::sqrt(dot);
//    }
//
//    return running;
//  });
//
//  usrData->events->registerEvent(Type::get("move_path"), [&] (const Type & type, const EventData &data) {
//    (void)type;
//    (void)data;
//
//    if (target() == nullptr) return failure;
//    if (this->vertex() == target()->vertex()) {
//      releasePath();
//      return success;
//    }
//    if (foundPath.path == nullptr && oldPath.path == nullptr) return failure;
//
//    // тут мы пытаемся идти по пути
//    // нужно ли вообще хранить путь в таком виде?
//    // здесь мы можем преобразовать путь с помощью фуннела
//    // фуннел будем выполнять при поиске
//    // теперь по идее мы должны передать путь в инпут компонент и по нему мы должны пройти
//
//    // предиктион тайм? вообще наверное по хорошему надо бы объеденить независимость от времени ИИ и физики
//    // но в текущем случае сделать это сложно, является ли это большой проблемой?
//    // тут определяется какие данные нужно взять для инпута чтобы потом двигаться в нужную сторону
//
//    RawPath* path = foundPath.path == nullptr ? oldPath.path : foundPath.path;
//    const size_t lastIndex = path->funnelData().size()-1;
//
//    if (lastIndex < currentPathSegment) {
//      releasePath();
//
//      states.setPathExisting(false);
//
//      std::cout << "path moving success" << "\n";
//
//      return success;
//    }
//
//    // нужно сделать проверку в правильном ли мы месте сейчас, как?
//
//    const simd::vec4 nextPos =
//            input->followPath(Global::ai()->getUpdateDelta(), path, currentPathSegment);
//    input->setMovement(1.0f, 0.0f, 0.0f);
//    //trans->rot() = simd::normalize(input->front());
//    const simd::vec4 velocity = usrData->phys->getVelocity();
//    const float dot = simd::dot(velocity, velocity);
//    if (dot > EPSILON) {
//      usrData->trans->rot() = velocity / std::sqrt(dot);
//    }
//
//    const simd::vec4 point = projectPointOnPlane(-PhysicsEngine::getGravityNorm(), usrData->trans->pos(), nextPos);
//    const float tolerance = 0.4f;
//    if (simd::distance2(usrData->trans->pos(), point) < tolerance) ++currentPathSegment;
//
//    PRINT_VAR("currentPathSegment", currentPathSegment)
//    PRINT_VAR("lastIndex   ", lastIndex)
//
//    std::cout << "path moving" << "\n";
//
//    return running;
//  });
//
//  usrData->events->registerEvent(Type::get("move"), [&] (const Type & type, const EventData &data) {
//    (void)type;
//    // не помешает еще сделать пользовательское движение в какие-нибудь стороны
//    // какие данные нужны? и нужно ли их хранить? хотя если мы будем вызывать это дело каждый кадр, то может и нет
//    // пока будем возвращать раннинг будем делать каждый раз при обновлении компонента
//
//    MoveEventData* move = reinterpret_cast<MoveEventData*>(data.userData);
//    if (move->type == MoveEvent::point) {
//      input->seek(simd::vec4(move->arr));
//    } else if (move->type == MoveEvent::direction) {
//      input->front() = simd::vec4(move->arr);
//    }
//
//    input->setMovement(1.0f, 0.0f, 0.0f);
//    const simd::vec4 velocity = usrData->phys->getVelocity();
//    const float dot = simd::dot(velocity, velocity);
//    if (dot > EPSILON) {
//      usrData->trans->rot() = velocity / std::sqrt(dot);
//    }
//
//    return running;
//  });
//
//  usrData->events->registerEvent(Type::get("find_path"), [&] (const Type & type, const EventData &data) {
//    (void)type;
//    (void)data;
//
//    const size_t currentTime = Global::mcsSinceEpoch();
//    if (foundPath.path != nullptr && (currentTime - pathfindingTime) > HALF_SECOND) { // HALF_SECOND
//      oldPath.path = foundPath.path;
//      oldPath.req = foundPath.req;
//
//      foundPath.path = nullptr;
//      foundPath.req.end = nullptr;
//      foundPath.req.start = nullptr;
//
//      states.setPathExisting(false);
//    }
//
//    if (target() == nullptr) return failure;
//    if (target()->vertex() == nullptr) return failure;
//    if (target()->vertex() == this->vertex()) return success;
//
//    if (foundPath.path == nullptr) {
//      if (foundPath.req.start != nullptr && foundPath.req.end != nullptr) {
//        const auto &pathData = Global::ai()->pathfindingSystem()->getPath(foundPath.req);
//
//        if (pathData.state == path_finding_state::delayed) return running;
//        else if (pathData.state == path_finding_state::has_path) {
//          if (oldPath.path != nullptr) {
//            Global::ai()->pathfindingSystem()->releasePath(oldPath.req);
//            oldPath.path = nullptr;
//          }
//
//          foundPath.path = pathData.path;
//
//          states.setPathExisting(true);
//          currentPathSegment = 1;
//
//          std::cout << "path finding success" << "\n";
//
//          return success;
//        } else if (pathData.state == path_finding_state::path_not_exist) {
//          states.setPathExisting(false);
//
//          if ((currentTime - pathfindingTime) > QUARTER_SECOND) {
//            Global::ai()->pathfindingSystem()->releasePath(foundPath.req);
//            foundPath.req.end = nullptr;
//            foundPath.req.start = nullptr;
//          }
//
//          std::cout << "path finding failed" << "\n";
//
//          return failure;
//        }
//      }
//
//      const FindRequest req{
//              pathFindType,
//              this->vertex(),
//              target()->vertex()
//      };
//      Global::ai()->pathfindingSystem()->queueRequest(req);
//
//      pathfindingTime = currentTime;
//      foundPath.req = req;
//
//      std::cout << "path finding running" << "\n";
//
//      return running;
//    }
//
//    std::cout << "path already fouded" << "\n";
//
//    return success;
//  });
}
                                                   
AIComponent::~AIComponent() {
//   if (internalIndexVal != SIZE_MAX) {
//    Global::ai()->removeComponent(this);
//     internalIndexVal = SIZE_MAX;
//   }
  
//   if (aiData.vertex != nullptr) aiData.vertex->removeObject(this);
//   else aiData.lastVertex->removeObject(this);
}

void AIComponent::update(const size_t &time) {
  // здесь мы обновляем ии, то есть делаем следующее:
  
  auto input = ent->at<InputComponent>(INPUT_COMPONENT_INDEX);
  input->setMovement(0.0f, 0.0f, 0.0f); // && this != group()->ai()
  if (hasGroup()) {
    // только выходим?
    return;
  }
  
  // время лучше перенести из аиДаты
  currentTime += time;
  
  if (currentTime < timeThreshold && runningNode != nullptr) {
    // обновляем только нод
    EntityAI* ptr = this;
    auto status = runningNode->update(ptr);
    
    if (status != tb::Node::status::running) runningNode = nullptr;
    
    return;
  }
  
  runningNode = nullptr;
  
//   std::cout << "Update tree" << "\n";
  
  if (currentTime >= timeThreshold || runningNode == nullptr) {
    // может ли у нас не быть aitree? не думаю
    // так мы правильно приводим к EntityAI
    EntityAI* ptr = this;
    tree->update(ptr, &runningNode);
    currentTime = 0;
  }
  
  // по идее на этом все
}

//void AIComponent::init(void* userData) {
//  (void)userData;
//
//  physics = getEntity()->get<PhysicsComponent2>().get();
//  trans = getEntity()->get<TransformComponent>().get();
//
//  localEvents = getEntity()->get<EventComponent>().get();
//  if (localEvents == nullptr) {
//    Global::console()->printE("Initializing AIComponent without EventComponent");
//    throw std::runtime_error("Initializing AIComponent without EventComponent");
//  }
//
//  input = getEntity()->get<AIInputComponent>().get();
//  if (input == nullptr) {
//    Global::console()->printE("Initializing AIComponent without AIInputComponent");
//    throw std::runtime_error("Initializing AIComponent without AIInputComponent");
//  }
//
//  Global::ai()->registerComponent(this);
//}
 
void AIComponent::updateAIData() {
  // тут обновляем поз, вел, дир
  // и прочие вещи
  
//   auto usrData = components();
  auto trans = ent->at<TransformComponent>(TRANSFORM_COMPONENT_INDEX);
  auto phys = ent->at<PhysicsComponent>(PHYSICS_COMPONENT_INDEX);
  if (trans.valid()) {
    trans->pos().storeu(pos);
    trans->rot().storeu(dir);
  }
  
  if (phys.valid()) {
    phys->getVelocity().storeu(vel);
    objectIndex = phys->getIndexContainer().objectDataIndex;
    
    auto cont = phys->getGround();
    if (cont != nullptr) {
      auto data = reinterpret_cast<UserDataComponent*>(cont->userData);
      if (data->aiComponent == nullptr) {
        PRINT_VAR("entity id", data->entity->id())
      }
      
      //vertex_t* vertex = const_cast<vertex_t*>(data->aiComponent->vertex()); // TODO: убрать const_cast
      vertex_t* groundVertex = data->vertex;
      if (m_vertex != groundVertex) {
        groundVertex->addObject(this);
        if (m_vertex != nullptr) m_vertex->removeObject(this);
        else if (lastVertex != nullptr) lastVertex->removeObject(this);
        
        lastVertex = m_vertex != nullptr ? m_vertex : lastVertex;
        m_vertex = groundVertex;
      }
    } else {
      if (m_vertex != nullptr) {
        lastVertex = m_vertex != nullptr ? m_vertex : lastVertex;
        m_vertex = nullptr;
      }
    }
  }
  
  //states.setOnGround(aiData.vertex != nullptr);
  
  // тут же мы должны обновить vertex на котором сейчас стоим
  // из физики мы должны вытянуть граунд
  // как это сделать я примерно понимаю, нужен указатель на вершину
  // неплохо было бы сохранить еще инфу о том находимся ли мы сейчас на земле (это скорее всего можно вывести из текущей вершины)
}

// void AIComponent::releasePath() {
//   if (foundPath.path == nullptr) {
//     Global::ai()->pathfindingSystem()->releasePath(oldPath.req);
//     oldPath.path = nullptr;
//     oldPath.req.end = nullptr;
//     oldPath.req.start = nullptr;
//   } else {
//     Global::ai()->pathfindingSystem()->releasePath(foundPath.req);
//     foundPath.path = nullptr;
//     foundPath.req.end = nullptr;
//     foundPath.req.start = nullptr;
//   }
// }

// const Type move_to_target = Type::get("move_to_target");
// const Type move_path = Type::get("move_path");
// const Type move = Type::get("move");
// const Type find_path = Type::get("find_path");

// event AIComponent::call(const Type &type, const EventData &data, yacs::entity* entity) {
//   (void)entity;
// 
//   if (type == move_to_target) {
//     if (target() == nullptr) return failure;
// 
//     // для начала мы должны разделить движение по пути и движение к объекту
//     // тут будет просто движение к объекту
//     // как его сделать? нам нужено поведение преследования
//     // по идее нам нужно просто вызвать функцию в инпуте
//     input->seek(target()->position());
//     input->setMovement(1.0f, 0.0f, 0.0f);
//     const simd::vec4 velocity = usrData->phys->getVelocity();
//     const float dot = simd::dot(velocity, velocity);
//     if (dot > EPSILON) {
//       usrData->trans->rot() = velocity / std::sqrt(dot);
//     }
// 
//     return running;
//   } else if (type == move_path) {
//     if (target() == nullptr) return failure;
//     if (this->vertex() == target()->vertex()) {
//       releasePath();
//       return success;
//     }
//     if (foundPath.path == nullptr && oldPath.path == nullptr) return failure;
// 
//     // тут мы пытаемся идти по пути
//     // нужно ли вообще хранить путь в таком виде?
//     // здесь мы можем преобразовать путь с помощью фуннела
//     // фуннел будем выполнять при поиске
//     // теперь по идее мы должны передать путь в инпут компонент и по нему мы должны пройти
// 
//     // предиктион тайм? вообще наверное по хорошему надо бы объеденить независимость от времени ИИ и физики
//     // но в текущем случае сделать это сложно, является ли это большой проблемой?
//     // тут определяется какие данные нужно взять для инпута чтобы потом двигаться в нужную сторону
// 
//     RawPath* path = foundPath.path == nullptr ? oldPath.path : foundPath.path;
//     const size_t lastIndex = path->funnelData().size()-1;
// 
//     if (lastIndex < currentPathSegment) {
//       releasePath();
// 
//       states.setPathExisting(false);
// 
//       std::cout << "path moving success" << "\n";
// 
//       return success;
//     }
// 
//     // нужно сделать проверку в правильном ли мы месте сейчас, как?
// 
//     const simd::vec4 nextPos =
//       input->followPath(Global::ai()->getUpdateDelta(), path, currentPathSegment);
//     input->setMovement(1.0f, 0.0f, 0.0f);
//     //trans->rot() = simd::normalize(input->front());
//     const simd::vec4 velocity = usrData->phys->getVelocity();
//     const float dot = simd::dot(velocity, velocity);
//     if (dot > EPSILON) {
//       usrData->trans->rot() = velocity / std::sqrt(dot);
//     }
// 
//     const simd::vec4 point = projectPointOnPlane(-PhysicsEngine::getGravityNorm(), usrData->trans->pos(), nextPos);
//     const float tolerance = 0.4f;
//     if (simd::distance2(usrData->trans->pos(), point) < tolerance) ++currentPathSegment;
// 
//     PRINT_VAR("currentPathSegment", currentPathSegment)
//     PRINT_VAR("lastIndex   ", lastIndex)
// 
//     std::cout << "path moving" << "\n";
// 
//     return running;
//   } else if (type == move) {
//     MoveEventData* move = reinterpret_cast<MoveEventData*>(data.userData);
//     if (move->type == MoveEvent::point) {
//       input->seek(simd::vec4(move->arr));
//     } else if (move->type == MoveEvent::direction) {
//       input->front() = simd::vec4(move->arr);
//     }
// 
//     input->setMovement(1.0f, 0.0f, 0.0f);
//     const simd::vec4 velocity = usrData->phys->getVelocity();
//     const float dot = simd::dot(velocity, velocity);
//     if (dot > EPSILON) {
//       usrData->trans->rot() = velocity / std::sqrt(dot);
//     }
// 
//     return running;
//   } else if (type == find_path) {
//     const size_t currentTime = Global::mcsSinceEpoch();
//     if (foundPath.path != nullptr && (currentTime - pathfindingTime) > HALF_SECOND) { // HALF_SECOND
//       oldPath.path = foundPath.path;
//       oldPath.req = foundPath.req;
// 
//       foundPath.path = nullptr;
//       foundPath.req.end = nullptr;
//       foundPath.req.start = nullptr;
// 
//       states.setPathExisting(false);
//     }
// 
//     if (target() == nullptr) return failure;
//     if (target()->vertex() == nullptr) return failure;
//     if (target()->vertex() == this->vertex()) return success;
// 
//     if (foundPath.path == nullptr) {
//       if (foundPath.req.start != nullptr && foundPath.req.end != nullptr) {
//         const auto &pathData = Global::ai()->pathfindingSystem()->getPath(foundPath.req);
// 
//         if (pathData.state == path_finding_state::delayed) return running;
//         else if (pathData.state == path_finding_state::has_path) {
//           if (oldPath.path != nullptr) {
//             Global::ai()->pathfindingSystem()->releasePath(oldPath.req);
//             oldPath.path = nullptr;
//           }
// 
//           foundPath.path = pathData.path;
// 
//           states.setPathExisting(true);
//           currentPathSegment = 1;
// 
//           std::cout << "path finding success" << "\n";
// 
//           return success;
//         } else if (pathData.state == path_finding_state::path_not_exist) {
//           states.setPathExisting(false);
// 
//           if ((currentTime - pathfindingTime) > QUARTER_SECOND) {
//             Global::ai()->pathfindingSystem()->releasePath(foundPath.req);
//             foundPath.req.end = nullptr;
//             foundPath.req.start = nullptr;
//           }
// 
//           std::cout << "path finding failed" << "\n";
// 
//           return failure;
//         }
//       }
// 
//       const FindRequest req{
//         pathFindType,
//         this->vertex(),
//         target()->vertex()
//       };
//       Global::ai()->pathfindingSystem()->queueRequest(req);
// 
//       pathfindingTime = currentTime;
//       foundPath.req = req;
// 
//       std::cout << "path finding running" << "\n";
// 
//       return running;
//     }
// 
//     std::cout << "path already fouded" << "\n";
// 
//     return success;
//   }
// 
//   return success;
// }

//PhysicsComponent* AIComponent::phys() const {
//  return physics;
//}
