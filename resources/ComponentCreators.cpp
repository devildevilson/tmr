#include "ComponentCreators.h"

#include "TransformComponent.h"
#include "InputComponent.h"
#include "AIInputComponent.h"
#include "PhysicsComponent.h"
#include "SkillComponent.h"
#include "GraphicComponets.h"
#include "AnimationComponent.h"
#include "SoundComponent.h"
#include "AIComponent.h"
#include "EffectComponent.h"
#include "InfoComponent.h"

#define CHECK_UINT32_DEFAULT_VALUE(val) (val == UINT32_MAX)
#define CHECK_FLOAT_DEFAULT_VALUE(val) (glm::floatBitsToUint(val) == UINT32_MAX)
#define CHECK_SIZE_T_DEFAULT_VALUE(val) (val == SIZE_MAX)

TransformComponentCreator::TransformComponentCreator(const CreateInfo &info) : ability(info.ability) {}
void TransformComponentCreator::create(yacs::entity* parent, yacs::entity* ent, const UniversalDataContainer* container) const {
  if (ability != nullptr) {
    if (ability->inheritTransform()) {
      auto trans = parent->get<TransformComponent>();
      
      // при удалении нужно учесть что транс взят у родительского энтити
      ent->set(trans);
      return;
    }
    
    ent->add<TransformComponent>();
    return;
  }
  
  void* ptr = container->get_data(DataIdentifier(TRANSFORM_COMPONENT_POSITION_DATA_IDENTIFIER));
  auto pos = reinterpret_cast<float*>(ptr);
  if (pos == nullptr) throw std::runtime_error("Could not find position in container");
  
  ptr = container->get_data(DataIdentifier(TRANSFORM_COMPONENT_DIRECTION_DATA_IDENTIFIER));
  auto dir = reinterpret_cast<float*>(ptr);
  if (dir == nullptr) throw std::runtime_error("Could not find direction in container");
  
  ASSERT(container->get_data_size(DataIdentifier(TRANSFORM_COMPONENT_POSITION_DATA_IDENTIFIER)) == sizeof(simd::vec4));
  ASSERT(container->get_data_size(DataIdentifier(TRANSFORM_COMPONENT_DIRECTION_DATA_IDENTIFIER)) == sizeof(simd::vec4));
  
  ent->add<TransformComponent>(simd::vec4(pos), simd::vec4(dir), simd::vec4(1.0f, 1.0f, 1.0f, 0.0f));
}

void InputComponentCreator::create(yacs::entity* parent, yacs::entity* ent, const UniversalDataContainer* container) const {
  (void)parent;
  
  void* ptr = container->get_data(DataIdentifier(INPUT_COMPONENT_DATA_IDENTIFIER));
  if (ptr == nullptr) throw std::runtime_error("UniversalDataContainer must contain input type");
  auto inputt = reinterpret_cast<enum type*>(ptr);
  
  switch (*inputt) {
    case type::standart: {
      ent->add<InputComponent>();
      
      break;
    }
    case type::user: {
      auto trans = ent->get<TransformComponent>();
      if (!trans.valid()) throw std::runtime_error("TransformComponent must be created");
      
      ent->add<UserInputComponent>(UserInputComponent::CreateInfo{trans.get()});
      
      break;
    }
    case type::ai: {
      auto trans = ent->get<TransformComponent>();
      if (!trans.valid()) throw std::runtime_error("TransformComponent must be created");
      
      ent->add<AIInputComponent>(AIInputComponent::CreateInfo{nullptr, trans.get()});
      
      break;
    }
    default: throw std::runtime_error("This input type is not implemented yet");
  }
}

PhysicsComponentCreator::PhysicsComponentCreator(const CreateInfo &info)
  : defaultCollisionGroup(info.defaultCollisionGroup), 
    defaultCollisionFilter(info.defaultCollisionFilter), 
    defaultOverbounce(info.defaultOverbounce), 
    defaultFriction(info.defaultFriction), 
    defaultMaxSpeed(info.defaultMaxSpeed), 
    defaultAcceleration(info.defaultAcceleration), 
    defaultStairHeight(info.defaultStairHeight) {}
