#ifndef OVERLAY_H
#define OVERLAY_H

#include "Engine.h"
#include "interface.h"
#include "interface_context.h"

namespace yacs {
  class entity;
}

struct nuklear_data;
class TimeMeter;
class InventoryComponent;
class AttributeComponent;
class EffectComponent;
class TransformComponent;

namespace devils_engine {
  namespace interface {
    class overlay { // : public Engine
    public:
      struct create_info {
        //nuklear_data* nuklear;
        context* nuklear;
        yacs::entity* player;
        TimeMeter* tm;
      };
      overlay(const create_info &info);
      
      void draw(const data::extent &screen_size) const;
    private:
      //nuklear_data* nuklear;
      context* nuklear;
      yacs::entity* player;
      TimeMeter* tm;
      
      InventoryComponent* inventory;
      AttributeComponent* attribs;
      EffectComponent* effects;
      TransformComponent* transform;
    };
  }
}

#endif
