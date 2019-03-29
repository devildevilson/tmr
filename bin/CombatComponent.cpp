#include "CombatComponent.h"

#include <unordered_set>
#include <glm/gtx/rotate_vector.hpp>
#include "Components.h"
#include "CollisionObject3.h"
#include "Globals.h"
#include "Utility.h"

void AttackState::setTransform(TransformComponent* trans) {
  this->trans = trans;
}

Slashing::Slashing(const std::string &name, 
                   const uint64_t &attackTime, 
                   const float &distance, 
                   const glm::vec3 &attackPlane, 
                   const float &angle, 
                   bool blocking, bool blockingMovement) : AttackState(name, attackTime, distance) {
  this->blocking = blocking;
  this->blockingMovement = blockingMovement;
  this->attackPlane = attackPlane;
  this->angle = angle;
}

Slashing::~Slashing() {}

// float sideOf(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &point, const glm::vec3 &normal) {
//   glm::vec3 vec = b - a;
//   vec = glm::cross(vec, normal);
//   return glm::dot(vec, point - a);
// }

void Slashing::update(const uint64_t &time) {
  // на основе предыдущей точки атаки, предыдущего направления, текущей точки, текущего направления
  // мне нужно создать Polygon
  // насколько здесь необходима нормализация?
  
  if (finished) return;
  
  if (resetB) {
    prevDir = glm::normalize(glm::rotate(trans->rot, -(angle / 2.0f), attackPlane));
    prevPos = trans->pos;
    resetB = false;
  }
  
  float num = float(time) / float(attackTime);
  float currentAngle = angle * num;
  accumAngle += currentAngle;
  glm::vec3 secondDir = glm::normalize(glm::rotate(trans->rot, accumAngle - (angle / 2.0f), attackPlane));
  
  glm::vec3 d1 = prevPos + prevDir*distance;
  
  Polygon p;
  p.normal = attackPlane;
  p.points.push_back(d1);
  p.points.push_back(prevPos);
  if (!vec_eq(prevPos, trans->pos)) {
    p.points.push_back(trans->pos);
    if (sideOf(prevPos, d1, trans->pos, attackPlane) > 0.0f) {
      std::swap(p.points[1], p.points[2]);
    }
  }
  p.points.push_back(p.points.back() + secondDir*distance);
  
  for (uint8_t i = 0; i < p.points.size(); ++i) {
    PRINT_VEC3("Point "+std::to_string(i), p.points[i])
  }
  
//   if (p.points.size() > 3 && sideOf(prevPos, d1, trans->pos, attackPlane) > 0.0f) {
//     std::swap(p.points[1], p.points[2]);
//   }
  
  // чекаем на пересечение этого Polygon с окружающим миром (readonly)
  p.computeCenter();
  Sphere s(p.barycenter, distance/2.0f);
  std::vector<CollisionComponent*> objs;
  std::unordered_set<CollisionComponent*> set;
  // octree->getObjCollideSphere(&s, objs);
  for (auto* obj : objs) {
    size_t size = set.size();
    set.insert(obj);
    if (size == set.size()) continue;
    
    if (obj->collide(&p)) {
      // attackFunction(trans->getEntity(), obj->getEntity());
    }
  }
  // запускаем функцию, которая различным образом будет воздействовать на разные объекты (не readonly)

  prevDir = secondDir;
  prevPos = trans->pos;
  
  if (accumAngle > angle) finished = true;
}

void Slashing::reset() {
  resetB = true;
  accumAngle = 0.0f;
}

bool Slashing::isFinished() const {
  return finished;
}

bool Slashing::isBlocking() const {
  return blocking;
}

bool Slashing::isBlockingMovement() const {
  return blockingMovement;
}

Combat::Combat() {}
Combat::~Combat() {}

void Combat::update(const uint64_t &time) {}

void Combat::init(void* userData) {}

void Combat::addChild(Controller* c) { (void)c; }

void Combat::changeState(const Type &type) {
  auto itr = states.find(type.getType());
  if (itr == states.end()) {
    current = nullptr;
    return;
  }
  
  current = itr->second;
}

void Combat::reset() {
  if (current != nullptr) current->reset();
}

bool Combat::isFinished() const {
  if (current != nullptr) return current->isFinished();
  
  return true;
}

bool Combat::isBlocking() const {
  if (current != nullptr) return current->isBlocking();
  
  return false;
}

bool Combat::isBlockingMovement() const {
  if (current != nullptr) return current->isBlockingMovement();
  
  return false;
}

void Combat::addAttack(const Type &type, AttackState* attack) {
  auto itr = states.find(type.getType());
  if (itr != states.end()) {
    Global::console()->printW("Entity " + std::to_string(getEntity()->getId()) + " already has got attack state " + attack->getName());
  }
  
  states[type.getType()] = attack;
}
