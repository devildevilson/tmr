#include "delayed_work_system.h"

#include "ThreadPool.h"
#include "Utility.h"

namespace devils_engine {
  namespace utils {
    delayed_work_system::delayed_work_system(const create_info &info) : pool(info.pool) {}
    void delayed_work_system::add_work(const std::function<void()> &func) {
      std::unique_lock<std::mutex> lock(mutex);
      funcs.push(func);
    }
    
    void delayed_work_system::do_works() {
      std::function<void()> func;
      while (true) {
        {
          std::unique_lock<std::mutex> lock(mutex);
          if (funcs.empty()) return;
          func = std::move(funcs.front());
          funcs.pop();
        }
        
        func();
      }
    }
    
    void delayed_work_system::do_works(const size_t &count) {
      std::function<void()> func;
      for (size_t i = 0; i < count; ++i) {
        {
          std::unique_lock<std::mutex> lock(mutex);
          if (funcs.empty()) return;
          func = std::move(funcs.front());
          funcs.pop();
        }
        
        func();
      }
    }
    
    void delayed_work_system::detach_works() {
      pool->submitbase([this] () {
        this->do_works();
      });
    }
    
    void delayed_work_system::detach_works(const size_t &count) {
      if (count == 0) return;
      
      pool->submitbase([this, count] () {
//         TimeLogDestructor appTime("works");
        this->do_works(count);
      });
    }
    
    void delayed_work_system::distribute_works() {
      {
        std::unique_lock<std::mutex> lock(mutex);
        while (!funcs.empty()) {
          pool->submitbase(funcs.front());
          funcs.pop();
        }
      }
    }
    
    void delayed_work_system::wait() {
      pool->wait();
    }
    
//     void delayed_work_system::do_part_works(const size_t &time) {
//       // тут возникает проблема с тем что количество работ уменьшается, и по этому в следующей
//       // итерации работ будет выполняться меньше, что приведет к неравномерному распределению
//       // с другой стороны, мы каждый (!) кадр будем немного добавлять дополнительной работы
//       // как отделить работу каждые update_time и работу которая приходит каждый кадр?
//       // можно сделать иначе: выполнять работу не в том кадре в котором происходит 
//       // обновление стейтов и прочего, чем это нам грозит? во первых это хорошо работает
//       // только при 60 кадрах в секунду и частоте обновления 30 кадров в секунду
//       // во вторых взаимодействие будет опаздывать на 1 кадр, не думаю что это критично
//       // важно чтобы ии выполнялось всегда после всех работ, для того чтобы правильно среагировать на 
//       // изменение состояния объекта на состояние удаления
//       
//       size_t current_time = 0;
//       size_t update_time = 33333;
//       current_time += time;
//       if (current_time >= update_time) {
//         detach_works();
//         current_time -= update_time;
//       } else {
//         const float factor = float(current_time) / float(update_time);
//         const size_t work_size = funcs.size() * factor; // funcs.size() - изменяема
//         detach_works(work_size);
//       }
//     }
    
    size_t delayed_work_system::works_count() const {
      std::unique_lock<std::mutex> lock(mutex);
      return funcs.size();
    }
  }
}
