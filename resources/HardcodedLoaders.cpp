#include "HardcodedLoaders.h"

#include "Globals.h"
#include "Utility.h"

#include "ImageLoader.h"

//#include "Components.h"
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
#include "AISystem.h"
#include "UserDataComponent.h"
#include "Graph.h"
#include "CameraComponent.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>

#include <random>

HardcodedAnimationLoader::HardcodedAnimationLoader(const CreateInfo &info) : textureLoader(info.textureLoader) {}

HardcodedAnimationLoader::~HardcodedAnimationLoader() {}

bool HardcodedAnimationLoader::canParse(const std::string &key) const {
  // animations ?
  return key == "animations";
}

bool HardcodedAnimationLoader::parse(const Modification* mod,
                                     const std::string &pathPrefix,
                                     const nlohmann::json &data,
                                     std::vector<Resource*> &resource,
                                     std::vector<ErrorDesc> &errors,
                                     std::vector<WarningDesc> &warnings) {
  (void)mod;
  (void)pathPrefix;
  (void)data;
  (void)resource;
  (void)errors;
  (void)warnings;

  // тут нужно загрузить данные из json
  // в общем то ничего необычного, 
  // кроме того что мне нужно еще придумать как реализовать анимацию передвигающихся текстур
  return false;
}

bool HardcodedAnimationLoader::forget(const ResourceID &name) {
  (void)name;
  return false;
}

Resource* HardcodedAnimationLoader::getParsedResource(const ResourceID &id) {
  (void)id;
  return nullptr;
}

const Resource* HardcodedAnimationLoader::getParsedResource(const ResourceID &id) const {
  (void)id;
  return nullptr;
}

bool HardcodedAnimationLoader::load(const ModificationParser* modifications, const Resource* resource) {
  (void)modifications;
  (void)resource;

  // вот тут как раз пример использования иерархии загрузки
  // у всех переданных сюда ресурсов депенденси мы передаем в textureLoader

  // а потом грузим собственно анимацию

  return false;
}

bool HardcodedAnimationLoader::unload(const ResourceID &id) {
  (void)id;
  return false;
}

void HardcodedAnimationLoader::end() {
  const auto &texture1  = textureLoader->resourceData(ResourceID::get("texture"));
  const auto &texture2  = textureLoader->resourceData(ResourceID::get("Rough Block Wall"));
  const auto &texture3  = textureLoader->resourceData(ResourceID::get("552c34f8e3469"));
  const auto &texture4  = textureLoader->resourceData(ResourceID::get("14626771132270"));
  const auto &texture5  = textureLoader->resourceData(ResourceID::get("7037.970"));
  const auto &texture6  = textureLoader->resourceData(ResourceID::get("n"));
  const auto &texture7  = textureLoader->resourceData(ResourceID::get("ne"));
  const auto &texture8  = textureLoader->resourceData(ResourceID::get("e"));
  const auto &texture9  = textureLoader->resourceData(ResourceID::get("se"));
  const auto &texture10 = textureLoader->resourceData(ResourceID::get("s"));
  const auto &texture11 = textureLoader->resourceData(ResourceID::get("sw"));
  const auto &texture12 = textureLoader->resourceData(ResourceID::get("w"));
  const auto &texture13 = textureLoader->resourceData(ResourceID::get("nw"));
  
  const Animation::CreateInfo animationInfo{
    {
      {
        { texture6->images[0], false, false },
        { texture7->images[0], false, false },
        { texture8->images[0], false, false },
        { texture9->images[0], false, false },
        { texture10->images[0], false, false },
        { texture11->images[0], false, false },
        { texture12->images[0], false, false },
        { texture13->images[0], false, false },
      },
      {
        { texture6->images[1], false, false },
        { texture7->images[1], false, false },
        { texture8->images[1], false, false },
        { texture9->images[1], false, false },
        { texture10->images[1], false, false },
        { texture11->images[1], false, false },
        { texture12->images[1], false, false },
        { texture13->images[1], false, false },
      },
      {
        { texture6->images[2], false, false },
        { texture7->images[2], false, false },
        { texture8->images[2], false, false },
        { texture9->images[2], false, false },
        { texture10->images[2], false, false },
        { texture11->images[2], false, false },
        { texture12->images[2], false, false },
        { texture13->images[2], false, false },
      },
      {
        { texture6->images[3], false, false },
        { texture7->images[3], false, false },
        { texture8->images[3], false, false },
        { texture9->images[3], false, false },
        { texture10->images[3], false, false },
        { texture11->images[3], false, false },
        { texture12->images[3], false, false },
        { texture13->images[3], false, false },
      }
    }
  };
  Global::animations()->createAnimation(ResourceID::get("walking"), animationInfo);
}

void HardcodedAnimationLoader::clear() {
  
}

size_t HardcodedAnimationLoader::overallState() const {
  
}

size_t HardcodedAnimationLoader::loadingState() const {
  
}

std::string HardcodedAnimationLoader::hint() const {
  
}

//HardcodedEntityLoader::HardcodedEntityLoader(const InitData &data) {
//  TransformComponent::setContainer(data.transforms);
//  GraphicComponent::setContainer(data.matrices);
//  GraphicComponent::setContainer(data.rotationDatas);
//  GraphicComponent::setContainer(data.textureContainer);
//  AnimationComponent::setStateContainer(data.stateContainer);
//  InputComponent::setContainer(data.inputs);
//  PhysicsComponent::setContainer(data.externalDatas);
//
//  state = 5504;
//}

HardcodedEntityLoader::HardcodedEntityLoader(ImageLoader* textureLoader) : player(nullptr), playerTransform(nullptr), camera(nullptr), input(nullptr), textureLoader(textureLoader) {
  state = 5504;
  
  Global g;
  g.setWorld(&world);
}

