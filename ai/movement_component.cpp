#include "movement_component.h"

#include "Globals.h"
#include "ai_component.h"
#include "vertex_component.h"
#include "pathfinder_system.h"
#include "global_components_indicies.h"
#include "EntityComponentSystem.h"
#include "TransformComponent.h"
#include "InputComponent.h"
#include "PhysicsComponent.h"
#include "Physics.h"
#include "shared_time_constant.h"

namespace devils_engine {
  namespace components {
    movement::movement(const create_info &info) : pathfinding_time(0), current_segment(0), ent(info.ent), path_find_type(info.path_find_type), current_path{nullptr, {utils::id(), nullptr, nullptr}}, old_path{nullptr, {utils::id(), nullptr, nullptr}} {}
      
    movement::path_state movement::find_path(const yacs::entity* target) {
      const size_t currentTime = Global::mcsSinceEpoch();
      if (current_path.path != nullptr && (currentTime - pathfinding_time) > HALF_SECOND) {
        old_path.path = current_path.path;
        old_path.req = current_path.req;
        current_path.path = nullptr;
        current_path.req.end = nullptr;
        current_path.req.start = nullptr;
      }
      
      if (target == nullptr) return path_state::not_found;
      
      // тут нужно найти вершину на которой мы сейчас стоим
      // эта инфа по идее должна лежать в ии компонентах
      components::basic_ai* ai = ent->at<components::tree_ai>(game::monster::ai).get();
      if (ai == nullptr) ai = ent->at<components::func_ai>(game::monster::ai).get();
      ASSERT(ai != nullptr);
      
      const components::basic_ai* target_ai = target->at<components::tree_ai>(game::monster::ai).get();
      if (target_ai == nullptr) target_ai = target->at<components::func_ai>(game::monster::ai).get();
      ASSERT(target_ai != nullptr);
      
      auto self_ground = ai->ground;
      ASSERT(self_ground != nullptr);
      
      auto target_ground = target_ai->ground;
      if (target_ground == nullptr) return path_state::not_found;
      
      auto self_vertex = self_ground->at<components::vertex>(game::wall::vertex);
      ASSERT(self_vertex != nullptr);
      
      auto target_vertex = target_ground->at<components::vertex>(game::wall::vertex);
      ASSERT(target_vertex != nullptr);
      
      if (target_vertex == self_vertex) return path_state::found;
      
      if (current_path.path == nullptr) {
        if (current_path.req.start != nullptr && current_path.req.end != nullptr) {
          const auto &path_data = Global::get<systems::pathfinder>()->get_path(current_path.req);
          
          if (path_data.state == path::find_state::delayed) return path_state::finding;
          else if (path_data.state == path::find_state::exist) {
            if (old_path.path != nullptr) release_old_path();

            current_path.path = path_data.path;

            current_segment = 1;

            std::cout << "path finding success" << "\n";

            return path_state::found;
          } else if (path_data.state == path::find_state::not_exist) {
            if ((currentTime - pathfinding_time) > QUARTER_SECOND) release_current_path();

            std::cout << "path finding failed" << "\n";

            return path_state::not_found;
          }
          
          const path::request req{
            path_find_type,
            self_vertex.get(),
            target_vertex.get()
          };
          Global::get<systems::pathfinder>()->queue_request(req);

          pathfinding_time = currentTime;
          current_path.req = req;

          std::cout << "path finding running" << "\n";

          return path_state::finding;
        }
      }
      
      std::cout << "path already fouded" << "\n";
      return path_state::found;
    }
    
    movement::state movement::travel_path() {
      if (current_path.path == nullptr && old_path.path == nullptr) return state::path_not_exist;
      
      auto trans = ent->at<TransformComponent>(game::entity::transform);
      auto input = ent->at<InputComponent>(game::entity::input);

      path::container* path = current_path.path == nullptr ? old_path.path : current_path.path;
      const size_t lastIndex = path->array.size()-1;

      if (lastIndex < current_segment) {
        release_paths();

        std::cout << "path moving success" << "\n";

        return state::end_travel;
      }

      // нужно сделать проверку в правильном ли мы месте сейчас, как?

      const simd::vec4 nextPos = follow_path(DELTA_TIME_CONSTANT, path, current_segment);
      input->setMovement(1.0f, 0.0f, 0.0f);
      //trans->rot() = simd::normalize(input->front());
      mod_entity_rotation();

      const simd::vec4 point = projectPointOnPlane(-PhysicsEngine::getGravityNorm(), trans->pos(), nextPos);
      const float tolerance = 0.4f;
      //if (simd::distance2(trans->pos(), point) < tolerance) ++current_segment;
      current_segment += uint32_t(simd::distance2(trans->pos(), point) < tolerance);

      PRINT_VAR("currentPathSegment", current_segment)
      PRINT_VAR("lastIndex   ", lastIndex)

      std::cout << "path moving" << "\n";

      return path == current_path.path ? state::travel_path : state::travel_old_path;
    }
    
