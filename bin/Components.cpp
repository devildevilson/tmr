#include "Components.h"

#include "Globals.h"
#include "VulkanRender.h"
// #include "Physics.h"
#include "GraphicComponets.h"
#include "AnimationComponent.h"
// #include "Animation.h"
#include "Utility.h"
#include "Optimizers.h"

#include "EventComponent.h"

#include <glm/gtx/rotate_vector.hpp>
// #define YACS_DEFINE_EVENT_TYPE
// #include "YACS.h"

// std::unordered_map<std::string, size_t> ComponentType::stringToType;
// const float quarterPI = glm::pi<float>() / 4.0f;
// const float eighthPI = glm::pi<float>() / 8.0f;

#define CHANCHED_PLACE 0
#define BLOCKING_PLACE (CHANCHED_PLACE+1)
#define BLOCKING_MOVEMENT_PLACE (BLOCKING_PLACE+1)

ControllerChildData::ControllerChildData() {}

ControllerChildData::ControllerChildData(const bool &changed, const bool &blocking, const bool &blockingMovement) {
  make(changed, blocking, blockingMovement);
}

void ControllerChildData::make(const bool &changed, const bool &blocking, const bool &blockingMovement) {
  cont = uint32_t(changed) << CHANCHED_PLACE |
         uint32_t(blocking) << BLOCKING_PLACE |
         uint32_t(blockingMovement) << BLOCKING_MOVEMENT_PLACE;
}

bool ControllerChildData::changed() const {
  const uint32_t mask = 0x1;
  return (cont >> CHANCHED_PLACE) & mask;
}

bool ControllerChildData::blocking() const {
  const uint32_t mask = 0x1;
  return (cont >> BLOCKING_PLACE) & mask;
}

bool ControllerChildData::blockngMovement() const {
  const uint32_t mask = 0x1;
  return (cont >> BLOCKING_MOVEMENT_PLACE) & mask;
}

CLASS_TYPE_DEFINE_WITH_NAME(InfoComponent, "InfoComponent")

InfoComponent::InfoComponent(const Type &type) : objType(type) {}

InfoComponent::~InfoComponent() {}

void InfoComponent::update(const size_t &time) {
//   (void)time;
//   
//   const auto &idxCont = phys2->getIndexContainer();
//   const Object &obj = Global::physics()->getObjectData(idxCont.objectDataIndex);
//   
//   std::string type;
//   switch(obj.objType.getObjType()) {
//     case BBOX_TYPE:
//       type = "Bounding box";
//       break;
//     case POLYGON_TYPE:
//       type = "Polygon";
//       break;
//     case SPHERE_TYPE:
//       type = "Sphere";
//       break;
//     default:
//       std::cout << "aaaaaaaaaaaaaa" << "\n";
//       throw std::runtime_error("Getting obj type failed");
//   }
//   
//   ImGui::Text("Object index: %zu, object type: %s", getEntity()->getId(), objType.getName().c_str());
//   ImGui::Text("Physics object index: %u", idxCont.objectDataIndex);
//   ImGui::Separator();
//   
//   if (trans != nullptr) {
//     ImGui::Text("Obj center: %.2f, %.2f, %.2f", trans->pos().x, trans->pos().y, trans->pos().z);
//     ImGui::Text("Obj dir   : %.2f, %.2f, %.2f", trans->rot().x, trans->rot().y, trans->rot().z);
//     ImGui::Text("Obj scale : %.2f, %.2f, %.2f", trans->scale().x, trans->scale().y, trans->scale().z);
//   } else {
// //     const glm::vec3 &center = coll->getCenter();
// //     ImGui::Text("Obj center: %.2f, %.2f, %.2f", center.x, center.y, center.z);
//   }
//   
//   if (obj.objType.getObjType() == POLYGON_TYPE) {
//     //Polygon* pl = (Polygon*)coll->getCollidable();
//     ImGui::Text("Plane has %u points", obj.vertexCount); 
// //     ImGui::Text("Plane normal: %.2f, %.2f, %.2f", pl->normal.x, pl->normal.y, pl->normal.z);
// //     ImGui::Text("Plane has %zu neighbors", Global::ai()->getGraph()->vertex(pl->graphIndex).degree());
//   }
//   
//   //if ()
//   //ImGui::Text("Current animation state: %s", graphic->getCurrentState()->getName().c_str());
//   ImGui::Text("Current animation state: default");
}

void InfoComponent::init(void* userData) {
  (void)userData;
  trans = getEntity()->get<TransformComponent>().get();
//   ai = getEntity()->get<LoneAi>().get();
  graphic = getEntity()->get<GraphicComponent>().get();
//   phys = getEntity()->get<PhysicComponent>().get();
//   coll = getEntity()->get<CollisionComponent>().get();
  phys2 = getEntity()->get<PhysicsComponent2>().get();
  anim = getEntity()->get<AnimationComponent>().get();
}

void InfoComponent::edit() {
  if (trans != nullptr) trans->uiDraw();
  if (graphic != nullptr) graphic->uiDraw();
//   if (ai != nullptr) ai->uiDraw();
}

std::string InfoComponent::getType() const {
  return objType.getName();
}

CLASS_TYPE_DEFINE_WITH_NAME(TransformComponent, "TransformComponent")

void TransformComponent::setContainer(Container<Transform>* container) {
  TransformComponent::container = container;
}

void TransformComponent::uiDraw() {
//   if (ImGui::CollapsingHeader("Transform")) {
//     Transform &trans = container->at(transformIndex);
//     
//     ImGui::Text("Position :");
//     ImGui::SameLine();
//     ImGui::DragFloat3("pos", reinterpret_cast<float*>(&trans.pos), 0.1f);
//     ImGui::Text("Direction:");
//     ImGui::SameLine();
//     ImGui::DragFloat3("dir", reinterpret_cast<float*>(&trans.rot), 0.1f);
//     ImGui::Text("Scale    :");
//     ImGui::SameLine();
//     ImGui::DragFloat3("scale", reinterpret_cast<float*>(&trans.scale), 0.1f);
//   }
}

simd::mat4 TransformComponent::getTransform(const bool rotation) const {
  simd::mat4 mat = simd::translate(simd::mat4(1.0f), pos());
  if (rotation) {
    const glm::vec3 &rot = Global::getPlayerRot();
    mat = simd::rotate(mat, glm::half_pi<float>() - rot.y, simd::vec4(0.0f, 1.0f, 0.0f, 0.0f));
  }
  mat = simd::scale(mat, scale());
  
  return mat;
}

Container<Transform>* TransformComponent::container = nullptr;

CLASS_TYPE_DEFINE_WITH_NAME(InputComponent, "InputComponent")

void InputComponent::setContainer(Container<InputData>* container) {
  InputComponent::container = container;
}

InputComponent::InputComponent() : physics2(nullptr), trans(nullptr) {
  inputIndex = container->insert({});
}

InputComponent::~InputComponent() {
  container->erase(inputIndex);
}

void InputComponent::init(void* userData) {
  (void)userData;
  
//   physic = getEntity()->get<PhysicComponent>().get();
//   if (physic == nullptr) {
//     Global::console()->printE("Entity " + std::to_string(getEntity()->getId()) + " does not have PhysicComponent!");
//     throw std::runtime_error("Entity " + std::to_string(getEntity()->getId()) + " does not have PhysicComponent!");
//   }
  
  trans = getEntity()->get<TransformComponent>().get();
  if (trans == nullptr) {
    Global::console()->printE("Entity " + std::to_string(getEntity()->getId()) + " does not have TransformComponent!");
    throw std::runtime_error("Entity " + std::to_string(getEntity()->getId()) + " does not have TransformComponent!");
  }
  
  physics2 = getEntity()->get<PhysicsComponent2>().get();
  if (physics2 == nullptr) {
    Global::console()->printE("Entity " + std::to_string(getEntity()->getId()) + " does not have PhysicsComponent!");
    throw std::runtime_error("Entity " + std::to_string(getEntity()->getId()) + " does not have PhysicsComponent!");
  }
}

