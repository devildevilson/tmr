#include "entity_creator.h"

#include "EntityComponentSystem.h"
#include "Utility.h"
#include "Globals.h"
#include "TransformComponent.h"
#include "PhysicsComponent.h"
#include "InputComponent.h"
// #include "SoundComponent.h"
#include "graphics_component.h"
#include "states_component.h"
#include "abilities_component.h"
#include "type_info_component.h"
// #include "UserDataComponent.h"
#include "ai_component.h"
#include "attributes_component.h"
#include "effects_component.h"
#include "inventory_component.h"
#include "global_components_indicies.h"
#include "movement_component.h"
#include "collision_property.h"
#include "core_funcs.h"
#include "delayed_work_system.h"

namespace devils_engine {
  namespace core {
    entity_creator::entity_creator(const struct create_info &info) 
      : float_init(info.float_init),
        int_init(info.int_init),
        abilities(info.abilities),
        states(info.states),
        id(info.id),
        default_state(info.default_state),
        pickup(info.pickup),
        intel(info.intel),
        physics(info.physics),
        cfunc(info.collision_func)
    {}
    
    yacs::entity* entity_creator::create(const yacs::entity* parent, const game::ability_t* ability, const simd::vec4 &pos, const simd::vec4 &rot, const simd::vec4 &vel) const {
      auto ent = Global::get<yacs::world>()->create_entity();
      
      // мне нужно как то определить какой ТИП объекта я хочу создать
      // монстр, предмет, декор
      // монстр - интеллект, предмет - свойство пикап, декор - ни того ни другого ???
      // в основном нужно только определить игрок/не игрок
      // не нужно опираться сильно на ТИПы, лучше создать без ограничений все компоненты если потребуется
      // так можно сделать какие нибудь интересные эффекты (например движущийся объект с интеллектом, который можно подобрать)
      
      const creation_data data{
        ent,
        parent,
        ability,
        pos,
        rot,
        vel
      };
      
      auto info = create_info(data);
//       auto user = create_usrdata(data);
      auto trans = create_transform(data);
      auto input = create_input(data);
      auto phys = create_physics(data);
      auto graphics = create_graphics(data);
      auto states = create_states(data);
//       auto sounds = create_sound(data);
      auto light = create_point_light(data);
      auto attribs = create_attributes(data);
      auto effects = create_effects(data);
//       auto collision = create_collision(data);
      
      if (cfunc) info->bit_container.set_collision_trigger(true);
      
      (void)info;
//       (void)user;
      (void)trans;
      (void)input;
      (void)phys;
      (void)graphics;
      (void)states;
//       (void)sounds;
      (void)light;
      (void)attribs;
      (void)effects;
//       (void)collision;
      
      return ent;
    }
    
    bool is_entity_type_eq(const yacs::entity* ent, const utils::id &id) {
      auto info = ent->at<components::type_info>(game::entity::type_info);
      return info->id == id;
    }
    
    void entity_creator::collision_func(yacs::entity* ent, yacs::entity* visitor, const utils::id &data) const {
//       auto second_info = ent->at<components::type_info>(game::entity::type_info);
//       std::cout << "ent " << second_info->id.name() << " deleted_state " << core::deleted_state(ent) << " coll trigger " << core::has_collision_trigger(ent) << "\n";
//       std::cout << "container " << second_info->bit_container.container << "\n";
      
      ASSERT(is_entity_type_eq(ent, id));
      
      if (!cfunc) return;
      if (core::deleted_state(ent)) return;
      if (core::deleted_state(visitor)) return;
      if (!core::has_collision_trigger(ent)) return;
      
      const utils::id final_data = data.valid() ? data : physics.collision_property;
      
      Global::get<utils::delayed_work_system>()->add_work([this, ent, visitor, final_data] () {
        if (core::deleted_state(ent)) return;
        if (core::deleted_state(visitor)) return;
        if (!core::has_collision_trigger(ent)) return;
        cfunc(ent, visitor, final_data);
      });
    }
    
