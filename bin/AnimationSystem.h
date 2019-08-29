#ifndef ANIMATION_SYSTEM_H
#define ANIMATION_SYSTEM_H

#include <vector>
#include <unordered_map>

#include "Engine.h"
#include "ArrayInterface.h"

#include "RenderStructures.h"
#include "PhysicsTemporary.h"

#include "ResourceID.h"

// этот класс по сути просто выставляет соответствие между состоянием энтити
// и состоянием анимации, конкретно здесь будет большинство состояний задействовано
// class AnimationComponent {
// public:
//   
// private:
//   AnimationUnitData data;
//   std::unordered_map<Type, AnimationState> states;
// };

// с состояниями такое дело
// задумка может и ничего, но я не могу придумать верный способ для того чтобы перейти от ИИ
// к непосредственно состояниям энтити, так как ии еще должно пройти некоторый слой вычислений
// то есть от ии должны вообще то говоря приходить некие команды вроде: сбежать, патрулировать и прочее
// а уже после них принимать решение о состоянии
// например что такое сбежать в понимании ии? это найти определенное направление от противника и переместиться на какое то расстояние
// + возможно найти укрытие, как это сделать? какие у меня входные данные?
// ии по идее должен знать о своих возможностях + о том что его окружает и как до этого добраться
// все эти данные мы должны получать из графа, прежде всего это данные о соседних нодах
// в нодах кстати может быть информация об обитателях данного нода, это значит что я могу быстро найти какие то соседние энтити
// в общем вот что, у ии должен быть объект по типу физического, который будет принадлежать нодам и его помощью будут выполняться различные действия
// объекты необходимо разделить на определенные типы, например союзные энтити 
// (естественно для каждого энтити держать список со всеми, например союзными, объектами это бред)
// неплохо было бы ВСЕ энтити запихнуть в эту систему, но для проджектайлов это скорее всего ненужно/невозможно сделать быстро
// если в физике необходимо только проверять объекты из ближайшего окружения, то в ии нужно проверять объекты из более широкого круга,
// но вычисления гораздо менее сложные,
// то есть, например, мне нужно сделать флокинг бехавиор (движение скопом, кажется), для этого мне нужно получить всех соседей
// у меня нет особо возможности опрашивать физику для этого, поэтому мне нужно пройтись по как раз таки этой ии системе
// и определить кто рядом, а кто нет, это можно сделать как я уже сказал, по нодам графа
// обходим необходимые объекты в ноде, составляем список, в зависимости от списка делаем флокинг
// это достаточно дорогие операции, то есть поиск среди кучи юнитов в ноде, но с другой стороны 
// это по идее быстрая проверка радиусов, то есть супер дешевая операция
// здесь главное правильно раскидать по тредам

//struct AnimType {
//  uint32_t container;
//
//  AnimType();
//  AnimType(const bool &repeated, const uint8_t &frameCount, const bool &dependant);
//
//  void makeType(const bool &repeated, const uint8_t &frameSize, const bool &dependant);
//
//  bool isRepeated() const;
//  uint8_t frameSize() const;
//  bool isDependant() const;
//};

// нужно будет изменить для гпу
// с другой стороны возможно не нужно их получать из основного класса
class Animation {
public:
  struct Image {
    ::Image image;
    bool flipU;
    bool flipV;
  };

  // по сути мне нужны теперь только фреймы, время и наверное свойства (только одно: повторение?) я буду передавать иначе
  struct CreateInfo {
    std::vector<std::vector<Animation::Image>> frames;
  };

  struct DependantInfo {
    ResourceID existingId;
    uint32_t animStart;
    uint32_t animEnd; // animEnd = lastIndex + 1, или size
  };

  Animation(const size_t &imageOffset, const uint8_t &animFrameSize, const size_t &frameStart, const size_t &frameCount);
  
//  bool isFinished(const size_t &time) const;
//  bool isRepeated() const;
  uint8_t frameSize() const;  // скорее всего один на всю анимацию
  uint32_t frameStart() const;
  uint32_t frameCount() const;
  size_t offset() const;
  size_t imagesCount() const;

  // нужно передавать текущее время и общее время
//  size_t getCurrentFrameOffset(const size_t &time) const; // возвращаем старт фрейма, к которому прибавляем нужную сторону
  size_t getCurrentFrameOffset(const size_t &currentTime, const size_t &animTime) const;
private:
//  AnimType type; // тип сократится до frameSize
  uint8_t animFrameSize;
//  size_t frameTime;

