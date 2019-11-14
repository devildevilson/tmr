#ifndef TRANSFORM_COMPONENT_H
#define TRANSFORM_COMPONENT_H

#include "Utility.h"
#include "EntityComponentSystem.h"
#include "Editable.h"
#include "ArrayInterface.h"
#include "PhysicsTemporary.h"

class TransformComponent : public Editable {
public:
  static void setContainer(Container<Transform>* container);
  
  TransformComponent();
  TransformComponent(const simd::vec4 &pos, const simd::vec4 &rot, const simd::vec4 &scale);
  ~TransformComponent();
  
  void uiDraw() override;
  
  simd::mat4 getTransform(const bool rotation = false) const;
  
  const simd::vec4 & pos() const;
  const simd::vec4 & rot() const;
  const simd::vec4 & scale() const;
  simd::vec4 & pos();
  simd::vec4 & rot();
  simd::vec4 & scale();

  uint32_t index() const;
  
  // как будет выглядеть сериализация? 2 функции, обходим компоненты при сохранении
  // нам бы писать сначало в память все данные, затем компресить с помощью zlib, и в конце дампить на диск
  // все это выполнимо, protobuf работает со с++ строками, он сам интересно увеличивает их размер?
  // протобаф может записывать в напрямую в память, по идее мы должны написать сообщения для каждого компонента который нуждается в сериализации
  // обходим каждого, записываем данные в память, как загружать? по идее нужно создать необходимое количество энтити и дальше что?
  // как мы узнаем что лежит в буфере? по идее мы должны расставлять всякие метки, с другой стороны повторяющиеся объекты можно засовывать 
  // в отдельные сообщения, то есть например все трансформы сначало, потом другие компоненты. не уверен что это нормально...
private:
  uint32_t transformIndex;

  static Container<Transform>* container;
};

#endif
