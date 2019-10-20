#ifndef HARDCODED_LOADERS_H
#define HARDCODED_LOADERS_H

#include "Loader.h"
#include "ResourceParser.h"
//#include "Manager.h"

#include "EntityComponentSystem.h"
#include "ArrayInterface.h"
#include "PhysicsTemporary.h"
#include "PhysicsUtils.h"
#include "RenderStructures.h"
#include "Type.h"

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
class HardcodedAnimationLoader : public Loader, public ResourceParser {
public:
  struct CreateInfo {
    ImageLoader* textureLoader;
  };
  
  HardcodedAnimationLoader(const CreateInfo &info);
  ~HardcodedAnimationLoader();

  bool canParse(const std::string &key) const override;
  
  bool parse(const Modification* mod,
             const std::string &pathPrefix,
             const nlohmann::json &data, 
             std::vector<Resource*> &resource, 
             std::vector<ErrorDesc> &errors, 
             std::vector<WarningDesc> &warnings) override;
  bool forget(const ResourceID &name) override;

  Resource* getParsedResource(const ResourceID &id) override;
  const Resource* getParsedResource(const ResourceID &id) const override;
  
  bool load(const ModificationParser* modifications, const Resource* resource) override;
  bool unload(const ResourceID &id) override;
  void end() override;
  
  void clear() override;
  
  size_t overallState() const override;
  size_t loadingState() const override;
  std::string hint() const override;
  
  
private:
  size_t state;
  std::atomic<size_t> accumulatedState;
  
//   AnimationSystem* system;
  ImageLoader* textureLoader;
};

// пачка материалов
class HardcodedMaterialLoader : public Loader, public ResourceParser {
public:
  
private:
  
};

class vertex_t;

// скорее всего количество данных для плоскости увеличится очень сильно
struct CreateWallInfo {
  std::string name;
//   std::string shapeName;
  Type shapeType;
  
  size_t indexOffset;
  size_t faceVertices;
  size_t faceIndex;
  
  // TextureData
  Texture wallTexture;
  float radius;
  vertex_t* vertex;
};

class EntityAI;

// нужно же добавить ResourceParser 
class HardcodedEntityLoader : public Loader, public ResourceParser {
public:
  HardcodedEntityLoader(ImageLoader* textureLoader);
  ~HardcodedEntityLoader();

  bool canParse(const std::string &key) const override;
  
  bool parse(const Modification* mod,
             const std::string &pathPrefix,
             const nlohmann::json &data,
             std::vector<Resource*> &resource,
             std::vector<ErrorDesc> &errors,
             std::vector<WarningDesc> &warnings) override;
  bool forget(const ResourceID &name) override;

  Resource* getParsedResource(const ResourceID &id) override;
  const Resource* getParsedResource(const ResourceID &id) const override;

  bool load(const ModificationParser* modifications, const Resource* resource) override;
  bool unload(const ResourceID &id) override;
  void end() override;
  
  void clear() override;
  
  size_t overallState() const override;
  size_t loadingState() const override;
  std::string hint() const override;
  
  // тут в будущем по идее должен быть доступ к фабрикам
  // ну то есть к методам которые создадут определенный тип противника
  // а сейчас мы просто пихнем создание в один метод
  // как инициализировать это дело?
  // мне нужны контейнеры с данными
  
  void create();
  yacs::entity* create(const Type &type, yacs::entity* parent, const UniversalDataContainer* container);
  void createWall(const CreateWallInfo &info);
  
  yacs::entity* getPlayer() const;
  TransformComponent* getPlayerTransform() const;
  CameraComponent* getCamera() const;
  UserInputComponent* getInput() const;
  EntityAI* getEntityBrain() const;
private:
  size_t state;
  std::atomic<size_t> accumulatedState;
  
  yacs::entity* player;
  TransformComponent* playerTransform;
  CameraComponent* camera;
  UserInputComponent* input;
  
  EntityAI* brainAI;
  
  ImageLoader* textureLoader;
  
  yacs::world world;
  
  std::unordered_map<Type, const yacs::entity*> types;
};

class HardcodedMapLoader : public Loader, public ResourceParser {
public:
  struct CreateInfo {
    yavf::Device* device;
    
    HardcodedEntityLoader* entityLoader;
    ImageLoader* loader;
  };
  
  HardcodedMapLoader(const CreateInfo &info);
  ~HardcodedMapLoader();

  bool canParse(const std::string &key) const override;

  bool parse(const Modification* mod,
             const std::string &pathPrefix,
             const nlohmann::json &data,
             std::vector<Resource*> &resource,
             std::vector<ErrorDesc> &errors,
             std::vector<WarningDesc> &warnings) override;
  bool forget(const ResourceID &name) override;

  Resource* getParsedResource(const ResourceID &id) override;
  const Resource* getParsedResource(const ResourceID &id) const override;

  bool load(const ModificationParser* modifications, const Resource* resource) override;
  bool unload(const ResourceID &id) override;
  void end() override;
  
  void clear() override;
  
  size_t overallState() const override;
  size_t loadingState() const override;
  std::string hint() const override;

  bool load(const size_t &index);
  
  yavf::Buffer* mapVertices() const;
  yavf::Buffer* mapIndices() const;
private:
  std::atomic<size_t> state;
  
  yavf::Device* device;
  
  // тут тоже должен быть YACS::World
  // то есть нужно его передавть по указателю
  // или совместить загрузку энтити с загрузкой карты?
  // мне кажется что совмещение - плохая идея
  
  // вот что: из загрузчика карты мы должны рассылать команды ЭнтитиЛоадеру,
  // который в свою очередь будет грузить необходимые нам Энтити,
  // в которые входят и плоскости карты, то есть туда мы передаем 
  // данные с карты, например, позиции, текстурки и прочие уникальные для конкретного энтити данные
  // как правильно задавать уникальные данные? наверное я отвечу на этот вопрос только тогда когда у меня будет 
  // хоть какая то КАРТА а не obj файл
  
  // нужно организовать виртуальный класс для этого
  HardcodedEntityLoader* entityLoader;
  ImageLoader* textureLoader;
  
  // здесь у нас буду храниться все вершины карты
  yavf::Buffer* vertices;
  yavf::Buffer* indices;
};

#endif