  uint32_t animFrameStart;
  uint32_t animFrameCount;
  // если animStart > animSize то мы идем в обратную сторону

  size_t imageOffset;
};

typedef uint32_t AnimationState;

class AnimationComponent;

// скорее всего такую систему будет сложно приспособить еще под что то другое (я имею ввиду другой тип анимаций)
// мне нужно у таких систем выделить набор методов вирутальных, которые позволят пользоваться неким Стейтом 
// в котором будет так или иначе хранится состояния для каждой системы
// пока не знаю как это сделать

// тут мы должны быстро обходить огромный (относительно) массив данных
// и вычислять ну очень простые вещи, будет много данных расфасованных по разным местам
// но вычисления однотипные
// на входе у нас текущее состояние и позиция и направление объекта (и позиция и направление игрока)
// на выходе текстурка
// что со временем? тип для каждого создавать специальный объект в котором будет время
// ну в принципе жизнеспособно

// тут должны быть переменные для зеркального отображения текстуры
// и для анимации перемещающейся текстуры (ну тип как лица в думе)
// на самом деле сюда неплохо было бы вынести и индекс сэмплера
// а не держать его в Texture

class AnimationSystem : public Engine {
public:
//   struct InputBuffers {
// //     ArrayInterface<AnimationState>* stateArray;  
//     ArrayInterface<uint32_t>* customTimeArray;  
//     ArrayInterface<Transform>* transforms;
//     //ArrayInterface<PlayerData>* playerData;
//   };
//   
//   struct OutputBuffers {
//     //ArrayInterface<Texture>* textures;
//     ArrayInterface<TextureData>* textures;
//   };
  
//   struct AnimationUnitCreateInfo {
//     uint32_t stateIndex;
//     uint32_t timeIndex;
//     uint32_t transformIndex;
//     uint32_t textureIndex;
//   };
  
//   struct AnimationUnitData {
//     uint32_t animationId;
//     uint32_t internalIndex;
//   };
  
//  struct AnimationCreateInfoNewFrames {
//    bool repeated;
//    uint32_t animationTime; // может size_t?
//    std::vector<std::vector<Animation::Image>> frames;
//  };
//
//  struct AnimationCreateInfoFromExisting {
//    bool repeated;
//    uint32_t animationTime; // может size_t?
//    //uint32_t textureOffset;
//    uint32_t existingId;
//    uint32_t animSize;
//  };
  
  //AnimationSystem();
  virtual ~AnimationSystem() = default;
  
//   virtual void setInputBuffers(const InputBuffers &buffers) = 0;
//   virtual void setOutputBuffers(const OutputBuffers &buffers) = 0;
  
  // здесь я должен передать индексы для инпут и аутпут буферов AnimationUnitData* data
  virtual void registerAnimationUnit(AnimationComponent* component) = 0;
  virtual void removeAnimationUnit(AnimationComponent* component) = 0;
  
//   virtual void precacheStateCount(const uint32_t &animationUnit, const uint32_t &stateCount) = 0;
//   virtual void setAnimation(const uint32_t &animationUnit, const AnimationState &state, const uint32_t &animId) = 0;
//   virtual void setAnimation(const uint32_t &animationUnit, const AnimationState &state, const std::string &animName) = 0;
  
  //void update(const uint64_t &time) override; // у меня время то будет отличаться для каждой анимации
  
  // вообще мне кажется что такие вещи как анимации текстурки и прочее (то есть ресурсы игры с диска... как то так)
  // лучше бы создавать и получать по какому-нибудь интерфесу
  virtual uint32_t createAnimation(const ResourceID &animId, const Animation::CreateInfo &info) = 0;
  virtual uint32_t createAnimation(const ResourceID &animId, const Animation::DependantInfo &info) = 0;
  
  virtual uint32_t getAnimationId(const ResourceID &animId) const = 0;
  
  virtual Animation & getAnimationById(const uint32_t &id) = 0;
  virtual const Animation & getAnimationById(const uint32_t &id) const = 0;
  
  virtual Animation & getAnimationByName(const ResourceID &animId) = 0;
  virtual const Animation & getAnimationByName(const ResourceID &animId) const = 0;
  
  virtual Animation::Image getAnimationTextureData(const size_t &index) const = 0;
};

#endif
