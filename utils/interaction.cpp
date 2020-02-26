#include "interaction.h"

#include "ArrayInterface.h"
#include "Physics.h"
#include "Globals.h"
#include "shared_time_constant.h"
#include "TransformComponent.h"
#include "PhysicsComponent.h"
#include "BroadphaseInterface.h"
// #include "UserDataComponent.h"
#include "HelperFunctions.h"

#include "delayed_work_system.h"
#include "game_funcs.h"
#include "core_funcs.h"

const float thickness = 0.2f;
const float stab_angle = glm::radians(30.0f);

namespace devils_engine {
  namespace core {
    Container<Transform>* interaction::transforms = nullptr;
    Container<simd::mat4>* interaction::matrices = nullptr;
    
    void slashing_interaction::update(const size_t &time) {
      current_time += time * speed;

//     if (currentTime >= delay + attackTime) finished = true;

      const uint32_t triggerSize = Global::get<PhysicsEngine>()->getTriggerPairsIndicesSize();
      const ArrayInterface<uint32_t>* triggerPairs = Global::get<PhysicsEngine>()->getTriggerPairsIndices();
      const ArrayInterface<OverlappingData>* datas = Global::get<PhysicsEngine>()->getOverlappingPairsData();

      const size_t finalCurrentTime = current_time;
      const size_t finalLastTime = last_time;
      
      // найдем 4 точки, 2 из которых это текущее положение и предыдущее
      // + 2 точки предыдущий дир повернутый на предыдущий угол, и текущий дир повернутый на текущий угол
      // плоскость поворота normal
      
      const float angle = end_angle - start_angle;
//       const float fractional_angle = angle / float(time);
      
      //const float halfAngle = angle / 2.0f; 
      const float currentAngle = start_angle + angle * (float(finalCurrentTime) / float(time)); //finalCurrentTime * fractional_angle;
      const float lastAngle = start_angle + angle * (float(finalLastTime) / float(time)); // (finalLastTime * (angle / float(time))) - halfAngle;

      const simd::vec4 pos = trans->pos();
      const simd::vec4 dir = trans->rot();
      
      const simd::vec4 simd_pos = simd::vec4(pos);
      const simd::vec4 simd_dir = simd::vec4(dir);
      //const simd::mat4 attackRot = simd::orientation(simd_dir, simd::normalize(projectVectorOnPlane(Global::get<PhysicsEngine>()->getGravityNorm(), simd_pos, simd_dir)));
      const simd::mat4 attackRot = simd::orientation(simd_dir, simd::vec4(0.0f, 0.0f, 1.0f, 0.0f));
      const simd::vec4 attackPlane = attackRot * PhysicsEngine::getOrientation() * simd::vec4(plane);
      const simd::vec4 vector1 = simd::normalize(simd::rotate(simd::mat4(1.0f), currentAngle, attackPlane) * simd_dir);
      const simd::vec4 vector2 = simd::normalize(simd::rotate(simd::mat4(1.0f), lastAngle, attackPlane) * simd::vec4(last_dir));

      // TODO: должна быть проверка на то что сами точки находятся слева/справа
      // то есть если мы атакуем в иную сторону чем двигается камера
      const simd::vec4 A1 = simd::vec4(last_pos);
      const simd::vec4 A2 = A1 + vector2 * distance;
      const simd::vec4 B1 = simd_pos;
      const simd::vec4 B2 = B1 + vector1 * distance;

      auto container = phys->getIndexContainer();

      for (uint32_t i = 0; i < triggerSize; ++i) {
        const uint32_t index = triggerPairs->at(i+1); // TODO: +1?
        const OverlappingData &data = datas->at(index);

        if (data.firstIndex != container.objectDataIndex && data.secondIndex != container.objectDataIndex) continue;

        const uint32_t objIndex = data.firstIndex == container.objectDataIndex ? data.secondIndex : data.firstIndex;

        const PhysicsIndexContainer* another = Global::get<PhysicsEngine>()->getIndexContainer(objIndex);
//         const BroadphaseProxy* proxy = Global::get<PhysicsEngine>()->getObjectBroadphaseProxy(another);
        
        auto another_ent = reinterpret_cast<yacs::entity*>(another->userData);

        //const bool ticklessObj = (proxy->collisionGroup() & ticklessObjectsType) != 0;

        const size_t obj_index = find(another_ent);
        if (obj_index != SIZE_MAX) {
          //if (itr->second.count >= tickCount && !ticklessObj) continue;
          
          if (current_time - objs[obj_index].inter_time >= tick_time) {
            objs[obj_index].inter_time = current_time;
          } else continue;
          
          if (objs[obj_index].ticks >= tick_count) continue;
          
//           const auto timePoint = std::chrono::steady_clock::now();
//           const auto diff = timePoint - itr->second.point;
//           const size_t mcs = std::chrono::duration_cast<std::chrono::microseconds>(diff).count();
//           if (mcs < tickTime) continue;
        }

        bool founded = false;
        const Object &anotherObj = Global::get<PhysicsEngine>()->getObjectData(another);
        if (anotherObj.objType.getObjType() == BBOX_TYPE) {
          const simd::vec4 pos = transforms->at(anotherObj.transformIndex).pos;
          const simd::vec4 extent = Global::get<PhysicsEngine>()->getObjectShapePoints(another)[0];
          const simd::vec4 scale = transforms->at(anotherObj.transformIndex).scale;

          const simd::mat4 matrix = anotherObj.coordinateSystemIndex == UINT32_MAX ? simd::mat4(1.0f) * simd::scale(simd::mat4(1.0f), scale) : matrices->at(anotherObj.coordinateSystemIndex) * simd::scale(simd::mat4(1.0f), scale);

          // теперь учитывает, возможна проблема со знаками
          uint32_t pointsLeft = 0;
          uint32_t pointsRight = 0;
          uint32_t pointsAbove = 0;
          uint32_t pointsBeneath = 0;
          const uint32_t pointsSize = 8;
          const simd::vec4 trans = pos * simd::vec4(1.0f, 1.0f, 1.0f, 0.0f);
          for (uint32_t j = 0; j < pointsSize; ++j) {
            const simd::vec4 point = getVertex(pos, extent, matrix, j);

            const float sideLeftBorder = sideOf(A1, A2, point, attackPlane);
            const float sideRightBorder = sideOf(B1, B2, point, attackPlane);

            const float dotPoint = simd::dot(point, attackPlane);
            const float dotPos = simd::dot(B1, attackPlane);

            pointsLeft += uint32_t(sideLeftBorder < 0.0f);
            pointsRight += uint32_t(sideRightBorder >= 0.0f);
            pointsAbove += uint32_t(dotPoint - dotPos >= thickness);
            pointsBeneath += uint32_t(dotPoint - dotPos < -thickness);
          }

          if (pointsLeft < pointsSize && pointsRight < pointsSize && pointsAbove < pointsSize && pointsBeneath < pointsSize) {
            // мы нашли объект
            // вообще у нас уже есть юзер дата компонент, может как-то его использовать? там пока что хранится не очень много полезных штук
//             auto usrData = reinterpret_cast<UserDataComponent*>(another->userData);
            yacs::entity* first = ent;
            yacs::entity* second = another_ent;
            auto interaction_effect = e;

            //interaction(event_type(), obj, usrData->entity, this);
            Global::get<utils::delayed_work_system>()->add_work([first, second, interaction_effect] () {
              game::damage_ent(first, second, interaction_effect);
            });
            founded = true;
          }
        } else if (anotherObj.objType.getObjType() == POLYGON_TYPE) {
          const uint32_t pointsSize = Global::get<PhysicsEngine>()->getObjectShapePointsSize(another);
          const simd::vec4* points = Global::get<PhysicsEngine>()->getObjectShapePoints(another);

          const simd::vec4 pos = anotherObj.transformIndex == UINT32_MAX ? simd::vec4(0.0f, 0.0f, 0.0f, 1.0f) : transforms->at(anotherObj.transformIndex).pos;
          const simd::vec4 scale = anotherObj.transformIndex == UINT32_MAX ? simd::vec4(1.0f, 1.0f, 1.0f, 1.0f) : transforms->at(anotherObj.transformIndex).scale;
          const simd::mat4 matrix = anotherObj.coordinateSystemIndex == UINT32_MAX ? simd::mat4(1.0f) * simd::scale(simd::mat4(1.0f), scale) : matrices->at(anotherObj.coordinateSystemIndex) * simd::scale(simd::mat4(1.0f), scale);

          // теперь учитывает, возможна проблема со знаками
          uint32_t pointsLeft = 0;
          uint32_t pointsRight = 0;
          uint32_t pointsAbove = 0;
          uint32_t pointsBeneath = 0;
          const simd::vec4 trans = pos * simd::vec4(1.0f, 1.0f, 1.0f, 0.0f);
          for (uint32_t j = 0; j < pointsSize; ++j) {
            const simd::vec4 point = ::transform(points[j], trans, matrix);

            const float sideLeftBorder = sideOf(A1, A2, point, attackPlane);
            const float sideRightBorder = sideOf(B1, B2, point, attackPlane);

            const float dotPoint = simd::dot(point, attackPlane);
            const float dotPos = simd::dot(B1, attackPlane);

            pointsLeft += uint32_t(sideLeftBorder < 0.0f);
            pointsRight += uint32_t(sideRightBorder >= 0.0f);
            pointsAbove += uint32_t(dotPoint - dotPos >= thickness);
            pointsBeneath += uint32_t(dotPoint - dotPos < -thickness);
          }

          if (pointsLeft < pointsSize && pointsRight < pointsSize && pointsAbove < pointsSize && pointsBeneath < pointsSize) {
            //auto usrData = reinterpret_cast<UserDataComponent*>(another->userData);

            //interaction(event_type(), obj, usrData->entity, this);
            yacs::entity* first = ent;
            yacs::entity* second = another_ent;
            auto interaction_effect = e;

            //interaction(event_type(), obj, usrData->entity, this);
            Global::get<utils::delayed_work_system>()->add_work([first, second, interaction_effect] () {
              game::damage_ent(first, second, interaction_effect);
            });
            founded = true;
          }
        } else if (anotherObj.objType.getObjType() == SPHERE_TYPE) {
          throw std::runtime_error("interaction can not collide sphere");
          // нужно переделать физическую сферу так или иначе
          // переделал уже
        }

//         if (itr != objects.end() && founded) {
//           ++itr->second.count;
//           itr->second.point = std::chrono::steady_clock::now();
//         } else if (itr == objects.end() && founded) {
//           objects.insert(std::make_pair(objIndex, ObjData{0, std::chrono::steady_clock::now()}));
//         }

        if (obj_index != SIZE_MAX && founded) {
          ++objs[obj_index].ticks;
          objs[obj_index].inter_time = current_time;
        } else if (obj_index == SIZE_MAX && founded) {
          objs.push_back({
            another_ent,
            current_time,
            0
          });
        }
      }
      
      trans->pos().storeu(last_pos);
      trans->rot().storeu(last_dir);
      last_time = current_time;
      
      if (current_time >= time) {
        to_deletion->unset<TransformComponent>();
        remove(to_deletion);
      }
    }
    
