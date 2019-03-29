#include "Physics.h"

PhysicsEngine::~PhysicsEngine() {}
  
glm::vec4 PhysicsEngine::getGravity() {
  return gravity;
}

glm::vec4 PhysicsEngine::getGravityNorm() {
  return gravityNorm;
}

float PhysicsEngine::getGravLenght() {
  return gravLength;
}

float PhysicsEngine::getGravLenght2() {
  return gravLength2;
}

// void PhysicsEngine::setGravity(const glm::vec4 &g) {
//   gravity = g;
//   gravityNorm = glm::normalize(g);
//   gravLength2 = glm::length(g);
//   gravLength = glm::sqrt(gravLength2);
//   orientation = glm::orientation(glm::vec3(gravityNorm.x, gravityNorm.y, gravityNorm.z), glm::vec3(0.0f, 1.0f, 0.0f));
// }

glm::vec4 PhysicsEngine::abscissa() {
  return orientation[0];
}

glm::vec4 PhysicsEngine::ordinate() {
  return orientation[1];
}

glm::vec4 PhysicsEngine::applicat() {
  return orientation[2];
}

glm::mat3 PhysicsEngine::getTransform() {
  return glm::mat3(orientation);
}

glm::mat4 PhysicsEngine::getOrientation() {
  return orientation;
}

uint32_t PhysicsEngine::getOverlappingDataSize() const {
  return overlappingDataSize;
}

uint32_t PhysicsEngine::getRayTracingSize() const {
  return rayTracingSize;
}

uint32_t PhysicsEngine::getFrustumTestSize() const {
  return frustumTestSize;
}

glm::vec4 PhysicsEngine::gravity;
glm::vec4 PhysicsEngine::gravityNorm;
float PhysicsEngine::gravLength2;
float PhysicsEngine::gravLength;
glm::mat4 PhysicsEngine::orientation;
