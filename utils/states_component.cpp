#include "states_component.h"

#include "Globals.h"
#include "EntityComponentSystem.h"
#include "ThreadPool.h"

#include "game_funcs.h"
#include "core_funcs.h"

#define MAX_STATE_CHANGES 10

namespace devils_engine {
  namespace components {
    void states::set(const core::state_t* state) {
      //auto old = current;
      do {
        ++counter;
        if (counter > MAX_STATE_CHANGES) {
          throw std::runtime_error("Long state sequence");
        }
        
        // такой объект подлежит удалению
        if (state == nullptr) {
          current = nullptr;
          break;
        }
        
        // не нужно чистить эту переменную так как она пойдет в action, с другой стороны чаще всего она будет 0
        current_time = 0;
        if (current != state) accumulated_time = 0;
        current = state;
        
        // далее тут используются m1 и m2 переменные
        
        if (current->action) {
          current->action(ent, accumulated_time);
          if (current == nullptr) break;
        }
        
        state = current->next;
        
      } while (current->time == 0);
      
      //if (old != current) current_time = 0;
      if (current == nullptr) core::remove(ent);
    }
    
    void states::update(const size_t &time) {
      // удаляем, остальные энтити к этому времени должны перестать ссылаться на нас
      // можно удалить через какое то время, но это не гарантирует того что мы правильно инвалидируем указатели
      // по идее мы должны удалить на предыдущем шаге
      // ии (валидируем указатели) -> удаляем энтити -> states::update
      if (current == nullptr) return;
      if (current_time == SIZE_MAX) {
        current_time = 0;
        accumulated_time = 0;
        set(current);
        return;
      }
      
      current_time += time;
      accumulated_time += time;
      counter = 0;
      if (current_time >= current->time) set(current->next);
    }
  }
  
//   namespace systems {
//     states::states(const create_info &info) : pool(info.pool) {}
//     void states::update(const size_t &time) {
//       const size_t count = Global::get<yacs::world>()->count_components<components::states>();
//       const size_t one_jobs = std::ceil(float(count)/float(pool->size()+1));
//       size_t start = 0;
//       for (size_t i = 0; i < pool->size()+1; ++i) {
//         const size_t job_count = std::min(one_jobs, count-start);
//         if (job_count == 0) break;
//         
//         pool->submitbase([start, job_count, time] () {
//           for (size_t i = start; i < start+job_count; ++i) {
//             auto states = Global::get<yacs::world>()->get_component<components::states>(i);
//             states->update(time);
//           }
//         });
//         
//         start += job_count;
//       }
//       
//       pool->compute();
//       pool->wait();
//     }
//   }
}
