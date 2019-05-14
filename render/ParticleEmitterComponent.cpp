#include "ParticleEmitterComponent.h"

#include "Globals.h"
#include "ParticleSystem.h"
#include "GraphicComponets.h"
#include "Components.h"
#include "Random.h"

ParticleEmitterComponent::ParticleEmitterComponent() : events(nullptr) {}
ParticleEmitterComponent::~ParticleEmitterComponent() {}

void ParticleEmitterComponent::update(const size_t &time) {
//   for (size_t i = 0; i < functions.size(); ++i) {
//     if (functions[i](time)) {
//       std::swap(functions[i], functions.back());
//       functions.pop_back();
//       --i;
//     }
//   }
  
  // тут будет один какой нибудь тип вывода частиц
  // понятно, что нужно будет создать несколько эмиттеров
  // причем еще должна быть возможность включать/выключать это дело
}

void ParticleEmitterComponent::init(void* userData) {
  (void)userData;
  
  events = getEntity()->get<EventComponent>().get();
  if (events == nullptr) {
    Global::console()->printE("Could not create ParticleEmitterComponent without EventComponent");
    throw std::runtime_error("Could not create ParticleEmitterComponent without EventComponent");
  }
}

// static const auto timedFunc = [&] (const size_t &time) {
//       const float coef = float(time) / float(data.generationTime);
//       const size_t partCount = coef * data.particleCount;
//       
//       for (size_t i = 0; i < partCount; ++i) {
//         
//       }
//     };

// void ParticleEmitterComponent::setFunc(const ParticleGenerationFromSurface &data) {
//   auto graphic = getEntity()->get<GraphicComponent>();
//   auto transform = getEntity()->get<TransformComponent>();
//   // физика, но мне в физике нужны только точки
//   auto physics = getEntity()->get<PhysicsComponent2>();
//   
//   struct EntityShapeData {
//     TransformComponent* transform;
//     uint32_t matrixIndex;
//     uint32_t rotationIndex;
//     uint32_t pointsCount;
//     const simd::vec4* pointsPtr;
//   };
//   
//   const uint32_t pointsCount = physics->getObjectShapePointsSize();
//   const simd::vec4* pointsPtr = physics->getObjectShapePoints();
//   struct WeightedRand {
//     float weight;
//     uint32_t index;
//   };
//   std::vector<WeightedRand> points;
//   float maxWeight = 0.0f;
//   
//   const simd::vec4 &firstPoint = pointsPtr[0];
//   for (uint32_t i = 0; i < pointsCount-2; ++i) {
//     const uint32_t j = (i+1)%pointsCount;
//     const uint32_t k = (j+1)%pointsCount;
//     
//     const simd::vec4 &point2 = pointsPtr[j];
//     const simd::vec4 &point3 = pointsPtr[k];
//     
//     const simd::vec4 &ab = point2 - firstPoint;
//     const simd::vec4 &ac = point3 - firstPoint;
//     
//     const float prod = simd::dot(ab, ac);
//     const float sinAngle = std::sqrt(1 - prod*prod);
//     
//     const float length1 = simd::length(ab);
//     const float length2 = simd::length(ac);
//     
//     const float area = (length1 * length2 * sinAngle) / 2.0f;
//     
//     maxWeight += area;
//     points.push_back({area, j});
//   }
//   
//   static const auto func = [] (const Type &type, 
//                                const EventData &evData, 
//                                const ParticleGenerationFromSurface &data, 
//                                const EntityShapeData &shapeData, 
//                                const float &maxWeight, 
//                                const std::vector<WeightedRand> &weights) {
//     const size_t partCount = data.particleCount;
//     for (size_t i = 0; i < partCount; ++i) {
//       // что тут? мы должны сгенерить по формулам предыдущим точку
//       // для этого нам нужно их получить + индексы матриц, ротатион
//       float rand = Global::random()->rangeF(0.0f, maxWeight);
//       uint32_t index;
//       for (uint32_t j = 0; j < weights.size(); ++j) {
//         rand -= weights[j].weight;
//         if (rand <= 0.0f) {
//           index = weights[j].index;
//           break;
//         }
//       }
//       
//       const simd::vec4 &firstPoint = shapeData.pointsPtr[0];
//       const simd::vec4 &secondPoint = shapeData.pointsPtr[index];
//       const simd::vec4 &thirdPoint = shapeData.pointsPtr[(index+1)%shapeData.pointsCount];
//       
//       float rand1 = Global::random()->rangeF(0.0f, 1.0f);
//       float rand2 = Global::random()->rangeF(0.0f, 1.0f);
//       if (rand1 + rand2 > 1.0f) {
//         rand1 = 1.0f - rand1;
//         rand2 = 1.0f - rand2;
//       }
//       
//       const float a = 1.0f - rand1 - rand2;
//       const float b = rand1;
//       const float c = rand2;
//       
//       const simd::vec4 randPoint = a*firstPoint + b*secondPoint + c*thirdPoint;
//       
//       glm::vec4 vector;
//       randPoint.storeu(&vector.x);
//       const Particle particle{
//         vector,
//         data.startingVel,
//         data.color,
//         0,
//         data.particleTime,
//         0,
//         0,
//         1.0f,
//         1.0f,
//         0.0f,
//         1.0f,
//         glm::uvec4(data.texture.imageArrayIndex, data.texture.imageArrayLayer, data.texture.samplerIndex, 0)
//       };
//       Global::particles()->addParticle(particle);
//     }
//     
//     return success;
//   };
//   
//   const EntityShapeData shapeData{
//     transform.get(),
//     graphic->getMatrixIndex(),
//     graphic->getRotationDataIndex(),
//     pointsCount,
//     pointsPtr
//   };
//   events->registerEvent(data.type, std::bind(func, std::placeholders::_1, std::placeholders::_2, data, shapeData, maxWeight, points));
// }
// 
// // не понимаю что можно сделать с временными функциями
// // + возможно источник частиц можно как то выключить?
// // то есть он работает какое то время а потом мы меняем стейт объекта
// // это все усложняет
// // в простых случаях можно обойтись только двумя состояниями: вкл, выкл
// 
// // короче видимо для постоянного создания частиц нужно создавать несколько разных компонентов
// // и в update разное создание частиц делать
// 
// void ParticleEmitterComponent::setFunc(const ParticleGenerationFromPoint &data) {
//   
// }

void ParticleEmitterComponent::setFunc(const Type& type, const std::function<event (const Type &, const EventData &)>& func) {
  events->registerEvent(type, func);
}

