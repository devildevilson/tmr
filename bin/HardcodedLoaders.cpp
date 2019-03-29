#include "HardcodedLoaders.h"

#include "Globals.h"
#include "Utility.h"

#include "TextureLoader.h"

#include "Components.h"
#include "GraphicComponets.h"
#include "AnimationComponent.h"
#include "AnimationSystem.h"
#include "EventComponent.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>

#include <random>

HardcodedAnimationLoader::HardcodedAnimationLoader(const CreateInfo &info) : textureLoader(info.textureLoader) {}

HardcodedAnimationLoader::~HardcodedAnimationLoader() {}

bool HardcodedAnimationLoader::parse(const std::string &pathPrefix, 
                                     const std::string &forcedNamePrefix, 
                                     const nlohmann::json &data, 
                                     std::vector<Resource*> &resource, 
                                     std::vector<ErrorDesc> &errors, 
                                     std::vector<WarningDesc> &warnings) {
  // тут нужно загрузить данные из json
  // в общем то ничего необычного, 
  // кроме того что мне нужно еще придумать как реализовать анимацию передвигающихся текстур
  return false;
}

bool HardcodedAnimationLoader::forget(const std::string &name) {
  return false;
}

std::unordered_map<std::string, Resource*> HardcodedAnimationLoader::getLoadedResource() {
  return std::unordered_map<std::string, Resource*>();
}

std::unordered_map<std::string, Conflict*> HardcodedAnimationLoader::getConflicts() {
  return std::unordered_map<std::string, Conflict*>();
}

bool HardcodedAnimationLoader::load(const std::string &name) {
  return false;
}

bool HardcodedAnimationLoader::unload(const std::string &name) {
  return false;
}

void HardcodedAnimationLoader::end() {
  const auto &texture1  = textureLoader->getTextures("texture");
  const auto &texture2  = textureLoader->getTextures("Rough Block Wall");
  const auto &texture3  = textureLoader->getTextures("552c34f8e3469");
  const auto &texture4  = textureLoader->getTextures("14626771132270");
  const auto &texture5  = textureLoader->getTextures("7037.970");
  const auto &texture6  = textureLoader->getTextures("n");
  const auto &texture7  = textureLoader->getTextures("ne");
  const auto &texture8  = textureLoader->getTextures("e");
  const auto &texture9  = textureLoader->getTextures("se");
  const auto &texture10 = textureLoader->getTextures("s");
  const auto &texture11 = textureLoader->getTextures("sw");
  const auto &texture12 = textureLoader->getTextures("w");
  const auto &texture13 = textureLoader->getTextures("nw");
  
  const AnimationSystem::AnimationCreateInfoNewFrames animationInfo{
    false,
    false,
    true,
    400000,
    "walking",
    {
      {
        { texture6[0], 0.0f, 0.0f },
        { texture7[0], 0.0f, 0.0f },
        { texture8[0], 0.0f, 0.0f },
        { texture9[0], 0.0f, 0.0f },
        { texture10[0], 0.0f, 0.0f },
        { texture11[0], 0.0f, 0.0f },
        { texture12[0], 0.0f, 0.0f },
        { texture13[0], 0.0f, 0.0f },
      },
      {
        { texture6[1], 0.0f, 0.0f },
        { texture7[1], 0.0f, 0.0f },
        { texture8[1], 0.0f, 0.0f },
        { texture9[1], 0.0f, 0.0f },
        { texture10[1], 0.0f, 0.0f },
        { texture11[1], 0.0f, 0.0f },
        { texture12[1], 0.0f, 0.0f },
        { texture13[1], 0.0f, 0.0f },
      },
      {
        { texture6[2], 0.0f, 0.0f },
        { texture7[2], 0.0f, 0.0f },
        { texture8[2], 0.0f, 0.0f },
        { texture9[2], 0.0f, 0.0f },
        { texture10[2], 0.0f, 0.0f },
        { texture11[2], 0.0f, 0.0f },
        { texture12[2], 0.0f, 0.0f },
        { texture13[2], 0.0f, 0.0f },
      },
      {
        { texture6[3], 0.0f, 0.0f },
        { texture7[3], 0.0f, 0.0f },
        { texture8[3], 0.0f, 0.0f },
        { texture9[3], 0.0f, 0.0f },
        { texture10[3], 0.0f, 0.0f },
        { texture11[3], 0.0f, 0.0f },
        { texture12[3], 0.0f, 0.0f },
        { texture13[3], 0.0f, 0.0f },
      }
    }
  };
  Global::animations()->createAnimation(animationInfo);
}

