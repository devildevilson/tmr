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
#include "StateController.h"
#include "AbilityType.h"

// у стен может быть трансформа (подъемники например), или это будут не стены, а составные объекты?
// конечно было бы удобно если бы рендер был примерно как в думе, 
// то есть пространство от низа до следующего полигона закрашивалось какой нибудь текстуркой
// так мне нужно городить некие составные объекты, которые я пока не уверен как будут использоваться
// еще я не уверен как на них будут отрисовываться декали (точнее я вообще не уверен как отрисовывать декали)

yacs::entity* WallCreator::create(yacs::entity* parent, void* data) const {
  (void)parent;
  if (data == nullptr) throw std::runtime_error("Didnt specified wall data");
  
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
    static_cast<uint32_t>(wallData->faceIndex),
    wallData->wallTexture,
    UINT32_MAX
  };
  auto graphics = ent->add<GraphicComponentIndexes>(graphInfo);
  
  auto userData = ent->add<UserDataComponent>();
  phys->setUserData(userData.get());
  
  const AIBasicComponent::CreateInfo aiInfo{
    wallData->radius,
    wallData->vertex,
    userData.get()
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
yacs::entity* ObjectCreator::create(yacs::entity* parent, void* data) const {
  if (data == nullptr) throw std::runtime_error("Didnt specified object data");
  
  auto objectData = reinterpret_cast<ObjectData*>(data);
  if (parent == nullptr && objectData->ability != nullptr) throw std::runtime_error("Could not use ability without object");
  // может ли вообще такое быть?
  
  auto ent = Global::world()->create_entity();
  
  auto trans = ent->add<TransformComponent>(simd::vec4(objectData->pos), simd::vec4(objectData->dir), simd::vec4(physData.width, physData.height, physData.width, 0.0f));
  
  InputComponent* input = nullptr;
  if (intelligence.tree == nullptr) {
    input = ent->add<InputComponent>().get();
  } else {
    input = ent->add<AIInputComponent>(AIInputComponent::CreateInfo{nullptr, trans.get()}).get();
  }
  // юзер инпут? вообще инпуты я так понимаю сильно преобразятся, и скорее всего останется один
  // так как у меня добавится компонент отвечающий за направление ентити
  
  float max_speed = 7.0f;
  for (const auto & tmp : attributesData.float_attribs) {
    if (tmp.type->id() == max_speed_id) {
      max_speed = tmp.baseValue;
      break;
    }
  }
  
  const PhysicsComponent::CreateInfo physInfo{
    {
      {0.0f, 0.0f, 0.0f, 0.0f},
      max_speed, 80.0f, 0.0f, 0.0f // ускорение? я раньше думал что это скорее всего неизменяемая переменная
    },
    {
      false,
      PhysicsType(true, BBOX_TYPE, true, false, true, true),
      1,
      1,
      physData.stairHeight,
      1.0f,
      4.0f,
      0.0f,
      
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
  auto graphics = ent->add<GraphicComponent>(GraphicComponent::CreateInfo{graphicsData.t, trans->index()});
  
  
  
  auto anims = ent->add<AnimationComponent>(AnimationComponent::CreateInfo{trans.get(), phys.get(), graphics.get()});
  auto sounds = ent->add<SoundComponent>(SoundComponent::CreateInfo{trans.get(), phys.get()});
  AttributeComponent* attribs = nullptr;
  if (!attributesData.int_attribs.empty() || !attributesData.float_attribs.empty()) {
    attribs = ent->add<AttributeComponent>(AttributeComponent::CreateInfo{phys.get(), nullptr, SIZE_MAX, attributesData.float_attribs, attributesData.int_attribs}).get();
  }
  if (statesData.type != nullptr) {
    auto cont = ent->add<StateController>(StateController::CreateInfo{anims.get(), sounds.get(), nullptr, attribs, nullptr, statesData.type});
    //std::cout << "current state: " << cont->state().name() << "\n";
  }
  const UserDataComponent entData{
    ent,
    trans.get(),
    graphics.get(),
    phys.get(),
    anims.get(),
    nullptr,
    nullptr,
    nullptr,
    nullptr
  };
  auto usrData = ent->add<UserDataComponent>(entData);
  phys->setUserData(usrData.get());
  
  AIBasicComponent* ai = nullptr;
  if (intelligence.tree == nullptr) {
    ai = ent->add<AIBasicComponent>(AIBasicComponent::CreateInfo{std::max(physData.height, physData.width), nullptr, usrData.get()}).get();
  } else {
    ai = ent->add<AIComponent>(AIComponent::CreateInfo{std::max(physData.height, physData.width), HALF_SECOND, nullptr, intelligence.tree, static_cast<AIInputComponent*>(input), usrData.get(), Type::get("default")}).get();
  }
  
  static size_t count = 0;
  auto info = ent->add<InfoComponent>(InfoComponent::CreateInfo{Type::get("Entity "+std::to_string(count)+" contructed in creator"), usrData.get()});
  ++count;
  
  return ent;
}
