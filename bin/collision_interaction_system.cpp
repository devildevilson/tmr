#include "collision_interaction_system.h"

#include "EntityComponentSystem.h"
#include "Globals.h"
#include "ThreadPool.h"
#include "Physics.h"
// #include "UserDataComponent.h"
#include "inventory_component.h"
#include "global_components_indicies.h"
#include "game_funcs.h"
#include "delayed_work_system.h"
#include "collision_property.h"
#include "type_info_component.h"
#include "interaction.h"
#include "core_funcs.h"
#include "entity_creator_resources.h"

namespace devils_engine {
//   void collision_processor(const size_t &start, const size_t &count, const size_t &time) {
//     (void)time;
//     const auto pairs = Global::get<PhysicsEngine>()->getOverlappingPairsData();
//     //const auto size = Global::get<PhysicsEngine>()->getOverlappingDataSize();
//     
//     for (size_t i = start; i < start+count; ++i) {
//       yacs::entity* first = nullptr;
//       yacs::entity* second = nullptr;
//       {
//         auto cont = Global::get<PhysicsEngine>()->getIndexContainer(pairs->at(i).firstIndex);
//         first = reinterpret_cast<yacs::entity*>(cont->userData);
//       }
//       
//       {
//         auto cont = Global::get<PhysicsEngine>()->getIndexContainer(pairs->at(i).secondIndex);
//         second = reinterpret_cast<yacs::entity*>(cont->userData);
//       }
//       
//       ASSERT(first != nullptr);
//       ASSERT(second != nullptr);
//       
//       auto first_info = first->at<components::type_info>(game::entity::type_info);
//       auto second_info = second->at<components::type_info>(game::entity::type_info);
//       
//       // несколько взаимодействий (есть еще немало взаимодействий по лучам, но скорее всего это не здесь)
//       // 1. подбор предмета
//       
//       {
//         auto inv = first->at<components::inventory>(game::player::inventory);
//         auto pickup = second->at<properties::pickup>(game::item::item_property);
//         if (inv.valid() && pickup.valid()) {
//           ASSERT(first_info->bit_container.is_player());
//           Global::get<utils::delayed_work_system>()->add_work([first, second] () {
//             game::item_pickup(first, second);
//           });
//         }
//       }
//       
//       {
//         auto inv = second->at<components::inventory>(game::player::inventory);
//         auto pickup = first->at<properties::pickup>(game::item::item_property);
//         if (inv.valid() && pickup.valid()) {
//           ASSERT(second_info->bit_container.is_player());
//           Global::get<utils::delayed_work_system>()->add_work([first, second] () {
//             game::item_pickup(second, first); // потом конечно функция должна вызываться по глобальному указателю
//           });
//         }
//       }
//       
//       // 2. коллбек при коллизии
//       
//       {
//         auto coll = first->at<properties::collision>(game::monster::collision_property);
//         if (coll.valid() && first_info->bit_container.has_collision_trigger()) {
//           Global::get<utils::delayed_work_system>()->add_work([coll, first, second] () {
//             (*coll->func)(first, second);
//           });
//         }
//       }
//       
//       {
//         auto coll = second->at<properties::collision>(game::monster::collision_property);
//         if (coll.valid() && second_info->bit_container.has_collision_trigger()) {
//           Global::get<utils::delayed_work_system>()->add_work([coll, second, first] () {
//             (*coll->func)(second, first);
//           });
//         }
//       }
//       
//       // 3. триггер сложной атаки
// //        complex_attack->update(time); 
//       // в этой функции мы будем добавлять работу в delayed_work_system
//       // причем это будет происходить наверное вне collision_interaction::update
//       
// //       {
// //         auto complex_attack = first->get<core::slashing_interaction>();
// //         if (complex_attack.valid()) {
// //           Global::get<utils::delayed_work_system>()->add_work([complex_attack, second] () {
// //             game::damage_ent(complex_attack->ent, second, complex_attack->e);
// //           });
// //         }
// //       }
// //       
// //       {
// //         auto complex_attack = second->get<core::slashing_interaction>();
// //         if (complex_attack.valid()) {
// //           Global::get<utils::delayed_work_system>()->add_work([complex_attack, first] () {
// //             game::damage_ent(complex_attack->ent, first, complex_attack->e);
// //           });
// //         }
// //       }
// //       
// //       {
// //         auto complex_attack = first->get<core::stabbing_interaction>();
// //         if (complex_attack.valid()) {
// //           Global::get<utils::delayed_work_system>()->add_work([complex_attack, second] () {
// //             game::damage_ent(complex_attack->ent, second, complex_attack->e);
// //           });
// //         }
// //       }
// //       
// //       {
// //         auto complex_attack = second->get<core::stabbing_interaction>();
// //         if (complex_attack.valid()) {
// //           Global::get<utils::delayed_work_system>()->add_work([complex_attack, first] () {
// //             game::damage_ent(complex_attack->ent, first, complex_attack->e);
// //           });
// //         }
// //       } 
//     }
//   }
  
