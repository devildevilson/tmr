#ifndef COMPONENTS_H
#define COMPONENTS_H

#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <functional>
#include <array>

// #include "TinyBehavior/TinyBehavior.h"
#include "Type.h"
// #include "AISystem.h"
#include "Controller.h"
// #include "CollisionObject2.h"
// #include "Animation.h"
//#include "CollisionObject2.h"
#include "Physics.h"
// #include "PhysicsTemporary.h"
#include "ArrayInterface.h"

#include "EntityComponentSystem.h"
#include "RenderStructures.h"

#include "Editable.h"

// #include "Pathway.h"

// static const float quarterPI = glm::pi<float>() / 4.0f;
// static const float eighthPI = glm::pi<float>() / 8.0f;

class GraphicState;
class CollisionComponent;
class PhysicComponent;
class TransformComponent;
class PhysicsComponent2;
class LoneAi;
class AnimationComponent;
class GraphicComponent;

// class AiComponent;
// class PhysicComponent;
// class GraphicComponent;

// class ComponentSystem {
// public:
//   void updateAI(YACS_UPDATE_TYPE time);
//   void updatePhysic(YACS_UPDATE_TYPE time);
//   void updateGraphic(YACS_UPDATE_TYPE time);
// private:
//   std::vector<AiComponent*> updatingAi;
//   std::vector<PhysicComponent*> updatingPhysic;
//   std::vector<GraphicComponent*> updatingGraphic;
//   
//   //World world;
// };

// class CollisionComponent;


// // это дело можно сделать темплейтом
// template <typename T>
// class GlobalContainer {
// public:
//   TransformContainer(yavf::Device* device);

//   // тут будет стандарный набор тип: ресайз, гет, 
//   void resize(const size_t &newSize);

//   T & at(const size_t &index);


//   void clear(); // потребуется для того чтобы удалить правильно массив
// private:
//   yavf::vector<T> array;
// };

struct InitComponents {
  std::string shapeName;
  bool dynamic;
  uint32_t objType;
};

class InfoComponent : public yacs::Component {
public:
  CLASS_TYPE_DECLARE
  
  InfoComponent(const Type &type);
  virtual ~InfoComponent();
  
  void update(const size_t &time = 0) override;
  void init(void* userData) override;
  
  void edit();
  
  std::string getType() const;
private:
  Type objType;
  TransformComponent* trans = nullptr;
//   LoneAi* ai = nullptr;
  GraphicComponent* graphic = nullptr;
//   PhysicComponent* phys = nullptr;
//   CollisionComponent* coll = nullptr;
  PhysicsComponent2* phys2 = nullptr;
  AnimationComponent* anim = nullptr;
};

// class Editable {
// public:
//   virtual void uiDraw() {};
// protected:
//   size_t graphicIndex = 0;
// };

class TransformComponent : public yacs::Component, public Editable {
public:
  CLASS_TYPE_DECLARE

  static void setContainer(Container<Transform>* container);
  static void setPrevContainer(Container<Transform>* prevContainer);
  
  TransformComponent() {
    transformIndex = container->insert({});
  }

//   TransformComponent(const glm::vec3 &pos, const glm::vec3 &rot, const glm::vec3 &scale) {
//     // здесь должен быть доступ к массиву трансформ, где и будут собственно храниться позиции и прочее
//     transformIndex = container->insert({simd::vec4(pos, 1.0f), simd::vec4(rot, 0.0f), simd::vec4(scale, 0.0f)});
// //     const uint32_t prevIndex = prevTransfoms->insert({simd::vec4(pos, 1.0f), simd::vec4(rot, 0.0f), simd::vec4(scale, 0.0f)});
//     
// //     ASSERT(transformIndex == prevIndex);
//   }

  TransformComponent(const simd::vec4 &pos, const simd::vec4 &rot, const simd::vec4 &scale) {
    // здесь должен быть доступ к массиву трансформ, где и будут собственно храниться позиции и прочее
    transformIndex = container->insert({pos, rot, scale});
//     const uint32_t prevIndex = prevTransfoms->insert({simd::vec4(pos, 1.0f), simd::vec4(rot, 0.0f), simd::vec4(scale, 0.0f)});
    
//     ASSERT(transformIndex == prevIndex);
  }

  ~TransformComponent() {
//     RegionLog rl("Destroy TransformComponent");
    //Global::physic2()->removeTransform(transformIndex);
    container->erase(transformIndex);
//     prevTransfoms->erase(transformIndex);
  }
  