// simd::vec4 InputComponent::predictPos(const size_t &predictionTime) const {
//   return trans->pos() + physics2->getVelocity() * MCS_TO_SEC(predictionTime);
// }
// 
// void InputComponent::seek(const simd::vec4 &target) {
//   frontVec = target - trans->pos();
//   updateInput();
// //   const glm::vec3 &dVel = target - trans->pos;
// //   return dVel - physic->getVelocity();
// }
// 
// void InputComponent::flee(const simd::vec4 &target) {
//   frontVec = trans->pos() - target;
//   updateInput();
// }
// 
// void InputComponent::followPath(const size_t &predictionTime, const Pathway &path, const size_t &currentPathSegmentIndex) {
// //   const float pathDistOffset = dir * MCS_TO_SEC(predictionTime) * physic->getSpeed();
//   
//   // предскажем нашу будущую позицию
//   const simd::vec4 &futurePos = predictPos(predictionTime);
//   const Path & pathSeg = path.getPathSegments()[currentPathSegmentIndex];
//   const Path & prevPathSeg = path.getPathSegments()[currentPathSegmentIndex-1];
//   
//   const simd::vec4 &nextPoint = pathSeg.point + pathSeg.lineDir * trans->scale().x;
//   const simd::vec4 &prevPoint = prevPathSeg.point + prevPathSeg.lineDir * trans->scale().x;
//   
//   // примерно так узнаем что мы двигаемся в правильную сторону
//   const bool rightWay2 = glm::dot(physics2->getVelocity(), nextPoint - prevPoint) > 0.0f;
//   
//   float dist;
//   glm::vec3 closest;
//   /*size_t ind = */path.getNearPathSegmentIndex(trans->pos(), closest, dist);
//   float dist2;
//   glm::vec3 closest2;
//   /*size_t ind2 = */path.getNearPathSegmentIndex(futurePos, closest2, dist2);
//   
// //   const bool ret = 
//   
//   if (dist2 < trans->scale().x && rightWay2) return;
//   
//   if (dist < trans->scale().x && rightWay2) {
//     frontVec = nextPoint - prevPoint;
//     return;
//   }
//   
//   frontVec = nextPoint - trans->pos();
//   
//   updateInput();
//   
//   // я примерно понимаю как заставить монстра идти по пути
//   // но я еще не понимаю как заставить кучу мостров следовать по пути той же кучей
//   // заставить их идти только по направлению? создавать группу в рантайме и уже исходя из нее действовать?
//   // flocking behavior
//   
//   // получим текущий пройденный путь и в будущем
// //   const float &currentPathDist = path.mapPointToPathDistance(trans->pos);
// //   // нам совсем не обязательно брать пройденный путь в будущем, чтобы узнать направление
// //   // для этого нам потребуется ближайший участок пути
// //   // и направление движения (например скорость)
// //   const float &futurePathDist  = path.mapPointToPathDistance(futurePos);
// //   
// //   // в правильную ли сторону мы направленны?
// //   const bool rightWay = pathDistOffset > 0 ? currentPathDist < futurePathDist : currentPathDist > futurePathDist;
// //   
// //   // найдем точку на (?) пути ближайшую к предсказанию
// //   glm::vec3 tangent;
// //   float outside;
// //   const glm::vec3 &onPath = path.mapPointToPath(futurePos, tangent, outside);
// //   
// // //   float dist;
// // //   size_t ind = path.getNearPathSegmentIndex(futurePos, dist);
// //   
// //   // корректировка пути ненужна если:
// //   // будущая позиция на пути
// //   // мы направленны в верную сторону
// //   if (outside < 0.0f && rightWay) {
// //     return glm::vec3(0.0f, 0.0f, 0.0f);
// //   }
// //   
// //   // иначе мы должны скорректировать путь по точке 
// //   // которую мы получиим сложив offset с текущей точкой на пути
// //   const float targetPathDist = currentPathDist + pathDistOffset;
// //   const glm::vec3 &target = path.mapPathDistanceToPoint(targetPathDist);
// //   
// // //   annotatePathFollowing(futurePos, onPath, target, outside);
// //   
// //   // здесь мы получаем вектор, который надо сложить с вектором скорости
// //   // в итоге мы получим вектор, который и будет являться необходимым нам направлением
// //   // по идее
// //   glm::vec3 newVel = physic->getVelocity() + seek(target);
// //   glm::vec3 newDir = glm::normalize(newVel); // как то так это выглядит
// //   // я так понимаю мне нужно переделать сам путь, чтобы он выдавал 
// //   // все проходимые вершины с нормализованным направлением
// //   // или не нужно?
// //   return seek(target);
// }

// void InputComponent::updateInput() {
//   container->at(inputIndex) = {
//     rightVec,
//     upVec,
//     frontVec,
//     simd::vec4(sideMove, upMove, forwardMove, 0.0f)
//   };
// }

simd::vec4 & InputComponent::front() {
  return container->at(inputIndex).front;
}

simd::vec4 & InputComponent::up() {
  return container->at(inputIndex).up;
}

simd::vec4 & InputComponent::right() {
  return container->at(inputIndex).right;
}

simd::vec4 & InputComponent::movementData() {
  return container->at(inputIndex).moves;
}

const simd::vec4 & InputComponent::front() const {
  return container->at(inputIndex).front;
}

const simd::vec4 & InputComponent::up() const {
  return container->at(inputIndex).up;
}

const simd::vec4 & InputComponent::right() const {
  return container->at(inputIndex).right;
}

const simd::vec4 & InputComponent::movementData() const {
  return container->at(inputIndex).moves;
}

void InputComponent::setMovement(const float &front, const float &up, const float &right) {
  container->at(inputIndex).moves = simd::vec4(right, up, front, 0.0f);
}

Container<InputData>* InputComponent::container = nullptr;

// void InputComponent::stayOnPath(const size_t &predictionTime, const Pathway &path) {
//   
// }

CLASS_TYPE_DEFINE_WITH_NAME(PhysicsComponent2, "PhysicsComponent2")

void PhysicsComponent2::setContainer(Container<ExternalData>* externalDatas) {
  PhysicsComponent2::externalDatas = externalDatas;
}

PhysicsComponent2::PhysicsComponent2(/*void* userData*/) {
  // const PhysicsObjectCreateInfo i{
  //   false,

  //   PhysicsType(true, BBOX_TYPE, true, false, true, true),
  //   1,     // collisionGroup
  //   1,     // collisionFilter

  //   1.0f,  // stairHeight
  //   40.0f, // acceleration
  //   1.0f,  // overbounce
  //   4.0f,  // groundFricion

  //   0.0f,  // radius
  //   pos,

  //   UINT32_MAX, // matrixIndex

  //   "boxShape"
  // };

  // Global::physic2()->add(i, &container);
  
  //userData.ent = getEntity();

  container = {
    UINT32_MAX,
    UINT32_MAX,
    UINT32_MAX,
    UINT32_MAX,
    UINT32_MAX,
    UINT32_MAX,
    &userData
  };

  // с ускорениями нужно поиграть побольше
  const ExternalData data{
    simd::vec4(0.0f, 0.0f, 0.0f, 0.0f),
    7.0f, 80.0f, 0.0f, 0.0f
  };

  externalDataIndex = externalDatas->insert(data);
}

PhysicsComponent2::~PhysicsComponent2() {
  if (container.internalIndex != UINT32_MAX) {
    Global::physics()->remove(&container);
  }

  externalDatas->erase(externalDataIndex);
}

void PhysicsComponent2::update(const size_t &time) {
  (void)time;
}

void PhysicsComponent2::init(void* userData) {
//   std::cout << "PhysicsComponent2::init" << "\n";
  
  trans = getEntity()->get<TransformComponent>().get();
  auto input = getEntity()->get<InputComponent>().get();
  if (input == nullptr) input = getEntity()->get<UserInputComponent>().get();
  auto graphic = getEntity()->get<GraphicComponent>().get();

  const uint32_t inputIndex = input == nullptr ? UINT32_MAX : input->inputIndex;
  const uint32_t transformIndex = trans == nullptr ? UINT32_MAX : trans->transformIndex;
  const uint32_t matrixIndex = graphic == nullptr ? UINT32_MAX : graphic->getMatrixIndex();
  const uint32_t rotationIndex = graphic == nullptr ? UINT32_MAX : graphic->getRotationDataIndex();

  InitComponents* initData = (InitComponents*)userData;
  
  this->userData.ent = getEntity();

  // физику я буду создавать здесь полюбому
  // так как мне требуется доступ к индексам некоторых вещей
  // индекс трансформы у меня хранится, например, в другом компоненте
  // в конструкторе все эти вещи нет возможности получить
  const PhysicsObjectCreateInfo i{
    false,

    PhysicsType(initData->dynamic, initData->objType, true, false, true, true),
    1,     // collisionGroup
    1,     // collisionFilter

    0.5f,  // stairHeight
    //40.0f, // acceleration
    1.0f,  // overbounce
    4.0f,  // groundFricion

    0.0f,  // radius

    inputIndex,
    transformIndex,
    externalDataIndex,
    matrixIndex,
    rotationIndex,

    //"boxShape"
    initData->shapeName
  };

  Global::physics()->add(i, &container);
}

