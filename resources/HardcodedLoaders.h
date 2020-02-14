#ifndef HARDCODED_LOADERS_H
#define HARDCODED_LOADERS_H

// #include "Loader.h"
// #include "ResourceParser.h"
//#include "Manager.h"

#include "EntityComponentSystem.h"
#include "ArrayInterface.h"
#include "PhysicsTemporary.h"
#include "PhysicsUtils.h"
#include "RenderStructures.h"
#include "Type.h"
#include "entity_loader.h"
#include "image_loader.h"

#include <atomic>

// так я медленно но верно подхожу к тому моменту когда мне нужно будет
// проработать загрузчик ресурсов, но я не могу до сих пор понять как он будет выглядеть
// он должен сначала распарсить json файлы, составить дерево зависимостей, где
// уровень будет корнем этого дерева и от него я должен понять какие конкретно ресурсы мне нужно загрузить
// затем загрузить только то что нужно конкретному уровню, и удалить дерево
// при загрузке следующего уровня, алгоритм повторяется 
// если в дереве есть ошибки загрузчик должен четко дать понять, сколько ошибок при парсинге
// типы ошибок, что еще?

class TransformComponent;
class CameraComponent;
class UserInputComponent;
class ImageLoader;
class AnimationSystem;
class UniversalDataContainer;

namespace yavf {
  class Buffer;
  class Device;
}

// как сделать анимацию двигающейся текстуры???
// class HardcodedAnimationLoader : public Loader, public ResourceParser {
// public:
//   struct CreateInfo {
//     ImageLoader* textureLoader;
//   };
//   
//   HardcodedAnimationLoader(const CreateInfo &info);
//   ~HardcodedAnimationLoader();
// 
//   bool canParse(const std::string &key) const override;
//   
//   bool parse(const Modification* mod,
//              const std::string &pathPrefix,
//              const nlohmann::json &data, 
//              std::vector<Resource*> &resource, 
//              std::vector<ErrorDesc> &errors, 
//              std::vector<WarningDesc> &warnings) override;
//   bool forget(const ResourceID &name) override;
// 
//   Resource* getParsedResource(const ResourceID &id) override;
//   const Resource* getParsedResource(const ResourceID &id) const override;
//   
//   bool load(const ModificationParser* modifications, const Resource* resource) override;
//   bool unload(const ResourceID &id) override;
//   void end() override;
//   
//   void clear() override;
//   
//   size_t overallState() const override;
//   size_t loadingState() const override;
//   std::string hint() const override;
//   
//   
// private:
//   size_t state;
//   std::atomic<size_t> accumulatedState;
//   
// //   AnimationSystem* system;
//   ImageLoader* textureLoader;
// };
// 
// // пачка материалов
// class HardcodedMaterialLoader : public Loader, public ResourceParser {
// public:
//   
// private:
//   
// };
// 
// class vertex_t;
// 
// // скорее всего количество данных для плоскости увеличится очень сильно
// struct CreateWallInfo {
//   std::string name;
//   Type shapeType;
//   
//   size_t indexOffset;
//   size_t faceVertices;
//   size_t faceIndex;
//   
//   Texture wallTexture;
//   float radius;
//   vertex_t* vertex;
// };
// 
// class EntityAI;
// 
// // нужно же добавить ResourceParser 
// class HardcodedEntityLoader : public Loader, public ResourceParser {
// public:
//   HardcodedEntityLoader(ImageLoader* textureLoader);
//   ~HardcodedEntityLoader();
// 
//   bool canParse(const std::string &key) const override;
//   
//   bool parse(const Modification* mod,
//              const std::string &pathPrefix,
//              const nlohmann::json &data,
//              std::vector<Resource*> &resource,
//              std::vector<ErrorDesc> &errors,
//              std::vector<WarningDesc> &warnings) override;
//   bool forget(const ResourceID &name) override;
// 
//   Resource* getParsedResource(const ResourceID &id) override;
//   const Resource* getParsedResource(const ResourceID &id) const override;
// 
//   bool load(const ModificationParser* modifications, const Resource* resource) override;
//   bool unload(const ResourceID &id) override;
//   void end() override;
//   
//   void clear() override;
//   
//   size_t overallState() const override;
//   size_t loadingState() const override;
//   std::string hint() const override;
//   
//   // тут в будущем по идее должен быть доступ к фабрикам
//   // ну то есть к методам которые создадут определенный тип противника
//   // а сейчас мы просто пихнем создание в один метод
//   // как инициализировать это дело?
//   // мне нужны контейнеры с данными
//   
//   void create();
//   yacs::entity* create(const Type &type, yacs::entity* parent, const UniversalDataContainer* container);
//   void createWall(const CreateWallInfo &info);
//   
//   yacs::entity* getPlayer() const;
//   TransformComponent* getPlayerTransform() const;
//   CameraComponent* getCamera() const;
//   UserInputComponent* getInput() const;
//   EntityAI* getEntityBrain() const;
// private:
//   size_t state;
//   std::atomic<size_t> accumulatedState;
//   
//   yacs::entity* player;
//   TransformComponent* playerTransform;
//   CameraComponent* camera;
//   UserInputComponent* input;
//   
//   EntityAI* brainAI;
//   
//   ImageLoader* textureLoader;
//   
//   yacs::world world;
//   
//   std::unordered_map<Type, const yacs::entity*> types;
// };
// 
// class EntityLoader;

class HardcodedMapLoader {
public:
  struct CreateInfo {
    yavf::Device* device;
    
    //HardcodedEntityLoader* entityLoader;
    devils_engine::resources::entity_loader* entityLoader;
    devils_engine::resources::image_loader* loader;
  };
  
  HardcodedMapLoader(const CreateInfo &info);
  ~HardcodedMapLoader();

//   bool canParse(const std::string &key) const override;
// 
//   bool parse(const Modification* mod,
//              const std::string &pathPrefix,
//              const nlohmann::json &data,
//              std::vector<Resource*> &resource,
//              std::vector<ErrorDesc> &errors,
//              std::vector<WarningDesc> &warnings) override;
//   bool forget(const ResourceID &name) override;

//   Resource* getParsedResource(const ResourceID &id) override;
//   const Resource* getParsedResource(const ResourceID &id) const override;

  bool load(const devils_engine::utils::id &id);
  bool unload(const devils_engine::utils::id &id);
  void end();
  
  void clear();

  bool load(const size_t &index);
  
  yavf::Buffer* mapVertices() const;
  yavf::Buffer* mapIndices() const;
  
  yacs::entity* getPlayer() const;
  TransformComponent* getPlayerTransform() const;
  CameraComponent* getCamera() const;
  UserInputComponent* getInput() const;
  yacs::entity* getEntityBrain() const;
private:
  std::atomic<size_t> state;
  
  yavf::Device* device;
  
  yacs::entity* player;
  TransformComponent* playerTransform;
  CameraComponent* camera;
  UserInputComponent* input;
  
  yacs::entity* brain_ent;
  
  // нужно организовать виртуальный класс для этого
  devils_engine::resources::entity_loader* entityLoader;
  devils_engine::resources::image_loader* loader;
  
  // здесь у нас буду храниться все вершины карты
  yavf::Buffer* vertices;
  yavf::Buffer* indices;
};

#endif