HardcodedEntityLoader::~HardcodedEntityLoader() {
  
}

bool HardcodedEntityLoader::canParse(const std::string &key) const {
  // ?
}

bool HardcodedEntityLoader::parse(const Modification* mod,
                                  const std::string &pathPrefix,
                                  const nlohmann::json &data,
                                  std::vector<Resource*> &resource,
                                  std::vector<ErrorDesc> &errors,
                                  std::vector<WarningDesc> &warnings) {
  // здесь мы чекаем json его как то 
  // то есть мы должны пропарсить и выделить ресурсы 
  // + должны запомнить какую-то мета информацию, чтобы потом по имени 
  // загрузить нужный ресурс
  
  return false;
}

bool HardcodedEntityLoader::forget(const ResourceID &name) {
  // тут мы просто забываем старые ресурсы чтобы потом мы не могли загрузить их по имени
  // например когда мы отключаем мод
  
  return false;
}

Resource* HardcodedEntityLoader::getParsedResource(const ResourceID &id) {
  return nullptr;
}

const Resource* HardcodedEntityLoader::getParsedResource(const ResourceID &id) const {
  return nullptr;
}

bool HardcodedEntityLoader::load(const ModificationParser* modifications, const Resource* resource) {
  // здесь мы по метоинформации загружаем необходимый ресурс
  // либо мы помечаем этот ресурс к загрузке
  // ну в общем мы делаем какие то вещи которые подходят для конкретного типа ресурса
  
  // здесь это не будет работать
  // хотя может понадобиться для загрузки фабрики
  
  // в будущем здесь (в подобном классе) будут создваться
  // игровые объекты разных форматов, как мне сделать для них возможность создания?
  
  return false;
}

bool HardcodedEntityLoader::unload(const ResourceID &id) {
  // это может ли потребоваться?
  // может
  
  return false;
}

void HardcodedEntityLoader::end() {
  // это тоже нет скорее всего
}

void HardcodedEntityLoader::clear() {
  // этот метод по идее для того чтобы чистить вещи которые остались после парсера
}

size_t HardcodedEntityLoader::overallState() const {
  return state;
}

size_t HardcodedEntityLoader::loadingState() const {
  return accumulatedState;
}

std::string HardcodedEntityLoader::hint() const {
  return "Feeding up monsters";
}

