#include "AIInputComponent.h"

#include "PathFindingPhase.h"
#include "Graph.h"

AIInputComponent::AIInputComponent(const CreateInfo &info) : physics(info.physics), trans(info.trans) {}
AIInputComponent::~AIInputComponent() {}

simd::vec4 AIInputComponent::predictPos(const size_t &predictionTime) const {
  return trans->pos() + physics->getVelocity() * MCS_TO_SEC(predictionTime);
}

void AIInputComponent::seek(const simd::vec4 &target) {
  front() = target - trans->pos();
}

void AIInputComponent::flee(const simd::vec4 &target) {
  front() = trans->pos() - target;
}

simd::vec4 getDirIndex(const simd::vec4 &in, uint32_t &index) {
  float dir[4];
  in.storeu(dir);

  index = glm::floatBitsToUint(dir[3]);
  return simd::vec4(dir[0], dir[1], dir[2], 0.0f);
}

simd::vec4 AIInputComponent::followPath(const size_t &predictionTime, const RawPath* path, const size_t &currentPathSegmentIndex) {
  // предскажем нашу будущую позицию
  const simd::vec4 &futurePos = predictPos(predictionTime);
  const FunnelPath* pathSeg = &path->funnelData()[currentPathSegmentIndex];
//   const RawPathPiece* prevPathSeg = &path->data()[path->getPrevSegmentIndex(currentPathSegmentIndex)];
//   const RawPathPiece* prevPathSeg = &path->graphData()[path->getPrevSegmentIndex2(currentPathSegmentIndex)];
  const FunnelPath* prevPathSeg = &path->funnelData()[currentPathSegmentIndex-1];

  uint32_t currentIndex = 0;
  const simd::vec4 finalCurrentDir = getDirIndex(pathSeg->edgeDir, currentIndex);
  uint32_t prevIndex = 0;
  const simd::vec4 finalPrevDir = getDirIndex(prevPathSeg->edgeDir, prevIndex);

  float scale[4];
  trans->scale().storeu(scale);
  const float offset = path->graphData()[currentIndex].toNextVertex != nullptr ? std::min(path->graphData()[currentIndex].toNextVertex->getWidth()/2.0f, scale[0]) : 0.0f;
  //const float offset = scale[0];
//   const simd::vec4 &nextPoint = simd::vec4(pathSeg->funnelPoint) + simd::vec4(pathSeg->edgeDir) * offset;
//   const simd::vec4 &prevPoint = simd::vec4(prevPathSeg->funnelPoint) + simd::vec4(prevPathSeg->edgeDir) * offset;
  const simd::vec4 &nextPoint = simd::vec4(pathSeg->funnelPoint) + simd::vec4(finalCurrentDir) * offset;
  const simd::vec4 &prevPoint = simd::vec4(prevPathSeg->funnelPoint) + simd::vec4(finalPrevDir) * offset;

  // примерно так узнаем что мы двигаемся в правильную сторону
  const bool rightWay2 = simd::dot(physics->getVelocity(), nextPoint - prevPoint) > 0.0f;

  float dist;
  simd::vec4 closest;
  /*size_t ind = */path->getNearPathSegmentIndex(trans->pos(), closest, dist);
  float dist2;
  simd::vec4 closest2;
  /*size_t ind2 = */path->getNearPathSegmentIndex(futurePos, closest2, dist2);

//   const bool ret =

  const float tolerance2 = offset * offset;

//   if (dist2 < tolerance2 && rightWay2) return nextPoint;
//
//   if (dist < tolerance2 && rightWay2) {
//     front() = nextPoint - prevPoint;
//     return nextPoint;
//   }

  seek(nextPoint);

  return nextPoint;
}

void AIInputComponent::stayOnPath(const size_t &predictionTime, const RawPath* path) {
  (void)predictionTime;
  (void)path;
}

void AIInputComponent::setPhysicsComponent(PhysicsComponent* physics) {
  this->physics = physics;
}