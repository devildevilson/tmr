#include "EntityCreators.h"

#include "EntityComponentSystem.h"
#include "Globals.h"

#include "TransformComponent.h"
#include "PhysicsComponent.h"
#include "InputComponent.h"
#include "AIInputComponent.h"
#include "InfoComponent.h"
#include "GraphicComponets.h"
#include "AnimationComponent.h"
#include "AnimationSystem.h"
#include "EventComponent.h"
#include "SoundComponent.h"
#include "AIComponent.h"
#include "UserDataComponent.h"
#include "Graph.h"
#include "CameraComponent.h"

class AbilityType;

yacs::entity* WallCreator::create(yacs::entity* parent, void* data) const {
  auto wallData = reinterpret_cast<WallData*>(data);
  
  auto ent = Global::world()->create_entity();
  
  const PhysicsComponent::CreateInfo physInfo{
    {},
    {
      false,
      PhysicsType(false, POLYGON_TYPE, true, false, true, true),
      1,
      1,
      0.0f,
      1.0f,
      4.0f,
      0.0f,
      
      UINT32_MAX,
      UINT32_MAX,
      UINT32_MAX,
      UINT32_MAX,
      UINT32_MAX,
      
      wallData->shapeType
    },
    nullptr
  };
  auto phys = ent->add<PhysicsComponent>(physInfo);
  
  const GraphicComponentIndexes::CreateInfo graphInfo{
    wallData->indexOffset,
    wallData->faceVertices,
    wallData->faceIndex,
    wallData->wallTexture,
    UINT32_MAX
  };
  auto graphics = ent->add<GraphicComponentIndexes>(graphInfo);
  
  auto userData = ent->add<UserDataComponent>();
  phys->setUserData(userData.get());
  
  const AIBasicComponent::CreateInfo aiInfo{
    wallData->radius,
    wallData->vertex,
    nullptr
  };
  auto aiBasic = ent->add<AIBasicComponent>(aiInfo);
  
  const InfoComponent::CreateInfo infoCompInfo{
    Type::get(wallData->name),
    userData.get()
  };
  auto infoComp = ent->add<InfoComponent>(infoCompInfo);
  
  userData->aiComponent = aiBasic.get();
  userData->anim = nullptr;
  userData->decalContainer = nullptr;
  userData->entity = ent;
  userData->events = nullptr;
  userData->graphic = graphics.get(),
  userData->phys = phys.get();
  userData->trans = nullptr;
  userData->vertex = wallData->vertex;
}

struct AbilityTypeContainer {
  const AbilityType* type;
};

yacs::entity* AbilityCreator::create(yacs::entity* parent, void* data) const {
  auto abilityType = reinterpret_cast<AbilityTypeContainer*>(data)->type;
  
  
}

ObjectCreator::ObjectCreator(const CreateInfo &info) : physData(info.physData) {}
yacs::entity* ObjectCreator::create(yacs::entity* parent, void* data) const {
  
}