void HardcodedAnimationLoader::clear() {
  
}

size_t HardcodedAnimationLoader::overallState() const {
  
}

size_t HardcodedAnimationLoader::loadingState() const {
  
}

std::string HardcodedAnimationLoader::hint() const {
  
}

HardcodedEntityLoader::HardcodedEntityLoader(const InitData &data) {
  TransformComponent::setContainer(data.transforms);
  GraphicComponent::setContainer(data.matrices);
  GraphicComponent::setContainer(data.rotationDatas);
  GraphicComponent::setContainer(data.textureContainer);
  AnimationComponent::setStateContainer(data.stateContainer);
  InputComponent::setContainer(data.inputs);
  PhysicsComponent2::setContainer(data.externalDatas);
  
  state = 5504;
}

HardcodedEntityLoader::HardcodedEntityLoader(TextureLoader* textureLoader) : player(nullptr), playerTransform(nullptr), camera(nullptr), input(nullptr), textureLoader(textureLoader) {
  state = 5504;
}

HardcodedEntityLoader::~HardcodedEntityLoader() {
  
}

bool HardcodedEntityLoader::parse(const std::string &pathPrefix, 
                                  const std::string &forcedNamePrefix, 
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

bool HardcodedEntityLoader::forget(const std::string &name) {
  // тут мы просто забываем старые ресурсы чтобы потом мы не могли загрузить их по имени
  // например когда мы отключаем мод
  
  return false;
}

std::unordered_map<std::string, Resource*> HardcodedEntityLoader::getLoadedResource() {
  return std::unordered_map<std::string, Resource*>();
}

std::unordered_map<std::string, Conflict*> HardcodedEntityLoader::getConflicts() {
  return std::unordered_map<std::string, Conflict*>();
}

bool HardcodedEntityLoader::load(const std::string &name) {
  // здесь мы по метоинформации загружаем необходимый ресурс
  // либо мы помечаем этот ресурс к загрузке
  // ну в общем мы делаем какие то вещи которые подходят для конкретного типа ресурса
  
  // здесь это не будет работать
  // хотя может понадобиться для загрузки фабрики
  
  // в будущем здесь (в подобном классе) будут создваться
  // игровые объекты разных форматов, как мне сделать для них возможность создания?
  
  return false;
}

bool HardcodedEntityLoader::unload(const std::string &name) {
  // это может ли потребоваться?
  
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
  
  {
    const RegisterNewShapeInfo info{
      {},
      {glm::vec4(0.5f, 0.5f, 0.5f, 0.0f)}
    };
    
    Global::physics()->registerShape("boxShape", BBOX_TYPE, info);
  }
  
  InitComponents initData{
    "boxShape",
    true,
    BBOX_TYPE
  };
  
  {
    yacs::Entity* ent1 = world.createEntity();
    playerTransform = ent1->assign<TransformComponent>(glm::vec3(1.0f, 5.0f, 2.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f)).get();
    
//     std::cout << "HardcodedEntityLoader::create 01" << "\n";
    
//     ent1->assign<InputComponent>();
    
    input = ent1->assign<UserInputComponent>(glm::vec3(0.0f, 0.0f, 1.0f)).get();
    camera = ent1->assign<CameraComponent>().get();
    
//     std::cout << "input ptr " << input << "\n";
    
    ent1->assign<PhysicsComponent2>();
    
    ent1->init(&initData);
    player = ent1;
  }
  
  initData.dynamic = false; //552c34f8e3469
  const Texture &noAnim = textureLoader->getTexture("14626771132270", 0);
//   std::cout << "Texture: " << noAnim.imageArrayIndex << " " << noAnim.imageArrayLayer << "\n";
  
//   throw std::runtime_error("check");
  
  {
    yacs::Entity* ent2 = world.createEntity();
    ent2->assign<TransformComponent>(glm::vec3(1.0f, 0.9f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
    ent2->assign<EventComponent>();
    ent2->assign<StateController>();
    ent2->assign<PhysicsComponent2>();
    ent2->assign<InfoComponent>(Type::get("Testing entity 1"));
    
    auto comp = ent2->assign<GraphicComponent>();
    
    ent2->init(&initData);
    
    comp->setTexture(noAnim);
  }
  
  {
    yacs::Entity* ent3 = world.createEntity();
    ent3->assign<TransformComponent>(glm::vec3(1.0f, -0.1f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
    ent3->assign<EventComponent>();
    ent3->assign<StateController>();
    ent3->assign<PhysicsComponent2>();
    ent3->assign<InfoComponent>(Type::get("Testing entity 2"));
    
    auto comp = ent3->assign<GraphicComponent>();
    ent3->init(&initData);
    
    comp->setTexture(noAnim);
  }
  
  initData.dynamic = true;
  
  {
    yacs::Entity* ent4 = world.createEntity();
    ent4->assign<TransformComponent>(glm::vec3(2.0f, 3.5f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
    auto events = ent4->assign<EventComponent>();
    ent4->assign<InputComponent>();
    auto states = ent4->assign<StateController>();
    ent4->assign<PhysicsComponent2>();
    ent4->assign<InfoComponent>(Type::get("Entity with animation"));
    auto anim = ent4->assign<AnimationComponent>();
    // тут должен быть AnimationComponent
    auto comp = ent4->assign<GraphicComponent>();
//     comp->setTexture(textureLoader->getTexture("n", 0));
//     ai = ent4->assign<LoneAi>(tree, 1, 200000).get();
    ent4->init(&initData);
    
    const EventData data{
      nullptr,
      nullptr
    };
    
    const Type t = Type::get("walking");
    anim->setAnimation(t, "walking");
    states->registerState(t, false, false, 400000);
    events->fireEvent(t, data);
//     
//     states->changeState(t);
    //objWithAnimation = ent4;
  }
  
  initData.dynamic = false;
  
  {
    const size_t objCount = 5000;
              
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dist(-99,99);
    //std::uniform_real_distribution<> distY(0,99);
    for (size_t i = 0; i < objCount; ++i) {
      glm::vec3 pos = glm::vec3(dist(gen), dist(gen), dist(gen));
      
      yacs::Entity* ent = world.createEntity();
      ent->assign<TransformComponent>(pos, glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
      ent->assign<EventComponent>();
      ent->assign<StateController>();
      ent->assign<PhysicsComponent2>();
      ent->assign<InfoComponent>(Type::get("Generated entity " + std::to_string(i)));
      auto comp = ent->assign<GraphicComponent>();
      ent->init(&initData);
      
      comp->setTexture(noAnim);
    }
  }
  
  {
    const size_t lightSize = 500;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dist(-9,9);
    std::uniform_real_distribution<> dist2(0,1);
    
    glm::vec3 firstPos = glm::vec3(0.0f, 0.4f, 0.0f);
    
        yacs::Entity* ent = world.createEntity();
    ent->assign<TransformComponent>(firstPos, glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
    ent->assign<Light>(2.0f, 0.1f, glm::vec3(1.0f, 1.0f, 1.0f));
    ent->assign<InfoComponent>(Type::get("Generated light " + std::to_string(0)));
    ent->init(nullptr);
    
    for (size_t i = 0; i < lightSize; ++i) {
      glm::vec3 pos = glm::vec3(dist(gen), dist(gen), dist(gen));
      glm::vec3 color = glm::vec3(dist2(gen), dist2(gen), dist2(gen));
      
            yacs::Entity* ent = world.createEntity();
      ent->assign<TransformComponent>(pos, glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
      ent->assign<Light>(2.0f, 0.1f, color);
      ent->assign<InfoComponent>(Type::get("Generated light " + std::to_string(i+1)));
      ent->init(nullptr);
    }
  }
}

void HardcodedEntityLoader::createWall(const CreateWallInfo &info) {
  yacs::Entity* wall = world.createEntity();
  
  wall->assign<InfoComponent>(Type::get(info.name));
  wall->assign<PhysicsComponent2>();
  auto comp = wall->assign<GraphicComponentIndexes, GraphicComponent>(info.indexOffset, info.faceVertices, info.faceIndex);
  
  InitComponents init{
    info.shapeName,
    false,
    POLYGON_TYPE
  };

  wall->init(&init);
  
  comp->setTexture(info.wallTexture);
}

yacs::Entity* HardcodedEntityLoader::getPlayer() const {
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

HardcodedMapLoader::HardcodedMapLoader(const CreateInfo &info) : state(0), device(info.device), entityLoader(info.entityLoader), textureLoader(info.loader) {
  vertices = device->create(yavf::BufferCreateInfo::buffer(100*sizeof(Vertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT), VMA_MEMORY_USAGE_GPU_ONLY);
  indices = device->create(yavf::BufferCreateInfo::buffer(100*sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT), VMA_MEMORY_USAGE_GPU_ONLY);
}

HardcodedMapLoader::~HardcodedMapLoader() {
  device->destroy(vertices);
  device->destroy(indices);
}

bool HardcodedMapLoader::parse(const std::string &pathPrefix, 
            const std::string &forcedNamePrefix, 
            const nlohmann::json &data, 
            std::vector<Resource*> &resource, 
            std::vector<ErrorDesc> &errors, 
            std::vector<WarningDesc> &warnings) {
  // здесь мы должны парсить все дополнитеьные данные карт
  // такие как название, настройки, возможно какие то особые варианты загрузки, ну и путь до файла
  // тут наверное будет накладно парсить сразу все карты, да и не нужно
  
  return false;
}

bool HardcodedMapLoader::forget(const std::string &name) {
  return false;
}

std::unordered_map<std::string, Resource*> HardcodedMapLoader::getLoadedResource() {
  return std::unordered_map<std::string, Resource*>();
}

std::unordered_map<std::string, Conflict*> HardcodedMapLoader::getConflicts() {
  return std::unordered_map<std::string, Conflict*>();
}

bool HardcodedMapLoader::load(const std::string &name) {
  // грузим карту по имени
  
  return name.compare("default") == 0;
}

bool HardcodedMapLoader::unload(const std::string &name) {
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
  const bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, path.c_str(), nullptr, false);

  if (!err.empty()) { // `err` may contain warning message.
    std::cerr << err << std::endl;
  }

  if (!ret) {
    throw std::runtime_error("Could not load map");
  }
  
  //std::unordered_set<uint32_t> setIndex;
  
  //Type defaultType = Type::get("default");
  
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
        const glm::vec4 v = glm::vec4(attrib.vertices[3*idx.vertex_index+0], attrib.vertices[3*idx.vertex_index+1], attrib.vertices[3*idx.vertex_index+2], 1.0f);
        shapeInfo.points.push_back(v);
        
//         globalIndicies.push_back(offset);
//         ++offset;
      }
      
      shapeInfo.faces.push_back(glm::vec4(0.0f, 0.0f, 0.0f , 0.0f));
      // где то еще вычисляем нормаль
      for (size_t i = 0; i < shapeInfo.points.size(); i++) {
        const size_t j = (i+1) % shapeInfo.points.size();
        const size_t k = (i+2) % shapeInfo.points.size();
        
//         PRINT_VEC4("i", shapeInfo.points[i])
//         PRINT_VEC4("k", shapeInfo.points[k])
//         PRINT_VEC4("j", shapeInfo.points[j])

        const glm::vec4 p = shapeInfo.points[j] - shapeInfo.points[i];
        const glm::vec4 q = shapeInfo.points[k] - shapeInfo.points[i];
        const glm::vec3 normal = glm::normalize(glm::cross(glm::vec3(p), glm::vec3(q)));
//         PRINT_VEC3("normal", normal)
        //float distance = -(normal.x*v1.x + normal.y*v1.y + normal.z*v1.z);

        if (!((normal.x != normal.x || normal.y != normal.y || normal.z != normal.z) ||
              (normal.x == 0.0f && normal.y == 0.0f && normal.z == 0.0f))) {
          shapeInfo.faces.back() = glm::vec4(normal, glm::uintBitsToFloat(0));
          break;
        }
      }
      
//       PRINT_VEC4("normal", shapeInfo.faces.back())
      
//       throw std::runtime_error("NAN");
      
      glm::vec3 x;
      glm::vec3 y;
      
      const glm::vec4 normal = shapeInfo.faces.back();
      if (fast_fabsf(normal.x) < EPSILON && fast_fabsf(normal.y) < EPSILON) {
        x = glm::vec3(1.0f, 0.0f, 0.0f);
        y = glm::vec3(0.0f, 1.0f, 0.0f);
      } else {
        x = glm::normalize(glm::vec3(-normal.y, normal.x, 0.0f));
        y = glm::normalize(glm::vec3(-normal.x*normal.z, -normal.y*normal.z, normal.x*normal.x + normal.y*normal.y));
      }
      
//       float a1 = glm::dot(x, v2-v1);
//       float b1 = glm::dot(y, v2-v1);
//       float a2 = glm::dot(x, v3-v1);
//       float b2 = glm::dot(y, v3-v1);
      
      const glm::vec3 v1 = glm::vec3(shapeInfo.points[0]);
      for (size_t i = 0; i < shapeInfo.points.size(); ++i) {
        if (i == 0) {
          const Vertex v{
            glm::vec4(v1, 1.0f),
            glm::vec4(normal.x, normal.y, normal.z, glm::uintBitsToFloat(f)),
            glm::vec2(0.0f, 0.0f)
          };
          
          verts.push_back(v);
          globalIndicies.push_back(verts.size()-1);
        } else {
          float a = glm::dot(x, glm::vec3(shapeInfo.points[i])-v1);
          float b = glm::dot(y, glm::vec3(shapeInfo.points[i])-v1);
          
          const Vertex v{
            shapeInfo.points[i],
            glm::vec4(normal.x, normal.y, normal.z, glm::uintBitsToFloat(f)),
            glm::vec2(a, b)
          };
          
          verts.push_back(v);
          globalIndicies.push_back(verts.size()-1);
        }
        
        const size_t j = (i+1) % shapeInfo.points.size();

        const glm::vec3 firstPoint = glm::vec3(shapeInfo.points[i]);
        const glm::vec3 secondPoint = glm::vec3(shapeInfo.points[j]);

        const glm::vec3 side = glm::cross(secondPoint - firstPoint, glm::vec3(shapeInfo.faces[0]));
        const glm::vec3 normSide = glm::normalize(side);

        shapeInfo.faces.push_back(glm::vec4(normSide, glm::uintBitsToFloat(i)));
        // нужно еще написать примерно такой же код для объектов (например, двери)
      }
      
      // так же нужно собрать граф, хотя возможно мы просто передаем какие нибудь данные создателю ии
      
      const std::string shapeName = "WallShape " + std::to_string(f);
      Global::physics()->registerShape(shapeName, POLYGON_TYPE, shapeInfo);
      
      const CreateWallInfo info{
        "Wall " + std::to_string(f),
        shapeName,
        index_offset,
        fv,
        f,
        textureLoader->getTexture("Rough Block Wall", 0)
      };
      entityLoader->createWall(info);
      
      index_offset += fv;
    }
  }
  
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