PhysicsComponentCreator::~PhysicsComponentCreator() {}

// в текущем виде это выглядит примерно так, что мне здесь откровенно не нравится?
// PhysicsComponent::CreateInfo лучше бы пересоздать и заполнить отдельно
// есть поиск компонентов, которое неизвестно созданы ли (нужно придерживаться строгой последовательности)
// инпут компонентов много и мне нужно будет искать все чтобы правильно индекс задать
void PhysicsComponentCreator::create(yacs::entity* parent, yacs::entity* ent, const UniversalDataContainer* container) const {
  static const ExternalData defaultData = {
    {0.0f, 0.0f, 0.0f, 0.0f},
    7.0f, 80.0f, 0.0f, 0.0f
  };
  
  (void)parent; // скорее всего потребуется в очень редких случаях
  
  void* ptr = container->get_data(DataIdentifier(PHYSICS_COMPONENT_TYPE_DATA_IDENTIFIER));
  if (ptr == nullptr) throw std::runtime_error("UniversalDataContainer must contain physcct");
  
  auto physcct = reinterpret_cast<enum type*>(ptr);
  
  ptr = container->get_data(DataIdentifier(PHYSICS_COMPONENT_INFO_DATA_IDENTIFIER));
  if (ptr == nullptr) throw std::runtime_error("UniversalDataContainer must contain physics create info");
  
  auto physcomp = reinterpret_cast<PhysicsComponent::CreateInfo*>(ptr);
  //memset(&physcomp->externalData, 0, sizeof(physcomp->externalData));
  physcomp->externalData = defaultData;
//   physcomp->externalData.acceleration = defaultData.acceleration;
//   physcomp->externalData.maxSpeed = defaultData.maxSpeed;
//   physcomp->externalData.additionalForce = defaultData.additionalForce;
  
  switch (*physcct) {
    case type::wall: {
      if (!physcomp->physInfo.shapeType.valid()) throw std::runtime_error("Shape type is not valid");
      if (physcomp->physInfo.type.getObjType() != POLYGON_TYPE) throw std::runtime_error("Phys type is not valid");
      physcomp->physInfo.type = PhysicsType(false, POLYGON_TYPE, true, false, true, true);
      
      if (CHECK_FLOAT_DEFAULT_VALUE(physcomp->physInfo.groundFricion)) physcomp->physInfo.groundFricion = defaultFriction;
      
      auto trans = ent->get<TransformComponent>();
      if (trans.valid()) physcomp->physInfo.transformIndex = trans->index();
      
      // так то у нас еще должны быть матрицы, значит мы должны еще найти графический компонент (потому что они сейчас там находятся)
      
      break;
    }
    case type::object: {
      if (physcomp->physInfo.type.getObjType() != BBOX_TYPE) throw std::runtime_error("Phys type is not valid");
      physcomp->physInfo.type = PhysicsType(true, BBOX_TYPE, true, false, true, true);
      
      physcomp->physInfo.groundFricion = defaultFriction;
      if (CHECK_FLOAT_DEFAULT_VALUE(physcomp->physInfo.stairHeight)) physcomp->physInfo.stairHeight = defaultStairHeight;
      
      //physcomp->externalData.additionalForce = simd::vec4(0.0f, 0.0f, 0.0f, 0.0f);
      memset(&physcomp->externalData.additionalForce, 0, sizeof(simd::vec4));
      
      auto trans = ent->get<TransformComponent>();
      if (!trans.valid()) throw std::runtime_error("Could not find transform component");
      physcomp->physInfo.transformIndex = trans->index();
      
      break;
    }
    case type::ability: {
      if (physcomp->physInfo.type.getObjType() != SPHERE_TYPE) throw std::runtime_error("Phys type is not valid");
      physcomp->physInfo.type = PhysicsType(true, SPHERE_TYPE, false, true, true, true); 
      
      physcomp->physInfo.groundFricion = defaultFriction;
      physcomp->physInfo.stairHeight = 0.1f;
      
      //physcomp->externalData.additionalForce = simd::vec4(0.0f, 0.0f, 0.0f, 0.0f);
      memset(&physcomp->externalData.additionalForce, 0, sizeof(simd::vec4));
      
      auto trans = ent->get<TransformComponent>();
      if (!trans.valid()) throw std::runtime_error("Could not find transform component");
      physcomp->physInfo.transformIndex = trans->index();
      
      break;
    }
    default: throw std::runtime_error("This physics type is not implemented yet");
  }
  
  if (CHECK_UINT32_DEFAULT_VALUE(physcomp->physInfo.collisionFilter)) physcomp->physInfo.collisionFilter = defaultCollisionFilter;
  if (CHECK_UINT32_DEFAULT_VALUE(physcomp->physInfo.collisionGroup)) physcomp->physInfo.collisionGroup = defaultCollisionGroup;
  if (CHECK_FLOAT_DEFAULT_VALUE(physcomp->physInfo.overbounce)) physcomp->physInfo.overbounce = defaultOverbounce;
  if (CHECK_FLOAT_DEFAULT_VALUE(physcomp->externalData.maxSpeed)) physcomp->externalData.maxSpeed = defaultMaxSpeed;
  if (CHECK_FLOAT_DEFAULT_VALUE(physcomp->externalData.acceleration)) physcomp->externalData.acceleration = defaultAcceleration;
  
  AIInputComponent* aiInput = nullptr;
  
  if (physcomp->physInfo.type.isDynamic()) {
    InputComponent* input = ent->get<InputComponent>().get();
    if (input == nullptr) input = ent->get<UserInputComponent>().get();
    if (input == nullptr) {
      aiInput = ent->get<AIInputComponent>().get();
      input = aiInput;
    }
    
    if (input == nullptr) throw std::runtime_error("Could not find InputComponent");
    
    physcomp->physInfo.inputIndex = input->inputIndex;
  }
  
  auto physComp = ent->add<PhysicsComponent>(*physcomp);
  if (aiInput != nullptr) aiInput->setPhysicsComponent(physComp.get());
}

