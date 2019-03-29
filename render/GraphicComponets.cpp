#include "GraphicComponets.h"

#include "Components.h"

CLASS_TYPE_DEFINE_WITH_NAME(GraphicComponent, "GraphicComponent")

void GraphicComponent::setContainer(Container<glm::mat4>* matrices) {
  GraphicComponent::matrices = matrices;
}

void GraphicComponent::setContainer(Container<RotationData>* rotationDatas) {
  GraphicComponent::rotationDatas = rotationDatas;
}

void GraphicComponent::setContainer(Container<TextureData>* textureContainer) {
  GraphicComponent::textureContainer = textureContainer;
}

void GraphicComponent::setOptimizer(MonsterOptimizer* mon) {
  GraphicComponent::optimizer = mon;
}

void GraphicComponent::setDebugOptimizer(MonsterDebugOptimizer* debugOptimizer) {
  GraphicComponent::debugOptimizer = debugOptimizer;
}

// GraphicComponent::GraphicComponent(const uint32_t &pipelineIndex) {
//   this->pipelineIndex = pipelineIndex;
// }

GraphicComponent::GraphicComponent() {
  textureContainerIndex = textureContainer->insert({{UINT32_MAX, UINT32_MAX, UINT32_MAX}, 0.0f, 0.0f});
}

GraphicComponent::~GraphicComponent() {
  textureContainer->erase(textureContainerIndex);
  if (optimiserIndex != UINT32_MAX) {
    optimizer->remove(optimiserIndex);
    optimiserIndex = UINT32_MAX;
  }
}

void GraphicComponent::update(const uint64_t &time) {
  (void)time;
  
  optimizer->markAsVisible(optimiserIndex);
}

void GraphicComponent::init(void* userData) {
  (void)userData;
  auto trans = getEntity()->get<TransformComponent>();
  
  const MonsterOptimizer::GraphicsIndices info{
    trans->transformIndex,
    matrixIndex,           // для монстров это поди не нужно
    rotationDataIndex,     // и это
    textureContainerIndex
  };
  
  optimiserIndex = optimizer->add(info);
}

void GraphicComponent::uiDraw() {
  
}

void GraphicComponent::drawBoundingShape(const glm::vec4 &color) const {
  auto trans = getEntity()->get<TransformComponent>();
  
  debugOptimizer->setDebugColor(trans->transformIndex, color);
}

void GraphicComponent::setTexture(const Texture &t) {
  textureContainer->at(textureContainerIndex).t = t;
  textureContainer->at(textureContainerIndex).movementU = 0.0f;
  textureContainer->at(textureContainerIndex).movementV = 0.0f;
}

void GraphicComponent::setTexture(const TextureData &t) {
  textureContainer->at(textureContainerIndex) = t;
}

// void GraphicComponent::drawBoundingShape(CollisionComponent* shape, const glm::vec4 &color) const {
//   
// }

uint32_t GraphicComponent::getMatrixIndex() const {
  return matrixIndex;
}

uint32_t GraphicComponent::getRotationDataIndex() const {
  return rotationDataIndex;
}

uint32_t GraphicComponent::getTextureContainerIndex() const {
  return textureContainerIndex;
}

Container<glm::mat4>* GraphicComponent::matrices = nullptr;
Container<RotationData>* GraphicComponent::rotationDatas = nullptr;
Container<TextureData>* GraphicComponent::textureContainer = nullptr;
MonsterOptimizer* GraphicComponent::optimizer = nullptr;
MonsterDebugOptimizer* GraphicComponent::debugOptimizer = nullptr;

CLASS_TYPE_DEFINE_WITH_NAME(GraphicComponentIndexes, "GraphicComponentIndexes")

GraphicComponentIndexes::GraphicComponentIndexes(const size_t &offset, const size_t &elemCount, const uint32_t &faceIndex) {
  this->faceIndex = faceIndex;
  this->offset = offset;
  this->elemCount = elemCount;
  
//   textureContainerIndex = textureContainer->insert({UINT32_MAX, UINT32_MAX, UINT32_MAX});
}

GraphicComponentIndexes::~GraphicComponentIndexes() {
//   textureContainer->erase(textureContainerIndex);
  if (optimiserIndex != UINT32_MAX) {
    optimizer->remove(optimiserIndex);
    optimiserIndex = UINT32_MAX;
  }
}

void GraphicComponentIndexes::setOptimizer(GeometryOptimizer* geo) {
  GraphicComponentIndexes::optimizer = geo;
}

void GraphicComponentIndexes::setDebugOptimizer(GeometryDebugOptimizer* debugOptimizer) {
  GraphicComponentIndexes::debugOptimizer = debugOptimizer;
}

void GraphicComponentIndexes::update(const uint64_t &time) {
  (void)time;
  
  optimizer->markAsVisible(optimiserIndex);
}

void GraphicComponentIndexes::init(void* userData) {
  (void)userData;
//   auto trans = getEntity()->get<TransformComponent>();
  
  const GeometryOptimizer::GraphicsIndices info{
//     trans->transformIndex,
    matrixIndex,
    rotationDataIndex,
    textureContainerIndex,
    
    static_cast<uint32_t>(offset),
    static_cast<uint32_t>(elemCount),
    faceIndex
  };
  
  optimiserIndex = optimizer->add(info);
}

void GraphicComponentIndexes::uiDraw() {
  
}

void GraphicComponentIndexes::drawBoundingShape(const glm::vec4 &color) const {
  debugOptimizer->setDebugColor(optimiserIndex, color);
}

GeometryOptimizer* GraphicComponentIndexes::optimizer = nullptr;
GeometryDebugOptimizer* GraphicComponentIndexes::debugOptimizer = nullptr;
