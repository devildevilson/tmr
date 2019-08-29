#ifndef PHYSICS_COMPONENT_H
#define PHYSICS_COMPONENT_H

#include "EntityComponentSystem.h"
#include "Physics.h"
#include "TransformComponent.h"
#include "UserData.h"

// в будущем конечно должно превратиться в несколько независимых компонентов
// тело, форма, ?
// + я тут подумал: мне совершенно не обязательно держать update, init методы в компоненте
// если я правильно сконструирую компонент, то мне особо не нужен init
// если я правильно воспользуюсь методами поиска в World, то мне не нужен update
// тогда я смогу свести компонент к 8 байтам без виртуальных функций
// 8 байт нужны для быстрого создания/удаления компонента О(1)
// нужны методы рандомных итераций по компонентам, я могу работать напрямую с вектором, но лучше возвращать компонент хендл

class PhysicsComponent {
public:
  static void setContainer(Container<ExternalData>* externalDatas);

  struct CreateInfo {
    ExternalData externalData;
    PhysicsObjectCreateInfo physInfo;
    void* userData;
  };
  PhysicsComponent(const CreateInfo &info);
  ~PhysicsComponent();

//  void update(const size_t &time = 0) override;
//  void init(void* userData) override;

  const PhysicsIndexContainer & getIndexContainer() const;
  
  simd::vec4 getVelocity() const;
  float getSpeed() const;
  float getMaxSpeed() const;
  
  uint32_t getObjectShapePointsSize() const;
  const simd::vec4* getObjectShapePoints() const; // может потребоваться изменить точки или поверхности
  uint32_t getObjectShapeFacesSize() const;
  const simd::vec4* getObjectShapeFaces() const;
  
  PhysicsIndexContainer* getGround() const;
  
  uint32_t getExternalDataIndex() const;
  void setUserData(void* ptr);
protected:
  uint32_t externalDataIndex;
//  TransformComponent* trans;
  PhysicsIndexContainer container;

  static Container<ExternalData>* externalDatas;
};

#endif