    bool movement::path_exist() const {
      return current_path.path != nullptr || old_path.path != nullptr;
    }
    
    void movement::move(const simd::vec4 &dir) {
      auto input = ent->at<InputComponent>(game::entity::input);
      input->front() = dir;
      input->setMovement(1.0f, 0.0f, 0.0f);
      mod_entity_rotation();
    }
    
    void movement::pursue(const yacs::entity* target) {
      auto input = ent->at<InputComponent>(game::entity::input);
      auto target_trans = target->at<TransformComponent>(game::entity::transform);
      seek(target_trans->pos());
      input->setMovement(1.0f, 0.0f, 0.0f);
      mod_entity_rotation();
    }
    
    void movement::flee(const yacs::entity* target) {
      auto input = ent->at<InputComponent>(game::entity::input);
      auto target_trans = target->at<TransformComponent>(game::entity::transform);
      flee(target_trans->pos());
      input->setMovement(1.0f, 0.0f, 0.0f);
      mod_entity_rotation();
    }
    
    simd::vec4 movement::predict_pos(const size_t &predictionTime) const {
      auto trans = ent->at<TransformComponent>(game::entity::transform);
      auto physics = ent->at<PhysicsComponent>(game::entity::physics);
      return trans->pos() + physics->getVelocity() * MCS_TO_SEC(predictionTime);
    }
    
    simd::vec4 movement::seek(const simd::vec4 &target) {
      auto input = ent->at<InputComponent>(game::entity::input);
      auto trans = ent->at<TransformComponent>(game::entity::transform);
      input->front() = target - trans->pos();
      return input->front();
    }
    
    simd::vec4 movement::flee(const simd::vec4 &target) {
      auto input = ent->at<InputComponent>(game::entity::input);
      auto trans = ent->at<TransformComponent>(game::entity::transform);
      input->front() = trans->pos() - target;
      return input->front();
    }
    
    simd::vec4 movement::follow_path(const size_t &predictionTime, const path::container* path, const size_t &currentPathSegmentIndex) {
      (void)predictionTime;
      // короч я допустил несколько ошибок в старом коде 
      // мне нужно наверное пересобрать код следования пути
      
    //   const simd::vec4 &futurePos = predictPos(predictionTime);
      const auto pathSeg = &path->array[currentPathSegmentIndex];
    //   const FunnelPath* prevPathSeg = &path->funnelData()[currentPathSegmentIndex-1];
      
    //   uint32_t currentIndex = 0, prevIndex = 0;
    //   const simd::vec4 finalCurrentDir = getDirIndex(pathSeg->edgeDir, currentIndex);
    //   const simd::vec4 finalPrevDir = getDirIndex(prevPathSeg->edgeDir, prevIndex);
      
      // для того чтобы отступ сделать нужно посчитать этот отступ для каждого
      // проблема в том как у нас фуннел алгоритм посчитан, он совершенно это не учитывает
      // лучше вот что сделать, тип поиска пути также должен содержать примерный отступ
      // этот отступ мы будем использовать для того чтобы правильно посчитать фуннел
      // а затем здесь все сведется просто к путешествию по фуннел точкам
      const simd::vec4 finalPathPoint = pathSeg->pos; 
      
      seek(finalPathPoint);
      return finalPathPoint;
    }
    
    void movement::stay_on_path(const size_t &predictionTime, const path::container* path) {
      (void)predictionTime;
      (void)path;
    }
    
    void movement::mod_entity_rotation() {
      auto physics = ent->at<PhysicsComponent>(game::entity::physics);
      auto trans = ent->at<TransformComponent>(game::entity::transform);
      const simd::vec4 velocity = physics->getVelocity();
      const float dot = simd::dot(velocity, velocity);
      if (dot > EPSILON) {
        trans->rot() = velocity / std::sqrt(dot);
      }
    }
    
    void movement::release_current_path() {
      if (current_path.path == nullptr) return;
      Global::get<systems::pathfinder>()->release_path(current_path.req);
      current_path.path = nullptr;
      current_path.req.end = nullptr;
      current_path.req.start = nullptr;
    }
    
    void movement::release_old_path() {
      if (old_path.path == nullptr) return;
      Global::get<systems::pathfinder>()->release_path(old_path.req);
      old_path.path = nullptr;
      old_path.req.end = nullptr;
      old_path.req.start = nullptr;
    }
    
    void movement::release_paths() {
      release_current_path();
      release_old_path();
    }
  }
}
