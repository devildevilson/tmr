#ifndef PARTICLE_EMITTER_COMPONENT_H
#define PARTICLE_EMITTER_COMPONENT_H

#include "Emitter.h"
#include "EntityComponentSystem.h"
#include "Utility.h"
#include "Type.h"
#include "RenderStructures.h"

#include "EventComponent.h"

#include <vector>
#include <functional>

class EventComponent;

// тут мне нужно что? эвент компонент 
// какой то способ создавать частицы, причем должен быть способ спавнить частицы из плоскости
// похоже что это способ создать точку в треугольники, для того чтобы создать точку в полигоне
// предлагают развесовать треугольники по площади и создавать точки от каждого треугольника
// if (r1 + r2 > 1) {
//   r1 = 1 - r1;
//   r2 = 1 - r2;
// }
// 
// a = 1 - r1 - r2;
// b = r1; 
// c = r2;
// 
// Q = a*A + b*B + c*C
// теперь понятно

// короч нужен рандом, причем у меня должен быть рандом который будет передаваться по сетке
// и рандом локальный

// компонент для создания частиц, что тут еще может потребоваться?
class ParticleEmitterComponent : public Emitter, public yacs::Component {
public:
//   struct CreateInfo {
//     glm::vec4 startingVel;
//     glm::vec4 color;
//   };
  //ParticleEmitterComponent(const CreateInfo &info);
  ParticleEmitterComponent();
  ~ParticleEmitterComponent();
  
  void update(const size_t &time) override;
  void init(void* userData) override;
  
  // функция для эвентов
  // для каких то частиц будет не лишним создать после смерти еще одну
  // также как создать партикл именно в определенной точке нахождения объекта
  // то есть нужно видимо точку перемножить со всеми матрицами поворота и прочего
//   struct ParticleGenerationFromSurface {
//     Type type;
//     size_t particleCount;
//     size_t generationTime;
//     uint32_t particleTime;
//     glm::vec4 startingVel; // скорость то тоже нужно генерировать, по крайней мере в некоторых местах
//     glm::vec4 color;
//     Texture texture;
//   };
//   void setFunc(const ParticleGenerationFromSurface &data);
//   
//   // так короч, есть два типа генерации частиц, со всей поверхности и из точки
//   struct ParticleGenerationFromPoint {
//     Type type;
//     size_t particleCount;
//     size_t generationTime;
//     uint32_t particleTime;
//     glm::vec4 startPos;
//     glm::vec4 color;
//     Texture texture;
//     
//     std::function<glm::vec4(const size_t&)> generateVelocity;
//   };
//   void setFunc(const ParticleGenerationFromPoint &data);
  
  void setFunc(const Type &type, const std::function<event(const Type &, const EventData &)> &func);
private:
  EventComponent* events;
//   glm::vec4 startingVel;
//   glm::vec4 color;
//   std::vector<std::function<bool(const size_t&)>> functions; // это для того чтобы создавать частицы позже по времени
};

#endif
