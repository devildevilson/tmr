#include "GraphicComponets.h"

//#include "Components.h"
//#include "TransformComponent.h"

void GraphicComponent::setContainer(Container<simd::mat4>* matrices) {
  GraphicComponent::matrices = matrices;
}

void GraphicComponent::setContainer(Container<RotationData>* rotationDatas) {
  GraphicComponent::rotationDatas = rotationDatas;
}

void GraphicComponent::setContainer(Container<Texture>* textureContainer) {
  GraphicComponent::textureContainer = textureContainer;
}

// void GraphicComponent::setOptimizer(MonsterOptimizer* mon) {
//   GraphicComponent::optimizer = mon;
// }

void GraphicComponent::setOptimizer(MonsterGPUOptimizer* mon) {
  GraphicComponent::optimizer = mon;
}

void GraphicComponent::setDebugOptimizer(MonsterDebugOptimizer* debugOptimizer) {
  GraphicComponent::debugOptimizer = debugOptimizer;
}

// GraphicComponent::GraphicComponent(const uint32_t &pipelineIndex) {
//   this->pipelineIndex = pipelineIndex;
// }

GraphicComponent::GraphicComponent(const CreateInfo &info) : matrixIndex(UINT32_MAX), rotationDataIndex(UINT32_MAX), textureContainerIndex(UINT32_MAX), transformIndex(info.transformIndex) {
  textureContainerIndex = textureContainer->insert(info.t);
}

GraphicComponent::~GraphicComponent() {
  textureContainer->erase(textureContainerIndex);
//   if (optimiserIndex != UINT32_MAX) {
//     optimizer->remove(optimiserIndex);
//     optimiserIndex = UINT32_MAX;
//   }
}

void GraphicComponent::update() { //const uint64_t &time
  const MonsterGPUOptimizer::GraphicsIndices info{
    transformIndex,
    matrixIndex,           // для монстров это поди не нужно
    rotationDataIndex,     // и это
    textureContainerIndex
  };
  optimizer->add(info);
}

void GraphicComponent::uiDraw() {
  
}

void GraphicComponent::drawBoundingShape(const simd::vec4 &color) const {
//   auto trans = getEntity()->get<TransformComponent>();
  
  debugOptimizer->setDebugColor(transformIndex, color);
}

void GraphicComponent::setImage(const Image &t) {
  textureContainer->at(textureContainerIndex).image = t;
}

void GraphicComponent::setTexture(const Texture &t) {
  textureContainer->at(textureContainerIndex) = t;
}

uint32_t GraphicComponent::getMatrixIndex() const {
  return matrixIndex;
}

uint32_t GraphicComponent::getRotationDataIndex() const {
  return rotationDataIndex;
}

uint32_t GraphicComponent::getTextureContainerIndex() const {
  return textureContainerIndex;
}

Texture & GraphicComponent::getTexture() {
  return textureContainer->at(textureContainerIndex);
}

const Texture & GraphicComponent::getTexture() const {
  return textureContainer->at(textureContainerIndex);
}

Container<simd::mat4>* GraphicComponent::matrices = nullptr;
Container<RotationData>* GraphicComponent::rotationDatas = nullptr;
Container<Texture>* GraphicComponent::textureContainer = nullptr;
MonsterGPUOptimizer* GraphicComponent::optimizer = nullptr;
MonsterDebugOptimizer* GraphicComponent::debugOptimizer = nullptr;

GraphicComponentIndexes::GraphicComponentIndexes(const CreateInfo &info) : GraphicComponent({info.transformIndex}), faceIndex(info.faceIndex), offset(info.offset), elemCount(info.elemCount) {}
GraphicComponentIndexes::~GraphicComponentIndexes() {}

void GraphicComponentIndexes::setOptimizer(GeometryGPUOptimizer* geo) {
  GraphicComponentIndexes::optimizer = geo;
}

void GraphicComponentIndexes::setDebugOptimizer(GeometryDebugOptimizer* debugOptimizer) {
  GraphicComponentIndexes::debugOptimizer = debugOptimizer;
}

void GraphicComponentIndexes::update() {
  const GeometryGPUOptimizer::GraphicsIndices info{
    UINT32_MAX,
    matrixIndex,
    rotationDataIndex,
    textureContainerIndex,
    
    static_cast<uint32_t>(offset),
    static_cast<uint32_t>(elemCount),
    faceIndex,
    0
  };
  optimizer->add(info);
}

//void GraphicComponentIndexes::init(void* userData) {
//  (void)userData;
//}

void GraphicComponentIndexes::uiDraw() {
  
}

void GraphicComponentIndexes::drawBoundingShape(const simd::vec4 &color) const {
  debugOptimizer->setDebugColor(transformIndex, color);
}

GeometryGPUOptimizer* GraphicComponentIndexes::optimizer = nullptr;
GeometryDebugOptimizer* GraphicComponentIndexes::debugOptimizer = nullptr;

void Light::setOptimizer(LightOptimizer* optimiser) {
  Light::optimiser = optimiser;
}

Light::Light(const CreateInfo &info) : GraphicComponent({info.transformIndex}), data(info.data) {}
Light::~Light() {}

void Light::update() {
  optimiser->add({data, transformIndex});
}

void Light::uiDraw() {

}

void Light::drawBoundingShape(const simd::vec4 &color) const {

}

LightOptimizer* Light::optimiser = nullptr;