AbilityTransformChanger::AbilityTransformChanger(const CreateInfo &info) : ability(info.ability) {}
void AbilityTransformChanger::create(yacs::entity* parent, yacs::entity* ent, const UniversalDataContainer* container) const {
  auto parent_trans = parent->get<TransformComponent>();
  auto parent_attribs = parent->get<AttributeComponent>();
  auto parent_phys = parent->get<PhysicsComponent>();
  auto trans = ent->get<TransformComponent>();
  auto phys = ent->get<PhysicsComponent>();
  
  const TransIn in{
    parent_trans->pos(),
    parent_trans->rot(),
    parent_phys->getVelocity()
  };
  
  const TransOut &out = ability->computeTransform(parent_attribs->get_finder<FLOAT_ATTRIBUTE_TYPE>(), parent_attribs->get_finder<INT_ATTRIBUTE_TYPE>(), in);
  trans->pos() = out.pos;
  trans->rot() = parent_trans->rot();
  phys->setVelocity(out.vel);
}

GraphicsComponentCreator::GraphicsComponentCreator(const CreateInfo &info) : defaultTexture(info.defaultTexture) {}
void GraphicsComponentCreator::create(yacs::entity* parent, yacs::entity* ent, const UniversalDataContainer* container) const {
  (void)parent;
  
  auto trans = ent->get<TransformComponent>();
  if (!trans.valid()) throw std::runtime_error("TransformComponent doesnt found");
  
  ent->add<GraphicComponent>(GraphicComponent::CreateInfo{defaultTexture, trans->index()});
}

void GraphicsIndexedComponentCreator::create(yacs::entity* parent, yacs::entity* ent, const UniversalDataContainer* container) const {
  (void)parent;
  
  auto trans = ent->get<TransformComponent>();
//   if (!trans.valid()) throw std::runtime_error("TransformComponent doesnt found");
  
  void* ptr = container->get_data(DataIdentifier(GRAPHICS_INDEXED_COMPONENT_DATA_IDENTIFIER));
  if (ptr == nullptr) throw std::runtime_error("Not enough data for graphics component");
  auto info = reinterpret_cast<GraphicComponentIndexes::CreateInfo*>(ptr);
  info->transformIndex = trans.valid() ? trans->index() : UINT32_MAX;
  
  ent->add<GraphicComponentIndexes>(*info);
}

