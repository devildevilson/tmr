#include "MovementComponent.h"

#include "Globals.h"

#include "InputComponent.h"
#include "TransformComponent.h"
#include "PhysicsComponent.h"

#include "EntityAI.h"
#include "AIComponent.h"
#include "PathFindingPhase.h"
#include "AISystem.h"
#include "global_components_indicies.h"

// это откровенно плохо, после того как добавятся еще компоненты это все полетит
// с другой стороны у меня наверное будет всего два компонента
EntityAI* getEntityAI(yacs::entity* ent) {
  EntityAI* ai = ent->at<AIComponent>(AI_COMPONENT_INDEX).get();
  if (ai == nullptr) throw std::runtime_error("Could not find AIComponent");
  return ai;
}

MovementComponent::MovementComponent(const CreateInfo &info) : ent(info.ent), pathfindingTime(0), currentPathSegment(0), pathFindType(info.pathFindType), foundPath{nullptr, {Type(), nullptr, nullptr}}, oldPath{nullptr, {Type(), nullptr, nullptr}} {}

path_state MovementComponent::findPath(const EntityAI* target) {
  const size_t currentTime = Global::mcsSinceEpoch();
  if (foundPath.path != nullptr && (currentTime - pathfindingTime) > HALF_SECOND) {
    oldPath.path = foundPath.path;
    oldPath.req = foundPath.req;
    foundPath.path = nullptr;
    foundPath.req.end = nullptr;
    foundPath.req.start = nullptr;
  }
  
  EntityAI* parent = getEntityAI(ent);
  
  if (target == nullptr) return path_state::not_found;
  if (target->vertex() == nullptr) return path_state::not_found;
  if (target->vertex() == parent->vertex()) return path_state::found;
  
  if (foundPath.path == nullptr) {
    if (foundPath.req.start != nullptr && foundPath.req.end != nullptr) {
      const auto &pathData = Global::ai()->pathfindingSystem()->getPath(foundPath.req);

      if (pathData.state == path_finding_state::delayed) return path_state::finding;
      else if (pathData.state == path_finding_state::has_path) {
        if (oldPath.path != nullptr) releaseOldPath();

        foundPath.path = pathData.path;

        currentPathSegment = 1;

        std::cout << "path finding success" << "\n";

        return path_state::found;
      } else if (pathData.state == path_finding_state::path_not_exist) {
        if ((currentTime - pathfindingTime) > QUARTER_SECOND) releaseCurrentPath();

        std::cout << "path finding failed" << "\n";

        return path_state::not_found;
      }
    }

    const FindRequest req{
      pathFindType,
      parent->vertex(),
      target->vertex()
    };
    Global::ai()->pathfindingSystem()->queueRequest(req);

    pathfindingTime = currentTime;
    foundPath.req = req;

    std::cout << "path finding running" << "\n";

    return path_state::finding;
  }

  std::cout << "path already fouded" << "\n";

  return path_state::found;
}

path_travel_state MovementComponent::travelPath() {
  if (foundPath.path == nullptr && oldPath.path == nullptr) return path_travel_state::path_not_exist;
  
  
  
//   if (target() == nullptr) return failure;
//   if (this->vertex() == target()->vertex()) {
//     releasePath();
//     return success;
//   }
  
  auto trans = ent->at<TransformComponent>(TRANSFORM_COMPONENT_INDEX);
  auto input = ent->at<InputComponent>(INPUT_COMPONENT_INDEX);

  RawPath* path = foundPath.path == nullptr ? oldPath.path : foundPath.path;
  const size_t lastIndex = path->funnelData().size()-1;

  if (lastIndex < currentPathSegment) {
    releasePaths();

    std::cout << "path moving success" << "\n";

    return path_travel_state::end_travel;
  }

  // нужно сделать проверку в правильном ли мы месте сейчас, как?

  const simd::vec4 nextPos = followPath(Global::ai()->getUpdateDelta(), path, currentPathSegment);
  input->setMovement(1.0f, 0.0f, 0.0f);
  //trans->rot() = simd::normalize(input->front());
  modEntityRotation();

  const simd::vec4 point = projectPointOnPlane(-PhysicsEngine::getGravityNorm(), trans->pos(), nextPos);
  const float tolerance = 0.4f;
  //if (simd::distance2(trans->pos(), point) < tolerance) ++currentPathSegment;
  currentPathSegment += uint32_t(simd::distance2(trans->pos(), point) < tolerance);

  PRINT_VAR("currentPathSegment", currentPathSegment)
  PRINT_VAR("lastIndex   ", lastIndex)

  std::cout << "path moving" << "\n";

  return path == foundPath.path ? path_travel_state::travel_path : path_travel_state::travel_old_path;
}

bool MovementComponent::pathExist() const {
  return foundPath.path != nullptr || oldPath.path != nullptr;
}

void MovementComponent::move(const simd::vec4 &dir) {
  //const simd::vec4 normdir = simd::normalize(dir);
  auto input = ent->at<InputComponent>(INPUT_COMPONENT_INDEX);
  input->front() = dir;
  input->setMovement(1.0f, 0.0f, 0.0f);
  modEntityRotation();
}

void MovementComponent::pursue(const EntityAI* target) {
  auto input = ent->at<InputComponent>(INPUT_COMPONENT_INDEX);
  seek(target->position());
  input->setMovement(1.0f, 0.0f, 0.0f);
  modEntityRotation();
}

