#ifndef HARDCODED_LOADERS_H
#define HARDCODED_LOADERS_H

#include "Loader.h"
#include "Manager.h"

#include "EntityComponentSystem.h"
#include "ArrayInterface.h"
#include "PhysicsTemporary.h"
#include "PhysicsUtils.h"
#include "RenderStructures.h"

#include <atomic>

class TransformComponent;
class CameraComponent;
class UserInputComponent;
class TextureLoader;
class AnimationSystem;

namespace yavf {
  class Buffer;
  class Device;
}

// как сделать анимацию двигающейся текстуры???
class HardcodedAnimationLoader : public Loader, public ResourceParser {
public:
  struct CreateInfo {
//     AnimationSystem* system;
    TextureLoader* textureLoader;
  };
  
  HardcodedAnimationLoader(const CreateInfo &info);
  ~HardcodedAnimationLoader();
  
  bool parse(const std::string &pathPrefix, 
             const std::string &forcedNamePrefix, 
             const nlohmann::json &data, 
             std::vector<Resource*> &resource, 
             std::vector<ErrorDesc> &errors, 
             std::vector<WarningDesc> &warnings) override;
  bool forget(const std::string &name) override;

  std::unordered_map<std::string, Resource*> getLoadedResource() override;
  std::unordered_map<std::string, Conflict*> getConflicts() override;
  
  bool load(const std::string &name) override;
  bool unload(const std::string &name) override;
  void end() override;
  
  void clear() override;
  
  size_t overallState() const override;
  size_t loadingState() const override;
  std::string hint() const override;
  
  
private:
  size_t state;
  std::atomic<size_t> accumulatedState;
  
//   AnimationSystem* system;
  TextureLoader* textureLoader;
};

// скорее всего количество данных для плоскости увеличится очень сильно
struct CreateWallInfo {
  std::string name;
  std::string shapeName;
  
  size_t indexOffset;
  size_t faceVertices;
  size_t faceIndex;
  
  Texture wallTexture;
};

// нужно же добавить ResourceParser 
class HardcodedEntityLoader : public Loader, public ResourceParser {
public:
  struct InitData {
    Container<Transform>* transforms;
    Container<InputData>* inputs;
    Container<RotationData>* rotationDatas;
    Container<simd::mat4>* matrices;
    Container<ExternalData>* externalDatas;
    Container<TextureData>* textureContainer;
    Container<uint32_t>* stateContainer;
    
    TextureLoader* textureLoader;
  };
  
  HardcodedEntityLoader(const InitData &data);
  HardcodedEntityLoader(TextureLoader* textureLoader);
  ~HardcodedEntityLoader();
  
  bool parse(const std::string &pathPrefix, 
             const std::string &forcedNamePrefix, 
             const nlohmann::json &data, 
             std::vector<Resource*> &resource, 
             std::vector<ErrorDesc> &errors, 
             std::vector<WarningDesc> &warnings) override;
  bool forget(const std::string &name) override;

  std::unordered_map<std::string, Resource*> getLoadedResource() override;
  std::unordered_map<std::string, Conflict*> getConflicts() override;
  
  bool load(const std::string &name) override;
  bool unload(const std::string &name) override;
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
  void createWall(const CreateWallInfo &info);
  
    yacs::Entity* getPlayer() const;
  TransformComponent* getPlayerTransform() const;
  CameraComponent* getCamera() const;
  UserInputComponent* getInput() const;
private:
  size_t state;
  std::atomic<size_t> accumulatedState;
  
    yacs::Entity* player;
  TransformComponent* playerTransform;
  CameraComponent* camera;
  UserInputComponent* input;
  
  TextureLoader* textureLoader;
  
    yacs::World world;
};

class HardcodedMapLoader : public Loader, public ResourceParser {
public:
  struct CreateInfo {
    yavf::Device* device;
    
    HardcodedEntityLoader* entityLoader;
    TextureLoader* loader;
  };
  
  HardcodedMapLoader(const CreateInfo &info);
  ~HardcodedMapLoader();
  
  bool parse(const std::string &pathPrefix, 
             const std::string &forcedNamePrefix, 
             const nlohmann::json &data, 
             std::vector<Resource*> &resource, 
             std::vector<ErrorDesc> &errors, 
             std::vector<WarningDesc> &warnings) override;
  bool forget(const std::string &name) override;

  std::unordered_map<std::string, Resource*> getLoadedResource() override;
  std::unordered_map<std::string, Conflict*> getConflicts() override;
  
  bool load(const std::string &name) override;
  bool unload(const std::string &name) override;
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
  TextureLoader* textureLoader;
  
  // здесь у нас буду храниться все вершины карты
  yavf::Buffer* vertices;
  yavf::Buffer* indices;
};

#endif