const PhysicsIndexContainer & PhysicsComponent2::getIndexContainer() const {
  return container;
}

simd::vec4 PhysicsComponent2::getVelocity() const {
  const glm::vec3 &vel = Global::physics()->getPhysicData(container.physicDataIndex).velocity;
  return simd::vec4(vel.x, vel.y, vel.z, 0.0f);
}

Container<ExternalData>* PhysicsComponent2::externalDatas = nullptr;

// void PhysicsComponent2::updateInput(const InputData &input) {
//   Global::physic2()->setInput(container.inputIndex, input);
// }

CLASS_TYPE_DEFINE_WITH_NAME(StateController, "StateController")

void StateController::setContainer(Container<uint32_t>* customTimeContainer) {
  StateController::customTimeContainer = customTimeContainer;
}

void StateController::addLoopedState(const Type &type) {
  StateController::loopedStates.insert(type.getType());
}

bool StateController::isStateLooped(const Type &type) {
  return true;
}

StateController::StateController() : customTimeIndex(UINT32_MAX), currentTime(0), localEvents(nullptr) {}
StateController::~StateController() {}

void StateController::update(const size_t &time) {
  currentTime += time;
  
  if (currentTime >= stateTime) {
    finished = true;
  }
  
  // мы умножаем время на определенный аттрибут
  // так же мы должны закинуть посчитаное время в customTimeContainer
  // мы должны как то определить конечное время
  // чет не хочется нагружать StateController
  // но мне нужно запомнить конечное время
  // а ну мы можем передать вермя в registerState
  
  // здешний атрибут обязан выдавать 1 по умолчанию
  // иных альтурнатив здесь не может быть
}

void StateController::init(void* userData) {
  (void)userData;
  
  localEvents = getEntity()->get<EventComponent>().get();
  if (localEvents == nullptr) {
    Global::console()->printE("Initializing state controller without event component");
    throw std::runtime_error("Initializing state controller without event component");
  }
}

// void StateController::addChild(Controller* c) {
//   childs.push_back(c);
// }
// 
// ControllerChildData StateController::changeState(const Type &type) {
//   bool blocking = false;
//   bool blockingMovement = false;
//   
//   const bool loop = isStateLooped(type);
//   for (auto* child : childs) {
//     const auto data = child->changeState(type);
//     blocking = blocking || data.blocking();
//     blockingMovement = blockingMovement || data.blockngMovement();
//     
//     if (!loop) nonFinishedStates += uint32_t(data.changed());
//   }
//   
//   finished = nonFinishedStates == 0;
//   
//   this->blocking = blocking;
//   this->blockingMovement = blockingMovement;
//   
//   return ControllerChildData(true, blocking, blockingMovement);
// }
// 
// void StateController::reset() {
//   for (auto* child : childs) {
//     child->reset();
//   }
// }
// 
// void StateController::finishCallback() {
//   --nonFinishedStates;
//   
//   finished = nonFinishedStates == 0;
// }

Type StateController::getCurrentState() const {
  return state;
}

bool StateController::isFinished() const {
//   for (auto* child : childs) {
//     if (!child->isFinished()) return false;
//   }
//   
//   return true;
  
  return finished;
}

bool StateController::isBlocking() const {
//   for (auto* child : childs) {
//     if (child->isBlocking()) return true;
//   }
//   
//   return false;
  
  return blocking;
}

bool StateController::isBlockingMovement() const {
//   for (auto* child : childs) {
//     if (child->isBlockingMovement()) return true;
//   }
//   
//   return false;
  
  return blockingMovement;
}

uint32_t StateController::getCustomTimeIndex() const {
  return customTimeIndex;
}

// сюда можно добавить общее время состояния
// это плохо тем что я не знаю какое время брать за основу, поэтому здесь может быть потенциально куча ошибок
// в принципе можно разработать последовательность по которой это время будет определяться
void StateController::registerState(const Type &type, const bool blocking, const bool blockingMovement, const size_t &stateTime) {
  static const auto func = [&] (const Type &type, const EventData &data, const bool blocking, const bool blockingMovement, const size_t &stateTime) {
    (void)data;
    this->blocking = blocking;
    this->blockingMovement = blockingMovement;
    this->state = type;
    
    // что-то тут еще должно быть для того чтобы определить закончилась ли анимация?
    // для того чтобы ответить на этот вопрос, нужно добавить счетчик времени
    // и обновлять этот компонент
    this->currentTime = 0;
    this->stateTime = stateTime;
    this->finished = false;
    
    return success;
  };
  
  localEvents->registerEvent(type, std::bind(func, std::placeholders::_1, std::placeholders::_2, blocking, blockingMovement, stateTime));
}

Container<uint32_t>* StateController::customTimeContainer = nullptr;
std::unordered_set<size_t> StateController::loopedStates;

