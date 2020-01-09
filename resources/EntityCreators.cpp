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

ObjectCreator::ObjectCreator(const CreateInfo &info) : physData(info.physData), graphicsData(info.graphicsData), attributesData(info.attributesData), effectsData(info.effectsData), inventoryData(info.inventoryData), weaponsData(info.weaponsData), abilitiesData(info.abilitiesData), intelligence(info.intelligence), statesData(info.statesData) {}
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
  
//   if (parent != nullptr && objectData->ability != nullptr) {
//     ASSERT(!objectData->ability->entityInfos().empty());
//     
//     auto parentAttribs = parent->get<AttributeComponent>();
//     auto parentTransform = parent->get<TransformComponent>();
//     //auto parentEffects = parent->get<EffectComponent>();
//     auto parentPhysics = parent->get<PhysicsComponent>();
//     
//     const auto &entInfo = objectData->ability->entityInfos()[objectData->entityIndex];
//     
//     TransOut transData;
//     TransformComponent* trans = nullptr;
//     if (objectData->ability->inheritTransform()) {
//       ent->set(parentTransform);
//       trans = parentTransform.get();
//     } else {
//       transData = entInfo.func(parentAttribs->get_finder<FLOAT_ATTRIBUTE_TYPE>(), parentAttribs->get_finder<INT_ATTRIBUTE_TYPE>(), TransIn{parentTransform->pos(), parentTransform->rot(), parentPhysics->getVelocity()});
//       trans = ent->add<TransformComponent>(transData.pos, transData.dir, simd::vec4(1.0f, 1.0f, 1.0f, 0.0f)).get();
//     }
//     
//     auto input = ent->add<InputComponent>();
//     
//     AttributeComponent* attribs = nullptr;
//     if (objectData->ability->inheritAttributes()) {
//       attribs = parentAttribs.get();
//       ent->set(parentAttribs);
//     } else {
//       auto float_attribs = attributesData.float_attribs;
//       auto int_attribs = attributesData.int_attribs;
//       entInfo.attribsFunc(parentAttribs->get_finder<FLOAT_ATTRIBUTE_TYPE>(), parentAttribs->get_finder<INT_ATTRIBUTE_TYPE>(), objectData->ability->floatAttributes(), objectData->ability->intAttributes(), float_attribs, int_attribs);
//       attribs = ent->add<AttributeComponent>(AttributeComponent::CreateInfo{nullptr, nullptr, SIZE_MAX, float_attribs, int_attribs}).get();
//     }
//     
//     
//   }
  
  static std::atomic<size_t> count(0);
  const size_t index = count.fetch_add(1);
  //auto info = 
  ent->add<InfoComponent>(InfoComponent::CreateInfo{Type::get("Entity "+std::to_string(index)+" contructed in creator"), ent});
  // InfoComponent мы должны создавать наверное в дебаге, хотя если этот компонент будет отвечать за редактуру энтити то он должен быть так же доступен в редакторе
  ASSERT(ent->at<InfoComponent>(INFO_COMPONENT_INDEX).valid());
  
  // все что нам нужно указать здесь это по идее графика и декали (еще возможно частицы, но их позже)
  // все графические данные короче должны оказаться здесь и больше ничего
  auto usrData = ent->add<UserDataComponent>();
  ASSERT(ent->at<UserDataComponent>(USER_DATA_COMPONENT_INDEX).valid());
  
  TransformComponent* trans = nullptr;
  if (createdByAbility) {
    if (objectData->ability->inheritTransform()) {
      if (parentTransform == nullptr) throw std::runtime_error("Parent object is invalid");
      ent->set(yacs::component_handle<TransformComponent>(parentTransform));
      trans = parentTransform;
    } else {
      TransOut transData = objectData->ability->computeTransform(parentAttribs->get_finder<FLOAT_ATTRIBUTE_TYPE>(), parentAttribs->get_finder<INT_ATTRIBUTE_TYPE>(), TransIn{parentTransform->pos(), parentTransform->rot(), parentPhysics->getVelocity()});
      trans = ent->add<TransformComponent>(transData.pos, transData.dir, simd::vec4(1.0f, 1.0f, 1.0f, 0.0f)).get();
    }
  } else {
    trans = ent->add<TransformComponent>(simd::vec4(objectData->pos), simd::vec4(objectData->dir), simd::vec4(physData.width, physData.height, physData.width, 0.0f)).get();
  }
  ASSERT(ent->at<TransformComponent>(TRANSFORM_COMPONENT_INDEX).valid());
  
  InputComponent* input = nullptr;
