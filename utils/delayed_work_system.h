#ifndef DELAYED_WORK_SYSTEM_H
#define DELAYED_WORK_SYSTEM_H

#include <mutex>
#include <functional>
#include <vector>
#include <queue>

namespace dt {
  class thread_pool;
}

namespace devils_engine {
  namespace utils {
    class delayed_work_system {
    public:
      struct create_info {
        dt::thread_pool* pool;
      };
      delayed_work_system(const create_info &info);
      
      void add_work(const std::function<void()> &func);
      void do_works();
      void do_works(const size_t &count);
      void detach_works();
      void detach_works(const size_t &count);
      void distribute_works();
      void wait();
//       void do_part_works(const size_t &time);
      size_t works_count() const;
    private:
      dt::thread_pool* pool;
      mutable std::mutex mutex;
      std::queue<std::function<void()>> funcs;
    };
  }
}

#endif
