#include "EntityCreators.h"

#include "EntityComponentSystem.h"
#include "Globals.h"

#include <atomic>

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
#include "StateController.h"
#include "AbilityType.h"
#include "EffectComponent.h"
#include "MovementComponent.h"
#include "InventoryComponent.h"
#include "AbilityComponent.h"
#include "Interactions.h"

#include "global_components_indicies.h"

// у стен может быть трансформа (подъемники например), или это будут не стены, а составные объекты?
// конечно было бы удобно если бы рендер был примерно как в думе, 
// то есть пространство от низа до следующего полигона закрашивалось какой нибудь текстуркой
// так мне нужно городить некие составные объекты, которые я пока не уверен как будут использоваться
// еще я не уверен как на них будут отрисовываться декали (точнее я вообще не уверен как отрисовывать декали)

yacs::entity* WallCreator::create(yacs::entity* parent, const void* data) const {
  (void)parent;
  if (data == nullptr) throw std::runtime_error("Didnt specified wall data");
  
  auto wallData = reinterpret_cast<const WallData*>(data);
  
  auto ent = Global::world()->create_entity();
  
  const InfoComponent::CreateInfo infoCompInfo{
    Type::get(wallData->name),
    ent
  };
  auto infoComp = ent->add<InfoComponent>(infoCompInfo);
  
  ent->set(yacs::component_handle<TransformComponent>(nullptr));
  ent->set(yacs::component_handle<InputComponent>(nullptr));
  
  const PhysicsComponent::CreateInfo physInfo{
    {},
    {
      PhysicsType(false, POLYGON_TYPE, true, false, true, true),
      WALL_COLLISION_TYPE,
      wall_collision_filter,
      0.0f,
      1.0f,
      4.0f,
      0.0f,
      1.0f,
      
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
    static_cast<uint32_t>(wallData->faceIndex),
    wallData->wallTexture,
    UINT32_MAX
  };
  auto graphics = ent->add<GraphicComponentIndexes>(graphInfo);
  
  // возможно стоит перенести выше
  auto userData = ent->add<UserDataComponent>();
  phys->setUserData(userData.get());
  
  const AIBasicComponent::CreateInfo aiInfo{
    entity_type::wall,
    wallData->radius,
    wallData->vertex,
    ent
  };
  auto aiBasic = ent->add<AIBasicComponent>(aiInfo);
  //PRINT(aiBasic.get())
  //aiBasic->
  
  userData->aiComponent = aiBasic.get();
//   userData->anim = nullptr;
  userData->decalContainer = nullptr;
  userData->entity = ent;
//   userData->events = nullptr;
  userData->graphic = graphics.get();
//   userData->phys = phys.get();
//   userData->trans = nullptr;
  userData->vertex = wallData->vertex;
  
  return ent;
}

// struct AbilityTypeContainer {
//   const AbilityType* type;
// };
// 
// yacs::entity* AbilityCreator::create(yacs::entity* parent, void* data) const {
//   auto abilityType = reinterpret_cast<AbilityTypeContainer*>(data)->type;
//   
//   
// }

const Type box_shape = Type::get("boxShape");
const Type max_speed_id = Type::get("actor_max_speed");

enum {
  ANGLE_NUM_INDEX,
  DISTANCE_NUM_INDEX,
  MIN_DIST_INDEX,
  TICK_COUNT_INDEX,
  ATTACK_SPEED_INDEX,
  ATTACK_TIME_INDEX,
  COUNT
};

ObjectCreator::Interaction_data::VariableType::VariableType() : container(0) {}
ObjectCreator::Interaction_data::VariableType::VariableType(const bool angleNum, const bool distanceNum, const bool minDistNum, const bool tickCountNum, const bool attackSpeedNum, const bool attackTimeNum) : container(0) {
  make(angleNum, distanceNum, minDistNum, tickCountNum, attackSpeedNum, attackTimeNum);
}

void ObjectCreator::Interaction_data::VariableType::make(const bool angleNum, const bool distanceNum, const bool minDistNum, const bool tickCountNum, const bool attackSpeedNum, const bool attackTimeNum) {
  container = (angleNum*(1 << ANGLE_NUM_INDEX)) | 
              (distanceNum*(1 << DISTANCE_NUM_INDEX)) | 
              (minDistNum*(1 << MIN_DIST_INDEX)) | 
              (tickCountNum*(1 << TICK_COUNT_INDEX)) | 
              (attackSpeedNum*(1 << ATTACK_SPEED_INDEX)) | 
              (attackTimeNum*(1 << ATTACK_TIME_INDEX));
}

bool ObjectCreator::Interaction_data::VariableType::isAngleNumber() const {
  const uint32_t mask = 1 << ANGLE_NUM_INDEX;
  return (container & mask) == mask;
}

bool ObjectCreator::Interaction_data::VariableType::isDistanceNumber() const {
  const uint32_t mask = 1 << DISTANCE_NUM_INDEX;
  return (container & mask) == mask;
}

bool ObjectCreator::Interaction_data::VariableType::isMinDistanceNumber() const {
  const uint32_t mask = 1 << MIN_DIST_INDEX;
  return (container & mask) == mask;
}

bool ObjectCreator::Interaction_data::VariableType::isTickCountNumber() const {
  const uint32_t mask = 1 << TICK_COUNT_INDEX;
  return (container & mask) == mask;
}

bool ObjectCreator::Interaction_data::VariableType::isAttackSpeedNumber() const {
  const uint32_t mask = 1 << ATTACK_SPEED_INDEX;
  return (container & mask) == mask;
}

bool ObjectCreator::Interaction_data::VariableType::isAttackTimeNumber() const {
  const uint32_t mask = 1 << ATTACK_TIME_INDEX;
  return (container & mask) == mask;
}

ObjectCreator::ObjectCreator(const CreateInfo &info) : physData(info.physData), graphicsData(info.graphicsData), attributesData(info.attributesData), effectsData(info.effectsData), inventoryData(info.inventoryData), weaponsData(info.weaponsData), abilitiesData(info.abilitiesData), intelligence(info.intelligence), statesData(info.statesData), interaction(info.interaction) {}

yacs::entity* ObjectCreator::create(yacs::entity* parent, const void* data) const {
  if (data == nullptr) throw std::runtime_error("Didnt specified object data");
  
  auto objectData = reinterpret_cast<const ObjectData*>(data);
  if (parent == nullptr && objectData->ability != nullptr) throw std::runtime_error("Could not use ability without object");
  // может ли вообще такое быть?
  
  auto ent = Global::world()->create_entity();
  const bool createdByAbility = parent != nullptr && objectData->ability != nullptr;
  AttributeComponent* parentAttribs = nullptr;
  TransformComponent* parentTransform = nullptr;
  EffectComponent* parentEffects = nullptr;
  PhysicsComponent* parentPhysics = nullptr;
  
  if (createdByAbility) {
    parentAttribs = parent->get<AttributeComponent>().get();
    parentTransform = parent->get<TransformComponent>().get();
    parentEffects = parent->get<EffectComponent>().get();
    parentPhysics = parent->get<PhysicsComponent>().get();
  }
  
  TransOut transOut;
  if (createdByAbility && !objectData->ability->inheritTransform()) {
    transOut = objectData->ability->computeTransform(
      parentAttribs->get_finder<FLOAT_ATTRIBUTE_TYPE>(), 
      parentAttribs->get_finder<INT_ATTRIBUTE_TYPE>(), 
      TransIn{parentTransform->pos(), parentTransform->rot(), parentPhysics->getVelocity()}
    );
  }
  
  const ComponentCreationData cData{
    ent,
    parent,
    parentAttribs,
    parentTransform,
    parentEffects,
    parentPhysics,
    objectData
  };
  
  auto info = create_info(cData);
  auto usrData = create_usrdata(cData);
  auto trans = create_transform(cData, &transOut);
  auto input = create_input(cData);
  auto vars = find_variables(cData);
  
  // не очевидно
  const PhysicsType physObjType(interaction.type == ObjectCreator::Interaction_data::type::none ? physData.dynamic : interaction.type == ObjectCreator::Interaction_data::type::impact, createdByAbility ? SPHERE_TYPE : BBOX_TYPE, true, false, true, true);
  auto phys = create_physics(cData, physObjType, vars, trans, input, usrData);
  auto graphics = create_graphics(cData, trans);
  auto anims = create_animation(cData);
  auto sounds = create_sound(cData, trans, phys);
  auto attribs = create_attributes(cData, phys);
  auto effects = create_effects(cData, attribs);
  auto cont = create_states(cData);
  auto inter = create_interaction(cData, vars, phys, trans);
  
  // мне нужно сделать пробуждение объекта
  // я думаю что можно сделать как с коллизион тайп
  // то есть я должен задать типы раздражителей
  // звук, коллизия, эвент, удар, использование и др
  // причем нужно еще указать используется ли этот раздражитель
  // лишь единожды или нет (по времени? это можно сделать в функции)
  // делается это с помощью uint32_t и побитовых операций
  
  auto mv = create_movement(cData, physObjType);
  auto inventory = create_inventory(cData);
  auto weapons = create_weapons(cData);
  auto abilities = create_abilities(cData);
  auto ai = create_ai(cData);
  
  usrData->aiComponent = ai;
  usrData->decalContainer = nullptr;
  usrData->entity = ent;
  usrData->graphic = graphics;
  usrData->vertex = nullptr;
  
  if (cont != nullptr) cont->setDefaultState();
  
  (void)info;
  (void)anims;
  (void)sounds;
  (void)effects;
  (void)inter;
  (void)mv;
  (void)inventory;
  (void)weapons;
  (void)abilities;
  
  return ent;
}

ObjectCreator::variables ObjectCreator::find_variables(const ComponentCreationData &data) const {
  variables vars{
    7.0f,
    1.0f,
    0,
    0.0f,
    0,
    0.0f,
    0.0f,
    0
  };
  
  // ********
  
  for (const auto & tmp : attributesData.float_attribs) {
    if (tmp.type->id() == max_speed_id) {
      vars.max_speed = tmp.baseValue;
      break;
    }
  }
  
  // ********
  
  if (interaction.type != ObjectCreator::Interaction_data::type::none && data.parent != nullptr) {
    if (interaction.variables.isDistanceNumber()) {
      vars.radius = interaction.distance.var;
    } else {
      if (!interaction.distance.attributeType.valid()) throw std::runtime_error("Bad distance attrib");
      if (data.parentAttribs == nullptr) throw std::runtime_error("Bad parent attribs");
      
      auto attr_float = data.parentAttribs->get<FLOAT_ATTRIBUTE_TYPE>(interaction.distance.attributeType);
      if (attr_float != nullptr) {
        vars.radius = attr_float->value();
      } else {
        auto attr_int = data.parentAttribs->get<INT_ATTRIBUTE_TYPE>(interaction.distance.attributeType);
        if (attr_int != nullptr) vars.radius = attr_int->value();
      }
    }
  }
  
  // ********
  
  vars.delay = data.obj_data->ability == nullptr ? 0 : data.obj_data->ability->delay();
  
  // ********
  
  if (interaction.type != ObjectCreator::Interaction_data::type::none && data.parent != nullptr) {
    if (interaction.variables.isAngleNumber()) {
      vars.angle = interaction.distance.var;
    } else {
      if (!interaction.angle.attributeType.valid()) throw std::runtime_error("Bad angle attrib");
      if (data.parentAttribs == nullptr) throw std::runtime_error("Bad parent attribs");
      
      auto attr_float = data.parentAttribs->get<FLOAT_ATTRIBUTE_TYPE>(interaction.angle.attributeType);
      if (attr_float != nullptr) {
        vars.angle = attr_float->value();
      } else {
        auto attr_int = data.parentAttribs->get<INT_ATTRIBUTE_TYPE>(interaction.angle.attributeType);
        if (attr_int != nullptr) vars.angle = attr_int->value();
      }
    }
  }
  
  // ********
  
  if (interaction.type != ObjectCreator::Interaction_data::type::none && data.parent != nullptr) {
    if (interaction.variables.isTickCountNumber()) {
      vars.tickCount = interaction.tickCount.var;
    } else {
      if (!interaction.tickCount.attributeType.valid()) throw std::runtime_error("Bad tickCount attrib");
      if (data.parentAttribs == nullptr) throw std::runtime_error("Bad parent attribs");
      
      auto attr_float = data.parentAttribs->get<FLOAT_ATTRIBUTE_TYPE>(interaction.tickCount.attributeType);
      if (attr_float != nullptr) {
        vars.tickCount = attr_float->value();
      } else {
        auto attr_int = data.parentAttribs->get<INT_ATTRIBUTE_TYPE>(interaction.tickCount.attributeType);
        if (attr_int != nullptr) vars.tickCount = attr_int->value();
      }
    }
  }
  
  // ********
  
  if (interaction.type != ObjectCreator::Interaction_data::type::none && data.parent != nullptr) {
    if (interaction.variables.isAttackSpeedNumber()) {
      vars.attackSpeed = interaction.attackSpeed.var;
    } else {
      if (!interaction.attackSpeed.attributeType.valid()) throw std::runtime_error("Bad attackSpeed attrib");
      if (data.parentAttribs == nullptr) throw std::runtime_error("Bad parent attribs");
      
      auto attr_float = data.parentAttribs->get<FLOAT_ATTRIBUTE_TYPE>(interaction.attackSpeed.attributeType);
      if (attr_float != nullptr) {
        vars.attackSpeed = attr_float->value();
      } else {
        auto attr_int = data.parentAttribs->get<INT_ATTRIBUTE_TYPE>(interaction.attackSpeed.attributeType);
        if (attr_int != nullptr) vars.attackSpeed = attr_int->value();
      }
    }
  }
  
  // ********
  
  if (interaction.type != ObjectCreator::Interaction_data::type::none && data.parent != nullptr) {
    if (interaction.variables.isMinDistanceNumber()) {
      vars.minDist = interaction.minDist.var;
    } else {
      if (!interaction.minDist.attributeType.valid()) throw std::runtime_error("Bad minDist attrib");
      if (data.parentAttribs == nullptr) throw std::runtime_error("Bad parent attribs");
      
      auto attr_float = data.parentAttribs->get<FLOAT_ATTRIBUTE_TYPE>(interaction.minDist.attributeType);
      if (attr_float != nullptr) {
        vars.minDist = attr_float->value();
      } else {
        auto attr_int = data.parentAttribs->get<INT_ATTRIBUTE_TYPE>(interaction.minDist.attributeType);
        if (attr_int != nullptr) vars.minDist = attr_int->value();
      }
    }
  }
  
  // ********
  
  if (interaction.type != ObjectCreator::Interaction_data::type::none && data.parent != nullptr) {
    if (interaction.variables.isAttackTimeNumber()) {
      vars.attackTime = interaction.attackTime.var;
    } else {
      if (!interaction.attackTime.attributeType.valid()) throw std::runtime_error("Bad attackTime attrib");
      if (data.parentAttribs == nullptr) throw std::runtime_error("Bad parent attribs");
      
      auto attr_float = data.parentAttribs->get<FLOAT_ATTRIBUTE_TYPE>(interaction.attackTime.attributeType);
      if (attr_float != nullptr) {
        vars.attackTime = attr_float->value();
      } else {
        auto attr_int = data.parentAttribs->get<INT_ATTRIBUTE_TYPE>(interaction.attackTime.attributeType);
        if (attr_int != nullptr) vars.attackTime = attr_int->value();
      }
    }
  }
  
  // ********
  
  return vars;
}

InfoComponent* ObjectCreator::create_info(const ComponentCreationData &data) const {
  static std::atomic<size_t> count(0);
  const size_t index = count.fetch_add(1);
  //auto info = 
  auto info = data.ent->add<InfoComponent>(InfoComponent::CreateInfo{Type::get("Entity "+std::to_string(index)+" contructed in creator"), data.ent}).get();
  // InfoComponent мы должны создавать наверное в дебаге, хотя если этот компонент будет отвечать за редактуру энтити то он должен быть так же доступен в редакторе
  ASSERT(data.ent->at<InfoComponent>(INFO_COMPONENT_INDEX).valid());
  return info;
}

UserDataComponent* ObjectCreator::create_usrdata(const ComponentCreationData &data) const {
  // все что нам нужно указать здесь это по идее графика и декали (еще возможно частицы, но их позже)
  // все графические данные короче должны оказаться здесь и больше ничего
  auto usrData = data.ent->add<UserDataComponent>().get();
  ASSERT(data.ent->at<UserDataComponent>(USER_DATA_COMPONENT_INDEX).valid());
  return usrData;
}

TransformComponent* ObjectCreator::create_transform(const ComponentCreationData &data, const TransOut* transData) const {
  if (data.obj_data->ability != nullptr && data.parent != nullptr && data.obj_data->ability->inheritTransform()) {
    if (data.parentTransform == nullptr) throw std::runtime_error("Parent object is invalid");
    data.ent->set(yacs::component_handle<TransformComponent>(data.parentTransform));
    ASSERT(data.ent->at<TransformComponent>(TRANSFORM_COMPONENT_INDEX).valid());
    return data.parentTransform;
  }
  
  if (data.obj_data->ability != nullptr && data.parent != nullptr && !data.obj_data->ability->inheritTransform()) {
//     TransOut transData = objectData->ability->computeTransform(parentAttribs->get_finder<FLOAT_ATTRIBUTE_TYPE>(), parentAttribs->get_finder<INT_ATTRIBUTE_TYPE>(), TransIn{parentTransform->pos(), parentTransform->rot(), parentPhysics->getVelocity()});
    auto trans = data.ent->add<TransformComponent>(transData->pos, transData->dir, simd::vec4(physData.width, physData.height, physData.width, 0.0f)).get();
    return trans;
  }
  
  auto trans = data.ent->add<TransformComponent>(simd::vec4(data.obj_data->pos), simd::vec4(data.obj_data->dir), simd::vec4(physData.width, physData.height, physData.width, 0.0f)).get();
  ASSERT(data.ent->at<TransformComponent>(TRANSFORM_COMPONENT_INDEX).valid());
  return trans;
}

InputComponent* ObjectCreator::create_input(const ComponentCreationData &data) const {
  auto input = data.ent->add<InputComponent>().get();
  ASSERT(data.ent->at<InputComponent>(INPUT_COMPONENT_INDEX).valid());
  return input;
}

PhysicsComponent* ObjectCreator::create_physics(const ComponentCreationData &data, const PhysicsType &physObjType, const variables &vars, TransformComponent* trans, InputComponent* input, UserDataComponent* usrData) const {
  const PhysicsComponent::CreateInfo physInfo{
    {
      {0.0f, 0.0f, 0.0f, 0.0f},
      vars.max_speed, 80.0f, 0.0f, 0.0f // ускорение? я раньше думал что это скорее всего неизменяемая переменная
    },
    {
      physObjType,
      interaction.type == ObjectCreator::Interaction_data::type::none ? physData.collisionGroup : INTERACTION_COLLISION_TYPE,
      interaction.type == ObjectCreator::Interaction_data::type::none ? physData.collisionFilter : interaction_collision_filter,
      physData.stairHeight,
      1.0f,
      4.0f,
      vars.radius, // нужно посчитать радиус 
      physData.gravCoef,
      
      input->inputIndex,
      trans->index(),
      UINT32_MAX,
      UINT32_MAX,
      UINT32_MAX,
      
      box_shape
    },
    usrData
  };
  auto phys = data.ent->add<PhysicsComponent>(physInfo).get();
  ASSERT(data.ent->at<PhysicsComponent>(PHYSICS_COMPONENT_INDEX).valid());
  //phys->setUserData(usrData);
  return phys;
}

GraphicComponent* ObjectCreator::create_graphics(const ComponentCreationData &data, TransformComponent* trans) const {
  //PRINT(std::to_string(graphicsData.t.image.index)+" "+std::to_string(graphicsData.t.image.layer))
  if (statesData.type == nullptr && (graphicsData.t.image.index == UINT32_MAX || graphicsData.t.image.layer == UINT32_MAX)) { 
    data.ent->set(yacs::component_handle<GraphicComponent>(nullptr));
    ASSERT(!data.ent->at<GraphicComponent>(GRAPHICS_COMPONENT_INDEX).valid());
    return nullptr;
  }
  
  auto graphics = data.ent->add<GraphicComponent>(GraphicComponent::CreateInfo{graphicsData.t, trans->index(), physData.height}).get();
  ASSERT(data.ent->at<GraphicComponent>(GRAPHICS_COMPONENT_INDEX).valid());
  return graphics;
}

AnimationComponent* ObjectCreator::create_animation(const ComponentCreationData &data) const {
  AnimationComponent* anims = nullptr;
  
  if (statesData.type != nullptr) {
    anims = data.ent->add<AnimationComponent>(AnimationComponent::CreateInfo{data.ent}).get();
    ASSERT(data.ent->at<AnimationComponent>(ANIMATION_COMPONENT_INDEX).valid());
  } else {
    data.ent->set(yacs::component_handle<AnimationComponent>(nullptr));
    ASSERT(!data.ent->at<AnimationComponent>(ANIMATION_COMPONENT_INDEX).valid());
  }
  
  return anims;
}

SoundComponent* ObjectCreator::create_sound(const ComponentCreationData &data, TransformComponent* trans, PhysicsComponent* phys) const {
  SoundComponent* sounds = nullptr;
  
  if (statesData.type != nullptr) {
    sounds = data.ent->add<SoundComponent>(SoundComponent::CreateInfo{trans, phys}).get();
    ASSERT(data.ent->at<SoundComponent>(SOUND_COMPONENT_INDEX).valid());
  } else {
    data.ent->set(yacs::component_handle<SoundComponent>(nullptr));
    ASSERT(!data.ent->at<SoundComponent>(SOUND_COMPONENT_INDEX).valid());
  }
  
  return sounds;
}

AttributeComponent* ObjectCreator::create_attributes(const ComponentCreationData &data, PhysicsComponent* phys) const {
  // как учесть что аттрибутов у абилки может не быть вовсе?
  AttributeComponent* attribs = nullptr;
  
  if (data.obj_data->ability != nullptr && data.parent != nullptr && data.obj_data->ability->inheritAttributes()) {
    if (data.parentAttribs == nullptr) throw std::runtime_error("Parent object is invalid");
    data.ent->set(yacs::component_handle<AttributeComponent>(data.parentAttribs));
    attribs = data.parentAttribs;
    ASSERT(data.ent->at<AttributeComponent>(ATTRIBUTE_COMPONENT_INDEX).valid());
    return attribs;
  }
  
  if (data.obj_data->ability != nullptr && data.parent != nullptr && data.obj_data->ability->hasComputeFunc()) {
    std::vector<AttributeComponent::InitInfo<FLOAT_ATTRIBUTE_TYPE>> float_attribs(data.obj_data->ability->floatAttributes().size());
    std::vector<AttributeComponent::InitInfo<INT_ATTRIBUTE_TYPE>> int_attribs(data.obj_data->ability->intAttributes().size());
    data.obj_data->ability->computeAttributes(data.parentAttribs->get_finder<FLOAT_ATTRIBUTE_TYPE>(), data.parentAttribs->get_finder<INT_ATTRIBUTE_TYPE>(), float_attribs, int_attribs);
    attribs = data.ent->add<AttributeComponent>(AttributeComponent::CreateInfo{data.ent, phys->getExternalDataIndex(), SIZE_MAX, float_attribs, int_attribs}).get();
    ASSERT(data.ent->at<AttributeComponent>(ATTRIBUTE_COMPONENT_INDEX).valid());
    return attribs;
  }
  
  if (!attributesData.int_attribs.empty() || !attributesData.float_attribs.empty()) {
    attribs = data.ent->add<AttributeComponent>(AttributeComponent::CreateInfo{data.ent, phys->getExternalDataIndex(), SIZE_MAX, attributesData.float_attribs, attributesData.int_attribs}).get();
    ASSERT(data.ent->at<AttributeComponent>(ATTRIBUTE_COMPONENT_INDEX).valid());
    return attribs;
  }
  
  data.ent->set(yacs::component_handle<AttributeComponent>(nullptr));
  ASSERT(!data.ent->at<AttributeComponent>(ATTRIBUTE_COMPONENT_INDEX).valid());
  return nullptr;
}

EffectComponent* ObjectCreator::create_effects(const ComponentCreationData &data, AttributeComponent* attribs) const {
  if (attribs == nullptr) {
    data.ent->set(yacs::component_handle<EffectComponent>(nullptr));
    ASSERT(!data.ent->at<EffectComponent>(EFFECT_COMPONENT_INDEX).valid());
    return nullptr;
  }
  
  if (data.obj_data->ability != nullptr && data.parent && data.obj_data->ability->inheritEffects()) {
    if (data.parentAttribs == nullptr) throw std::runtime_error("Parent object is invalid");
    data.ent->set(yacs::component_handle<EffectComponent>(data.parentEffects));
    ASSERT(data.ent->at<EffectComponent>(EFFECT_COMPONENT_INDEX).valid());
    return data.parentEffects;
  }
  
  EffectComponent* effects = data.ent->add<EffectComponent>(EffectComponent::CreateInfo{attribs}).get();
  ASSERT(data.ent->at<EffectComponent>(EFFECT_COMPONENT_INDEX).valid());
  for (auto effect : effectsData.initialEffects) {
    ComputedEffectContainer cont(effect->baseValues().size());
    if (!effect->baseValues().empty() && effect->type().compute_effect()) {
      // эффекты которые необходимо вычислить перед использованием
      // не должны быть начальными эффектами
      // в этом случае мы по идее должны выдать ворнинг
      
      memcpy(cont.bonusTypes.array, effect->baseValues().data(), effect->baseValues().size()*sizeof(effect->baseValues()[0]));
    }
    effect->resist(attribs->get_finder<FLOAT_ATTRIBUTE_TYPE>(), attribs->get_finder<INT_ATTRIBUTE_TYPE>(), &cont);
    effects->addEffect(cont, effect, nullptr);
  }
  
  for (const auto & mod : effectsData.eventEffects) {
    effects->addEventEffect(mod.first, mod.second);
  }
  
  return effects;
}

StateController* ObjectCreator::create_states(const ComponentCreationData &data) const {
  if (statesData.type == nullptr) {
    data.ent->set(yacs::component_handle<StateController>(nullptr));
    ASSERT(!data.ent->at<StateController>(STATE_CONTROLLER_INDEX).valid());
    return nullptr;
  }
  
  StateController* cont = data.ent->add<StateController>(StateController::CreateInfo{data.ent, statesData.type}).get();
  ASSERT(data.ent->at<StateController>(STATE_CONTROLLER_INDEX).valid());
  //std::cout << "current state: " << cont->state().name() << "\n";
  return cont;
}

MovementComponent* ObjectCreator::create_movement(const ComponentCreationData &data, const PhysicsType &physObjType) const {
  // не все объекты динамические, мы должны создавать еще и статические объекты по примерно тем же правилам
  // не все динамические объекты нуждаются в MovementComponent
  
  if (!physObjType.isDynamic() || intelligence.tree == nullptr) {
    data.ent->set(yacs::component_handle<MovementComponent>(nullptr));
    ASSERT(!data.ent->at<MovementComponent>(MOVEMENT_COMPONENT_INDEX).valid());
    return nullptr;
  }
  
  MovementComponent* mv = data.ent->add<MovementComponent>(MovementComponent::CreateInfo{
//       nullptr,
//       input,
//       phys.get(),
//       trans.get(),
    data.ent,
    intelligence.func
  }).get();
    
  if (mv == nullptr) throw std::runtime_error("sfqwegglfgpoeraijvpijawerv "+std::to_string(yacs::component_storage<MovementComponent>::type));
  ASSERT(data.ent->at<MovementComponent>(MOVEMENT_COMPONENT_INDEX).valid());
  
  return mv;
}

InventoryComponent* ObjectCreator::create_inventory(const ComponentCreationData &data) const {
  data.ent->set(yacs::component_handle<InventoryComponent>(nullptr));
  ASSERT(!data.ent->at<InventoryComponent>(INVENTORY_COMPONENT_INDEX).valid());
  return nullptr;
}

WeaponsComponent* ObjectCreator::create_weapons(const ComponentCreationData &data) const {
  data.ent->set(yacs::component_handle<WeaponsComponent>(nullptr));
  ASSERT(!data.ent->at<WeaponsComponent>(WEAPONS_COMPONENT_INDEX).valid());
  return nullptr;
}

AbilityComponent* ObjectCreator::create_abilities(const ComponentCreationData &data) const {
  data.ent->set(yacs::component_handle<AbilityComponent>(nullptr));
  ASSERT(!data.ent->at<AbilityComponent>(ABILITIES_COMPONENT_INDEX).valid());
  return nullptr;
}

AIBasicComponent* ObjectCreator::create_ai(const ComponentCreationData &data) const {
  if (intelligence.tree == nullptr) {
    AIBasicComponent* ai = data.ent->add<AIBasicComponent>(AIBasicComponent::CreateInfo{
      entity_type::decoration,
      std::max(physData.height, physData.width), 
      nullptr, // мы каким то образом должны задать вершину этим объектам
      data.ent
    }).get();
    
//     PRINT(ai)
//     PRINT("entity size "+std::to_string(data.ent->components_count()))
    
    ASSERT(data.ent->at<AIBasicComponent>(AI_COMPONENT_INDEX).valid());
    return ai;
  } 
  
  AIComponent* ai = data.ent->add<AIComponent>(AIComponent::CreateInfo{
    entity_type::npc,
    std::max(physData.height, physData.width), 
    HALF_SECOND, 
    nullptr, 
    intelligence.tree, 
    data.ent
  }).get();
  ASSERT(data.ent->at<AIComponent>(AI_COMPONENT_INDEX).valid());
  return ai;
}

Interaction* ObjectCreator::create_interaction(const ComponentCreationData &data, const variables &vars, PhysicsComponent* physics, TransformComponent* trans) const {
  // компонент интеракции, как его создать?
  Interaction* inter = nullptr;
  switch (interaction.type) {
    case ObjectCreator::Interaction_data::type::target: {
      //inter = data.ent->add<TargetInteraction>(TargetInteraction::CreateInfo{vars.delay, data.ent, ???, Type::get("skill")}).get();
      throw std::runtime_error("TargetInteraction is not done yet");
      break;
    }
    
    case ObjectCreator::Interaction_data::type::ray: {
      inter = data.ent->add<RayInteraction>(RayInteraction::CreateInfo{
        RayInteraction::type::first_min, 
        1000.0f, 
        0.0f, 
        data.parentPhysics->getIndexContainer().objectDataIndex, 
        interaction_collision_filter, 
        vars.delay, 
        data.ent, 
        trans, 
        Type::get("attack")
      }).get();
      
      ASSERT(data.ent->at<ImpactInteraction>(INTERACTION_INDEX).get() != nullptr);
      break;
    }
    
    case ObjectCreator::Interaction_data::type::slashing: {
      inter = data.ent->add<SlashingInteraction>(SlashingInteraction::CreateInfo{
        vars.delay,
        vars.attackTime,
        vars.attackTime / vars.tickCount,
        vars.tickCount,
        UINT32_MAX,
        0.5f,
        angle,
        vars.radius,
        vars.attackSpeed,
        {0.0f, 1.0f, 0.0f, 0.0f},
        data.ent,
        physics,
        trans,
        Type::get("attack")
      }).get();
      
      ASSERT(data.ent->at<ImpactInteraction>(INTERACTION_INDEX).get() != nullptr);
      break;
    }
    
    case ObjectCreator::Interaction_data::type::stabbing: {
      inter = data.ent->add<StabbingInteraction>(StabbingInteraction::CreateInfo{
        vars.delay,
        vars.attackTime,
        vars.attackTime / vars.tickCount,
        vars.tickCount,
        UINT32_MAX,
        0.5f,
        vars.minDist,
        vars.radius,
        vars.attackSpeed,
        angle,
        1,
        1,
        data.ent,
        physics,
        trans,
        Type::get("attack")
      }).get();
      
      ASSERT(data.ent->at<ImpactInteraction>(INTERACTION_INDEX).get() != nullptr);
      break;
    }
    
    case ObjectCreator::Interaction_data::type::impact: {
      inter = data.ent->add<ImpactInteraction>(ImpactInteraction::CreateInfo{
        vars.delay,
        data.ent,
        physics,
        trans,
        Type::get("attack")
      }).get();
      
      ASSERT(data.ent->at<ImpactInteraction>(INTERACTION_INDEX).get() != nullptr);
      break;
    }
    
    case ObjectCreator::Interaction_data::type::none: {
      // нужно положить нулл
      data.ent->set(yacs::component_handle<ImpactInteraction>(nullptr));
      ASSERT(data.ent->at<ImpactInteraction>(INTERACTION_INDEX).get() == nullptr);
      break;
    }
  }
  
  return inter;
}
