#include "core_funcs.h"

#include "game_funcs.h"
#include "EntityComponentSystem.h"
#include "Globals.h"
#include "delayed_work_system.h"
#include "global_components_indicies.h"
#include "type_info_component.h"
#include "ai_component.h"
#include "TransformComponent.h"
#include "interaction.h"
#include "PhysicsComponent.h"
#include "shared_collision_constants.h"
#include "entity_creator_resources.h"
// #include "UserDataComponent.h"
#include "collision_property.h"
#include "sound_system.h"
#include "random.h"
#include "map_data.h"

namespace devils_engine {
  namespace core {
    // быстрая проверка
    bool is_player(const yacs::entity* ent) {
      if (ent == nullptr) return false;
      auto info = ent->at<components::type_info>(game::entity::type_info);
      return info->bit_container.is_player();
    }
    
    bool is_dead(const yacs::entity* ent) {
      if (ent == nullptr) return false;
      auto info = ent->at<components::type_info>(game::entity::type_info);
      return info->bit_container.is_dead();
    }
    
    // нужно ли возвращать предыдущее значение? почему бы и нет
    bool set_dead(yacs::entity* ent) {
      if (ent == nullptr) return false;
      auto info = ent->at<components::type_info>(game::entity::type_info);
      return info->bit_container.set_dead(true);
    }
    
    bool deleted_state(const yacs::entity* ent) {
      if (ent == nullptr) return false;
      auto info = ent->at<components::type_info>(game::entity::type_info);
      return info->bit_container.delete_next();
    }
    
    bool has_collision_trigger(const yacs::entity* ent) {
      if (ent == nullptr) return false;
      auto info = ent->at<components::type_info>(game::entity::type_info);
      return info->bit_container.has_collision_trigger();
    }
    
    bool set_collision_trigger(yacs::entity* ent, const bool value) {
      if (ent == nullptr) return false;
      auto info = ent->at<components::type_info>(game::entity::type_info);
      return info->bit_container.set_collision_trigger(value);
    }
    
    void remove(yacs::entity* ent) {
      //ASSERT(false);
      
      if (ent == nullptr) return;
      //if (deleted_state(ent)) return;
      
      auto info = ent->at<components::type_info>(game::entity::type_info);
      const bool was_deleted = info->bit_container.set_delete(true);
      if (!was_deleted) {
        Global::get<utils::delayed_work_system>()->add_work([ent] () {
          Global::get<yacs::world>()->destroy_entity(ent);
        });
      }
      
      // для того чтобы это работало нормально у нас должно быть
      // функции ии -> функции состояния -> функции аттрибутов -> проверки взаимодействий (?) -> вызов последовательно всех задач добавленных в систему
      // нужен ли мьютекс энтити? нужно написать сначало по максимуму функции, а потом чекать что где у меня вызывается
      // возможно проверки взаимодействий нужно перенести позже задач, хотя разницы наверное нет
    }
    
    bool set_bit(yacs::entity* ent, const uint32_t &index, const bool value) {
      auto info = ent->at<components::type_info>(game::entity::type_info);
      return info->bit_container.set_bit(index, value);
    }
    
    bool get_bit(const yacs::entity* ent, const uint32_t &index) {
      auto info = ent->at<components::type_info>(game::entity::type_info);
      return info->bit_container.get_bit(index);
    }
    
    int64_t get_int(const yacs::entity* ent, const uint32_t &index) {
      auto info = ent->at<components::type_info>(game::entity::type_info);
      return info->int_container.get(index);
    }
    
    int64_t set_int(yacs::entity* ent, const uint32_t &index, const int64_t &value) {
      auto info = ent->at<components::type_info>(game::entity::type_info);
      return info->int_container.set(index, value);
    }
    
    int64_t inc_int(yacs::entity* ent, const uint32_t &index, const int64_t &value) {
      auto info = ent->at<components::type_info>(game::entity::type_info);
      return info->int_container.inc(index, value);
    }
    
    int64_t dec_int(yacs::entity* ent, const uint32_t &index, const int64_t &value) {
      auto info = ent->at<components::type_info>(game::entity::type_info);
      return info->int_container.dec(index, value);
    }
    