//   if (intelligence.tree == nullptr) {
    input = ent->add<InputComponent>().get();
    ASSERT(ent->at<InputComponent>(INPUT_COMPONENT_INDEX).valid());
//   } else {
//     input = ent->add<AIInputComponent>(AIInputComponent::CreateInfo{nullptr, trans.get()}).get();
//   }
  // юзер инпут? вообще инпуты я так понимаю сильно преобразятся, и скорее всего останется один
  // так как у меня добавится компонент отвечающий за направление ентити
  // инпуты нужны только в случае динамического объекта
  
  float max_speed = 7.0f;
  for (const auto & tmp : attributesData.float_attribs) {
    if (tmp.type->id() == max_speed_id) {
      max_speed = tmp.baseValue;
      break;
    }
  }
  
  const PhysicsType physObjType(true, createdByAbility ? SPHERE_TYPE : BBOX_TYPE, true, false, true, true);
  const PhysicsComponent::CreateInfo physInfo{
    {
      {0.0f, 0.0f, 0.0f, 0.0f},
      max_speed, 80.0f, 0.0f, 0.0f // ускорение? я раньше думал что это скорее всего неизменяемая переменная
    },
    {
      false,
      physObjType,
      1,
      1,
      physData.stairHeight,
      1.0f,
      4.0f,
      1.0f, // радиус какой? мы должны брать его из каких то аттрибутов
      
      input->inputIndex,
      trans->index(),
      UINT32_MAX,
      UINT32_MAX,
      UINT32_MAX,
      
      box_shape
    },
    nullptr
  };
  auto phys = ent->add<PhysicsComponent>(physInfo);
  ASSERT(ent->at<PhysicsComponent>(PHYSICS_COMPONENT_INDEX).valid());
  phys->setUserData(usrData.get());
  
  // не всегда
  auto graphics = ent->add<GraphicComponent>(GraphicComponent::CreateInfo{graphicsData.t, trans->index()});
  ASSERT(ent->at<GraphicComponent>(GRAPHICS_COMPONENT_INDEX).valid());
  
  // у нас по идее должен быть отпределен стейт контроллер для этого
  if (statesData.type != nullptr) {
    auto anims = ent->add<AnimationComponent>(AnimationComponent::CreateInfo{ent});
    ASSERT(ent->at<AnimationComponent>(ANIMATION_COMPONENT_INDEX).valid());
    
    auto sounds = ent->add<SoundComponent>(SoundComponent::CreateInfo{trans, phys.get()});
    ASSERT(ent->at<SoundComponent>(SOUND_COMPONENT_INDEX).valid());
  } else {
    ent->set(yacs::component_handle<AnimationComponent>(nullptr));
    ASSERT(!ent->at<AnimationComponent>(ANIMATION_COMPONENT_INDEX).valid());
    ent->set(yacs::component_handle<SoundComponent>(nullptr));
    ASSERT(!ent->at<SoundComponent>(SOUND_COMPONENT_INDEX).valid());
  }
  
  // как учесть что аттрибутов у абилки может не быть вовсе?
  AttributeComponent* attribs = nullptr;
  if (createdByAbility) {
    if (objectData->ability->inheritAttributes()) {
      if (parentAttribs == nullptr) throw std::runtime_error("Parent object is invalid");
      ent->set(yacs::component_handle<AttributeComponent>(parentAttribs));
      attribs = parentAttribs;
      ASSERT(ent->at<AttributeComponent>(ATTRIBUTE_COMPONENT_INDEX).valid());
    } else if (objectData->ability->hasComputeFunc()) {
      std::vector<AttributeComponent::InitInfo<FLOAT_ATTRIBUTE_TYPE>> float_attribs(objectData->ability->floatAttributes().size());
      std::vector<AttributeComponent::InitInfo<INT_ATTRIBUTE_TYPE>> int_attribs(objectData->ability->intAttributes().size());
      objectData->ability->computeAttributes(parentAttribs->get_finder<FLOAT_ATTRIBUTE_TYPE>(), parentAttribs->get_finder<INT_ATTRIBUTE_TYPE>(), float_attribs, int_attribs);
      attribs = ent->add<AttributeComponent>(AttributeComponent::CreateInfo{ent, phys->getExternalDataIndex(), SIZE_MAX, float_attribs, int_attribs}).get();
      ASSERT(ent->at<AttributeComponent>(ATTRIBUTE_COMPONENT_INDEX).valid());
    }
  } else if (!attributesData.int_attribs.empty() || !attributesData.float_attribs.empty()) {
    attribs = ent->add<AttributeComponent>(AttributeComponent::CreateInfo{ent, phys->getExternalDataIndex(), SIZE_MAX, attributesData.float_attribs, attributesData.int_attribs}).get();
    ASSERT(ent->at<AttributeComponent>(ATTRIBUTE_COMPONENT_INDEX).valid());
  } else {
    ent->set(yacs::component_handle<AttributeComponent>(nullptr));
    ASSERT(!ent->at<AttributeComponent>(ATTRIBUTE_COMPONENT_INDEX).valid());
  }
  
  EffectComponent* effects = nullptr; // тут еще как то мы должны определить нужны ли эффекты
  if (attribs != nullptr) {
    if (createdByAbility && objectData->ability->inheritEffects()) {
      if (parentAttribs == nullptr) throw std::runtime_error("Parent object is invalid");
      ent->set(yacs::component_handle<EffectComponent>(parentEffects));
      effects = parentEffects;
      ASSERT(ent->at<EffectComponent>(EFFECT_COMPONENT_INDEX).valid());
    } else {
      effects = ent->add<EffectComponent>(EffectComponent::CreateInfo{attribs}).get();
      ASSERT(ent->at<EffectComponent>(EFFECT_COMPONENT_INDEX).valid());
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
    }
  } else {
    ent->set(yacs::component_handle<EffectComponent>(nullptr));
    ASSERT(!ent->at<EffectComponent>(EFFECT_COMPONENT_INDEX).valid());
  }
  
  StateController* cont = nullptr;
  if (statesData.type != nullptr) {
    cont = ent->add<StateController>(StateController::CreateInfo{ent, statesData.type}).get();
    ASSERT(ent->at<StateController>(STATE_CONTROLLER_INDEX).valid());
    //std::cout << "current state: " << cont->state().name() << "\n";
  } else {
    ent->set(yacs::component_handle<StateController>(nullptr));
    ASSERT(!ent->at<StateController>(STATE_CONTROLLER_INDEX).valid());
  }
  
  // мне нужно сделать пробуждение объекта
  // я думаю что можно сделать как с коллизион тайп
  // то есть я должен задать типы раздражителей
  // звук, коллизия, эвент, удар, использование и др
  // причем нужно еще указать используется ли этот раздражитель
  // лишь единожды или нет (по времени? это можно сделать в функции)
  // делается это с помощью uint32_t и побитовых операций
  
  // не все объекты динамические, мы должны создавать еще и статические объекты по примерно тем же правилам
  MovementComponent* mv = nullptr;
  if (physObjType.isDynamic()) {
    mv = ent->add<MovementComponent>(MovementComponent::CreateInfo{
//       nullptr,
//       input,
//       phys.get(),
//       trans.get(),
      ent,
      intelligence.func
    }).get();
    
    if (mv == nullptr) throw std::runtime_error("sfqwegglfgpoeraijvpijawerv "+std::to_string(yacs::component_storage<MovementComponent>::type));
    ASSERT(ent->at<MovementComponent>(MOVEMENT_COMPONENT_INDEX).valid());
  }
  
  ent->set(yacs::component_handle<InventoryComponent>(nullptr));