void HardcodedEntityLoader::create() {
  // тут мы просто запилим создание всех объектов
  // что делать с созданием игрока?
  // нам тут еще нужна возможность взять текстурки и еще какие то дополнительные вещи
  // нам нужно еще какие то данные инициализации
  // как их передать?

  const Type shape = Type::get("boxShape");
  {
    const RegisterNewShapeInfo info{
      {},
      {simd::vec4(0.5f, 0.5f, 0.5f, 0.0f)}
    };
    
    Global::physics()->registerShape(shape, BBOX_TYPE, info);
  }

  PhysicsComponent::CreateInfo physInfo{
    {
      simd::vec4(0.0f, 0.0f, 0.0f, 0.0f),
      7.0f, 80.0f, 0.0f, 0.0f
    },
    {
      false,

      PhysicsType(true, BBOX_TYPE, true, false, true, true),
      1,     // collisionGroup
      1,     // collisionFilter

      0.5f,  // stairHeight
      //40.0f, // acceleration
      1.0f,  // overbounce
      4.0f,  // groundFriction

      0.0f,  // radius

      UINT32_MAX,
      UINT32_MAX,
      UINT32_MAX,
      UINT32_MAX,
      UINT32_MAX,

      //"boxShape"
      shape
    },
    nullptr
  };
  
  {
    // нужно будет у игрока сделать интеллектуальные анимации, то есть анимации переходят из одного состояния в другое сами собой и по заданым условиям
    // например, анимация атаки у война (размашистый удар справа на лево) переходит в анимацию смены ориентации меча
    // если какое то время меч находится в левом положении, то мы должны "переложить" его на правую сторону
    // то есть при нажатии клавиши мы можем перейти в следующее состояние, которое, например, на какое то время заблокирует возможность двигаться 
    // (или даже возможно передаст какие то дополнительные данные), это состояние перейдет в состояние смены положения, которое забокирует возможность атаковать
    // сменив положение, игрок снова получит возможность атаковать, атака пройдет на этот раз слева на право
    // не атаковав через какое то время запустится анимация смены положения меча,
    // то есть [дефолт] -> [атака справа] -> [смена направления налево] -> [атака слева] -> [смена направления направо] -> [дефолт]
    //                                                                  -> [перекладываем направо] -> [дефолт]
    // у нас есть следующее измение по времени, и изменение по нажатым клавишам, изменение по условиям?
    // должны быть еще координаты где рисовать текстурки (мы должны сымитировать движения как в думе)
    
    yacs::entity* ent1 = world.create_entity();
    playerTransform = ent1->add<TransformComponent>(simd::vec4(1.0f, 5.0f, 2.0f, 1.0f),
                                                    simd::vec4(0.0f, 0.0f, 1.0f, 0.0f),
                                                    simd::vec4(1.0f, 1.0f, 1.0f, 0.0f)).get();
    
//     std::cout << "HardcodedEntityLoader::create 01" << "\n";
    
//     ent1->add<InputComponent>();
    
    input = ent1->add<UserInputComponent>(UserInputComponent::CreateInfo{playerTransform}).get();

    physInfo.physInfo.inputIndex = input->inputIndex;
    physInfo.physInfo.transformIndex = playerTransform->index();
    auto phys = ent1->add<PhysicsComponent>(physInfo);

    camera = ent1->add<CameraComponent>(CameraComponent::CreateInfo{playerTransform, input, phys.get()}).get();
    
    const AIBasicComponent::CreateInfo aiInfo{
      0.5f,
      nullptr
    };
    auto ai = ent1->add<AIBasicComponent>(aiInfo);

    const UserDataComponent entData{
      ent1,
      playerTransform,
      nullptr, // потом по идее в этом компоненте будем рисовать основной игровой интерфейс
      phys.get(),
      nullptr, // этот компонент для игрока тоже пригодится
      nullptr,
      ai.get(),
      nullptr,
      nullptr
    };
    auto usrData = ent1->add<UserDataComponent>(entData);
    phys->setUserData(usrData.get());
    
//    ent1->init(&initData);
    player = ent1;
    
    // нужно будет еще задавать позиции, направление и прочее
  }
  
//  initData.dynamic = false; //552c34f8e3469
  const Image &noAnim = textureLoader->image(ResourceID::get("14626771132270"), 0);
  const Texture noAnimT = {
    noAnim,
    0,
    0.0f,
    0.0f
  };
//   std::cout << "Texture: " << noAnim.imageArrayIndex << " " << noAnim.imageArrayLayer << "\n";
  
//   throw std::runtime_error("check");
  
  {
    yacs::entity* ent2 = world.create_entity();
    auto trans = ent2->add<TransformComponent>(simd::vec4(1.0f, 0.9f, 1.0f, 1.0f), simd::vec4(0.0f, 0.0f, 1.0f, 0.0f),
                                               simd::vec4(1.0f, 1.0f, 1.0f, 0.0f));
    auto events = ent2->add<EventComponent>();
//    ent2->add<StateController>();
    physInfo.physInfo.type = PhysicsType(false, BBOX_TYPE, true, false, true, true);
    physInfo.physInfo.inputIndex = UINT32_MAX;
    physInfo.physInfo.transformIndex = trans->index();
    auto phys = ent2->add<PhysicsComponent>(physInfo);
    
    auto comp = ent2->add<GraphicComponent>(GraphicComponent::CreateInfo{noAnimT, trans->index()});

    const UserDataComponent entData{
      ent2,
      trans.get(),
      comp.get(),
      phys.get(),
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      events.get()
    };
    auto usrData = ent2->add<UserDataComponent>(entData);
    ent2->add<InfoComponent>(InfoComponent::CreateInfo{Type::get("Testing entity 1"), usrData.get()});
    phys->setUserData(usrData.get());
    
//    comp->setTexture(noAnim);
  }
  
  {
    yacs::entity* ent3 = world.create_entity();
    auto trans = ent3->add<TransformComponent>(simd::vec4(1.0f, 0.1f, 0.0f, 1.0f), simd::vec4(0.0f, 0.0f, 1.0f, 0.0f),
                                               simd::vec4(1.0f, 1.0f, 1.0f, 0.0f));
    auto events = ent3->add<EventComponent>();

    physInfo.physInfo.inputIndex = UINT32_MAX;
    physInfo.physInfo.transformIndex = trans->index();
    auto phys = ent3->add<PhysicsComponent>(physInfo);
    
    auto comp = ent3->add<GraphicComponent>(GraphicComponent::CreateInfo{noAnimT, trans->index()});

    const UserDataComponent entData{
      ent3,
      trans.get(),
      comp.get(),
      phys.get(),
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      events.get()
    };
    auto usrData = ent3->add<UserDataComponent>(entData);
    ent3->add<InfoComponent>(InfoComponent::CreateInfo{Type::get("Testing entity 2"), usrData.get()});
    phys->setUserData(usrData.get());
  }
  
//  initData.dynamic = true;
  
  {
    yacs::entity* ent4 = world.create_entity();
    auto trans = ent4->add<TransformComponent>(simd::vec4(2.0f, 3.5f, 1.0f, 1.0f), simd::vec4(1.0f, 0.0f, 0.0f, 0.0f),
                                  simd::vec4(1.0f, 1.0f, 1.0f, 0.0f));
    auto events = ent4->add<EventComponent>();
    //ent4->add<InputComponent>();
//     auto states = ent4->add<StateController>();

    const AIInputComponent::CreateInfo aiInfo {
      nullptr,
      trans.get()
    };
    auto input = ent4->add<AIInputComponent>(aiInfo);

    physInfo.physInfo.type = PhysicsType(true, BBOX_TYPE, true, false, true, true);
    physInfo.physInfo.inputIndex = input->inputIndex;
    physInfo.physInfo.transformIndex = trans->index();
    auto phys = ent4->add<PhysicsComponent>(physInfo);
    input->setPhysicsComponent(phys.get());

    auto comp = ent4->add<GraphicComponent>(GraphicComponent::CreateInfo{noAnimT, trans->index()});

    const AnimationComponent::CreateInfo animInfo{
      trans.get(),
      phys.get(),
      comp.get()
    };
    auto anim = ent4->add<AnimationComponent>(animInfo);

    const SoundComponent::CreateInfo soundInfo{
      trans.get(),
      phys.get(),
    };
    auto sound = ent4->add<SoundComponent>(soundInfo);

    const UserDataComponent entData{
      ent4,
      trans.get(),
      comp.get(),
      phys.get(),
      anim.get(),
      nullptr,
      nullptr,
      nullptr,
      events.get()
    };
    auto usrData = ent4->add<UserDataComponent>(entData);
    
    const AIComponent::CreateInfo info{
      0.5f,
      HALF_SECOND,
      nullptr,
      Global::ai()->getBehaviourTreePointer(Type::get("simple_tree")),

//      phys.get(),
//      trans.get(),
      input.get(),
      usrData.get(),

      Type::get("default")
    };
    brainAI = ent4->add<AIComponent>(info).get();
    usrData->aiComponent = brainAI;

    ent4->add<InfoComponent>(InfoComponent::CreateInfo{Type::get("Entity with animation"), usrData.get()});
    phys->setUserData(usrData.get());
    
    const EventData data{
      nullptr,
      nullptr
    };
    
//    const Type t = Type::get("walking");
    // это задавать по идее нужно через стейт
    const SoundComponent::PlayInfo sInfo {
      ResourceID::get("default_sound"),
      400000,
      false,
      true,
      false,
      true,
      {0.0f, 0.0f, 0.0f, 0.0f},
      100.0f,
      1.0f,
      1.0f,
      1.0f
    };
    sound->play(sInfo);

    const AnimationComponent::PlayInfo aInfo {
      ResourceID::get("walking"),
      1.0f,
      1600000,
      true
    };
    anim->play(aInfo);
//     states->registerState(t, false, false, 400000);
//    events->fireEvent(t, data);
//     
//     states->changeState(t);
    //objWithAnimation = ent4;
  }
  
//  initData.dynamic = false;
  
  {
    const size_t objCount = 5000;
              
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dist(-99,99);
    //std::uniform_real_distribution<> distY(0,99);
    for (size_t i = 0; i < objCount; ++i) {
      const simd::vec4 pos = simd::vec4(dist(gen), dist(gen), dist(gen), 1.0f);
      
      yacs::entity* ent = world.create_entity();
      auto trans = ent->add<TransformComponent>(pos, simd::vec4(0.0f, 0.0f, 1.0f, 0.0f), simd::vec4(1.0f, 1.0f, 1.0f, 0.0f));
      auto events = ent->add<EventComponent>();
      auto comp = ent->add<GraphicComponent>(GraphicComponent::CreateInfo{noAnimT, trans->index()});
//      ent->add<StateController>();
      physInfo.physInfo.type = PhysicsType(false, BBOX_TYPE, true, false, false, true);
      physInfo.physInfo.inputIndex = UINT32_MAX;
      physInfo.physInfo.transformIndex = trans->index();
      auto phys = ent->add<PhysicsComponent>(physInfo);

      const UserDataComponent entData{
        ent,
        trans.get(),
        comp.get(),
        phys.get(),
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        events.get()
      };
      auto usrData = ent->add<UserDataComponent>(entData);
      ent->add<InfoComponent>(InfoComponent::CreateInfo{Type::get("Generated entity " + std::to_string(i)), usrData.get()});

      phys->setUserData(usrData.get());
//      comp->setTexture(noAnim);
    }
  }
  
  {
    const size_t lightSize = 500;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dist(-9,9);
    std::uniform_real_distribution<> dist2(0,1);
    
    const simd::vec4 firstPos = simd::vec4(0.0f, 0.4f, 0.0f, 1.0f);
    const float lightRadius = 2.0f;

    {
      yacs::entity* ent = world.create_entity();
      auto trans = ent->add<TransformComponent>(firstPos, simd::vec4(0.0f, 0.0f, 1.0f, 0.0f), simd::vec4(1.0f, 1.0f, 1.0f, 0.0f));
      physInfo.physInfo.type = PhysicsType(false, SPHERE_TYPE, false, false, false, true);
      physInfo.physInfo.inputIndex = UINT32_MAX;
      physInfo.physInfo.transformIndex = trans->index();
      physInfo.physInfo.radius = lightRadius;
      auto phys = ent->add<PhysicsComponent>(physInfo);

      const Light::CreateInfo lightInfo{
        {
          {0.0f, 0.0f, 0.0f, lightRadius},
          {1.0f, 1.0f, 1.0f, 0.1f}
        },
        trans->index()
      };
      auto light = ent->add<Light>(lightInfo);

      const UserDataComponent entData{
        ent,
        trans.get(),
        light.get(),
        phys.get(),
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr
      };
      auto usrData = ent->add<UserDataComponent>(entData);
      ent->add<InfoComponent>(InfoComponent::CreateInfo{Type::get("Generated light " + std::to_string(0)), usrData.get()});
      phys->setUserData(usrData.get());
    }
    
    for (size_t i = 0; i < lightSize; ++i) {
      const simd::vec4 pos = simd::vec4(dist(gen), dist(gen), dist(gen), 1.0f);
      glm::vec3 color = glm::vec3(dist2(gen), dist2(gen), dist2(gen));
      
      yacs::entity* ent = world.create_entity();
      auto trans = ent->add<TransformComponent>(pos, simd::vec4(0.0f, 0.0f, 1.0f, 0.0f), simd::vec4(1.0f, 1.0f, 1.0f, 0.0f));
      physInfo.physInfo.type = PhysicsType(false, SPHERE_TYPE, false, false, true, true);
      physInfo.physInfo.inputIndex = UINT32_MAX;
      physInfo.physInfo.transformIndex = trans->index();
      physInfo.physInfo.radius = lightRadius;
      auto phys = ent->add<PhysicsComponent>(physInfo);

      const Light::CreateInfo lightInfo{
        {
          {0.0f, 0.0f, 0.0f, lightRadius},
          {color.x, color.y, color.z, 0.1f}
        },
        trans->index()
      };
      auto light = ent->add<Light>(lightInfo);

      const UserDataComponent entData{
        ent,
        trans.get(),
        light.get(),
        phys.get(),
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr
      };
      auto usrData = ent->add<UserDataComponent>(entData);
      ent->add<InfoComponent>(InfoComponent::CreateInfo{Type::get("Generated light " + std::to_string(i+1)), usrData.get()});
      phys->setUserData(usrData.get());
    }
  }
}