// CLASS_TYPE_DEFINE_WITH_NAME(GraphicComponent, "GraphicComponent")
// 
// void GraphicComponent::setContainer(Container<simd::mat4>* matrices) {
//   GraphicComponent::matrices = matrices;
// }
// 
// void GraphicComponent::setContainer(Container<RotationData>* rotationDatas) {
//   GraphicComponent::rotationDatas = rotationDatas;
// }
// 
// void GraphicComponent::setContainer(Container<Texture>* textureContainer) {
//   GraphicComponent::textureContainer = textureContainer;
// }
// 
// // GraphicComponent::GraphicComponent(const uint32_t &pipelineIndex) {
// //   this->pipelineIndex = pipelineIndex;
// // }
// 
// GraphicComponent::GraphicComponent() {
//   textureContainerIndex = textureContainer->insert({UINT32_MAX, UINT32_MAX, UINT32_MAX});
// }
// 
// GraphicComponent::~GraphicComponent() {
//   textureContainer->erase(textureContainerIndex);
// }
// 
// void GraphicComponent::update(const size_t &time) {
//   (void)time;
//   
//   // тут в новом апдейте, теперь только передать данные нужному оптимизеру
//   
// 
//   Texture texture;
//   
//   if (getEntity()->getId() == 3) {
//     //const glm::vec3 pos = trans->pos;
//     const glm::vec3 pos = glm::vec3(trans->getPos());
//     
//     glm::vec3 dir = Global::getPlayerPos() - pos;
//     
//     // это не особо решает проблему с изменением координат
//     // скорее всего мне потребуется умножать на матрицу вектор, чтобы привести его в обатное состояние
//     // но теперь мне скорее всего этого будет достаточно
//     glm::vec3 dirOnGround = Physics::projectVectorOnPlane(-Physics::getGravityNorm(), pos, dir);
//     
//     dir = glm::normalize(dirOnGround);
//     
//     float angle2 = glm::acos(glm::dot(trans->rot, dir));
//     // проверим сторону
//     if (sideOf(pos, pos+trans->rot, Global::getPlayerPos(), -Physics::getGravityNorm()) > 0.0f) angle2 = -angle2;
//     
//     // поправка на 22.5 градусов (так как 0 принадлежит [-22.5, 22.5))
//     angle2 -= eighthPI;
//     
//     if (angle2 < 0.0f) angle2 = angle2 + glm::two_pi<float>();
//     if (angle2 > glm::two_pi<float>()) angle2 = angle2 - glm::two_pi<float>();
//     int a = glm::floor(angle2 / quarterPI);
// 
//     // я не понимаю почему 
//     a = (a + 5) % 8;
//     
// //     std::cout << "Side: " << a << "\n";
// 
//     texture = currentState->getTexture(a);
//   } else {
//     texture = currentState->getTexture(0);
//   }
// 
//   DrawInfo info{
//     texture,
//     //trans->pos,
//     glm::vec3(trans->getPos()),
//     trans->rot,
//     trans->scale,
//     glm::vec3(0.0f, 0.0f, 0.0f),
//     0, 0, 0
//   };
//   
//   Global::render()->add(RENDER_TYPE_MONSTER, info);
// }
// 
// void GraphicComponent::init(void* userData) {
//   (void)userData;
// //   trans = getEntity()->get<TransformComponent>().get();
//   
// //   if (trans == nullptr) {
// //     Global::console()->printE("Entity " + std::to_string(getEntity()->getId()) + " does not have TransformComponent!");
// //     throw std::runtime_error("Entity " + std::to_string(getEntity()->getId()) + " does not have TransformComponent!");
// //   }
// //   
// //   getEntity()->get<StateController>()->addChild(this);
//   
//   
// }
// 
// void GraphicComponent::uiDraw() {
//   if (ImGui::CollapsingHeader("Graphic")) {
//     ImGui::Text("Monster and decoration standart mesh");
//     if (ImGui::IsItemHovered()) {
//       ImGui::BeginTooltip();
//       ImGui::Text("You cannot change it.");
//       ImGui::EndTooltip();
//     }
//     
//     ImGui::Text("Current state: %s", currentState->getName().c_str());
//     
// //     ImGui::Text("Texture scale:");
// //     ImGui::SameLine();
// //     ImGui::DragFloat3("scale", &scale, 0.1f);
//   }
// }
// 
// void GraphicComponent::drawBoundingShape(CollisionComponent* shape, const simd::vec4 &color) const {
//   // if (shape->getType() != AABBOX) {
//   //   Global::console()->print("Entity's (id: " + std::to_string(getEntity()->getId()) + ") shape is not AABB");
//   // }
//   
//   // DrawInfo info;
//   // info.relativeOnPRotOnly = false;
//   // info.useIndexBuffer = true;
//   // info.viewIndex = 0;
//   // info.vertexCount = 6*4;
//   // info.firstVertex = 0;
//   // info.pipelineIndex = Global::render()->getDevice()->getBasicAABBPIndex();
//   // info.translation = shape->getCenter();
//   // info.scale = trans->scale; //((AABB*)shape)->extent;
//   // info.color = color;
//   // info.vertexBuffer = Global::getAABB();
//   // info.indexBuffer = Global::getAABBIdx();
//   // Global::render()->addDrawCommand(info);
//   
//   (void)shape;
//   
//   simd::mat4 mat = glm::translate(simd::mat4(), trans->pos);
//   mat = glm::scale(mat, trans->scale);
//   
//   Global::render()->addDebugDraw([mat, color] (yavf::GraphicTask* task) {
//     PushConst consts{
//       mat,
//       color,
//       glm::vec3(0.0f, 0.0f, 0.0f)
//     };
//     
//     task->setConsts(0, sizeof(PushConst), &consts);
//     task->drawIndirect(Global::getIndirectBuffer(), 2);
//   });
// }
// 
// // void GraphicComponent::addChild(Controller* c) { (void)c; }
// // 
// // void GraphicComponent::changeState(const Type &type) {
// //   auto itr = states.find(type.getType());
// //   if (itr == states.end()) return;
// //   
// //   currentState = itr->second;
// // }
// // 
// // void GraphicComponent::reset() {
// //   currentState->reset();
// // }
// // 
// // bool GraphicComponent::isFinished() const {
// //   return currentState->isFinished();
// // }
// // 
// // bool GraphicComponent::isBlocking() const {
// //   return currentState->isBlocking();
// // }
// // 
// // bool GraphicComponent::isBlockingMovement() const {
// //   return currentState->isBlockingMovement();
// // }
// // 
// // void GraphicComponent::setAnim(const Type &type, GraphicState* anim) {
// //   states[type.getType()] = anim;
// //   
// //   if (currentState == nullptr) currentState = anim;
// // }
// // 
// // GraphicState* GraphicComponent::getCurrentState() const {
// //   return currentState;
// // }
// 
// uint32_t GraphicComponent::getMatrixIndex() const {
//   return matrixIndex;
// }
// 
// uint32_t GraphicComponent::getRotationDataIndex() const {
//   return rotationDataIndex;
// }
// 
// uint32_t GraphicComponent::getTextureContainerIndex() const {
//   return textureContainerIndex;
// }
// 
// Container<simd::mat4>* GraphicComponent::matrices = nullptr;
// Container<RotationData>* GraphicComponent::rotationDatas = nullptr;
// Container<Texture>* GraphicComponent::textureContainer = nullptr;
// MonsterOptimizer* GraphicComponent::optimizer = nullptr;
// 
// CLASS_TYPE_DEFINE_WITH_NAME(GraphicComponentIndexes, "GraphicComponentIndexes")
// 
// // GraphicComponentIndexes::GraphicComponentIndexes(const uint32_t &pipelineIndex, const size_t &offset, const size_t &elemCount) : GraphicComponent(pipelineIndex) {
// //   this->offset = offset;
// //   this->elemCount = elemCount;
// // }
// 
// GraphicComponentIndexes::GraphicComponentIndexes(const size_t &offset, const size_t &elemCount) : GraphicComponent() {
//   this->offset = offset;
//   this->elemCount = elemCount;
// }
// 
// GraphicComponentIndexes::GraphicComponentIndexes(const size_t &offset, const size_t &elemCount, const uint32_t &faceIndex) : GraphicComponent() {
//   this->offset = offset;
//   this->elemCount = elemCount;
//   this->faceIndex = faceIndex;
// }
// 
// GraphicComponentIndexes::~GraphicComponentIndexes() {}
// 
// void GraphicComponentIndexes::update(const size_t &time) {
//   const DrawInfo info{
//     currentState->getTexture(),
//     glm::vec3(0.0f, 0.0f, 0.0f),
//     glm::vec3(0.0f, 0.0f, 0.0f),
//     glm::vec3(0.0f, 0.0f, 0.0f),
//     glm::vec3(0.0f, 0.0f, 0.0f),
//     faceIndex,
//     elemCount,
//     offset
//   };
// 
//   Global::render()->add(RENDER_TYPE_GEOMETRY, info);
// }
// 
// void GraphicComponentIndexes::init(void* userData) {
// //   GraphicComponent::init(userData);
//   (void)userData;
//   getEntity()->get<StateController>()->addChild(this);
// }
// 
// void GraphicComponentIndexes::uiDraw() {
//   if (ImGui::CollapsingHeader("Graphic")) {
//     if (Global::getMapIndex()->ptr() == nullptr) {
//       ImGui::Text("(some string for error ex:) The game must be in \'dev\' mode");
//     } else {
//       if (Global::getMapVertex()->ptr() == nullptr) {
//         Global::console()->printE("Ptr to map vertices is NULL!");
//         throw std::runtime_error("Ptr to map vertices is NULL!");
//       }
//       
//       uint32_t* idx = (uint32_t*)Global::getMapIndex()->ptr();
//       idx = idx + offset;
//       Vertex* verts = (Vertex*)Global::getMapVertex()->ptr();
//       ImGui::Text("Fast actions with UV coords:");
//       if (ImGui::Button("Turn left")) {
// //         std::cout << "Turning left" << "\n";
//         
//         glm::vec2 tmp = verts[idx[elemCount-1]].texCoord;
//         for (int64_t i = elemCount-2; i >= 0; --i) {
//           //verts[idx[i]].texCoord = tmp;
//           std::swap(verts[idx[i]].texCoord, tmp);
//         }
//         
//         std::swap(verts[idx[elemCount-1]].texCoord, tmp);
//         graphicIndex = (graphicIndex - 1) % elemCount;
//       }
//       
//       ImGui::SameLine();
//       
//       if (ImGui::Button("Turn right")) {
// //         std::cout << "Turning right" << "\n";
//         
//         glm::vec2 tmp = verts[idx[0]].texCoord;
//         for (size_t i = 1; i < elemCount; ++i) {
//           //verts[idx[i]].texCoord = tmp;
//           std::swap(verts[idx[i]].texCoord, tmp);
//         }
//         
//         std::swap(verts[idx[0]].texCoord, tmp);
//         graphicIndex = (graphicIndex + 1) % elemCount;
//       }
//       
//       ImGui::SameLine();
//       
//       if (ImGui::Button("Try fix")) {
// //         size_t index = 0;
// //         for (size_t i = 0; i < indicies->param.dataCount; ++i) {
// //           if (vec_eq(verts[idx[i]].texCoord, glm::vec2(0.0f, 0.0f))) {
// //             index = i;
// //             break;
// //           }
// //         }
//         
//         CollisionComponent* coll = getEntity()->get<CollisionComponent>().get();
//         Polygon* poly = (Polygon*)coll;
//         
//         glm::vec3 x, y;
//         
//         if (fast_fabsf(poly->normal.x) < EPSILON && fast_fabsf(poly->normal.y) < EPSILON) {
//           x = glm::vec3(1.0f, 0.0f, 0.0f);
//           y = glm::vec3(0.0f, 1.0f, 0.0f);
//         } else {
//           x = glm::normalize(glm::vec3(-poly->normal.y, poly->normal.x, 0.0f));
//           y = glm::normalize(glm::vec3(-poly->normal.x*poly->normal.z, -poly->normal.y*poly->normal.z, poly->normal.x*poly->normal.x + poly->normal.y*poly->normal.y));
//         }
//         
//         float angle = (float)graphicIndex * (glm::two_pi<float>()/(float)elemCount);
//         glm::vec3 newX = glm::normalize(glm::rotate(x, angle, poly->normal));
//         glm::vec3 newY = glm::normalize(glm::rotate(y, angle, poly->normal));
//         
//         size_t index2 = graphicIndex;
//         for (size_t i = 1; i < elemCount; ++i) {
//           index2 = (index2 + 1) % elemCount;
//           
//           float a = glm::dot(newX, verts[idx[index2]].pos-verts[idx[graphicIndex]].pos);
//           float b = glm::dot(newY, verts[idx[index2]].pos-verts[idx[graphicIndex]].pos);
//           
//           verts[idx[index2]].texCoord = glm::vec2(a, b);
//         }
//       }
//       
//       ImGui::SameLine();
//       
//       if (ImGui::Button("Try fix 2")) {
//         float angle = 0.0f;
//         for (size_t i = 0; i < graphicIndex; ++i) {
//           size_t j = (i + 1) % elemCount;
//           size_t k = (i - 1) % elemCount;
//           
//           angle = angle + Physics::angleVec2(verts[idx[j]].pos-verts[idx[i]].pos, verts[idx[k]].pos-verts[idx[i]].pos);
//         }
//         
//         CollisionComponent* coll = getEntity()->get<CollisionComponent>().get();
//         Polygon* poly = (Polygon*)coll;
//         
//         glm::vec3 x, y;
//         
//         if (fast_fabsf(poly->normal.x) < EPSILON && fast_fabsf(poly->normal.y) < EPSILON) {
//           x = glm::vec3(1.0f, 0.0f, 0.0f);
//           y = glm::vec3(0.0f, 1.0f, 0.0f);
//         } else {
//           x = glm::normalize(glm::vec3(-poly->normal.y, poly->normal.x, 0.0f));
//           y = glm::normalize(glm::vec3(-poly->normal.x*poly->normal.z, -poly->normal.y*poly->normal.z, poly->normal.x*poly->normal.x + poly->normal.y*poly->normal.y));
//         }
//         
//         glm::vec3 newX = glm::normalize(glm::rotate(x, angle, poly->normal));
//         glm::vec3 newY = glm::normalize(glm::rotate(y, angle, poly->normal));
//         
//         size_t index2 = graphicIndex;
//         for (size_t i = 1; i < elemCount; ++i) {
//           index2 = (index2 + 1) % elemCount;
//           
//           float a = glm::dot(newX, verts[idx[index2]].pos-verts[idx[graphicIndex]].pos);
//           float b = glm::dot(newY, verts[idx[index2]].pos-verts[idx[graphicIndex]].pos);
//           
//           verts[idx[index2]].texCoord = glm::vec2(a, b);
//         }
//       }
//       
//       ImGui::SameLine();
//       
//       if (ImGui::Button("Try fix 3")) {
//         CollisionComponent* coll = getEntity()->get<CollisionComponent>().get();
//         Polygon* poly = (Polygon*)coll;
//         
//         glm::vec3 x, y;
//         
//         if (fast_fabsf(poly->normal.x) < EPSILON && fast_fabsf(poly->normal.y) < EPSILON) {
//           x = glm::vec3(1.0f, 0.0f, 0.0f);
//           y = glm::vec3(0.0f, 1.0f, 0.0f);
//         } else {
//           x = glm::normalize(glm::vec3(-poly->normal.y, poly->normal.x, 0.0f));
//           y = glm::normalize(glm::vec3(-poly->normal.x*poly->normal.z, -poly->normal.y*poly->normal.z, poly->normal.x*poly->normal.x + poly->normal.y*poly->normal.y));
//         }
//         
//         float angle = (float)graphicIndex*(glm::two_pi<float>()/4.0f);
//         glm::vec3 newX = glm::normalize(glm::rotate(x, angle, poly->normal));
//         glm::vec3 newY = glm::normalize(glm::rotate(y, angle, poly->normal));
//         
//         size_t index2 = graphicIndex;
//         for (size_t i = 1; i < elemCount; ++i) {
//           index2 = (index2 + 1) % elemCount;
//           
//           float a = glm::dot(newX, verts[idx[index2]].pos-verts[idx[graphicIndex]].pos);
//           float b = glm::dot(newY, verts[idx[index2]].pos-verts[idx[graphicIndex]].pos);
//           
//           verts[idx[index2]].texCoord = glm::vec2(a, b);
//         }
//       }
//       
//       if (ImGui::Button("Mirror U")) {
//         for (size_t i = 0; i < elemCount; ++i) {
//           verts[idx[i]].texCoord.x = 1.0f - verts[idx[i]].texCoord.x;
//         }
//       }
//       
//       ImGui::SameLine();
//       
//       if (ImGui::Button("Mirror W")) {
//         for (size_t i = 0; i < elemCount; ++i) {
//           verts[idx[i]].texCoord.y = 1.0f - verts[idx[i]].texCoord.y;
//         }
//       }
//       
//       ImGui::Separator();
//       
//       static float oldScale = 1.0f;
//       static float newScale = 1.0f;
//       
//       ImGui::Text("Scale:");
//       ImGui::SameLine();
//       ImGui::DragFloat("###scale", &newScale, 0.001f);
//       
//       float diff = newScale - oldScale;
//       if (!f_eq(diff, 0.0f)) {
//         
//         for (size_t i = 0; i < elemCount; ++i) {
//           verts[idx[i]].texCoord *= (diff + 1.0f);
//         }
//         
//         oldScale = newScale;
//       }
//       
//       static glm::vec2 oldUV = glm::vec2(0.0f, 0.0f);
//       static glm::vec2 newUV = glm::vec2(0.0f, 0.0f);
//       
//       ImGui::Text("Shift:");
//       ImGui::SameLine();
//       ImGui::DragFloat2("###shift", (float*)&newUV, 0.001f);
//       
//       glm::vec2 diffUV = newUV - oldUV;
//       if (!f_eq(diffUV.x, 0.0f) || !f_eq(diffUV.y, 0.0f)) {
// //         std::cout << "Moving" << "\n";
//         
//         for (size_t i = 0; i < elemCount; ++i) {
//           verts[idx[i]].texCoord += diffUV;
//         }
//         
//         oldUV = newUV;
//       }
//       
//       for (size_t i = 0; i < elemCount; ++i) {
//         if (ImGui::CollapsingHeader(("Vertex "+std::to_string(idx[i])).c_str())) {
//           ImGui::Text("Pos: (%.4f, %.4f, %.4f)", verts[idx[i]].pos.x, verts[idx[i]].pos.y, verts[idx[i]].pos.z);
//           if (ImGui::IsItemHovered()) {
//             ImGui::BeginTooltip();
//             ImGui::Text("You cannot change it here.");
//             ImGui::EndTooltip();
//           }
//           ImGui::Text("TexCoord:");
//           ImGui::SameLine();
//           ImGui::DragFloat2((("###uv"+std::to_string(idx[i])).c_str()), (float*)&verts[idx[i]].texCoord, 0.001f);
//           
//           static glm::vec2 oldUV2 = glm::vec2(0.0f, 0.0f);
//           static glm::vec2 newUV2 = glm::vec2(0.0f, 0.0f);
//           
//           ImGui::Text("Shift:");
//           ImGui::SameLine();
//           ImGui::DragFloat2((("###shift"+std::to_string(idx[i])).c_str()), (float*)&newUV2, 0.001f);
//           
//           glm::vec2 diffUV = newUV2 - oldUV2;
//           if (!f_eq(diffUV.x, 0.0f) || !f_eq(diffUV.y, 0.0f)) {
//             verts[idx[i]].texCoord += diffUV;
//             
//             oldUV2 = newUV2;
//           }
//         }
//       }
//     }
//     
//     ImGui::Text("Current state: %s", currentState->getName().c_str());
//     
// //     ImGui::Text("Texture scale:");
// //     ImGui::SameLine();
// //     ImGui::DragFloat3("scale", &scale, 0.1f);
//   }
// }
// 
// void GraphicComponentIndexes::drawBoundingShape(CollisionComponent* shape, const simd::vec4 &color) const {
//   size_t count = elemCount;
//   size_t indexOffset = offset;
// //   simd::vec4 color1 = color;
// //   color1.w = 0.5f;
//   
//   if (shape && shape->getType() != PLANE) {
//     std::cout << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" << "\n";
//     exit(-1);
//   }
// 
//   Global::render()->addDebugDraw([color, shape, count, indexOffset] (yavf::GraphicTask* task) {
//     struct PushConst {
//       simd::mat4 mat;
//       simd::vec4 color;
//       glm::vec3 normal;
//     };
//     
//     PushConst consts{
//       simd::mat4(1.0f),
//       color,
//       //((Polygon*)shape)->normal
//       ((Polygon*)shape->getCollidable())->normal
//     };
//     
//     task->setConsts(0, sizeof(PushConst), &consts);
//     task->drawIndexed(count, 1, indexOffset, 0, 0);
//   });
// }

