#ifndef COMPONENT_CREATORS_H
#define COMPONENT_CREATORS_H

#include "ComponentCreator.h"
#include "RenderStructures.h"
#include "AttributesComponent.h"

namespace tb {
  class BehaviorTree;
}

class AbilityType;

#define TRANSFORM_COMPONENT_POSITION_DATA_IDENTIFIER "pos"
#define TRANSFORM_COMPONENT_DIRECTION_DATA_IDENTIFIER "dir"

class TransformComponentCreator : public CreateComponent {
public:
  struct CreateInfo {
    const AbilityType* ability;
  };
  TransformComponentCreator(const CreateInfo &info);
  
  void create(yacs::entity* parent, yacs::entity* ent, const UniversalDataContainer* container) const override;
private:
  const AbilityType* ability;
};

#define INPUT_COMPONENT_DATA_IDENTIFIER "inputt"

class InputComponentCreator : public CreateComponent {
public:
  enum class type : uint8_t {
    standart,
    user,
    ai,
    count
  };
  
  void create(yacs::entity* parent, yacs::entity* ent, const UniversalDataContainer* container) const override;
};

#define PHYSICS_COMPONENT_TYPE_DATA_IDENTIFIER "physcct"
#define PHYSICS_COMPONENT_INFO_DATA_IDENTIFIER "physcomp"

class PhysicsComponentCreator : public CreateComponent {
public:
  enum class type : uint8_t {
    wall,
    object,
    ability,
    count
  };
  
  struct CreateInfo {
    uint32_t defaultCollisionGroup;
    uint32_t defaultCollisionFilter;
    float defaultOverbounce;
    float defaultFriction;
    float defaultMaxSpeed;
    float defaultAcceleration;
    float defaultStairHeight;
  };
  PhysicsComponentCreator(const CreateInfo &info);
  ~PhysicsComponentCreator();
  
  void create(yacs::entity* parent, yacs::entity* ent, const UniversalDataContainer* container) const override;
private:
  uint32_t defaultCollisionGroup;
  uint32_t defaultCollisionFilter;
  float defaultOverbounce;
  float defaultFriction;
  float defaultMaxSpeed;
  float defaultAcceleration;
  float defaultStairHeight;
};

class AbilityTransformChanger : public CreateComponent {
public:
  struct CreateInfo {
    const AbilityType* ability;
  };
  AbilityTransformChanger(const CreateInfo &info);
  
  void create(yacs::entity* parent, yacs::entity* ent, const UniversalDataContainer* container) const override;
private:
  const AbilityType* ability;
};

class GraphicsComponentCreator : public CreateComponent {
public:
  struct CreateInfo {
    Texture defaultTexture;
  };
  GraphicsComponentCreator(const CreateInfo &info);
  
  void create(yacs::entity* parent, yacs::entity* ent, const UniversalDataContainer* container) const override;
private:
  Texture defaultTexture;
};

#define GRAPHICS_INDEXED_COMPONENT_DATA_IDENTIFIER "gici"

class GraphicsIndexedComponentCreator : public CreateComponent {
public:
  void create(yacs::entity* parent, yacs::entity* ent, const UniversalDataContainer* container) const override;
};

#define LIGHT_COMPONENT_DATA_IDENTIFIER "lii"

class LightComponentCreator : public CreateComponent {
public:
  void create(yacs::entity* parent, yacs::entity* ent, const UniversalDataContainer* container) const override;
};

class AnimationComponentCreator : public CreateComponent {
public:
  void create(yacs::entity* parent, yacs::entity* ent, const UniversalDataContainer* container) const override;
};

class SoundComponentCreator : public CreateComponent {
public:
  void create(yacs::entity* parent, yacs::entity* ent, const UniversalDataContainer* container) const override;
};

class UserDataComponentCreator : public CreateComponent {
public:
  void create(yacs::entity* parent, yacs::entity* ent, const UniversalDataContainer* container) const override;
};

#define AI_BASIC_COMPONENT_DATA_IDENTIFIER "aibasici"

class AIBasicComponentCreator : public CreateComponent {
public:
  void create(yacs::entity* parent, yacs::entity* ent, const UniversalDataContainer* container) const override;
};

class AIComponentCreator : public CreateComponent {
public:
  struct CreateInfo {
    float radius;
    size_t updateTime;
    tb::BehaviorTree* behaviorTree;
    Type pathfindingType;
  };
  AIComponentCreator(const CreateInfo &info);
  
  void create(yacs::entity* parent, yacs::entity* ent, const UniversalDataContainer* container) const override;
private:
  float radius;
  size_t updateTime;
  tb::BehaviorTree* behaviorTree;
  Type pathfindingType;
  
  // вершина?
};

class AttributeCreatorComponent : public CreateComponent {
public:
  struct CreateInfo {
    const AbilityType* ability;
    std::vector<AttributeComponent::InitInfo<FLOAT_ATTRIBUTE_TYPE>> floatInit;
    std::vector<AttributeComponent::InitInfo<INT_ATTRIBUTE_TYPE>> intInit;
  };
  AttributeCreatorComponent(const CreateInfo &info);

  void create(yacs::entity* parent, yacs::entity* ent, const UniversalDataContainer* container) const override;
private:
  const AbilityType* ability;
  std::vector<AttributeComponent::InitInfo<FLOAT_ATTRIBUTE_TYPE>> floatInit;
  std::vector<AttributeComponent::InitInfo<INT_ATTRIBUTE_TYPE>> intInit;
};

class EffectComponentCreator : public CreateComponent {
public:
  void create(yacs::entity* parent, yacs::entity* ent, const UniversalDataContainer* container) const override;
};

class InteractionComponentCreateInfo : public CreateComponent {
public:
  void create(yacs::entity* parent, yacs::entity* ent, const UniversalDataContainer* container) const override;
};

#define INFO_COMPONENT_DATA_IDENTIFIER "name"

class InfoComponentCreator : public CreateComponent {
public:
  void create(yacs::entity* parent, yacs::entity* ent, const UniversalDataContainer* container) const override;
};

#endif