    int64_t and_int(yacs::entity* ent, const uint32_t &index, const int64_t &value) {
      auto info = ent->at<components::type_info>(game::entity::type_info);
      return info->int_container.con(index, value);
    }
    
    int64_t or_int (yacs::entity* ent, const uint32_t &index, const int64_t &value) {
      auto info = ent->at<components::type_info>(game::entity::type_info);
      return info->int_container.dis(index, value);
    }
    
    int64_t xor_int(yacs::entity* ent, const uint32_t &index, const int64_t &value) {
      auto info = ent->at<components::type_info>(game::entity::type_info);
      return info->int_container.ex_dis(index, value);
    }
    
    simd::vec4 player_pos() {
      auto trans = Global::player()->at<TransformComponent>(game::entity::transform);
      return trans->pos();
    }
    
    void teleport_entity(yacs::entity* ent, const simd::vec4 &new_pos) {
      if (ent == nullptr) return;
      
      Global::get<utils::delayed_work_system>()->add_work([ent, new_pos] () {
        components::basic_ai* ai = ent->at<components::func_ai>(game::monster::ai).get();
        if (ai == nullptr) ai = ent->at<components::tree_ai>(game::monster::ai).get();
        if (ai == nullptr) return;
        auto trans = ent->at<TransformComponent>(game::entity::transform);
        trans->pos() = new_pos;
        ai->teleported();
      });
    }
    
    void create_entity(const utils::id &type, const yacs::entity* parent, const game::ability_t* ability, const simd::vec4 &pos, const simd::vec4 &rot, const simd::vec4 &vel) {
      if (!type.valid()) return;
      // тут мы тоже добавляем создание в систему
      auto creator = Global::get<game::entity_creators_container>()->get(type);
      Global::get<utils::delayed_work_system>()->add_work([creator, parent, ability, pos, rot, vel] () {
        creator->create(parent, ability, pos, rot, vel);
      });
    }
    
    complex_attack_state slashing_attack(
      yacs::entity* ent, 
      const game::effect_t* effect, 
      const float &distance, 
      const float &start_angle, 
      const float &end_angle, 
      const size_t &time, 
      const float &speed, 
      const float plane[4], 
      const uint32_t &tick_count, 
      const size_t &tick_time
    ) {
      // тут нужно делать две вещи: проверять есть ли уже у нас такая атака
      // и если нет то создавать, причем создать мы можем как захотим
      // то есть ненужно огромную функцию криэйт энтити определять для этого
      // в таком энтити должно быть 3 компонента:
      // трансформ (наследуем от энтити), физика, интеракция
      // в интеракции мы можем запомнить энтити и эффект
      const size_t count = Global::get<yacs::world>()->count_components<core::slashing_interaction>();
      for (size_t i = 0; i < count; ++i) {
        auto handle = Global::get<yacs::world>()->get_component<core::slashing_interaction>(i);
        if (handle->ent == ent && handle->e == effect) {
          if (deleted_state(handle->to_deletion)) return complex_attack_state::done;
          else return complex_attack_state::proccessing;
        }
      }
      
      Global::get<utils::delayed_work_system>()->add_work([
        ent,
        effect,
        distance,
        start_angle,
        end_angle,
        time,
        speed,
        plane,
        tick_count,
        tick_time
      ] () {
        auto inter_ent = Global::get<yacs::world>()->create_entity();
        auto trans = ent->at<TransformComponent>(game::entity::transform);
        inter_ent->set<TransformComponent>(trans);
        const PhysicsComponent::CreateInfo info{
          {
            {0.0f, 0.0f, 0.0f, 0.0f},
            0.0f, 0.0f, 0.0f, 0.0f
          },
          {
            PhysicsType(false, SPHERE_TYPE, false, true, false, false),
            INTERACTION_COLLISION_TYPE,
            interaction_collision_filter,
            interaction_trigger_filter,
            0.0f,
            0.0f, 
            0.0f,
            distance,
            0.0f,
            UINT32_MAX,
            trans->index(),
            UINT32_MAX,
            UINT32_MAX,
            UINT32_MAX,
            Type()
          },
          inter_ent
        };
        auto phys = inter_ent->add<PhysicsComponent>(info);
        auto inter = inter_ent->add<core::slashing_interaction>();
        
        inter->ent = ent;
        inter->e = effect;
        inter->t = core::interaction::type::slashing;
        inter->distance = distance;
        inter->start_angle = start_angle;
        inter->end_angle = end_angle;
        inter->time = time;
        inter->speed = speed;
        inter->plane[0] = plane[0];
        inter->plane[1] = plane[1];
        inter->plane[2] = plane[2];
        inter->plane[3] = 0.0f;
        inter->tick_count = tick_count;
        inter->tick_time = tick_time;
        inter->current_time = 0;
        inter->last_time = 0;
        trans->pos().storeu(inter->last_pos),
        trans->rot().storeu(inter->last_dir);
        inter->trans = trans.get();
        inter->phys = phys.get();
        inter->to_deletion = inter_ent;
      });
      
      return complex_attack_state::creating;
    }
    
