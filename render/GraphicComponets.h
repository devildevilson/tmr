#ifndef GRAPHIC_COMPONENTS_H
#define GRAPHIC_COMPONENTS_H

#include "Optimizers.h"

#include "EntityComponentSystem.h"

#include "Editable.h"

// короче RotationData нужно обновлять где-то здесь
// иначе получается какая то херня, для этого, конечно,
// нужно бы как нибудь это использовать
// пока пусть это полежит на затворках

class GraphicComponent : public yacs::Component, /*public Controller,*/ public Editable {
public:
  CLASS_TYPE_DECLARE
  
  static void setContainer(Container<glm::mat4>* matrices);
  static void setContainer(Container<RotationData>* rotationDatas);
  static void setContainer(Container<TextureData>* textureContainer);
  
  // тут по идее мы тем же макаром добавляем оптимизер
  static void setOptimizer(MonsterOptimizer* mon);
  static void setDebugOptimizer(MonsterDebugOptimizer* debugOptimizer);

//   GraphicComponent(const uint32_t &pipelineIndex);
  GraphicComponent();
  virtual ~GraphicComponent();
  
  void update(const uint64_t &time = 0) override;
  void init(void* userData) override;
  
  void uiDraw() override;
  
  virtual void drawBoundingShape(const glm::vec4 &color) const;
  
  // это нужно в основном для того чтобы не городить анимационный компонент для каждого энтити
  void setTexture(const Texture &t);
  void setTexture(const TextureData &t);

  uint32_t getMatrixIndex() const;
  uint32_t getRotationDataIndex() const;
  uint32_t getTextureContainerIndex() const;
protected:
  uint32_t matrixIndex = UINT32_MAX;
  uint32_t rotationDataIndex = UINT32_MAX;
  uint32_t textureContainerIndex = UINT32_MAX;
  uint32_t optimiserIndex = UINT32_MAX;
  
  // эти контейнеры будут использованы скорее всего только в одном потомке (тот что будет отвечать за отрисовку сложных объектов)
  // должны ли быть здесь матрицы или все же они должны быть в другом месте? (и данные о повороте тоже?)
  // суть в том что по логике это трансформа объекта, и по идее эти вещи должны быть там
  // проблема возникает в случае когда мне не нужна дополнительная позиция
  // городить тогда абстракцию? ладно оставлю пока здесь (хотя это и не очевидно)
  static Container<glm::mat4>* matrices;
  static Container<RotationData>* rotationDatas;
  static Container<TextureData>* textureContainer;
  
  static MonsterOptimizer* optimizer;
  static MonsterDebugOptimizer* debugOptimizer;
};

class GraphicComponentIndexes : public GraphicComponent {
public:
  CLASS_TYPE_DECLARE

  static void setOptimizer(GeometryOptimizer* geo);
  static void setDebugOptimizer(GeometryDebugOptimizer* debugOptimizer);
  
  GraphicComponentIndexes(const size_t &offset, const size_t &elemCount, const uint32_t &faceIndex);
  virtual ~GraphicComponentIndexes();
  
  void update(const uint64_t &time = 0) override;
  void init(void* userData) override;
  
  void uiDraw() override;
  
  void drawBoundingShape(const glm::vec4 &color) const override;
protected:
  // Buffer indicies;
  uint32_t faceIndex;
  size_t offset;
  size_t elemCount;
  
  static GeometryOptimizer* optimizer;
  static GeometryDebugOptimizer* debugOptimizer;
};

#endif
