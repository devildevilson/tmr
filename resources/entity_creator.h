#ifndef ENTITY_CREATOR_H
#define ENTITY_CREATOR_H

#include "Type.h"
#include "id.h"
#include "attribute.h"
#include "attributes_component.h"
#include "Utility.h"

class TransformComponent;
class PhysicsComponent;
class InputComponent;
struct UserDataComponent;
class SoundComponent;

namespace tb {
  class BehaviorTree;
}

namespace devils_engine {
  namespace game {
    struct ability_t;
  }
  
  namespace properties {
    struct pickup;
    struct collision;
  }
  
  namespace components {
    struct type_info;
    struct states;
    struct input;
    struct sprite_graphics;
    struct point_light;
    struct basic_ai;
    struct tree_ai;
    struct func_ai;
    class attributes;
    class effects;
    class inventory;
    class movement;
    class abilities;
  }
  
  namespace core {
    struct state_t;
    
    class entity_creator {
    public:
      using collision_func_t = std::function<void(yacs::entity*, yacs::entity*, const utils::id &)>;
      
      struct pickup_data {
        utils::id id;
        size_t quantity;
      };
      
      struct intelligence {
        tb::BehaviorTree* tree; // как это будет выглядить?
        utils::id func;
      };
      
      struct phys_data {
        bool dynamic;
        uint32_t collisionGroup;
        uint32_t collisionFilter;
        uint32_t collisionTrigger;
        float stairHeight;
        float height;
        float width;
        float gravCoef;
        utils::id collision_property;
      };
      
      struct create_info {
        std::vector<components::attributes::create_info::init<core::float_type>> float_init;
        std::vector<components::attributes::create_info::init<core::int_type>> int_init;
        std::vector<const game::ability_t*> abilities;
        std::vector<const state_t*> states;
        utils::id id;
        const state_t* default_state;
        pickup_data pickup;
        intelligence intel;
        phys_data physics;
        collision_func_t collision_func;
      };
      entity_creator(const struct create_info &info);
      // дополнительно потребуются флаги и тэги
      yacs::entity* create(const yacs::entity* parent, const game::ability_t* ability, const simd::vec4 &pos, const simd::vec4 &rot, const simd::vec4 &vel) const;
      
      //void pickup_func(yacs::entity* ent, yacs::entity* item, const utils::id &id, const size_t &quantity) const;
      // по идее этой функции достаточно для того чтобы сделать и взаимодействие с итемом и просто коллизию
      // количество мы добавим в аттрибуты
      void collision_func(yacs::entity* ent, yacs::entity* visitor, const utils::id &data) const; 
    private:
      struct creation_data {
        yacs::entity* ent;
        const yacs::entity* parent;
        const game::ability_t* ability;
        simd::vec4 pos;
        simd::vec4 rot;
        simd::vec4 vel;
      };
      
      std::vector<components::attributes::create_info::init<core::float_type>> float_init;
      std::vector<components::attributes::create_info::init<core::int_type>> int_init;
      // нахрена мне абилки тут нужны?
      std::vector<const game::ability_t*> abilities;
      std::vector<const state_t*> states;
      utils::id id;
      const state_t* default_state;
      pickup_data pickup;
      intelligence intel;
      phys_data physics;
      collision_func_t cfunc;
      
      components::type_info* create_info(const creation_data &data) const;
//       UserDataComponent* create_usrdata(const creation_data &data) const;
      TransformComponent* create_transform(const creation_data &data) const;
      InputComponent* create_input(const creation_data &data) const;
      PhysicsComponent* create_physics(const creation_data &data) const;
      components::sprite_graphics* create_graphics(const creation_data &data) const;
      components::point_light* create_point_light(const creation_data &data) const;
      components::states* create_states(const creation_data &data) const;
//       SoundComponent* create_sound(const creation_data &data) const;
      components::attributes* create_attributes(const creation_data &data) const;
      components::effects* create_effects(const creation_data &data) const;
      properties::collision* create_collision(const creation_data &data) const;
      components::movement* create_movement(const creation_data &data) const;
      components::inventory* create_inventory(const creation_data &data) const;
      //WeaponsComponent* create_weapons(const creation_data &data) const;
      components::abilities* create_abilities(const creation_data &data) const;
      components::basic_ai* create_ai(const creation_data &data) const;
    };
  }
}

#endif
