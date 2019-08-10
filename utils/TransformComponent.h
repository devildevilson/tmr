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
//  static void setPrevContainer(Container<Transform>* prevContainer);
  
  TransformComponent();
  TransformComponent(const simd::vec4 &pos, const simd::vec4 &rot, const simd::vec4 &scale);
  ~TransformComponent();
  
//  void update(const size_t &time = 0) override;
//  void init(void* userData) override;
  
  void uiDraw() override;
  
  simd::mat4 getTransform(const bool rotation = false) const;
  
  const simd::vec4 & pos() const;
  const simd::vec4 & rot() const;
  const simd::vec4 & scale() const;
  simd::vec4 & pos();
  simd::vec4 & rot();
  simd::vec4 & scale();

  uint32_t index() const;
private:
  uint32_t transformIndex;

  static Container<Transform>* container;
};

#endif
