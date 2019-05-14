#include "DecalComponent.h"

#include "Globals.h"

#include "DecalOptimizer.h"
#include "GraphicComponets.h"
#include "Components.h"

CLASS_TYPE_DEFINE_WITH_NAME(DecalContainerComponent, "DecalContainerComponent")

void DecalContainerComponent::setOptimizer(DecalOptimizer* opt) {
  DecalContainerComponent::optimizer = opt;
}

DecalContainerComponent::DecalContainerComponent() : transform(nullptr), graphic(nullptr) {}
DecalContainerComponent::~DecalContainerComponent() {}

void DecalContainerComponent::update(const size_t &time) {
  (void)time;
  // обновляем оптимизер
  
  for (size_t i = 0; i < decals.size(); ++i) {
    const DecalOptimizer::Data data {
      transform == nullptr ? UINT32_MAX : transform->transformIndex,
      decals[i].textureContainerIndex,
      graphic->getMatrixIndex(),
      graphic->getRotationDataIndex(),
      decals[i].vertices
    };
    optimizer->add(data);
  }
}

void DecalContainerComponent::init(void* userData) {
  (void)userData;
  
  transform = getEntity()->get<TransformComponent>().get();
  // может ли не быть графики у таких энтити
  // вряд ли
  graphic = getEntity()->get<GraphicComponent>().get();
  if (graphic == nullptr) {
    Global::console()->printW("Could not create decal container without graphics");
    throw std::runtime_error("Could not create decal container without graphics");
  }
}

UniqueID DecalContainerComponent::addDecal(const std::vector<Vertex> &vertices, const uint32_t &faceIndex, const uint32_t &textureContainerIndex) {
  UniqueID id;
  decals.emplace_back(vertices, faceIndex, textureContainerIndex, id);
  return id;
}

void DecalContainerComponent::removeDecal(const UniqueID &id) {
  for (size_t i = 0; i < decals.size(); ++i) {
    if (id == decals[i].id) {
      std::swap(decals[i], decals.back());
      decals.pop_back();
      break;
    }
  }
}

DecalOptimizer* DecalContainerComponent::optimizer = nullptr;

CLASS_TYPE_DEFINE_WITH_NAME(DecalComponent, "DecalComponent")

void DecalComponent::setContainer(Container<TextureData>* textureContainer) {
  DecalComponent::textureContainer = textureContainer;
}
  
DecalComponent::DecalComponent() {
  textureIndex = textureContainer->insert({});
}

DecalComponent::DecalComponent(const TextureData &texture) {
  textureIndex = textureContainer->insert(texture);
}

DecalComponent::~DecalComponent() {
  textureContainer->erase(textureIndex);
}

void DecalComponent::update(const size_t &time) {
  (void)time;
  // ничего
}

void DecalComponent::init(void* userData) {
  (void)userData;
  // ничего
}

void DecalComponent::add(const ContainerData &data) {
  decals.push_back(data);
}

void DecalComponent::clear() {
  for (size_t i = 0; i < decals.size(); ++i) {
    decals[i].comp->removeDecal(decals[i].id);
  }
  
  decals.clear();
}

uint32_t DecalComponent::textureContainerIndex() const {
  return textureIndex;
}

void DecalComponent::setTexture(const TextureData &texture) {
  textureContainer->at(textureIndex) = texture;
}
