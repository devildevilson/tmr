#ifndef OPTIMIZER_H
#define OPTIMIZER_H

// нинужен
// #include "yavf.h"

//#include "PhysicsTemporary.h"
//#include "ArrayInterface.h"

// че здесь делать с рендерингом на разных устройствах?
// дублировать данные ой как не хочется, но их к сожалению так или иначе придется копировать по буферам
// или нет? помоему у вулкана был какой-то механизм связанный с созданием VkBuffer на основе уже существующих данных
// надо почекать
class Optimizer {
public:
  // сюда 200% что то еще добавится (хотя бы номер устройства)
//   struct SetupInfo {
//     VulkanRender* render;
//   };
  
  // что тут может добавиться? 
  // мне нужно как то передавать текстурки
  // в принципе передавать их в массиве - идея неплохая
  // нужно тогда штуку с анимациями задизайнить одновременно с этим
//   struct InputBuffers {
//     ArrayInterface<Transform>* transforms;
//     ArrayInterface<glm::mat4>* matrices;
//     ArrayInterface<uint32_t>* rotationDatasCount;
//     ArrayInterface<RotationData>* rotationDatas;
//     ArrayInterface<glm::uvec4>* textures;
//     
//     ArrayInterface<BroadphasePair>* frustumPairs; // тут скорее всего это все же ненужно
//   };
  
  virtual ~Optimizer() {}
  
  // скорее всего сетап мне нужно вынести во вне
  //virtual void setup(const SetupInfo &info) = 0;
  
//   virtual void setInputBuffers(const InputBuffers &buffers) = 0;
  
//   virtual void setRenderTarget(yavf::RenderTarget* target) = 0;
  
  virtual void prepare() = 0;
//   virtual void populateSecondaryOcclusionTask() = 0;
//   virtual void occlusionTest(yavf::GraphicTask* task) = 0;
//   virtual yavf::TaskInterface* secondaryOcclusionTask() = 0;
  
  virtual void optimize() = 0;
//   virtual void populateSecondaryDrawTask() = 0;
//   virtual void draw(yavf::GraphicTask* task) = 0;
//   virtual yavf::TaskInterface* secondaryDrawTask() = 0;
  
  virtual void clear() = 0;
  virtual size_t size() const = 0;
  
  // пайплайны должны быть локальными по идее
  // как сделать создание компонентов объекта?
  // вообще рендер объектов это очень индивидуальная штука
  // и возможно я должен создание объектов вынести за пределы виртуальных классов
  // тут просто ничего особо не придумаешь
  
  // как сделать дебаг? то есть по идее это просто еще один оптимизер
  // но с другой стороны как контролировать это дело в ран тайме?
  // ну видимо в реализации я должен учитывать все это дело
  // должны быть какие-нибудь изменяемые константы
  
  // возможно для того чтобы создавать объекты в оптимизерах
  // мне нужно сделать прокси, который будет позволять еще помечать объект как объект к обработке
  // другое дело, что скорее всего придется пользоваться определенным количеством виртуальных функции
  // для большого количества объектов
  // с другой стороны мы можем выдать компонентам указатель на необходимый оптимизер
  // и внутре компонента разбираться со всеми этими делами
  // думаю что это предпочтительнее
  
private:
  // именно здесь будут лежать пайплайны, пул проксей, дескрипторы ресурсов и прочее прочее
  // нам нужно любыми способами иметь возможность помечать 
  
  // пайплайны здесь лежать не будут
  // но здесь должен быть легкий способ получить доступ к ресурсам
};

#endif