    size_t slashing_interaction::find(yacs::entity* entity) const {
      for (size_t i = 0; i < objs.size(); ++i) {
        if (objs[i].ent == entity) return i;
      }
      return SIZE_MAX;
    }
    
    void stabbing_interaction::update(const size_t &time) {
      current_time += time * speed;
        //if (currentTime >= delayTime + attackTime) finished = true;

      auto container = phys->getIndexContainer();

      //const size_t updateTime = currentTime - delayTime;
      const float diff = max_dist - min_dist;
      const float inc = std::min((float(current_time) / float(time)) * diff, diff);
      const float currentRadius = glm::uintBitsToFloat(Global::get<PhysicsEngine>()->getObjectData(&container).faceCount);

      const simd::vec4 simd_pos = trans->pos();
      const simd::vec4 simd_dir = trans->rot();

      const simd::mat4 attackRot = simd::orientation(simd_dir, simd::vec4(0.0f, 0.0f, 1.0f, 0.0f));
      const simd::vec4 attackPlane = attackRot * PhysicsEngine::getOrientation() * simd::vec4(0.0f, 1.0f, 0.0f, 0.0f);

      // здесь нам нужно три точки: позиция + две точки на сфере
      const float halfAngle = stab_angle / 2.0f;
      const simd::vec4 vector1 = simd::normalize(simd::rotate(simd::mat4(1.0f),  halfAngle, attackPlane) * simd_dir);
      const simd::vec4 vector2 = simd::normalize(simd::rotate(simd::mat4(1.0f), -halfAngle, attackPlane) * simd_dir);

      const simd::vec4 A = simd_pos;
      const simd::vec4 B = A + vector1 * currentRadius;
      const simd::vec4 C = A + vector2 * currentRadius;

      const uint32_t triggerSize = Global::get<PhysicsEngine>()->getTriggerPairsIndicesSize();
      const ArrayInterface<uint32_t>* triggerPairs = Global::get<PhysicsEngine>()->getTriggerPairsIndices();
      const ArrayInterface<OverlappingData>* datas = Global::get<PhysicsEngine>()->getOverlappingPairsData();
      for (uint32_t i = 0; i < triggerSize; ++i) {
        const uint32_t index = triggerPairs->at(i+1);
        const OverlappingData &data = datas->at(index);

        if (data.firstIndex != container.objectDataIndex && data.secondIndex != container.objectDataIndex) continue;

        const uint32_t objIndex = data.firstIndex == container.objectDataIndex ? data.secondIndex : data.firstIndex;

        const PhysicsIndexContainer* another = Global::get<PhysicsEngine>()->getIndexContainer(objIndex);
//         const BroadphaseProxy* proxy = Global::get<PhysicsEngine>()->getObjectBroadphaseProxy(another);

        //const bool ticklessObj = (proxy->collisionGroup() & ticklessObjectsType) != 0;
        
        auto another_ent = reinterpret_cast<yacs::entity*>(another->userData);

        const auto obj_index = find(another_ent);
        if (obj_index != SIZE_MAX) {
          //if (itr->second.count >= tickCount && !ticklessObj) continue;

          if (current_time - objs[obj_index].inter_time >= tick_time) {
            objs[obj_index].inter_time = current_time;
          } else continue;
          
          if (objs[obj_index].ticks >= tick_count) continue;
          
//           const auto timePoint = std::chrono::steady_clock::now();
//           const auto diff = timePoint - itr->second.point;
//           const size_t mcs = std::chrono::duration_cast<std::chrono::microseconds>(diff).count();
//           if (mcs < tickTime) continue;
        }

        bool founded = false;
        const Object &anotherObj = Global::get<PhysicsEngine>()->getObjectData(another);
        if (anotherObj.objType.getObjType() == BBOX_TYPE) {
          const simd::vec4 pos = transforms->at(anotherObj.transformIndex).pos;
          const simd::vec4 extent = Global::get<PhysicsEngine>()->getObjectShapePoints(another)[0];
          const simd::vec4 scale = transforms->at(anotherObj.transformIndex).scale;

          const simd::mat4 matrix = anotherObj.coordinateSystemIndex == UINT32_MAX ? simd::mat4(1.0f) * simd::scale(simd::mat4(1.0f), scale) : matrices->at(anotherObj.coordinateSystemIndex) * simd::scale(simd::mat4(1.0f), scale);

          // теперь учитывает, возможна проблема со знаками
          uint32_t pointsLeft = 0;
          uint32_t pointsRight = 0;
          uint32_t pointsAbove = 0;
          uint32_t pointsBeneath = 0;
          const uint32_t pointsSize = 8;
          const simd::vec4 trans = pos * simd::vec4(1.0f, 1.0f, 1.0f, 0.0f);
          for (uint32_t j = 0; j < pointsSize; ++j) {
            const simd::vec4 point = getVertex(pos, extent, matrix, j);

            const float sideLeftBorder = sideOf(A, B, point, attackPlane);
            const float sideRightBorder = sideOf(A, C, point, attackPlane);

            const float dotPoint = simd::dot(point, attackPlane);
            const float dotPos = simd::dot(A, attackPlane);

            pointsLeft += uint32_t(sideLeftBorder < 0.0f);
            pointsRight += uint32_t(sideRightBorder >= 0.0f);
            pointsAbove += uint32_t(dotPoint - dotPos >= thickness);
            pointsBeneath += uint32_t(dotPoint - dotPos < -thickness);
          }

          if (pointsLeft < pointsSize && pointsRight < pointsSize && pointsAbove < pointsSize && pointsBeneath < pointsSize) {
            // мы нашли объект
            // вообще у нас уже есть юзер дата компонент, может как-то его использовать? там пока что хранится не очень много полезных штук
            //auto usrData = reinterpret_cast<UserDataComponent*>(another->userData);
            
            yacs::entity* first = ent;
            yacs::entity* second = another_ent;
            auto interaction_effect = e;

            //interaction(event_type(), obj, usrData->entity, this);
            Global::get<utils::delayed_work_system>()->add_work([first, second, interaction_effect] () {
              game::damage_ent(first, second, interaction_effect);
            });
            founded = true;
          }
        } else if (anotherObj.objType.getObjType() == POLYGON_TYPE) {
          const uint32_t pointsSize = Global::get<PhysicsEngine>()->getObjectShapePointsSize(another);
          const simd::vec4* points = Global::get<PhysicsEngine>()->getObjectShapePoints(another);

          const simd::vec4 pos = anotherObj.transformIndex == UINT32_MAX ? simd::vec4(0.0f, 0.0f, 0.0f, 1.0f) : transforms->at(anotherObj.transformIndex).pos;
          const simd::vec4 scale = anotherObj.transformIndex == UINT32_MAX ? simd::vec4(1.0f, 1.0f, 1.0f, 1.0f) : transforms->at(anotherObj.transformIndex).scale;
          const simd::mat4 matrix = anotherObj.coordinateSystemIndex == UINT32_MAX ? simd::mat4(1.0f) * simd::scale(simd::mat4(1.0f), scale) : matrices->at(anotherObj.coordinateSystemIndex) * simd::scale(simd::mat4(1.0f), scale);

          // теперь учитывает, возможна проблема со знаками
          uint32_t pointsLeft = 0;
          uint32_t pointsRight = 0;
          uint32_t pointsAbove = 0;
          uint32_t pointsBeneath = 0;
          const simd::vec4 trans = pos * simd::vec4(1.0f, 1.0f, 1.0f, 0.0f);
          for (uint32_t j = 0; j < pointsSize; ++j) {
            const simd::vec4 point = ::transform(points[j], trans, matrix);

            const float sideLeftBorder = sideOf(A, B, point, attackPlane);
            const float sideRightBorder = sideOf(A, C, point, attackPlane);

            const float dotPoint = simd::dot(point, attackPlane);
            const float dotPos = simd::dot(A, attackPlane);

            pointsLeft += uint32_t(sideLeftBorder < 0.0f);
            pointsRight += uint32_t(sideRightBorder >= 0.0f);
            pointsAbove += uint32_t(dotPoint - dotPos >= thickness);
            pointsBeneath += uint32_t(dotPoint - dotPos < -thickness);
          }

          if (pointsLeft < pointsSize && pointsRight < pointsSize && pointsAbove < pointsSize && pointsBeneath < pointsSize) {
            yacs::entity* first = ent;
            yacs::entity* second = another_ent;
            auto interaction_effect = e;

            //interaction(event_type(), obj, usrData->entity, this);
            Global::get<utils::delayed_work_system>()->add_work([first, second, interaction_effect] () {
              game::damage_ent(first, second, interaction_effect);
            });
            founded = true;
          }
        } else if (anotherObj.objType.getObjType() == SPHERE_TYPE) {
          throw std::runtime_error("interaction can not collide sphere");
          // нужно переделать физическую сферу так или иначе
          // переделал уже
        }

        if (obj_index != SIZE_MAX && founded) {
          ++objs[obj_index].ticks;
          objs[obj_index].inter_time = current_time;
        } else if (obj_index == SIZE_MAX && founded) {
          objs.push_back({
            another_ent,
            current_time,
            0
          });
        }
      }

      const float nextRadius = min_dist + inc;
      Global::get<PhysicsEngine>()->getObjectData(&container).faceCount = glm::floatBitsToUint(nextRadius);
      last_time = current_time;
      
      if (current_time >= time) {
        to_deletion->unset<TransformComponent>();
        remove(to_deletion);
      }
    }
    
    size_t stabbing_interaction::find(yacs::entity* entity) const {
      for (size_t i = 0; i < objs.size(); ++i) {
        if (objs[i].ent == entity) return i;
      }
      return SIZE_MAX;
    }
  }
}