void HardcodedEntityLoader::createWall(const CreateWallInfo &info) {
  yacs::entity* wall = world.create_entity();

  const PhysicsComponent::CreateInfo physInfo{
    {
      simd::vec4(0.0f, 0.0f, 0.0f, 0.0f),
      7.0f, 80.0f, 0.0f, 0.0f
    },
    {
      false,

      PhysicsType(false, POLYGON_TYPE, true, false, true, true),
      1,     // collisionGroup
      1,     // collisionFilter

      0.5f,  // stairHeight
      //40.0f, // acceleration
      1.0f,  // overbounce
      4.0f,  // groundFriction

      0.0f,  // radius

      UINT32_MAX,
      UINT32_MAX,
      UINT32_MAX,
      UINT32_MAX,
      UINT32_MAX,

      //"boxShape"
      info.shapeType
    },
    nullptr
  };

  auto phys = wall->add<PhysicsComponent>(physInfo);
  const GraphicComponentIndexes::CreateInfo gInfo{
    info.indexOffset,
    info.faceVertices,
    static_cast<uint32_t>(info.faceIndex),
    UINT32_MAX
  };
  auto comp = wall->add<GraphicComponentIndexes>(gInfo);

  auto usrData = wall->add<UserDataComponent>();
  
  const AIBasicComponent::CreateInfo aiInfo{
    info.radius,
    info.vertex,
    usrData.get()
  };
  auto ai = wall->add<AIBasicComponent>(aiInfo).get();

  usrData->entity = wall;
  usrData->trans = nullptr;
  usrData->graphic = comp.get();
  usrData->phys = phys.get();
  usrData->anim = nullptr;
  usrData->decalContainer = nullptr;
  usrData->aiComponent = ai;
  usrData->vertex = info.vertex;
  usrData->events = nullptr;

  const UserDataComponent entData{
    wall,
    nullptr,
    comp.get(),
    phys.get(),
    nullptr,
    nullptr,
    ai,
    info.vertex,
    nullptr
  };
  //auto usrData = wall->add<UserDataComponent>(entData);
  wall->add<InfoComponent>(InfoComponent::CreateInfo{Type::get(info.name), usrData.get()});

  phys->setUserData(usrData.get());
  comp->setTexture(info.wallTexture);
  
//   if (ai->vertex() == nullptr) {
//     throw std::runtime_error("cdefvwdvsabsfbn");
//   }
  
//   info.vertex->addObject(ai);
  
  // как будет выглядеть в итоге фактори для энтити? мне нужно передать информацию для создания во все энтити + информацию в инит
  // может быть нужно запомнить все конструкторы в одной большой структуре, а дальше по ситуации создавать
  // нет, лучше наверное узнавать из парсера карты что у нас определено для конкретного объекта
  
  (void)ai;
}

