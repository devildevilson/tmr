#ifndef DECAL_COMPONENT_H
#define DECAL_COMPONENT_H

#include "EntityComponentSystem.h"
#include "UniqueID.h"
#include "RenderStructures.h"
#include "ArrayInterface.h"

class DecalOptimizer;
class TransformComponent;
class GraphicComponent;

// этот компонент мы создаем стенам, чтобы они могли содержать декалину
class DecalContainerComponent : public yacs::Component {
public:
  CLASS_TYPE_DECLARE
  
  struct DecalData {
    std::vector<Vertex> vertices;
    uint32_t faceIndex; // от этого скроее всего можно избавиться
    uint32_t textureContainerIndex;
    UniqueID id;
  };
  
  static void setOptimizer(DecalOptimizer* opt);
  
  DecalContainerComponent();
  ~DecalContainerComponent();
  
  void update(const size_t &time) override;
  void init(void* userData) override;
  
  //UniqueID addDecal(const size_t &offset, const size_t &elemCount, const uint32_t &faceIndex);
  UniqueID addDecal(const std::vector<Vertex> &vertices, const uint32_t &faceIndex, const uint32_t &textureContainerIndex);
  void removeDecal(const UniqueID &id);
private:
  TransformComponent* transform;
  GraphicComponent* graphic;
  
  // декалей может быть несколько
  // они должны перемещаться вслед за объектом
  // нам нужен декал оптимизер
  // мы должны как то уметь декали удалять
  
  // здесь будет очень небольшое количество данных
  // но обойтись без выделения памяти тут не выйдет верно?
  // эти операции будут происходить не часто
  // так что может быть и нормально
  std::vector<DecalData> decals;
  
  // оптимизер, в который мы будем складывать декали
  static DecalOptimizer* optimizer;
};

// этот компонент мы даем собственно декали
// мы должны хранить указатели на то куда попала наша декаль
class DecalComponent : public yacs::Component {
public:
  CLASS_TYPE_DECLARE
  
  struct ContainerData {
    DecalContainerComponent* comp;
    UniqueID id;
  };
  
  static void setContainer(Container<TextureData>* textureContainer);
  
  DecalComponent();
  DecalComponent(const TextureData &texture);
  ~DecalComponent();
  
  void update(const size_t &time) override;
  void init(void* userData) override;
  
  void add(const ContainerData &data);
  void clear();
  
  uint32_t textureContainerIndex() const;
  void setTexture(const TextureData &texture);
private:
  uint32_t textureIndex;
  std::vector<ContainerData> decals;
  
  static Container<TextureData>* textureContainer;
};

// при удалении мира, я не знаю в каком порядке у нас будут удаляться компоненты
// поэтому не могу гарантировать что ремувДекал сработает правильно
// что делать в этом случае? не вызывать ремувДекал в деструкторе
// проблемы будут если я забуду сделать clear при удалении декали

#endif