CLASS_TYPE_DEFINE_WITH_NAME(Light, "Light")

void Light::setOptimizer(LightOptimizer* optimizer) {
  Light::optimizer = optimizer;
}

Light::Light(const float &radius, const float& cutoff, const glm::vec3 &color) {
  this->color = color;
  this->radius = radius;
  this->cutoff = cutoff;
}

Light::~Light() {}

void Light::update(const size_t &time) {
  (void)time;
  
//   const LightData data{
//     trans->pos(),
//     radius,
//     color,
//     cutoff
//   };
//   
//   Global::render()->addLight(data);
  
  // здесь мы должны в будущем передать вещи в оптимизеры
  // сюда мы приходим если свет попадает в фрустум
}

void Light::init(void* userData) {
  (void)userData;
  
  trans = getEntity()->get<TransformComponent>().get();
  
  if (trans == nullptr) {
    Global::console()->printE("Entity " + std::to_string(getEntity()->getId()) + " does not have TransformComponent!");
    throw std::runtime_error("Entity " + std::to_string(getEntity()->getId()) + " does not have TransformComponent!");
  }
  
  float arr[4];
  trans->pos().store(arr);
  
  const LightOptimizer::LightRegisterInfo info{
    glm::vec3(arr[0], arr[1], arr[2]),
    radius,
    color,
    cutoff,
    
    trans->transformIndex
  };
  optimizer->add(info);
}