    components::type_info* entity_creator::create_info(const creation_data &data) const {
      auto info = data.ent->add<components::type_info>();
      info->created_ability = data.ability;
      info->ent = data.ent;
      info->parent = data.parent;
      info->id = id;
      info->states_count = states.size();
      info->states = states.empty() ? nullptr : states.data();
//       info->collision_property = physics.collision_property;
//       info->item_property = pickup.id;
//       info->item_quantity = pickup.quantity;
      //if (!cfunc) info->bit_container.set_collision_trigger(true);
      return info.get();
    }
    
//     UserDataComponent* entity_creator::create_usrdata(const creation_data &data) const {
//       auto comp = data.ent->add<UserDataComponent>().get();
//       ASSERT(data.ent->at<UserDataComponent>(game::entity::user_data).valid());
//       return comp;
//     }
    
    TransformComponent* entity_creator::create_transform(const creation_data &data) const {
      auto comp = data.ent->add<TransformComponent>(data.pos, data.rot, simd::vec4(physics.width, physics.height, physics.width, 0.0f)).get();
      ASSERT(data.ent->at<TransformComponent>(game::entity::transform).valid());
      return comp;
    }
    
    InputComponent* entity_creator::create_input(const creation_data &data) const {
      if (!physics.dynamic) {
        data.ent->set(yacs::component_handle<InputComponent>(nullptr));
        ASSERT(!data.ent->at<InputComponent>(game::entity::input).valid());
        return nullptr;
      }
      
      auto comp = data.ent->add<InputComponent>().get();
      ASSERT(data.ent->at<InputComponent>(game::entity::input).valid());
      return comp;
    }
    
    const Type box_shape = Type::get("boxShape");
    
    PhysicsComponent* entity_creator::create_physics(const creation_data &data) const {
      auto input = data.ent->at<InputComponent>(game::entity::input);
      auto trans = data.ent->at<TransformComponent>(game::entity::transform);
//       auto user = data.ent->at<UserDataComponent>(game::entity::user_data);
      
      const bool visible = !states.empty();
      // энтити без состояний скорее всего будет невидим, лучи?
      //                                                             rays  visible
      const PhysicsType type(physics.dynamic, BBOX_TYPE, true, true, true, visible);
      const PhysicsComponent::CreateInfo info{
        {
          {0.0f, 0.0f, 0.0f, 0.0f},
          7.0f, 80.0f, 0.0f, 0.0f // скорость нужно добавить в инициализацию
        },
        {
          type,
          physics.collisionGroup,
          physics.collisionFilter,
          physics.collisionTrigger,
          physics.stairHeight,
          1.0f,
          4.0f,
          1.0f, // нужно посчитать радиус 
          physics.gravCoef,
          
          input.valid() ? input->inputIndex : UINT32_MAX,
          trans->index(),
          UINT32_MAX,
          UINT32_MAX,
          UINT32_MAX,
          
          box_shape
        },
        data.ent
      };
      
      auto comp = data.ent->add<PhysicsComponent>(info).get();
      comp->setVelocity(data.vel);
      ASSERT(data.ent->at<PhysicsComponent>(game::entity::physics).valid());
      return comp;
    }
    
    components::sprite_graphics* entity_creator::create_graphics(const creation_data &data) const {
      if (states.empty()) {
        data.ent->set(yacs::component_handle<components::sprite_graphics>(nullptr));
        ASSERT(!data.ent->at<components::sprite_graphics>(game::entity::graphics).valid());
        return nullptr;
      }
      
      auto comp = data.ent->add<components::sprite_graphics>().get();
      ASSERT(data.ent->at<components::sprite_graphics>(game::entity::graphics).valid());
      comp->ent = data.ent;
      return comp;
    }
    
    components::point_light* entity_creator::create_point_light(const creation_data &data) const {
      // нужно доделать
      data.ent->set(yacs::component_handle<components::point_light>(nullptr));
      ASSERT(!data.ent->at<components::point_light>(game::monster::light).valid());
      return nullptr;
    }
    
