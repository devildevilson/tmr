#include "ai_component.h"

#include "Globals.h"
#include "tiny_behaviour/TinyBehavior.h"
#include "game_funcs.h"
#include "PhysicsComponent.h"
#include "global_components_indicies.h"
#include "shared_collision_constants.h"
#include "vertex_component.h"
#include "graph.h"
// #include "UserDataComponent.h"
#include "core_funcs.h"

namespace devils_engine {
  namespace components {
    basic_ai::basic_ai(const create_info &info) : ent(info.ent), ray_index(UINT32_MAX), ground(nullptr), old_ground(nullptr) {
      for (size_t i = 0; i < memory_max_size; ++i) {
        data[i].first = utils::id();
        data[i].second = nullptr;
      }
    }
    
    void basic_ai::new_frame() {
      if (ground == nullptr) {
        // кидаем луч вниз
        // на этом кадре лучше не обновляться
        // для одиночных лучей с пересечением земли нужно сделать отдельную функцию
        // для одиночных лучей пересекающих только боксовые и сферические объекты можно сделать тоже отдельную функцию
        
        if (ray_index == UINT32_MAX) {
          auto trans = ent->at<TransformComponent>(game::entity::transform);
          auto phys = ent->at<PhysicsComponent>(game::entity::physics);
          const RayData ray(trans->pos(), PhysicsEngine::getGravityNorm(), 0.0f, 10000.0f, phys->getIndexContainer().objectDataIndex, WALL_COLLISION_TYPE | DOOR_COLLISION_TYPE);
          ray_index = Global::get<PhysicsEngine>()->add_ray_poligons(ray);
        } else {
          // получаем объект с которым пересеклись
          auto cont = Global::get<PhysicsEngine>()->get_ray_polygons(ray_index);
          auto ent = reinterpret_cast<yacs::entity*>(cont->userData);
          ground = ent;
          ray_index = UINT32_MAX;
        }
      } else {
        // проверяем стоим ли мы на земле до сих пор
        // нужно гарантировать что указатель указывает только на землю
        
        auto trans = ent->at<TransformComponent>(game::entity::transform);
        auto phys = ground->at<PhysicsComponent>(game::entity::physics);
        simd::vec4 p;
        const bool collide = phys->collide_ray(trans->pos(), PhysicsEngine::getGravityNorm(), p);
        if (!collide) {
          auto vertex = ground->at<components::vertex>(game::wall::vertex);
          size_t mem = 0;
          auto edge = vertex->next_edge(mem);
          while (edge != nullptr) {
            auto new_vertex = edge->vertices.first == vertex.get() ? edge->vertices.second : edge->vertices.first;
            auto new_vertex_ent = new_vertex->entity();
            auto new_vertex_phys = new_vertex_ent->at<PhysicsComponent>(game::entity::physics);
            
            const bool collide = new_vertex_phys->collide_ray(trans->pos(), PhysicsEngine::getGravityNorm(), p);
            if (collide) {
              old_ground = ground;
              ground = new_vertex_ent;
              break;
            }
            
            edge = vertex->next_edge(mem);
          }
        }
      }
      
      for (size_t i = 0; i < memory_max_size; ++i) {
        if (core::deleted_state(data[i].second)) data[i].second = nullptr;
      }
    }
    
    void basic_ai::teleported() {
      old_ground = ground;
      ground = nullptr;
    }
    
    tree_ai::tree_ai(const create_info &info) : basic_ai({info.ent}), tree(info.tree), running_node(nullptr), update_time(info.update_time), current_time(0) {}
    void tree_ai::update(const size_t &time) {
      new_frame();
      current_time += time;
      if (ground == nullptr) return;
      
      if (running_node != nullptr && current_time < update_time) {
        auto state = running_node->update(ent, &running_node);
        if (state != tb::Node::status::running) running_node = nullptr;
        return;
      }
      
      running_node = nullptr;
      if (running_node == nullptr || current_time >= update_time) tree->update(ent, &running_node);
    }
    
    func_ai::func_ai(const create_info &info) : basic_ai({info.ent}), last_status(ai::status::running), ai_func(info.func), update_time(info.update_time), current_time(0) {}
    void func_ai::update(const size_t &time) {
      new_frame();
      current_time += time;
      if (ground == nullptr) return;
      if (last_status == ai::status::running || current_time >= update_time) last_status = ai_func(ent);
    }
  }
}
