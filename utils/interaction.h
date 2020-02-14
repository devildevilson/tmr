#ifndef INTERACTION_H
#define INTERACTION_H

#include <cstddef>
#include <cstdint>
#include <vector>
#include <functional>

#include "ArrayInterface.h"
#include "PhysicsTemporary.h"

// по сути нам просто нужен способ передать интеракцию дальше по стеку исполнения
// скорее всего единственное для чего это нужно так это правильное положение декали
// декалина должна пересечь стену чтобы на ней появиться 

namespace yacs {
  class entity;
}

class TransformComponent;
class PhysicsComponent;

namespace devils_engine {
  namespace game {
    struct effect_t;
  }
  
  namespace core {
    class interaction {
    public:
      enum class type {
        slashing,
        stabbing,
        none
      };
      
      yacs::entity* ent;
      const game::effect_t* e;
      type t;
      
      static Container<Transform>* transforms;
      static Container<simd::mat4>* matrices;
    };
    
    class slashing_interaction : public interaction {
    public:
      struct objects {
        yacs::entity* ent;
        size_t inter_time;
        uint32_t ticks;
      };
      
      float distance;
      float start_angle;
      float end_angle;
      size_t time;
      float speed; 
      float plane[4];
      uint32_t tick_count;
      size_t tick_time;
      
      size_t current_time;
      size_t last_time;
      float last_pos[4];
      float last_dir[4];
      std::vector<objects> objs;
      TransformComponent* trans;
      PhysicsComponent* phys;
      
      yacs::entity* to_deletion;
      
      void update(const size_t &time); // делаем необходимые дела заполняя систему работ
      size_t find(yacs::entity* entity) const;
    };
    
    class stabbing_interaction : public interaction {
    public:
      struct objects {
        yacs::entity* ent;
        size_t inter_time;
        uint32_t ticks;
      };
      
      float min_dist;
      float max_dist;
      size_t time;
      float speed; 
      uint32_t tick_count;
      size_t tick_time;
      
      size_t current_time;
      size_t last_time;
      std::vector<objects> objs;
      TransformComponent* trans;
      PhysicsComponent* phys;
      
      yacs::entity* to_deletion;
      
      // здесь еще видимо нужно таскать с собой функцию, хотя с другой стороны ее можно задать глобально
      //std::function<void(yacs::entity*, yacs::entity*, const game::effect_t*, const interaction*, const uint32_t&)> func;
      
      void update(const size_t &time); // делаем необходимые дела заполняя систему работ
      size_t find(yacs::entity* entity) const;
    };
  }
}

#endif