//   ASSERT(!ent->at<InventoryComponent>(INVENTORY_COMPONENT_INDEX).valid());
  
  ent->set(yacs::component_handle<WeaponsComponent>(nullptr));
//   ASSERT(!ent->at<WeaponsComponent>(WEAPONS_COMPONENT_INDEX).valid());
  
  ent->set(yacs::component_handle<AbilityComponent>(nullptr));
//   ASSERT(!ent->at<AbilityComponent>(ABILITIES_COMPONENT_INDEX).valid());
  
  AIBasicComponent* ai = nullptr;
  if (intelligence.tree == nullptr) {
    ai = ent->add<AIBasicComponent>(AIBasicComponent::CreateInfo{
      entity_type::decoration,
      std::max(physData.height, physData.width), 
      nullptr, // мы каким то образом должны задать вершину этим объектам
      ent
    }).get();
    ASSERT(ent->at<AIBasicComponent>(AI_COMPONENT_INDEX).valid());
  } else {
    ai = ent->add<AIComponent>(AIComponent::CreateInfo{
      entity_type::npc,
      std::max(physData.height, physData.width), 
      HALF_SECOND, 
      nullptr, 
      intelligence.tree, 
      ent
    }).get();
    ASSERT(ent->at<AIComponent>(AI_COMPONENT_INDEX).valid());
  }
  
  usrData->aiComponent = ai;
  usrData->decalContainer = nullptr;
  usrData->entity = ent;
  usrData->graphic = graphics.get();
  usrData->vertex = nullptr;
  
  if (cont != nullptr) cont->setDefaultState();
  
  return ent;
}