    bool remove_slashing_attack(yacs::entity* ent, const game::effect_t* effect) {
      // ищем среди компонентов интеракции компонент с таким энтити и эффектом
      // и удаляем, удаление по идее раньше чем следующий тик атаки
      
      const size_t count = Global::get<yacs::world>()->count_components<core::slashing_interaction>();
      for (size_t i = 0; i < count; ++i) {
        auto handle = Global::get<yacs::world>()->get_component<core::slashing_interaction>(i);
        if (handle->ent == ent && handle->e == effect) {
          handle->to_deletion->unset<TransformComponent>();
          remove(handle->to_deletion);
          return true;
        }
      }
      
      return false;
    }
    
    complex_attack_state stabbing_attack(
      yacs::entity* ent, 
      const game::effect_t* effect, 
      const float &min_dist, 
      const float &max_dist, 
      const size_t &time, 
      const float &speed, 
      const uint32_t &tick_count, 
      const size_t &tick_time
    ) {
      const size_t count = Global::get<yacs::world>()->count_components<core::stabbing_interaction>();
      for (size_t i = 0; i < count; ++i) {
        auto handle = Global::get<yacs::world>()->get_component<core::stabbing_interaction>(i);
        if (handle->ent == ent && handle->e == effect) {
          if (deleted_state(handle->to_deletion)) return complex_attack_state::done;
          else return complex_attack_state::proccessing;
        }
      }
      
      Global::get<utils::delayed_work_system>()->add_work([
        ent,
        effect,
        min_dist,
        max_dist,
        time,
        speed,
        tick_count,
        tick_time
      ] () {
        auto inter_ent = Global::get<yacs::world>()->create_entity();
        auto trans = ent->at<TransformComponent>(game::entity::transform);
        inter_ent->set<TransformComponent>(trans);
        const PhysicsComponent::CreateInfo info{
          {
            {0.0f, 0.0f, 0.0f, 0.0f},
            0.0f, 0.0f, 0.0f, 0.0f
          },
          {
            PhysicsType(false, SPHERE_TYPE, false, true, false, false),
            INTERACTION_COLLISION_TYPE,
            interaction_collision_filter,
            interaction_trigger_filter,
            0.0f,
            0.0f, 
            0.0f,
            min_dist,
            0.0f,
            UINT32_MAX,
            trans->index(),
            UINT32_MAX,
            UINT32_MAX,
            UINT32_MAX,
            Type()
          },
          inter_ent
        };
        auto phys = inter_ent->add<PhysicsComponent>(info);
        auto inter = inter_ent->add<core::stabbing_interaction>();
        
        inter->ent = ent;
        inter->e = effect;
        inter->t = core::interaction::type::stabbing;
        inter->min_dist = min_dist;
        inter->max_dist = max_dist;
        inter->time = time;
        inter->speed = speed;
        inter->tick_count = tick_count;
        inter->tick_time = tick_time;
        inter->current_time = 0;
        inter->last_time = 0;
        inter->trans = trans.get();
        inter->phys = phys.get();
        inter->to_deletion = inter_ent;
      });
      
      return complex_attack_state::creating;
    }
    
    bool remove_stabbing_attack(yacs::entity* ent, const game::effect_t* effect) {
      const size_t count = Global::get<yacs::world>()->count_components<core::stabbing_interaction>();
      for (size_t i = 0; i < count; ++i) {
        auto handle = Global::get<yacs::world>()->get_component<core::stabbing_interaction>(i);
        if (handle->ent == ent && handle->e == effect) {
          handle->to_deletion->unset<TransformComponent>();
          remove(handle->to_deletion);
          return true;
        }
      }
      
      return false;
    }
    