yacs::entity* HardcodedEntityLoader::getPlayer() const {
  return player;
}

TransformComponent* HardcodedEntityLoader::getPlayerTransform() const {
  return playerTransform;
}

CameraComponent* HardcodedEntityLoader::getCamera() const {
  return camera;
}

UserInputComponent* HardcodedEntityLoader::getInput() const {
  return input;
}

EntityAI* HardcodedEntityLoader::getEntityBrain() const {
  return brainAI;
}

HardcodedMapLoader::HardcodedMapLoader(const CreateInfo &info) : state(0), device(info.device), entityLoader(info.entityLoader), textureLoader(info.loader) {
  vertices = device->create(yavf::BufferCreateInfo::buffer(100*sizeof(Vertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT), VMA_MEMORY_USAGE_GPU_ONLY);
  indices = device->create(yavf::BufferCreateInfo::buffer(100*sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT), VMA_MEMORY_USAGE_GPU_ONLY);
}

HardcodedMapLoader::~HardcodedMapLoader() {
  device->destroy(vertices);
  device->destroy(indices);
}

bool HardcodedMapLoader::canParse(const std::string &key) const {
  return false;
}

bool HardcodedMapLoader::parse(const Modification* mod,
                               const std::string &pathPrefix,
                               const nlohmann::json &data,
                               std::vector<Resource*> &resource,
                               std::vector<ErrorDesc> &errors,
                               std::vector<WarningDesc> &warnings) {
  // здесь мы должны парсить все дополнитеьные данные карт
  // такие как название, настройки, возможно какие то особые варианты загрузки, ну и путь до файла
  // тут наверное будет накладно парсить сразу все карты, да и не нужно
  
  return false;
}

bool HardcodedMapLoader::forget(const ResourceID &name) {
  return false;
}

Resource* HardcodedMapLoader::getParsedResource(const ResourceID &id) {
  return nullptr;
}

const Resource* HardcodedMapLoader::getParsedResource(const ResourceID &id) const {
  return nullptr;
}

bool HardcodedMapLoader::load(const ModificationParser* modifications, const Resource* resource) {
  // грузим карту по имени
  
  return resource->id() == ResourceID::get("default");
}

bool HardcodedMapLoader::unload(const ResourceID &id) {
  // не особ понятно что делаем
  
  return false;
}

void HardcodedMapLoader::end() {
  // тут скорее всего последовательно вызываем лоадеры и у них вызываем end()
  // а после загружаем саму карту
  // нам могут при загрузке потребоваться данные о текстурах и анимациях и прочие вещи
  // скорее всего end() будет вызываться несколько раз? нужно предусмотреть это
  // хотя было бы неплохо не здесь вызывать end()
  // а последовательно вовне
  
//   textureLoader->end();
//   entityLoader->end();
  
  // также тут нужно будет заполнять граф
  
  const std::string path = Global::getGameDir() + "models/box4.obj";
  
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  // в будущем тут у нас должен быть мой собственный загрузчик
  
  std::string err;
  std::string warn;
  const bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str(), nullptr, false);

  if (!err.empty()) { // `err` may contain warning message.
    std::cerr << err << std::endl;
  }

  if (!ret) {
    throw std::runtime_error("Could not load map");
  }
  
  //std::unordered_set<uint32_t> setIndex;
  
  //Type defaultType = Type::get("default");
  
  Graph* graph = Global::ai()->getGraph();
  
  std::vector<Vertex> verts;
  std::vector<uint32_t> globalIndicies;
  // Loop over shapes
  for (size_t s = 0; s < shapes.size(); ++s) {
    size_t index_offset = 0;
//     size_t offset = 0;
    
    std::cout << "Indices: " << shapes[s].mesh.indices.size() << "\n";
    //for (size_t i = 0; i < shapes[s].mesh.indices.size(); i++) std::cout << "Index " << i << ": " << shapes[s].mesh.indices[i].vertex_index << "\n";
    std::cout << "Vertices: " << attrib.vertices.size() / 3 << "\n";
    std::cout << "Num face vertices: " << shapes[s].mesh.num_face_vertices.size() << "\n";
    
    for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); ++f) {
      const size_t fv = shapes[s].mesh.num_face_vertices[f];
      
      RegisterNewShapeInfo shapeInfo{};
      
      if (fv < 3) {
        // является ли отсутствие нормального фейса критической ошибкой?
        // наверное нет, нужно их только пропускать
        Global::console()->printW("Bad face " + std::to_string(f) + " in map");
        //throw std::runtime_error("Bad face in map");
        index_offset += fv;
        continue;
      }
      
      for (size_t i = 0; i < fv; ++i) {
        // собираем verts
        // + собираем индексы
        
        tinyobj::index_t idx = shapes[s].mesh.indices[index_offset+i];
        const simd::vec4 v = simd::vec4(attrib.vertices[3*idx.vertex_index+0], attrib.vertices[3*idx.vertex_index+1], attrib.vertices[3*idx.vertex_index+2], 1.0f);
        shapeInfo.points.push_back(v);
        
//         globalIndicies.push_back(offset);
//         ++offset;
      }
      
      shapeInfo.faces.emplace_back(0.0f, 0.0f, 0.0f, 0.0f);
      // где то еще вычисляем нормаль
      for (size_t i = 0; i < shapeInfo.points.size(); i++) {
        const size_t j = (i+1) % shapeInfo.points.size();
        const size_t k = (i+2) % shapeInfo.points.size();
        
//         PRINT_VEC4("i", shapeInfo.points[i])
//         PRINT_VEC4("k", shapeInfo.points[k])
//         PRINT_VEC4("j", shapeInfo.points[j])

        const simd::vec4 p = shapeInfo.points[j] - shapeInfo.points[i];
        const simd::vec4 q = shapeInfo.points[k] - shapeInfo.points[i];
        const simd::vec4 normal = simd::normalize(simd::cross(p, q));
        
        float arr[4];
        normal.store(arr);
//         PRINT_VEC3("normal", normal)
        //float distance = -(normal.x*v1.x + normal.y*v1.y + normal.z*v1.z);

//         if (!((normal.x != normal.x || normal.y != normal.y || normal.z != normal.z) ||
//               (normal.x == 0.0f && normal.y == 0.0f && normal.z == 0.0f))) {
        if (!((arr[0] != arr[0] || arr[1] != arr[1] || arr[2] != arr[2]) ||
              (arr[0] == 0.0f && arr[1] == 0.0f && arr[2] == 0.0f))) {
          shapeInfo.faces.back() = simd::vec4(arr[0], arr[1], arr[2], glm::uintBitsToFloat(0));
          break;
        }
      }
      
//       PRINT_VEC4("normal", shapeInfo.faces.back())
      
//       throw std::runtime_error("NAN");
      
      simd::vec4 x;
      simd::vec4 y;
      
      const simd::vec4 normal = shapeInfo.faces.back();
      float normalArr[4];
      normal.storeu(normalArr);
        
      if (fast_fabsf(normalArr[0]) < EPSILON && fast_fabsf(normalArr[1]) < EPSILON) {
        x = simd::vec4(1.0f, 0.0f, 0.0f, 0.0f);
        y = simd::vec4(0.0f, 1.0f, 0.0f, 0.0f);
      } else {
        x = simd::normalize(simd::vec4(-normalArr[1], normalArr[0], 0.0f, 0.0f));
        y = simd::normalize(simd::vec4(-normalArr[0]*normalArr[2], -normalArr[1]*normalArr[2], normalArr[0]*normalArr[0] + normalArr[1]*normalArr[1], 0.0f));
//         x = simd::normalize(glm::vec3(-normal.y, normal.x, 0.0f));
//         y = simd::normalize(glm::vec3(-normal.x*normal.z, -normal.y*normal.z, normal.x*normal.x + normal.y*normal.y));
      }
      
//       std::cout << "\n";
//       std::cout << "face " << f << "\n";
//       
//       PRINT_VEC4("normal ", normal)
      
      const simd::vec4 v1 = shapeInfo.points[0];
      for (size_t i = 0; i < shapeInfo.points.size(); ++i) {
        if (i == 0) {
          const Vertex v{
            v1,
            simd::vec4(normalArr[0], normalArr[1], normalArr[2], glm::uintBitsToFloat(f)),
            glm::vec2(0.0f, 0.0f)
          };
          
          verts.push_back(v);
          globalIndicies.push_back(verts.size()-1);
        } else {
          const float a = simd::dot(x, shapeInfo.points[i]-v1);
          const float b = simd::dot(y, shapeInfo.points[i]-v1);
          
          const Vertex v{
            shapeInfo.points[i],
            simd::vec4(normalArr[0], normalArr[1], normalArr[2], glm::uintBitsToFloat(f)),
            glm::vec2(a, b)
          };
          
          verts.push_back(v);
          globalIndicies.push_back(verts.size()-1);
        }
        
        const size_t j = (i+1) % shapeInfo.points.size();

        const simd::vec4 firstPoint = shapeInfo.points[i];
        const simd::vec4 secondPoint = shapeInfo.points[j];

        const simd::vec4 side = simd::cross(secondPoint - firstPoint, shapeInfo.faces[0]);
        const simd::vec4 normSide = simd::normalize(side);
        float arr[4];
        normSide.store(arr);

        shapeInfo.faces.emplace_back(arr[0], arr[1], arr[2], glm::uintBitsToFloat(i));
        // нужно еще написать примерно такой же код для объектов (например, двери)
      }
      
      simd::vec4 shapeMin = shapeInfo.points[0];
      simd::vec4 shapeMax = shapeInfo.points[0];
      simd::vec4 center;
      for (size_t i = 0; i < shapeInfo.points.size(); ++i) {
        center += shapeInfo.points[i];
        shapeMin = simd::min(shapeMin, shapeInfo.points[i]);
        shapeMax = simd::max(shapeMax, shapeInfo.points[i]);
      }
      center /= shapeInfo.points.size();
      
      // так же нужно собрать граф, хотя возможно мы просто передаем какие нибудь данные создателю ии
      
      glm::vec4 normalData;
      glm::vec4 centerData;
      normal.storeu(&normalData.x);
      center.storeu(&centerData.x);
      const GraphVertex data{
        normalData,
        centerData,
        {}
      };
      vertex_t* vertex = graph->addVertex(data);
      
      for (size_t i = 0; i < graph->order(); ++i) {
        vertex_t* vert = graph->vertex(i);
        // вот и проблемы, в таком виде я не знаю точек вершины
        if (vert->objCount() == 0) continue;
        
        const EntityAI* aiVert = vert->at(0);
        //const AIBasicComponent* comp = static_cast<const AIBasicComponent*>(aiVert);
        auto comp = static_cast<const AIBasicComponent*>(aiVert);
        PhysicsComponent* phys = comp->components()->phys;
//        yacs::entity* ent = comp->getEntity();
//
//        if (ent == nullptr) throw std::runtime_error("kjcvsdkvnaldnvsljbn");
//
//        const auto phys = ent->get<PhysicsComponent2>();
        const uint32_t pointsCount = phys->getObjectShapePointsSize();
        const simd::vec4* points = phys->getObjectShapePoints();
        
        simd::vec4 edgePoints[2];
        uint8_t founded = 0;
        for (uint32_t j = 0; j < pointsCount; ++j) {
          for (uint32_t l = 0; l < shapeInfo.points.size(); ++l) {
            if (founded > 1) break;
            
            const size_t nextJ = (j+1)%pointsCount;
            const size_t nextL = (l+1)%shapeInfo.points.size();
            
            const bool config1 = (simd::distance2(points[j], shapeInfo.points[l]) < EPSILON) && (simd::distance2(points[nextJ], shapeInfo.points[nextL]) < EPSILON);
            const bool config2 = (simd::distance2(points[j], shapeInfo.points[nextL]) < EPSILON) && (simd::distance2(points[nextJ], shapeInfo.points[l]) < EPSILON);
            
            if (config1) {
              edgePoints[0] = points[j];
              edgePoints[1] = points[nextJ];
              founded = 10;
            }
            
            if (config2) {
              edgePoints[0] = points[j];
              edgePoints[1] = points[nextJ];
              founded = 10;
            }
            
//             if (simd::distance2(points[j], shapeInfo.points[l]) < EPSILON) {
// //               edgePoints[founded] = points[j];
//               edgePoints[0] = points[j];
// //               ++founded;
//               
//               if (simd::distance2(points[nextJ], shapeInfo.points[nextL]) < EPSILON) {
//                 edgePoints[1] = points[nextJ];
//                 founded = 10;
//               }
//             }
          }
          
          if (founded > 1) break;
        }
        
        if (founded > 1) {
          glm::vec4 dir;
          simd::vec4 simd_dir = simd::normalize(edgePoints[1] - edgePoints[0]);
          simd_dir.storeu(&dir.x);
          
          glm::vec4 aP;
          glm::vec4 bP;
          edgePoints[0].storeu(&aP.x);
          edgePoints[1].storeu(&bP.x);
          
          const EdgeData data{
            false,
            true, // кажется используется только в fake edge
            getAngle(vertex->getVertexData()->normal, vert->getVertexData()->normal),
            glm::distance(vertex->getVertexData()->center, vert->getVertexData()->center),
            simd::distance(edgePoints[0], edgePoints[1]),
            0.0f,
            dir,
            LineSegment(aP, bP)
          };
          
          graph->addEdge(vertex, vert, data);
        }
      }
      
      const std::string shapeName = "WallShape " + std::to_string(f);
      const Type shapeType = Type::get(shapeName);
      Global::physics()->registerShape(shapeType, POLYGON_TYPE, shapeInfo);
      
      const float radius = simd::distance(shapeMin, shapeMax) / 2.0f;
      
      const CreateWallInfo info{
        "Wall " + std::to_string(f),
//         shapeName,
        shapeType,
        index_offset,
        fv,
        f,
        {
          textureLoader->image(ResourceID::get("Rough Block Wall"), 0),
          0,0.0f,0.0f
        },
        radius,
        vertex
      };
      entityLoader->createWall(info);
      
      index_offset += fv;
    }
  }
  
  std::cout << "Created " << graph->order() << " vertices" << "\n";
  const size_t edges = graph->size();
  std::cout << "Created " << edges << " edges" << "\n";
  
  // мы еще должны найти fake edges
  // fake edge работают неверно (хотя возможно они создаются неверно)
   for (size_t i = 0; i < graph->order(); ++i) {
     vertex_t* vert1 = graph->vertex(i);

     for (size_t j = 0; j < vert1->degree(); ++j) {
       edge_t* edge = vert1->edge(j);

       if (edge->getAngle() < PASSABLE_ANGLE) continue;

       vertex_t* vert2 = edge->first() == vert1 ? edge->second() : edge->first();

       for (size_t e = 0; e < vert2->degree(); ++e) {
         edge_t* secondEdge = vert2->edge(e);

         if (secondEdge->isFake()) continue;
         if (secondEdge->first() == vert1 || secondEdge->second() == vert1) continue;

         vertex_t* another = secondEdge->first() == vert1 ? secondEdge->second() : secondEdge->first();
         if (vert1->hasEdge(another)) continue;

         const float angle = getAngle(vert1->getVertexData()->normal, another->getVertexData()->normal);
         if (angle >= PASSABLE_ANGLE) continue;

         glm::vec4 left1, right1, left2, right2;
         edge->getSegment().leftRight(vert1->getVertexData()->center, vert1->getVertexData()->normal, left1, right1);
         secondEdge->getSegment().leftRight(vert1->getVertexData()->center, vert1->getVertexData()->normal, left2, right2);
         float d1 = glm::distance(left1, left2);
         float d2 = glm::distance(right1, right2);

 //         const float height1 = glm::distance(edge->getSegment().a, secondEdge->getSegment().a);
 //         const float height2 = glm::distance(edge->getSegment().a, secondEdge->getSegment().b);
 //         const float height3 = glm::distance(edge->getSegment().b, secondEdge->getSegment().a);
 //         const float height4 = glm::distance(edge->getSegment().b, secondEdge->getSegment().b);
 //         const float height = std::min(std::min(height1, height2), std::min(height3, height4));

         const float width1 = glm::distance(edge->getSegment().a, edge->getSegment().b);
         const float width2 = glm::distance(secondEdge->getSegment().a, secondEdge->getSegment().b);

 //         LineSegment line;
 //
 //         if (height == height1) {
 //           line.a = edge->getSegment().a;
 //           line.b = edge->getSegment().b;
 //         } else if (height == height2) {
 //           line.a = edge->getSegment().a;
 //           line.b = edge->getSegment().b;
 //         } else if (height == height3) {
 //           line.a = edge->getSegment().b;
 //           line.b = edge->getSegment().a;
 //         } else {
 //           line.a = edge->getSegment().b;
 //           line.b = edge->getSegment().a;
 //         }

         const EdgeData data{
           true,
           true, // кажется используется только в fake edge, или не используется вообще
           secondEdge->getAngle(),
           glm::distance(vert1->getVertexData()->center, edge->getSegment().closestPoint(vert1->getVertexData()->center)) +
             glm::distance(another->getVertexData()->center, secondEdge->getSegment().closestPoint(another->getVertexData()->center)),
           std::min(width1, width2),
           d1 > d2 ? d2 : d1,
           d1 > d2 ? glm::normalize(left1 - right1) : glm::normalize(right1 - left1),
           d1 > d2 ? LineSegment(right1, left1) : LineSegment(left1, right1)
         };

         graph->addEdge(vert1, another, data);
       }
     }
   }
  
  std::cout << "Created " << (graph->size() - edges) << " fake edges" << "\n";
  
  // тут нужно перекопировать verts и globalIndicies в буферы
  yavf::Buffer stagingVert(device, yavf::BufferCreateInfo::buffer(verts.size()*sizeof(Vertex), 
                                                                  VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT), 
                           VMA_MEMORY_USAGE_CPU_ONLY);
  yavf::Buffer stagingIndices(device, yavf::BufferCreateInfo::buffer(globalIndicies.size()*sizeof(uint32_t), 
                                                                     VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT), 
                              VMA_MEMORY_USAGE_CPU_ONLY);
  
  memcpy(stagingVert.ptr(), verts.data(), verts.size()*sizeof(Vertex));
  memcpy(stagingIndices.ptr(), globalIndicies.data(), globalIndicies.size()*sizeof(uint32_t));
  
  vertices->recreate(verts.size()*sizeof(Vertex));
  indices->recreate(globalIndicies.size()*sizeof(uint32_t));
  
  yavf::TransferTask* task = device->allocateTransferTask();
  
  task->begin();
  task->copy(&stagingVert, vertices);
  task->copy(&stagingIndices, indices);
  task->end();
  
  task->start();
  task->wait();
  
  device->deallocate(task);
  // это все и по идее
}

void HardcodedMapLoader::clear() {
  // чистим дополнительные данные
}

size_t HardcodedMapLoader::overallState() const {
  // как считать оверал стейт? по количеству разных объектов?
  // нам тогда нужно будет парсить карту предварительно, может ли это быть долго?
  // либо ставить тут условное число
  
  return 100;
}

size_t HardcodedMapLoader::loadingState() const {
  return state;
}

std::string HardcodedMapLoader::hint() const {
  return "Syntesis level geometry";
}

bool HardcodedMapLoader::load(const size_t &index) {
  // нам скорее всего пригодиться грузить карты по какому-то индексу
  // например загрузить первую карту, или например, мы получили номер карты после загрузки
  
  return index == 0;
}

yavf::Buffer* HardcodedMapLoader::mapVertices() const {
  return vertices;
}

yavf::Buffer* HardcodedMapLoader::mapIndices() const {
  return indices;
}
