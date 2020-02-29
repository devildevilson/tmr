#include "TransformComponent.h"

#include "Globals.h"

//CLASS_TYPE_DEFINE_WITH_NAME(TransformComponent, "TransformComponent")

void TransformComponent::setContainer(Container<Transform>* container) {
  TransformComponent::container = container;
}

//void TransformComponent::setPrevContainer(Container<Transform>* prevContainer) {
//
//}

TransformComponent::TransformComponent() {
  transformIndex = container->insert({});
}

TransformComponent::TransformComponent(const simd::vec4 &pos, const simd::vec4 &rot, const simd::vec4 &scale) {
  // здесь должен быть доступ к массиву трансформ, где и будут собственно храниться позиции и прочее
  transformIndex = container->insert({pos, rot, scale});
}

TransformComponent::~TransformComponent() {
  //std::cout << "container " << container << "\n";
  container->erase(transformIndex);
}

//void TransformComponent::update(const size_t &time = 0) { (void)time; }
//void TransformComponent::init(void* userData) { (void)userData; }

void TransformComponent::uiDraw() {
  
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

const simd::vec4 & TransformComponent::pos() const {
  return container->at(transformIndex).pos;
}

const simd::vec4 & TransformComponent::rot() const {
  return container->at(transformIndex).rot;
}

const simd::vec4 & TransformComponent::scale() const {
  return container->at(transformIndex).scale;
}

simd::vec4 & TransformComponent::pos() {
  return container->at(transformIndex).pos;
}

simd::vec4 & TransformComponent::rot() {
  return container->at(transformIndex).rot;
}

simd::vec4 & TransformComponent::scale() {
  return container->at(transformIndex).scale;
}

uint32_t TransformComponent::index() const {
  return transformIndex;
}

Container<Transform>* TransformComponent::container = nullptr;