  void update(const size_t &time = 0) override { (void)time; }
  void init(void* userData) override { (void)userData; }
  
  void uiDraw() override;
  
  simd::mat4 getTransform(const bool rotation = false) const;

//   void updatePos() {
//     container->at(transformIndex).pos = simd::vec4(pos, 1.0f);
//   }

//   simd::vec4 getPos() {
//     return container->at(transformIndex).pos;
//   }
  
  const simd::vec4 & pos() const {
    return container->at(transformIndex).pos;
  }
  
  const simd::vec4 & rot() const {
    return container->at(transformIndex).rot;
  }
  
  const simd::vec4 & scale() const {
    return container->at(transformIndex).scale;
  }
  
  simd::vec4 & pos() {
    return container->at(transformIndex).pos;
  }
  
  simd::vec4 & rot() {
    return container->at(transformIndex).rot;
  }
  
  simd::vec4 & scale() {
    return container->at(transformIndex).scale;
  }
  
//   simd::vec4 mixpos(const float &alpha) const {
//     return glm::mix(prevTransfoms->at(transformIndex).pos, container->at(transformIndex).pos, alpha);
//   }
//   
//   simd::vec4 mixrot(const float &alpha) const {
//     //glm::mix(prevTransfoms->at(transformIndex).rot, container->at(transformIndex).rot, alpha);
//     (void)alpha;
//     return container->at(transformIndex).rot;
//   }
//   
//   simd::vec4 mixscale(const float &alpha) const {
//     //glm::mix(prevTransfoms->at(transformIndex).scale, container->at(transformIndex).scale, alpha);
//     (void)alpha;
//     return container->at(transformIndex).scale;
//   }

  uint32_t transformIndex;
  
//   glm::vec3 pos;
//   glm::vec3 rot;
//   glm::vec3 scale;

  //static std::vector<Transform> array;
  // тут наверное лучше сделать статический указать на структуру с массивом
  // хотя не понятно как быть с временем жизни? в принципе, если этот компонент будет удаляться позже структуры
  // то почему бы и нет
  // я хотел выделить в отдельную переменную чтобы иметь возможность создавать какие то объекты вне компонентов
  // например мне пригодится это использовать для создания "мелких" объектов (свет, частицы и прочее)
  // static uint32_t freeIndex;
  static Container<Transform>* container;
//   static Container<Transform>* prevTransfoms;
};

class InputComponent : public yacs::Component {
public:
  CLASS_TYPE_DECLARE

  static void setContainer(Container<InputData>* container);
  //InputComponent(ArrayInterface<InputData>* arr, uint32_t index);
  InputComponent();
  ~InputComponent();
  
  void update(const size_t &time = 0) override { (void)time; }
  void init(void* userData) override;// { (void)userData; }
  
//   simd::vec4 predictPos(const size_t &predictionTime) const;
//   
//   void seek(const simd::vec4 &target);
//   void flee(const simd::vec4 &target);
//   void followPath(const size_t &predictionTime, const Pathway &path, const size_t &currentPathSegmentIndex); // , const size_t &currentPathSegmentIndex
//   void stayOnPath(const size_t &predictionTime, const Pathway &path);

//   void updateInput();
  
  simd::vec4 & front();
  simd::vec4 & up();
  simd::vec4 & right();
  simd::vec4 & movementData();
  const simd::vec4 & front() const;
  const simd::vec4 & up() const;
  const simd::vec4 & right() const;
  const simd::vec4 & movementData() const;
  
  void setMovement(const float &front, const float &up, const float &right);
  
  uint32_t inputIndex;
//   float forwardMove = 0.0f;
//   float sideMove = 0.0f;
//   float upMove = 0.0f;
  
//   PhysicComponent* physic = nullptr;
  PhysicsComponent2* physics2;
  TransformComponent* trans;
  
//   simd::vec4 frontVec;
//   simd::vec4 upVec;
//   simd::vec4 rightVec;

  static Container<InputData>* container;
};

struct PhysUserData {
    yacs::Entity* ent;
};

class PhysicsComponent2 : public yacs::Component {
public:
  CLASS_TYPE_DECLARE

  static void setContainer(Container<ExternalData>* externalDatas);

  PhysicsComponent2(/*void* userData = nullptr*/);
  ~PhysicsComponent2();

  void update(const size_t &time = 0) override;
  void init(void* userData) override;

  const PhysicsIndexContainer & getIndexContainer() const;
  
  simd::vec4 getVelocity() const;