void MovementComponent::flee(const EntityAI* target) {
  auto input = ent->at<InputComponent>(INPUT_COMPONENT_INDEX);
  flee(target->position());
  input->setMovement(1.0f, 0.0f, 0.0f);
  modEntityRotation();
}

// void MovementComponent::updateVertex() {
//   if (physics != nullptr) {
//     //objectIndex = physics->getIndexContainer().objectDataIndex;
//     
//     auto cont = physics->getGround();
//     if (cont != nullptr) {
//       auto data = reinterpret_cast<UserDataComponent*>(cont->userData);
//       if (data->aiComponent == nullptr) {
//         PRINT_VAR("entity id", data->entity->id())
//       }
//       
//       //vertex_t* vertex = const_cast<vertex_t*>(data->aiComponent->vertex()); // TODO: убрать const_cast
//       vertex_t* currentVertex = data->vertex;
//       if (vertex != currentVertex) {
//         currentVertex->addObject(this);
//         if (vertex != nullptr) vertex->removeObject(this);
//         else if (aiData.lastVertex != nullptr) aiData.lastVertex->removeObject(this);
//         
//         aiData.lastVertex = aiData.vertex != nullptr ? aiData.vertex : aiData.lastVertex;
//         aiData.vertex = vertex;
//       }
//     } else {
//       if (aiData.vertex != nullptr) {
//         aiData.lastVertex = aiData.vertex != nullptr ? aiData.vertex : aiData.lastVertex;
//         aiData.vertex = nullptr;
//       }
//     }
//   }
// }

simd::vec4 MovementComponent::predictPos(const size_t &predictionTime) const {
  auto trans = ent->at<TransformComponent>(TRANSFORM_COMPONENT_INDEX);
  auto physics = ent->at<PhysicsComponent>(PHYSICS_COMPONENT_INDEX);
  return trans->pos() + physics->getVelocity() * MCS_TO_SEC(predictionTime);
}

simd::vec4 MovementComponent::seek(const simd::vec4 &target) {
  auto input = ent->at<InputComponent>(INPUT_COMPONENT_INDEX);
  auto trans = ent->at<TransformComponent>(TRANSFORM_COMPONENT_INDEX);
  input->front() = target - trans->pos();
  return input->front();
}

simd::vec4 MovementComponent::flee(const simd::vec4 &target) {
  auto input = ent->at<InputComponent>(INPUT_COMPONENT_INDEX);
  auto trans = ent->at<TransformComponent>(TRANSFORM_COMPONENT_INDEX);
  input->front() = trans->pos() - target;
  return input->front();
}

// simd::vec4 getDirIndex(const simd::vec4 &in, uint32_t &index) {
//   float dir[4];
//   in.storeu(dir);
// 
//   index = glm::floatBitsToUint(dir[3]);
//   return simd::vec4(dir[0], dir[1], dir[2], 0.0f);
// }

simd::vec4 MovementComponent::followPath(const size_t &predictionTime, const RawPath* path, const size_t &currentPathSegmentIndex) {
  (void)predictionTime;
  // короч я допустил несколько ошибок в старом коде 
  // мне нужно наверное пересобрать код следования пути
  
//   const simd::vec4 &futurePos = predictPos(predictionTime);
  const FunnelPath* pathSeg = &path->funnelData()[currentPathSegmentIndex];
//   const FunnelPath* prevPathSeg = &path->funnelData()[currentPathSegmentIndex-1];
  
//   uint32_t currentIndex = 0, prevIndex = 0;
//   const simd::vec4 finalCurrentDir = getDirIndex(pathSeg->edgeDir, currentIndex);
//   const simd::vec4 finalPrevDir = getDirIndex(prevPathSeg->edgeDir, prevIndex);
  
  // для того чтобы отступ сделать нужно посчитать этот отступ для каждого
  // проблема в том как у нас фуннел алгоритм посчитан, он совершенно это не учитывает
  // лучше вот что сделать, тип поиска пути также должен содержать примерный отступ
  // этот отступ мы будем использовать для того чтобы правильно посчитать фуннел
  // а затем здесь все сведется просто к путешествию по фуннел точкам
  const simd::vec4 finalPathPoint = pathSeg->funnelPoint; 
  
  seek(finalPathPoint);
  return finalPathPoint;
}

void MovementComponent::stayOnPath(const size_t &predictionTime, const RawPath* path) {
  (void)predictionTime;
  (void)path;
}

void MovementComponent::modEntityRotation() {
  auto physics = ent->at<PhysicsComponent>(PHYSICS_COMPONENT_INDEX);
  auto trans = ent->at<TransformComponent>(TRANSFORM_COMPONENT_INDEX);
  const simd::vec4 velocity = physics->getVelocity();
  const float dot = simd::dot(velocity, velocity);
  if (dot > EPSILON) {
    trans->rot() = velocity / std::sqrt(dot);
  }
}

void MovementComponent::releaseCurrentPath() {
  if (foundPath.path == nullptr) return;
  
  Global::ai()->pathfindingSystem()->releasePath(foundPath.req);
  foundPath.path = nullptr;
  foundPath.req.end = nullptr;
  foundPath.req.start = nullptr;
}

void MovementComponent::releaseOldPath() {
  if (oldPath.path == nullptr) return;
  
  Global::ai()->pathfindingSystem()->releasePath(oldPath.req);
  oldPath.path = nullptr;
  oldPath.req.end = nullptr;
  oldPath.req.start = nullptr;
}

void MovementComponent::releasePaths() {
  releaseCurrentPath();
  releaseOldPath();
}
