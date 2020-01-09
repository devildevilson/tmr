#ifndef DELAYED_TIMED_WORK_SYSTEM_H
#define DELAYED_TIMED_WORK_SYSTEM_H

#include "ThreadPool.h"

class DelayedTimedWorkSystem {
public:
  struct CreateInfo {
    dt::thread_pool* pool;
  };
  DelayedTimedWorkSystem(const CreateInfo &info);
  
  void update(const size_t &time);
  
  void add_work(const size_t &delay, const std::function<void()> &task);
private:
  struct Task {
    size_t time;
    std::function<void()> task;
  };
  
  dt::thread_pool* pool;
  std::mutex mutex;
  size_t currentTime;
  size_t nextIndex;
  std::vector<Task> tasks;
};

#endif
