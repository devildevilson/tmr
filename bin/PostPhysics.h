#ifndef POST_PHYSICS_H
#define POST_PHYSICS_H

#include "Engine.h"

#include "Globals.h"
#include "Physics.h"
#include "GraphicComponets.h"
#include "Components.h"

#include "ThreadPool.h"

class PostPhysics : public Engine {
public:
  PostPhysics(dt::thread_pool* pool, yacs::Entity* player, TransformComponent* playerTransform);
  ~PostPhysics();
  
  void update(const uint64_t &time) override;
private:
  dt::thread_pool* pool;
  yacs::Entity* player;
  TransformComponent* playerTransform;
};

#endif
