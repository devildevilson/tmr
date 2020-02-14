#ifndef GRAPHIC_COMPONENTS_H
#define GRAPHIC_COMPONENTS_H

#include "Editable.h"

#include "Utility.h"
#include "ArrayInterface.h"
#include "RenderStructures.h"
#include "PhysicsTemporary.h"

// короче RotationData нужно обновлять где-то здесь
// иначе получается какая то херня, для этого, конечно,
// нужно бы как нибудь это использовать
// пока пусть это полежит на затворках

class GraphicUpdater {
public:
  virtual ~GraphicUpdater() = default;

  virtual void update() = 0;
  virtual void drawBoundingShape(const simd::vec4 &color) const = 0;
};

class TransformComponent;
class MonsterGPUOptimizer;
class MonsterDebugOptimizer;
class GeometryGPUOptimizer;
class GeometryDebugOptimizer;
class LightOptimizer;

class GraphicComponent : public Editable, public GraphicUpdater {
public:
  static void setContainer(Container<simd::mat4>* matrices);
  static void setContainer(Container<RotationData>* rotationDatas);
  static void setContainer(Container<Texture>* textureContainer);
  
  // тут по идее мы тем же макаром добавляем оптимизер
  static void setOptimizer(MonsterGPUOptimizer* mon);
  static void setDebugOptimizer(MonsterDebugOptimizer* debugOptimizer);

//   GraphicComponent(const uint32_t &pipelineIndex);
  struct CreateInfo {
    Texture t;
    uint32_t transformIndex;
    float scale;
  };
  GraphicComponent(const CreateInfo &info);
  ~GraphicComponent();

  void update() override;
//  void update(const uint64_t &time = 0) override;
//  void init(void* userData) override;
  
  void uiDraw() override;
  
  void drawBoundingShape(const simd::vec4 &color) const override;
  
  // это нужно в основном для того чтобы не городить анимационный компонент для каждого энтити
  void setImage(const Image &t);
  void setTexture(const Texture &t);

  uint32_t getMatrixIndex() const;
  uint32_t getRotationDataIndex() const;
  uint32_t getTextureContainerIndex() const;

  Texture & getTexture();
  const Texture & getTexture() const;
protected:
  uint32_t matrixIndex;
  uint32_t rotationDataIndex;
  uint32_t textureContainerIndex;
//   uint32_t optimiserIndex = UINT32_MAX;
  
  //TransformComponent* trans;
  uint32_t transformIndex;
  
  // нужен иной скейл для графического компонента
  // наверное пригодится только одна переменная, какая? height? скорее всего этого достаточно
  float scale;
  
  // эти контейнеры будут использованы скорее всего только в одном потомке (тот что будет отвечать за отрисовку сложных объектов)
  // должны ли быть здесь матрицы или все же они должны быть в другом месте? (и данные о повороте тоже?)
  // суть в том что по логике это трансформа объекта, и по идее эти вещи должны быть там
  // проблема возникает в случае когда мне не нужна дополнительная позиция
  // городить тогда абстракцию? ладно оставлю пока здесь (хотя это и не очевидно)
  static Container<simd::mat4>* matrices;
  static Container<RotationData>* rotationDatas;
  static Container<Texture>* textureContainer;
  
  static MonsterGPUOptimizer* optimizer;
  static MonsterDebugOptimizer* debugOptimizer;
};

class GraphicComponentIndexes : public GraphicComponent {
public:
  static void setOptimizer(GeometryGPUOptimizer* geo);
  static void setDebugOptimizer(GeometryDebugOptimizer* debugOptimizer);

  struct CreateInfo {
    size_t offset;
    size_t elemCount;
    uint32_t faceIndex;

    Texture t;
    uint32_t transformIndex;
  };
  GraphicComponentIndexes(const CreateInfo &info);
  ~GraphicComponentIndexes();
  
  void update() override; //const uint64_t &time = 0
//  void init(void* userData) override;
  
  void uiDraw() override;
  
  void drawBoundingShape(const simd::vec4 &color) const override;
protected:
  // Buffer indicies;
  uint32_t faceIndex;
  size_t offset;
  size_t elemCount;
  
  static GeometryGPUOptimizer* optimizer;
  static GeometryDebugOptimizer* debugOptimizer;
};

class Light : public Editable, public GraphicUpdater {
public:
  static void setOptimizer(LightOptimizer* optimiser);

  struct CreateInfo {
    LightData data;
    uint32_t transformIndex;
  };
  Light(const CreateInfo &info);
  ~Light();

  void update() override;

  void uiDraw() override;

  void drawBoundingShape(const simd::vec4 &color) const override;
protected:
  LightData data;
  uint32_t transformIndex;

  static LightOptimizer* optimiser;
};

#endif