  namespace systems {
    collision_interaction::collision_interaction(const create_info &info) : pool(info.pool) {}
    void collision_interaction::update(const size_t &time) {
      static const auto func = [] (const size_t &start, const size_t &count, const size_t &time) {
        (void)time;
        const auto pairs = Global::get<PhysicsEngine>()->getOverlappingPairsData();
        
        for (size_t i = start; i < start+count; ++i) {
          if (pairs->at(i).hasCollision == 0) continue; // так мы отмечаем пустые слоты 
          
//           if (pairs->at(i).firstIndex == UINT32_MAX || pairs->at(i).secondIndex == UINT32_MAX) {
//             std::cout << "firstIndex  " << pairs->at(i).firstIndex << "\n";
//             std::cout << "secondIndex " << pairs->at(i).secondIndex << "\n";
//             ASSERT(false);
//           }
          
          yacs::entity* first = nullptr;
          yacs::entity* second = nullptr;
          {
            auto cont = Global::get<PhysicsEngine>()->getIndexContainer(pairs->at(i).firstIndex);
            if (cont == nullptr) continue;
            first = reinterpret_cast<yacs::entity*>(cont->userData);
          }
          
          {
            auto cont = Global::get<PhysicsEngine>()->getIndexContainer(pairs->at(i).secondIndex);
            if (cont == nullptr) continue;
            second = reinterpret_cast<yacs::entity*>(cont->userData);
          }
          
          ASSERT(first != nullptr);
          ASSERT(second != nullptr);
          
          auto first_info = first->at<components::type_info>(game::entity::type_info);
          auto second_info = second->at<components::type_info>(game::entity::type_info);
          
          //std::cout << first_info->id.name() << " collide " << second_info->id.name() << "\n";
          {
            auto creator = Global::get<game::entity_creators_container>()->get(first_info->id);
            if (creator != nullptr) creator->collision_func(first, second, utils::id());
          }
          
          {
            auto creator = Global::get<game::entity_creators_container>()->get(second_info->id);
            if (creator != nullptr) creator->collision_func(second, first, utils::id());
          }
        }
      };
      
      //const auto pairs = Global::get<PhysicsEngine>()->getOverlappingPairsData();
      const auto size = Global::get<PhysicsEngine>()->getOverlappingDataSize();
//       std::cout << "collision size " << size << "\n";
      
      const size_t count = size;
      const size_t one_jobs = std::ceil(float(count)/float(pool->size()+1));
      size_t start = 0;
      for (size_t i = 0; i < pool->size()+1; ++i) {
        const size_t job_count = std::min(one_jobs, count-start);
        if (job_count == 0) break;
        
        pool->submitbase([start, job_count, time] () {
          //collision_processor(start, job_count, time);
          func(start, job_count, time);
        });
        
        start += job_count;
      }
      
      pool->compute();
      pool->wait();
    }
  }
}
