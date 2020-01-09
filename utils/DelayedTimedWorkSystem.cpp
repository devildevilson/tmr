#include "DelayedTimedWorkSystem.h"

DelayedTimedWorkSystem::DelayedTimedWorkSystem(const CreateInfo &info) : pool(info.pool), currentTime(0), nextIndex(SIZE_MAX) {}
void DelayedTimedWorkSystem::update(const size_t &time) {
  currentTime += time;
  
  for (size_t i = 0; i < tasks.size(); ++i) {
    if (currentTime >= tasks[i].time && tasks[i].task) { //!= nullptr
      pool->submitnr(tasks[i].task);
      tasks[i].task = nullptr;
      tasks[i].time = nextIndex;
      nextIndex = i;
    }
  }
  
  pool->compute();
  pool->wait();
}

void DelayedTimedWorkSystem::add_work(const size_t &delay, const std::function<void()> &task) {
  std::unique_lock<std::mutex> lock(mutex);
  
  if (nextIndex != SIZE_MAX) {
    nextIndex = tasks[nextIndex].time;
    tasks[nextIndex].time = delay + currentTime;
    tasks[nextIndex].task = task;
  } else {
    tasks.push_back({delay + currentTime, task});
  }
}