LightOptimizer* Light::optimizer = nullptr;

CLASS_TYPE_DEFINE_WITH_NAME(UserInputComponent, "UserInputComponent")

UserInputComponent::UserInputComponent() : horisontalAngleSum(0.0f), verticalAngleSum(0.0f), states(nullptr), trans(nullptr), rotation(0.0f) {}
UserInputComponent::~UserInputComponent() {}

void UserInputComponent::update(const size_t &time) { (void)time; }

void UserInputComponent::init(void* userData) {
  (void)userData;
  
//   std::cout << "UserInputComponent::init" << "\n";
  
//   states = getEntity()->get<StateController>().get();
//   
//   if (states == nullptr) {
//     Global::console()->printE("Entity " + std::to_string(getEntity()->getId()) + " does not have StateController!");
//     throw std::runtime_error("Entity " + std::to_string(getEntity()->getId()) + " does not have StateController!");
//   }
  
//   input = getEntity()->get<InputComponent>().get();
//   
//   if (input == nullptr) {
//     Global::console()->printE("Entity " + std::to_string(getEntity()->getId()) + " does not have InputComponent!");
//     throw std::runtime_error("Entity " + std::to_string(getEntity()->getId()) + " does not have InputComponent!");
//   }
  
  trans = getEntity()->get<TransformComponent>().get();
  
  if (trans == nullptr) {
    Global::console()->printE("Entity " + std::to_string(getEntity()->getId()) + " does not have TransformComponent!");
    throw std::runtime_error("Entity " + std::to_string(getEntity()->getId()) + " does not have TransformComponent!");
  }
  
  const simd::vec4 &startingDir = trans->rot();
  cartesianToSpherical(startingDir, horisontalAngleSum, verticalAngleSum);
    
  horisontalAngleSum = glm::degrees(horisontalAngleSum);
  verticalAngleSum = glm::degrees(verticalAngleSum);
}

void UserInputComponent::mouseMove(const float &horisontalAngle, const float &verticalAngle) {
  static bool first = true;
  if (first) {
//     std::cout << "horisontalAngleSum " << horisontalAngleSum << " verticalAngleSum " << verticalAngleSum << "\n";
//     cartesianToSpherical(startingDir, horisontalAngleSum, verticalAngleSum);
    
//     horisontalAngleSum = glm::degrees(horisontalAngleSum);
//     verticalAngleSum = glm::degrees(verticalAngleSum);
    
    first = false;
  }
  
  horisontalAngleSum += horisontalAngle;
  verticalAngleSum += verticalAngle;
  
  if (horisontalAngleSum > 360.0f) horisontalAngleSum = horisontalAngleSum - 360.0f;
  if (horisontalAngleSum < 0.0f)   horisontalAngleSum = horisontalAngleSum + 360.0f;
  
  if (verticalAngleSum > 180.0f - 0.01f) verticalAngleSum = 180.0f - 0.01f;
  if (verticalAngleSum < 0.0f   + 0.01f) verticalAngleSum = 0.0f   + 0.01f;
  
  float x = -glm::radians(horisontalAngleSum); // азимут
  float y = -glm::radians(verticalAngleSum); // зенит
  
  rotation.y = x; // yaw 
  rotation.x = y; // pitch
  
  simd::vec4 front;
  front.x = glm::sin(y) * glm::cos(x); // r = 1.0f
  front.y = -glm::cos(y);
  front.z = glm::sin(y) * glm::sin(x);
  front.w = 0.0f;
  
  front = simd::normalize(front);
  
  simd::vec4 right;
  right.x = glm::cos(x - glm::half_pi<float>());
  right.y = 0.0f;
  right.z = glm::sin(x - glm::half_pi<float>());
  right.w = 0.0f;
  
  right = simd::normalize(right);
  
  simd::vec4 up = simd::normalize(simd::cross(right, front));
  
  // transform я должен получать из вне
  const simd::mat4 &transform = Global::physics()->getOrientation();
  front = transform * front;
  right = transform * right;
  up = transform * up;
  this->front() = front;
  this->InputComponent::right() = right;
  this->up() = up;
  playerRotation = rotation;
  {
    Global g;
    g.setPlayerRot(glm::vec4(rotation, 0.0f));
  }
  
  trans->rot() = front;
  
  setMovement(0.0f, 0.0f, 0.0f);
//   input->forwardMove = 0.0f;
//   input->sideMove = 0.0f;
//   input->upMove = 0.0f;

//   input->updateInput();
}

void UserInputComponent::forward() {
//   input->forwardMove = 1.0f;
//   input->updateInput();
  movementData().z = 1.0f;
}

void UserInputComponent::backward() {
//   input->forwardMove = -1.0f;
//   input->updateInput();
  movementData().z = -1.0f;
}

void UserInputComponent::left() {
//   input->sideMove = -1.0f;
//   input->updateInput();
  movementData().x = -1.0f;
}

void UserInputComponent::right() {
//   input->sideMove = 1.0f;
//   input->updateInput();
  movementData().x = 1.0f;
}

void UserInputComponent::jump() {
//   input->upMove = 1.0f;
//   input->updateInput();
  movementData().y = 1.0f;
}

CLASS_TYPE_DEFINE_WITH_NAME(CameraComponent, "CameraComponent")

//#define PRINT_VEC(name, vec) std::cout << name << " (" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ")" << "\n";