  // сюда приходит input
  // + здесь должна обновляться всякая дополнительная информация
  // например максимальная скорость
  // либо мы можем изменять здесь вещи непосредственно в момент изменения
  // то есть наример сразу после измеения характеристик
  // теперь мы используем глобальный (для физики и инпут компонента) буфер, и соответственно инпут данные обновляются там
  // void updateInput(const InputData &input);
protected:
  // здесь конечно будет еще доступ к характеристикам

  uint32_t externalDataIndex = UINT32_MAX;
  TransformComponent* trans = nullptr;
  PhysicsIndexContainer container;
  PhysUserData userData;

  static Container<ExternalData>* externalDatas;
};

class EventComponent;

// так, мне нужно как то получить конец эвента
// первое что приходит в голову, это отсылать конец эвента в эвенте
// но это капец, не надо ничего никуда отсылать
// обновляем стейт, если время вылезает за пределы то это значит состояние закончилось
// в этом случае даже мы можем вызвать какой-нибудь эвент
// никаких идиотских виртуальных функций которые возвращают только bool значение
class StateController : public yacs::Component/*, public Controller*/ {
public:
  CLASS_TYPE_DECLARE
  
  static void setContainer(Container<uint32_t>* customTimeContainer);
  static void addLoopedState(const Type &type);
  static bool isStateLooped(const Type &type);
  
  StateController();
  ~StateController();
  
  void update(const size_t &time = 0) override;
  void init(void* userData) override;
  
//   void addChild(Controller* c) override;
//   ControllerChildData changeState(const Type &type) override;
//   void reset() override;
//   void finishCallback() override;
//   
//   bool isFinished() const override;
//   bool isBlocking() const override;
//   bool isBlockingMovement() const override;
  
  Type getCurrentState() const;
  bool isFinished() const;
  bool isBlocking() const;
  bool isBlockingMovement() const;
  
  uint32_t getCustomTimeIndex() const;
  
  // сюда мы можем дополнительно накинуть каких-нибудь атрибутов, вроде скорости атаки
  void registerState(const Type &type, const bool blocking, const bool blockingMovement, const size_t &stateTime);
private:
  bool finished;
  bool blocking;
  bool blockingMovement;
  uint32_t customTimeIndex;
  Type state;
//   uint32_t nonFinishedStates = 0;
//   std::vector<Controller*> childs;
  size_t currentTime;
  size_t stateTime;
  EventComponent* localEvents;
  
  static Container<uint32_t>* customTimeContainer;
  
  static std::unordered_set<size_t> loopedStates;
  
  // че со временем? у меня должны быть атрибуты, которые могут поменять каким либо образом время
  // время будет влиять на скорость обработки того или иного состояния энтити
  // тип в теории все звучит более менее, пока мне ненужно совмещать бег с чем то другим
  // в принципе тут скорее всего будет задействован только звук, так как спрайты анимации не смешать
  // где еще могут быть проблемы?
  
  // проблема прежде всего в том что я не знаю как мне сделать атрибуты
};

class LightOptimizer;

class Light : public yacs::Component {
public:
  CLASS_TYPE_DECLARE
  
  static void setOptimizer(LightOptimizer* optimizer);
  
  Light(const float &radius, const float& cutoff, const glm::vec3 &color);
  virtual ~Light();
  
  void update(const size_t &time = 0) override;
  void init(void* userData) override;
  
  
protected:
  glm::vec3 posDelta; // для того чтобы чуть чуть менять положение источника света
  glm::vec3 color;
  float radius = 0.0f;
  float cutoff = 0.0f;
  
  TransformComponent* trans = nullptr;
  
  static LightOptimizer* optimizer;
};

//yacs::Component
class UserInputComponent : public InputComponent {
public:
  CLASS_TYPE_DECLARE
  
  UserInputComponent();
  ~UserInputComponent();
  
  void update(const size_t &time = 0) override;
  void init(void* userData) override;
  
  void mouseMove(const float &horisontalAngle, const float &verticalAngle);
  
  void forward();
  void backward();
  void left();
  void right();
  void jump();
private:
  float horisontalAngleSum, verticalAngleSum;
  
  StateController* states;
//   InputComponent* input = nullptr;
  TransformComponent* trans;
  
  glm::vec3 rotation;
};

class CameraComponent : public yacs::Component {
public:
  CLASS_TYPE_DECLARE
  
  void update(const size_t &time = 0) override;
  void init(void* userData) override;
private:
  TransformComponent* trans = nullptr;
  InputComponent* input = nullptr;
  
  //simd::mat4 view;
};

#endif