void LightComponentCreator::create(yacs::entity* parent, yacs::entity* ent, const UniversalDataContainer* container) const {
  (void)parent;
  
  auto trans = ent->get<TransformComponent>();
  if (!trans.valid()) throw std::runtime_error("TransformComponent doesnt found");
  
  void* ptr = container->get_data(DataIdentifier(LIGHT_COMPONENT_DATA_IDENTIFIER));
  if (ptr == nullptr) throw std::runtime_error("Not enough data for light component");
  auto info = reinterpret_cast<float*>(ptr);
  
  const Light::CreateInfo i{
    {
      {0.0f, 0.0f, 0.0f, info[3]},
      {info[0], info[1], info[2], 0.1f}
    },
    trans->index()
  };
  
  ent->add<Light>(i);
}

void AnimationComponentCreator::create(yacs::entity* parent, yacs::entity* ent, const UniversalDataContainer* container) const {
  (void)parent;
  
  auto trans = ent->get<TransformComponent>();
  if (!trans.valid()) throw std::runtime_error("TransformComponent doesnt found");
  
  auto phys = ent->get<PhysicsComponent>();
  if (!phys.valid()) throw std::runtime_error("PhysicsComponent doesnt found");
  
  GraphicComponent* comp = ent->get<GraphicComponent>().get();
  if (comp == nullptr) comp = ent->get<GraphicComponentIndexes>().get();
  if (comp == nullptr) throw std::runtime_error("GraphicComponent doesnt found");
  
  const AnimationComponent::CreateInfo animInfo{
    trans.get(),
    phys.get(),
    comp
  };
  ent->add<AnimationComponent>(animInfo);
}

void SoundComponentCreator::create(yacs::entity* parent, yacs::entity* ent, const UniversalDataContainer* container) const {
  (void)parent;
  
  auto trans = ent->get<TransformComponent>();
  if (!trans.valid()) throw std::runtime_error("TransformComponent doesnt found");
  
  auto phys = ent->get<PhysicsComponent>();
  if (!phys.valid()) throw std::runtime_error("PhysicsComponent doesnt found");
  
  const SoundComponent::CreateInfo soundInfo{
    trans.get(),
    phys.get(),
  };
  ent->add<SoundComponent>(soundInfo);
}

void UserDataComponentCreator::create(yacs::entity* parent, yacs::entity* ent, const UniversalDataContainer* container) const {
  auto trans = ent->get<TransformComponent>();
  
  GraphicUpdater* comp = ent->get<GraphicComponent>().get();
  if (comp == nullptr) comp = ent->get<GraphicComponentIndexes>().get();
  if (comp == nullptr) comp = ent->get<Light>().get();
  
  auto phys = ent->get<PhysicsComponent>();
  
  const UserDataComponent userData{
    ent,
    trans.get(),
    comp,
    phys.get(),
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr
  };
  auto usr = ent->add<UserDataComponent>(userData);
  if (phys.valid()) phys->setUserData(usr.get());
}

void AIBasicComponentCreator::create(yacs::entity* parent, yacs::entity* ent, const UniversalDataContainer* container) const {
  (void)parent;
  
  void* ptr = container->get_data(DataIdentifier(AI_BASIC_COMPONENT_DATA_IDENTIFIER));
  if (ptr == nullptr) throw std::runtime_error("Not enough data for AIBasicComponent");
  auto info = reinterpret_cast<AIBasicComponent::CreateInfo*>(ptr);
  
  auto usrData = ent->get<UserDataComponent>();
  if (!usrData.valid()) throw std::runtime_error("UserDataComponent doesnt found");
  
  info->components = usrData.get();
  auto ai = ent->add<AIBasicComponent>(*info);
  usrData->aiComponent = ai.get();
  usrData->vertex = info->currentVertex;
}