void CameraComponent::update(const size_t &time) {
  (void)time;
  
  static const simd::mat4 toVulkanSpace = simd::mat4(1.0f, 0.0f, 0.0f, 0.0f,
                                                     0.0f,-1.0f, 0.0f, 0.0f,
                                                     0.0f, 0.0f, 1.0f, 0.0f,
                                                     0.0f, 0.0f, 0.0f, 1.0f);
  
  //glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
  const simd::vec4 pos = trans->pos();
  //const glm::vec3 pos = trans->pos;
  //const simd::vec4 &changePos = trans->getPos();
  //TransformComponent::container->at(trans->transformIndex).pos = simd::vec4(trans->pos, 1.0f);
  const simd::mat4 view = toVulkanSpace * simd::lookAt(pos, pos + input->front(), simd::vec4(0.0f, 1.0f, 0.0f, 0.0f)); // input->upVec
  //view = glm::lookAt(pos, pos + input->frontVec, glm::vec3(0.0f, 1.0f, 0.0f)); // input->upVec
  
//   PRINT_VEC("view ", view[0])
//   PRINT_VEC("     ", view[1])
//   PRINT_VEC("     ", view[2])
//   PRINT_VEC("     ", view[3])
   
  Global::render()->setView(view);
  Global::render()->setCameraPos(pos);
  Global::render()->setCameraDir(input->front());
  
  Global::render()->updateCamera();
}

void CameraComponent::init(void* userData) {
  (void)userData;
  
//   std::cout << "CameraComponent::init" << "\n";
  
  trans = getEntity()->get<TransformComponent>().get();
  
  if (trans == nullptr) {
    Global::console()->printE("Entity " + std::to_string(getEntity()->getId()) + " does not have TransformComponent!");
    throw std::runtime_error("Entity " + std::to_string(getEntity()->getId()) + " does not have TransformComponent!");
  }
  
  input = getEntity()->get<InputComponent>().get();
  if (input == nullptr) input = getEntity()->get<UserInputComponent>().get();
  
  if (input == nullptr) {
    Global::console()->printE("Entity " + std::to_string(getEntity()->getId()) + " does not have InputComponent!");
    throw std::runtime_error("Entity " + std::to_string(getEntity()->getId()) + " does not have InputComponent!");
  }
}

