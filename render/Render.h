// что мне делать с рендером?
// мне необходимо привести его к виду: render->update();
// что это означает? это означает, что мне необходимо сделать:
// 1) запустить оптимизеры для своего типа данных
// 1)б) возможно запустить также окклюжен тест
// 2) начать отрисовку и взять какие то вещи из оптимизера (собственно составить секондари комманд буферы и собрать команды)
// 3) дальше все нарисовать, да и все

// для того чтобы мне сделать 1 и 2, мне нужно передать данные из фрустум теста в оптимизеры
// и на его основе начать обрабатывать объекты в зависимости от типа
// следовательно мне нужен какой то перевод из одних индексов в другие
// ко всему прочему мне нужно сделать так чтобы при создании объекта (точнее графического компонента)
// данные появлялись в рендере (в оптимизерах?)

#ifndef RENDER_H
#define RENDER_H

#include <vector>

// #define YAVF_DEBUG_REPORT_EXTENSION
// #define YAVF_INFO_REPORT
// #include "yavf.h"

#include <glm/glm.hpp>

#include "RenderStage.h"
#include "StageContainer.h"

#include "Engine.h"

#include "RenderStructures.h"

//class VulkanRender;

// enum RenderType : uint32_t {
//   RENDER_TYPE_GEOMETRY = 0,
//   RENDER_TYPE_MONSTER,
//   RENDER_TYPE_LIGHT,
//   RENDER_TYPE_DEBUG,
//   RENDER_TYPE_MAX
// };

class Constant {
public:
  template <typename T>
  T get() {
    return reinterpret_cast<T&>(value);
  }
  
  void set(const bool &val);
  void set(const size_t &val);
  void set(const float &val);
private:
  size_t value;
};

class Render : public Engine {
public:
  // : opts(RENDER_TYPE_MAX, nullptr)
  
  Render(const size_t &stageContainerSize) : stageContainer(stageContainerSize), perspective(true) {}
  
  virtual ~Render() {
    for (uint32_t i = 0; i < stages.size(); ++i) {
      stageContainer.destroyStage(stages[i]);
    }
  }
  
  // вот эти функции тоже было бы неплохо сделать виртуальными
  // ну и вообще переделать, разные юниформ данные могут потребоваться
  bool isPrespective() const;
  void toggleProjection();
  Matrices getMatrices() const;
  void setView(const glm::mat4 &view);
  void setPersp(const glm::mat4 &persp);
  void setOrtho(const glm::mat4 &ortho);
  void setCameraDir(const glm::vec4 &dir);
  void setCameraPos(const glm::vec4 &pos);
  void setCameraDim(const uint32_t &width, const uint32_t &height);
  
  glm::mat4 getViewProj() const;
  glm::mat4 getView() const;
  glm::mat4 getPersp() const;
  glm::mat4 getOrtho() const;
  
//   template <typename T>
//   void setOptimizer(Optimizer* opt) {
//     opts[T::renderType] = opt;
//   }
//   
//   template <typename T>
//   T* getOptimizer() {
//     return static_cast<T*>(opts[T::renderType]);
//   }
  
  // это кстати мне тоже здесь не нужно
  template <typename T, typename... Args>
  T* addStage(Args&&... args) {
    // нужно ли мне добавить какой-нибудь индекс стейджу?
    T* ptr = stageContainer.addStage<T>(std::forward<Args>(args)...);
    stages.push_back(ptr);
    
    return ptr;
  }
  
  // тут еще возможно будет что то для оклюжен куллинга
  // окклюжен куллинг должен полностью уйти в рендер стейджы
  
  // переименовать
  virtual void updateCamera() = 0;
  
  // здесь у нас по идее просто обходит массив stages
  // вызывая виртуальные функции
  // как передать данные в RenderStage?
  // нам нужно передать устройство, по идее устройство передается на инициализации, то есть никаких проблем
  // где хранить все необходимые ресурсы? ну явно не в рендере
  //virtual void draw() = 0; // update вместо этого
  virtual void start() = 0;
  virtual void wait() = 0;
  
  // методы для загрузки тектур
  
  //void clear(); // ???
  
  virtual void printStats() = 0;
protected:
  // естественно все эти вещи должны быть дублированы для каждого устройства 
  // + я еще не продумал как отрисовывать на разные экраны все эти вещи
  // по идее копирование в рендертаргет это тоже отдельный стейдж, то есть ничего криминального
  // возможно мне пригодится отделить некоторые таски друг от друга
  // (например, чтобы отрисовать часть вещей до того как будет доступно следующее изображение свопчейна)
  // (в принципе, это тоже стейджы, но как правильно заполнить submit'ы? дать возможность добавить несколько таскИнтерфейсов, предварительно заполнив у них семафоры?)
  // (вообще звучит как план)
  // а, вопрос заключается в том где стартовать таски? в стейджах не вариант на самом деле
  // задать отдельные стейджы которые только нужны для старта и окончания тасков......... (в общем ничего криминального не вижу, но эт очень странно)
  std::vector<RenderStage*> stages;
  
  StageContainer stageContainer;
  
  bool perspective;
  Matrices matrices;
};

// struct Vertex {
//   glm::vec3 pos;
//   glm::vec4 color;
//   glm::vec2 texCoord;
// };

// struct PushConst {
//   glm::mat4 mat;
//   glm::vec4 color;
//   glm::vec3 normal;
// };
// 
// struct DrawInfo {
//   Texture texture;
//   glm::vec3 pos;
//   glm::vec3 rot;
//   glm::vec3 scale;
//   glm::vec3 normal;
//   uint32_t faceIndex;
//   size_t elemCount;
//   size_t offset;
// };
// 
// struct LightData {
//   glm::vec3 pos;
//   float radius;
//   glm::vec3 color;
//   float cutoff;
// };

// class VulkanRender;
// 
// class Optimizer {
// public:
//   virtual ~Optimizer() {}
//   
//   virtual void setup(VulkanRender* render) = 0;
//   
//   virtual void add(const DrawInfo &info) = 0;
//   virtual void prepare(const size_t &offset) = 0;
//   virtual void occlusion(yavf::GraphicTask* task) = 0;
//   virtual void optimize() = 0;
//   virtual void draw(yavf::GraphicTask* task) = 0;
//   virtual void clear() = 0;
//   virtual size_t size() const = 0;
// };
// 
// class Render {
// public:
//   Render();
//   virtual ~Render();
// 
//   void add(const uint32_t &type, const DrawInfo &info);
//   void optimize();
//   virtual void draw() = 0;
//   virtual void start() = 0;
//   virtual void wait() = 0;
//   void clear();
// protected:
//   std::vector<Optimizer*> opts;
// };

#endif