AIComponentCreator::AIComponentCreator(const AIComponentCreator::CreateInfo& info) : radius(info.radius), updateTime(info.updateTime), behaviorTree(info.behaviorTree), pathfindingType(info.pathfindingType) {}
void AIComponentCreator::create(yacs::entity* parent, yacs::entity* ent, const UniversalDataContainer* container) const {
  // во первых у нас для типа ии, basic - тип для того чтобы можно было бы учитывать объект в графе
  // и основной - собственно для взаимодействий
  // создавать их я так понимаю скорее всего нужно отдельно
  
  auto input = ent->get<AIInputComponent>();
  if (!input.valid()) throw std::runtime_error("AIInputComponent doesnt found");
  
  auto usrData = ent->get<UserDataComponent>();
  if (!usrData.valid()) throw std::runtime_error("UserDataComponent doesnt found");
  
  const AIComponent::CreateInfo info{
    radius,
    updateTime,
    nullptr,
    behaviorTree,

//      phys.get(),
//      trans.get(),
    input.get(),
    usrData.get(),

    pathfindingType
  };
  auto ai = ent->add<AIComponent>(info).get();
  usrData->aiComponent = ai;
  usrData->vertex = nullptr;
}

AttributeCreatorComponent::AttributeCreatorComponent(const AttributeCreatorComponent::CreateInfo& info) : ability(info.ability), floatInit(info.floatInit), intInit(info.intInit) {}
void AttributeCreatorComponent::create(yacs::entity* parent, yacs::entity* ent, const UniversalDataContainer* container) const {
  auto phys = ent->get<PhysicsComponent>();
  if (!phys.valid()) throw std::runtime_error("PhysicsComponent doesnt found");
  
  if (ability == nullptr) {
    const AttributeComponent::CreateInfo info{
      phys.get(),
      nullptr,
      SIZE_MAX,
      floatInit,
      intInit
    };
    ent->add<AttributeComponent>(info);
    return;
  }
  
  auto attribs = parent->get<AttributeComponent>();
  if (!attribs.valid()) throw std::runtime_error("Could not find parent AttributeComponent");
  
  if (ability->inheritAttributes()) {
    ent->set(attribs);
    return;
  }
  
  std::vector<AttributeComponent::InitInfo<FLOAT_ATTRIBUTE_TYPE>> floatInit(ability->floatAttributesList().size());
  std::vector<AttributeComponent::InitInfo<INT_ATTRIBUTE_TYPE>> intInit(ability->intAttributesList().size());
  ability->computeAttributes(attribs->get_finder<FLOAT_ATTRIBUTE_TYPE>(), attribs->get_finder<INT_ATTRIBUTE_TYPE>(), floatInit, intInit);
  
  const AttributeComponent::CreateInfo info{
    phys.get(),
    nullptr,
    SIZE_MAX,
    floatInit,
    intInit
  };
  ent->add<AttributeComponent>(info);
}

void EffectComponentCreator::create(yacs::entity* parent, yacs::entity* ent, const UniversalDataContainer* container) const {
  (void)parent;
  
  auto attribs = ent->get<AttributeComponent>();
  if (!attribs.valid()) throw std::runtime_error("AttributeComponent doesnt found");
  
  const EffectComponent::CreateInfo info{
    attribs.get()
  };
  ent->add<EffectComponent>(info);
}

void InteractionComponentCreateInfo::create(yacs::entity* parent, yacs::entity* ent, const UniversalDataContainer* container) const {
  
}

void InfoComponentCreator::create(yacs::entity* parent, yacs::entity* ent, const UniversalDataContainer* container) const {
  (void)parent;
  
  auto usrData = ent->get<UserDataComponent>();
  if (!usrData.valid()) throw std::runtime_error("UserDataComponent doesnt found");
  
  void* ptr = container->get_data(DataIdentifier(INFO_COMPONENT_DATA_IDENTIFIER));
  if (ptr == nullptr) throw std::runtime_error("Could not find name for info component");
  auto type = reinterpret_cast<Type*>(ptr);
  
  ent->add<InfoComponent>(InfoComponent::CreateInfo{*type, usrData.get()});
}