    components::states* entity_creator::create_states(const creation_data &data) const {
      if (states.empty()) {
        data.ent->set(yacs::component_handle<components::states>(nullptr));
        ASSERT(!data.ent->at<components::states>(game::entity::states).valid());
        return nullptr;
      }
      
      auto comp = data.ent->add<components::states>().get();
      ASSERT(data.ent->at<components::states>(game::entity::states).valid());
      comp->ent = data.ent;
      comp->current = states[0];
      comp->current_time = SIZE_MAX;
      return comp;
    }
    
//     SoundComponent* entity_creator::create_sound(const creation_data &data) const {
//       // звук тоже нужно переделать, возможно даже отдельный компонент не нужен
//       data.ent->set(yacs::component_handle<SoundComponent>(nullptr));
//       ASSERT(!data.ent->at<SoundComponent>(game::entity::sounds).valid());
//       return nullptr;
//     }
    
    components::attributes* entity_creator::create_attributes(const creation_data &data) const {
      if (float_init.empty() && int_init.empty()) {
        data.ent->set(yacs::component_handle<components::attributes>(nullptr));
        ASSERT(!data.ent->at<components::attributes>(game::monster::attributes).valid());
        return nullptr;
      }
      
      auto comp = data.ent->add<components::attributes>(components::attributes::create_info{data.ent, float_init, int_init}).get();
      ASSERT(data.ent->at<components::attributes>(game::monster::attributes).valid());
      return comp;
    }
    
    components::effects* entity_creator::create_effects(const creation_data &data) const {
      if (float_init.empty() && int_init.empty()) {
        data.ent->set(yacs::component_handle<components::effects>(nullptr));
        ASSERT(!data.ent->at<components::effects>(game::monster::effects).valid());
        return nullptr;
      }
      
      auto comp = data.ent->add<components::effects>(components::effects::create_info{data.ent}).get();
      ASSERT(data.ent->at<components::effects>(game::monster::effects).valid());
      return comp;
    }
    
//     properties::collision* entity_creator::create_collision(const creation_data &data) const {
//       if (collision_func) {
//         auto comp = data.ent->add<properties::collision>().get();
//         comp->func = &collision_func;
//         auto info = data.ent->at<components::type_info>(game::entity::type_info);
//         info->bit_container.set_collision_trigger(true);
//         ASSERT(data.ent->at<properties::collision>(game::monster::collision_property).valid());
//         return comp;
//       }
//       
//       data.ent->set<>(yacs::component_handle<properties::collision>(nullptr));
//       ASSERT(!data.ent->at<properties::collision>(game::monster::collision_property).valid());
//       return nullptr;
//     }
    
    components::movement* entity_creator::create_movement(const creation_data &data) const {
      if (!physics.dynamic) {
        data.ent->set(yacs::component_handle<components::movement>(nullptr));
        ASSERT(!data.ent->at<components::movement>(game::monster::movement).valid());
        return nullptr;
      }
      
      auto comp = data.ent->add<components::movement>(components::movement::create_info{data.ent, intel.func}).get();
      ASSERT(data.ent->at<components::movement>(game::monster::movement).valid());
      return comp;
    }
    
    components::inventory* entity_creator::create_inventory(const creation_data &data) const {
      // инвентарь будет только у игрока
      (void)data;
      return nullptr;
    }
    
    components::abilities* entity_creator::create_abilities(const creation_data &data) const {
      // абилки не нужны вовсе
      (void)data;
      return nullptr;
    }
    
    components::basic_ai* entity_creator::create_ai(const creation_data &data) const {
      // как будет выглядеть ии я так и не понимаю
      if (intel.tree != nullptr) {
        auto comp = data.ent->add<components::tree_ai>(components::tree_ai::create_info{data.ent, intel.tree, HALF_SECOND}).get();
        ASSERT(data.ent->at<components::movement>(game::monster::movement).valid());
        return comp;
      }
      
      data.ent->set<>(yacs::component_handle<components::tree_ai>(nullptr));
      ASSERT(!data.ent->at<components::tree_ai>(game::monster::ai).valid());
      return nullptr;
    }
  }
}