    ray_state check_visability(const yacs::entity* ent, const yacs::entity* target) {
      // необходимо прокинуть луч к таргету, причем нужно запомнить как то индекс
      // запомнить индекс или запомнить ответ, в любом случае некоторые функции
      // могут не вызваться в следующем кадре
      // лучше бы здесь наверное передавать функцию, которую мы вызовем в случае если мы видим объект
      // с передачей функции проблема заключается в том что мы ее вызываем неизвестно где
      // а нам тут скорее нужно быстренько проверить и вернуться обратно в код
      // но проверка видимости в любом случае полезная вещь
    }
    
    bool ray_shot(yacs::entity* ent, const simd::vec4 &dir, const game::effect_t* effect) {
      return ray_shot(ent, dir, effect, game::damage_ent);
    }
    
    bool ray_shot(yacs::entity* ent, const simd::vec4 &dir, const game::effect_t* effect, const interaction_callback &callback) {
      // можно создать как компонент в мире
      // создавать нужно после проверки физики
      auto trans = ent->at<TransformComponent>(game::entity::transform);
      auto phys = ent->at<PhysicsComponent>(game::entity::physics);
      if (!trans.valid()) throw std::runtime_error("Trying shoot ray from obj without transform");
      const uint32_t ign = phys->getIndexContainer().objectDataIndex;
      const RayData ray(trans->pos(), simd::normalize(dir), ign, interaction_collision_filter);
      //const uint32_t ray_index = Global::get<PhysicsEngine>()->add(ray);
      const uint32_t ray_index = Global::get<PhysicsEngine>()->add_ray_boxes(ray);
      
      Global::get<utils::delayed_work_system>()->add_work([ent, effect, ray_index, callback] () {
//         const auto ray_count = Global::get<PhysicsEngine>()->getRayTracingSize();
//         const auto data = Global::get<PhysicsEngine>()->getRayTracingData();
//         for (size_t i = 0; i < ray_count; ++i) {
//           const auto &o = data->at(i);
//           if (o.firstIndex != ray_index) continue;
//           const auto cont = Global::get<PhysicsEngine>()->getIndexContainer(o.secondIndex);
//           yacs::entity* target = reinterpret_cast<UserDataComponent*>(cont->userData)->entity;
//           game::damage_ent(ent, target, effect); 
//         }
        const auto ret = Global::get<PhysicsEngine>()->get_ray_boxes(ray_index);
        yacs::entity* target = reinterpret_cast<yacs::entity*>(ret->userData);
        callback(ent, target, effect);
      });
      
      return true;
    }
    
    void hit_target(yacs::entity* ent, yacs::entity* target, const game::effect_t* effect) {
      // просто добавляем задачу в систему
      Global::get<utils::delayed_work_system>()->add_work([ent, target, effect] () {
        game::damage_ent(ent, target, effect);
      });
    }
    
    void hit_target(yacs::entity* ent, yacs::entity* target, const game::effect_t* effect, const interaction_callback &callback) {
      Global::get<utils::delayed_work_system>()->add_work([ent, target, effect, callback] () {
        callback(ent, target, effect);
      });
    }
    
    bool start_sound(const sound::info &info) {
      return Global::get<systems::sound>()->play(info);
    }
    
    bool stop_sound(const yacs::entity* ent) {
      return Global::get<systems::sound>()->stop(ent);
    }
    
    utils::id current_map() {
      return Global::get<game::map_data_container>()->current;
    }
    
    utils::id current_episod() {
      return Global::get<game::map_data_container>()->episod;
    }
  }
  
  namespace random {
    uint32_t get() {
      return Global::get<utils::random>()->get();
    }
    
    float norm() {
      return Global::get<utils::random>()->norm();
    }
    
    bool chance(const float &percent) {
      return Global::get<utils::random>()->chanse(percent);
    }
    
    int32_t range(const int32_t &min, const int32_t &max) {
      return Global::get<utils::random>()->rangei(min, max);
    }
    
    uint32_t dice(const uint32_t &rolls, const uint32_t &faces) {
      return Global::get<utils::random>()->dice(rolls, faces);
    }
  }
}