// AiComponent::AiComponent(const size_t &type, const size_t &treeInterval) {
//   this->type = type;
//   this->treeInterval = treeInterval;
// }
// 
// AiComponent::~AiComponent() {}
// 
// void AiComponent::uiDraw() {
//   if (ImGui::CollapsingHeader("AI")) {
//     ImGui::Text("Intellect:");
//     ImGui::SameLine();
//     int tmp = intellect;
//     ImGui::DragInt("intel", &tmp);
//     intellect = tmp;
//     
//     int treshold = this->treshold;
//     ImGui::Text("Target treshold:");
//     ImGui::SameLine();
//     ImGui::InputInt("treshold", &treshold);
//     this->treshold = treshold;
//   }
// }
// 
// // bool AiComponent::needUpdate() const {
// //   return needUpdateB;
// // }
// // 
// // void AiComponent::setUpdateNeeded() {
// //   needUpdateB = true;
// // }
// 
// bool AiComponent::hasPath() const {
//   return path.isValid();
// }
// 
// void AiComponent::clearPath() {
//   path.clear();
// }
// 
// void AiComponent::findPath(const Type &type, const size_t &intellect) {
//   path.clear();
//   if (target == nullptr) return;
//   const size_t &objVert    = collisionComponent->getGraphVertexIndex();
//   const size_t &targetVert = target->get<CollisionComponent>()->getGraphVertexIndex();
//   
// //   Global::ai()->getPath(type, objVert, targetVert, path, intellect);
//   Global::ai()->getPath(type.getType(), objVert, targetVert, trans->pos, path, intellect);
// //   if (!path.empty()) path[0].point = trans->pos;
//   pathIndex = 1;
// }
// 
// void AiComponent::findPath() {
//   path.clear();
//   if (target == nullptr) return;
//   const size_t &objVert    = collisionComponent->getGraphVertexIndex();
//   const size_t &targetVert = target->get<CollisionComponent>()->getGraphVertexIndex();
//   
//   Global::ai()->getPath(type, objVert, targetVert, trans->pos, path, intellect);
//   //if (!path.empty()) path[0].point = trans->pos;
//   pathIndex = 1;
// }
// 
// size_t AiComponent::pathSize() const {
//   return path.waypointsSize();
// }
// 
// // Path & AiComponent::get(const size_t &index) {
// //   return path.getPathSegments()[index];
// // }
// 
// const Path & AiComponent::get(const size_t &index) const {
//   return path.getPathSegments()[index];
// }
// 
// // void AiComponent::followPath() {
// //   currentGoal.valid = false;
// // }
// 
// bool AiComponent::followPath() {
//   if (!path.isValid()) return false;
//   if (pathIndex >= pathSize()) return false;
//   
//   if (!path.isOnPath(collisionComponent->getGraphVertexIndex())) return false;
// 
//   glm::vec3 closest = path.getPathSegments()[pathIndex].point + path.getPathSegments()[pathIndex].lineDir * trans->scale().x; 
//   glm::vec3 pathPoint = projectPointOnPlane(-PhysicsEngine::getGravityNorm(), trans->pos(), closest);
//   
//   input->forwardMove = ACCELERATION;
//   // если здесь возвращать bool?
//   // пока что я думаю надо остановиться на этом
//   input->followPath(lastTime, path, pathIndex);
//   
//   // pathIndex < pathSize() &&
//   if (glm::distance2(trans->pos, pathPoint) <= radius) ++pathIndex;
//   
//   return true;
// }
// 
// bool AiComponent::pathPassed() const {
//   if (!path.isValid()) return false;
//   
//   if (pathIndex >= pathSize()) return true;
//   
//   return false;
// }
// 
// void AiComponent::setGoal(const glm::vec3 &dir, const float &dist, const size_t &walkTime) {
//   currentGoal.dir = dir;
//   currentGoal.dist = dist;
//   currentGoal.walkTime = walkTime;
//   currentGoal.valid = true;
// }
// 
// void AiComponent::setGoal(const glm::vec3 &point) {
//   //Actor* actor = (Actor*)getEntity();
//   currentGoal.dir = glm::normalize(point - trans->pos);
//   currentGoal.dist = glm::distance(point, trans->pos);
//   currentGoal.walkTime = UINT64_MAX;
//   currentGoal.valid = true;
// }
// 
// void AiComponent::targetAsGoal() {
//   auto targetTransform = target->get<TransformComponent>().get();
//   
//   currentGoal.dir = glm::normalize(targetTransform->pos - trans->pos);
//   currentGoal.dist = glm::distance(targetTransform->pos, trans->pos);
//   currentGoal.walkTime = UINT64_MAX;
//   currentGoal.valid = true;
// }
// 
// bool AiComponent::hasGoal() const {
//   return currentGoal.valid;
// }
// 
// bool AiComponent::isGoalFulfilled() const {
//   //Actor* actor = (Actor*)getEntity();
//   return (glm::distance2(runningTreeNodeStart, trans->pos) > currentGoal.dist * currentGoal.dist) || (runningTreeNodeAcum > currentGoal.walkTime);
// }
// 
// Goal & AiComponent::getGoal() {
//   return currentGoal;
// }
// 
// const Goal & AiComponent::getGoal() const {
//   return currentGoal;
// }
// 
// void AiComponent::moveTowardsGoal() {
//   input->frontVec = currentGoal.dir;
//   trans->rot = input->frontVec;
//   input->forwardMove = ACCELERATION;
// }
// 
// // size_t AiComponent::getTargetId() const {
// //   return targetId;
// // }
// // 
// // GameObject* AiComponent::getTarget() const {
// //   return GameObjectContainer::getObject(targetId);
// // }
// 
// YACS::Entity* AiComponent::getTarget() const {
//   return target;
// }
// 
// bool AiComponent::hasTarget() const {
//   return target != nullptr;
// }
// 
// float AiComponent::getDistanceSqToTarget(const bool &accurate) const {
//   //Actor* actor = (Actor*)getEntity();
//   if (!accurate) return glm::distance2(target->get<TransformComponent>()->pos, trans->pos);
//   
//   return glm::distance2(target->get<CollisionComponent>()->closestPoint(trans->pos), trans->pos);
// }
// 
// // void AiComponent::setTarget(const size_t& id) {
// //   targetId = id;
// // }
// 
// void AiComponent::setTarget(YACS::Entity* ent) {
//   target = ent;
// }
// 
// // void AiComponent::moveTowardsTarget() {
// //   //Actor* actor = getEntity();
// //   input->frontVec = glm::normalize(target->pos() - actor->pos());
// //   actor->rot() = input->frontVec;
// //   input->forwardMove = ACCELERATION;
// // //   getObjPtr()->setRot(frontVec);
// // //   forwardMove = ACCELERATION;
// // }
// 
// // size_t AiComponent::getBlockingObjectId() const {
// //   return getObjPtr()->getBlockingId();
// // }
// 
// // GameObject* AiComponent::getBlockingObject() const {
// //   return GameObjectContainer::getObject(getObjPtr()->getBlockingId());
// // }
// 
// YACS::Entity* AiComponent::getBlockingObject() const {
//   return collisionComponent->blocking;
// }
// 
// glm::vec3 AiComponent::getBlockingDir() const {
//   return collisionComponent->blockingDir;
// }
// 
// bool AiComponent::hasBlockingObject() const {
//   return collisionComponent->blocking != nullptr;
// }
// 
// // size_t AiComponent::getGroundId() const {
// //   return getObjPtr()->getGroundId();
// // }
// // 
// // GameObject* AiComponent::getGround() const {
// //   return GameObjectContainer::getObject(getObjPtr()->getGroundId());
// // }
// // 
// // size_t AiComponent::getLastGroundId() const {
// //   return getObjPtr()->getLastGroundId();
// // }
// // 
// // GameObject* AiComponent::getLastGround() const {
// //   return GameObjectContainer::getObject(getObjPtr()->getLastGroundId());
// // }
// // 
// // size_t AiComponent::getTargetGroundId() const {
// //   return getTarget()->getGroundId();
// // }
// // 
// // GameObject* AiComponent::getTargetGround() const {
// //   return GameObjectContainer::getObject(getTarget()->getGroundId());
// // }
// // 
// // size_t AiComponent::getLastTargetGroundId() const {
// //   return getTarget()->getLastGroundId();
// // }
// // 
// // GameObject* AiComponent::getLastTargetGround() const {
// //   return GameObjectContainer::getObject(getTarget()->getLastGroundId());
// // }
// 
// YACS::Entity* AiComponent::getGround() const {
//   return collisionComponent->ground;
// }
// 
// YACS::Entity* AiComponent::getLastGround() const {
//   return collisionComponent->lastGround;
// }
// 
// YACS::Entity* AiComponent::getTargetGround() const {
//   return target->get<CollisionComponent>()->ground;
// }
// 
// YACS::Entity* AiComponent::getLastTargetGround() const {
//   return target->get<CollisionComponent>()->lastGround;
// }
// 
// bool AiComponent::isNearEdge(const float &rad, NearEdge &data) const {
//   //YACS::Entity* obj = getEntity();
//   const vertex_t &vert = Global::ai()->getGraph()->vertex(collisionComponent->getGraphVertexIndex());
//   
//   //NearEdge minData;
//   float minDist = 1000.0f;
//   size_t index = UINT64_MAX;
//   for (size_t i = 0; i < vert.degree(); ++i) {
//     const edge_t &edge = vert[i];
//     
//     if (edge.isFake()) continue;
//     
//     float dist = glm::distance2(edge.getSegment()->closestPoint(trans->pos), trans->pos);
//     if (dist < rad && minDist > dist) {
//       minDist = dist;
//       index = i;
//     }
//   }
//   
//   if (index == UINT64_MAX) return false;
//   
//   const edge_t &edge = vert[index];
//   data.edge = edge;
//   data.dist = minDist;
//   data.crossDir = edge.getSegment()->closestPoint(vert.getVertex()) - vert.getVertex();
//   
//   return true;
// }
// 
// bool AiComponent::hasEdge() const {
// //   GameObject* ground = getGround();
// //   if (ground == nullptr) ground = getLastGround();
// //   if (ground == nullptr) return false;
// //   
// //   GameObject* targetGround = getTargetGround();
// //   if (targetGround == nullptr) targetGround = getLastTargetGround();
// //   if (targetGround == nullptr) return false;
//   
//   size_t selfVertex = collisionComponent->getGraphVertexIndex();
//   if (selfVertex == UINT64_MAX) return false;
//   size_t targetVertex = target->get<CollisionComponent>()->getGraphVertexIndex();
//   if (targetVertex == UINT64_MAX) return false;
//   
//   return Global::ai()->getGraph()->hasEdge(selfVertex, targetVertex);
// }
// 
// edge_t AiComponent::getEdge() const {
//   edge_t edge;
//   
// //   GameObject* ground = getGround();
// //   if (ground == nullptr) ground = getLastGround();
// //   if (ground == nullptr) return edge;
// //   
// //   GameObject* targetGround = getTargetGround();
// //   if (targetGround == nullptr) targetGround = getLastTargetGround();
// //   if (targetGround == nullptr) return edge;
//   
//   size_t selfVertex = collisionComponent->getGraphVertexIndex();
//   if (selfVertex == UINT64_MAX) return edge;
//   size_t targetVertex = target->get<CollisionComponent>()->getGraphVertexIndex();
//   if (targetVertex == UINT64_MAX) return edge;
//   
//   Global::ai()->getGraph()->hasEdge(selfVertex, targetVertex, edge);
//   
//   return edge;
// }
// 
// void AiComponent::setPathCheckRadius(const float &radius) {
//   this->radius = radius;
// }
// 
// void AiComponent::toggleForceUpdateEveryFrame() {
//   forceUpdateEveryFrame = !forceUpdateEveryFrame;
// }
// 
// void AiComponent::forceUpdate() {
//   forceUpdateB = true;
// }
// 
// void AiComponent::initAi() {
//   input = getEntity()->get<InputComponent>().get();
//   
//   if (input == nullptr) {
//     Global::console()->printE("Entity " + std::to_string(getEntity()->getId()) + " does not have InputComponent!");
//     throw std::runtime_error("Entity " + std::to_string(getEntity()->getId()) + " does not have InputComponent!");
//   }
//   
//   trans = getEntity()->get<TransformComponent>().get();
//   
//   if (trans == nullptr) {
//     Global::console()->printE("Entity " + std::to_string(getEntity()->getId()) + " does not have TransformComponent!");
//     throw std::runtime_error("Entity " + std::to_string(getEntity()->getId()) + " does not have TransformComponent!");
//   }
//   
//   stateController = getEntity()->get<StateController>().get();
//   
//   if (stateController == nullptr) {
//     Global::console()->printE("Entity " + std::to_string(getEntity()->getId()) + " does not have StateController!");
//     throw std::runtime_error("Entity " + std::to_string(getEntity()->getId()) + " does not have StateController!");
//   }
//   
//   collisionComponent = getEntity()->get<CollisionComponent>().get();
//   
//   if (collisionComponent == nullptr) {
//     Global::console()->printE("Entity " + std::to_string(getEntity()->getId()) + " does not have CollisionComponent!");
//     throw std::runtime_error("Entity " + std::to_string(getEntity()->getId()) + " does not have CollisionComponent!");
//   }
// }
// 
// CLASS_TYPE_DEFINE_WITH_NAME(LoneAi, "LoneAi")
// 
// LoneAi::LoneAi(TinyBehavior::BehaviorTree* tree, const size_t &type, const size_t &treeInterval) : AiComponent(type, treeInterval) {
//   this->tree = tree;
// }
// 
// LoneAi::~LoneAi() {}
// 
// void LoneAi::update(const size_t &time) {
//   input->forwardMove = 0.0f;
//   input->sideMove = 0.0f;
//   input->upMove = 0.0f;
//   
//   //YACS::Entity* obj = getEntity();
//   lastTime = time;
//   timeAc += time;
//   if (forceUpdateEveryFrame || forceUpdateB || timeAc > treeInterval) {
//     timeAc = 0;
//     if (forceUpdateB) runningTreeNodeAcum = 0;
//     runningTreeNodeStart = trans->pos;
//     tree->update(this);
//     currentTreeNode = tree->getRunning();
//     forceUpdateB = false;
//   }
//   
//   if (currentTreeNode != nullptr) {
//     TinyBehavior::Node::Status status = currentTreeNode->update(this);
//     if (status != TinyBehavior::Node::Status::Running) {
//       forceUpdateB = true;
//     }
//   }
//   
//   runningTreeNodeAcum += time;
// }
// 
// void LoneAi::init(void* userData) {
//   (void)userData;
//   initAi();
// }
// 
// void LoneAi::setIntellect(const size_t &intellect) {
//   this->intellect = intellect;
// }
