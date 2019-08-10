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

#include "RenderStage.h"
#include "TypelessContainer.h"

#include "Engine.h"

#include "RenderStructures.h"

class Render : public Engine {
public:
  // : opts(RENDER_TYPE_MAX, nullptr)
  
  Render(const size_t &stageContainerSize) : perspective(true), stageContainer(stageContainerSize), matrices(new Matrices()) {}
  
  virtual ~Render() {
    for (uint32_t i = 0; i < stages.size(); ++i) {
      stageContainer.destroy(stages[i]);
    }
    
    delete matrices;
  }
  
  // вот эти функции тоже было бы неплохо сделать виртуальными
  // ну и вообще переделать, разные юниформ данные могут потребоваться
  bool isPrespective() const;
  void toggleProjection();
  Matrices* getMatrices();
  void setView(const simd::mat4 &view);
  void setPersp(const simd::mat4 &persp);
  void setOrtho(const simd::mat4 &ortho);
  void setCameraDir(const simd::vec4 &dir);
  void setCameraPos(const simd::vec4 &pos);
  void setCameraDim(const uint32_t &width, const uint32_t &height);
  
  simd::mat4 getViewProj() const;
  simd::mat4 getView() const;
  simd::mat4 getPersp() const;
  simd::mat4 getOrtho() const;
  
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
    T* ptr = stageContainer.create<T>(std::forward<Args>(args)...);
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
  bool perspective;
  std::vector<RenderStage*> stages;
  TypelessContainer stageContainer;
  Matrices* matrices;
};

#endif
