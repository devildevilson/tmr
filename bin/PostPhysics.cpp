#include "PostPhysics.h"

#include "Variable.h"

PostPhysics::PostPhysics(dt::thread_pool* pool, yacs::Entity* player, TransformComponent* playerTransform) : pool(pool), player(player), playerTransform(playerTransform) {}
PostPhysics::~PostPhysics() {}

void PostPhysics::update(const uint64_t &time) {
  (void)time;
  
  static const auto func = [&] (const ArrayInterface<BroadphasePair>* pairs, const size_t &start, const size_t &count) {
    for (size_t i = start; i < start+count; ++i) {
      const uint32_t index = pairs->at(i+1).secondIndex;
      
      PhysUserData* userData = reinterpret_cast<PhysUserData*>(Global::physics()->getUserData(index));
      if (userData->ent == player) continue;
      
      if (userData->graphicComponent != nullptr) userData->graphicComponent->update();
      
      // потом нужно все же будет добавить отрисовку дебага
    }
  };
  
  const uint32_t rayOutputCount = Global::physics()->getRayTracingSize();
  const auto rayData = Global::physics()->getRayTracingData();
  
  const uint32_t frustumOutputCount = Global::physics()->getFrustumTestSize();
  const auto frustumPairs = Global::physics()->getFrustumPairs();
  
//     std::cout << "ray intersection count " << rayOutputCount << "\n";
//     for (uint32_t i = 0; i < rayOutputCount; ++i) {
//       std::cout << "ray intersect " << rayData->at(i).secondIndex << "\n";
//     }
  
//     std::cout << "frustum intersection count " << frustumOutputCount << "\n";
  for (uint32_t i = 0; i < frustumOutputCount; ++i) {
    const uint32_t index = frustumPairs->at(i+1).secondIndex;
    PhysUserData* userData = reinterpret_cast<PhysUserData*>(Global::physics()->getUserData(index));
    if (userData->ent == player) continue;
    
//       auto info = userData->ent->get<InfoComponent>();
//       std::cout << "name " << info->getType() << "\n";
    
//     auto graphic = userData->ent->get<GraphicComponent>();
//     graphic->update();
    if (userData->graphicComponent != nullptr) userData->graphicComponent->update();
    //if (userData->decalContainer != nullptr) // также реисуем декали
    
//     static cvar debugDraw("debugDraw");
//     if (bool(debugDraw.getFloat())) {
//       bool rayCollide = false;
//       for (uint32_t j = 0; j < rayOutputCount; ++j) {
//         const auto &ray = rayData->at(j);
//         
//         if (ray.secondIndex == index) rayCollide = true;
//       }
//       
//       if (rayCollide) graphic->drawBoundingShape(simd::vec4(1.0f, 0.0f, 0.0f, 0.5f));
//       else graphic->drawBoundingShape(simd::vec4(0.0f, 1.0f, 0.0f, 0.5f));
//     }
    
    // тут будет еще и обновление других компонентов, нужно наверное упрятать в какую-нибудь "систему"
    // чтобы здесь глаза не мазолила
    
    // как отключать отрисовку дебага?
    // мне нужно отключить ее здесь + ничего не делать в рендер стейджах
    // + не помешает на релизе вообще не создавать рендер стейджы для дебага
  }
  
//   const size_t count = std::ceil(float(frustumOutputCount) / float(pool->size()+1));
//   size_t start = 0;
//   for (uint32_t i = 0; i < pool->size()+1; ++i) {
//     const size_t jobCount = std::min(count, frustumOutputCount-start);
//     if (jobCount == 0) break;
// 
//     pool->submitnr(func, frustumPairs, start, jobCount);
// 
//     start += jobCount;
//   }
//   
//   pool->compute();
//   pool->wait();
  
  {
    Global g;
    g.setPlayerPos(playerTransform->pos());
  }
}
