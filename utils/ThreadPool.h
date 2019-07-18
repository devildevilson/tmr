#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <vector>
#include <atomic>

#include <iostream>

// барьер у меня все равно кислый
// он не учитывает те задачи, которые добавляются в самих задачах
// в этом случае либо запоминать выполнение последней задачи с добавлением
// либо самому делить на большее количество задач

namespace dt {
  // нужно еще придумать что делать с чтением переменных? mutable mutex ?
  class thread_pool {
  public:
    thread_pool(const size_t &size);
    ~thread_pool();

    template<class F, class... Args>
    auto submit(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
      using return_type = typename std::result_of<F(Args...)>::type;

      auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

      std::future<return_type> res = task->get_future();
      {
        std::unique_lock<std::mutex> lock(mutex);
        if (stop) throw std::runtime_error("Could not submit new task");
        tasks.emplace([task] () { (*task)(); });

        condition.notify_one();
      }

      return res;
    }

    template<class F, class... Args>
    void submitnr(F&& f, Args&&... args) {
      auto task = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
      {
        std::unique_lock<std::mutex> lock(mutex);
        if (stop) throw std::runtime_error("Could not submit new task");
        tasks.emplace(task);

        condition.notify_one();
      }
    }

    void barrier();
    void compute();
    void compute(const size_t &count);
    void wait(); // просто ждет всех

    bool is_dependent(const std::thread::id &id) const;
    uint32_t thread_index(const std::thread::id &id) const;

    size_t size() const;
    size_t tasks_count() const;
    size_t working_count() const;
  private:
    struct BarrierData {
      size_t taskCount;
      // думаю что ничего тут дополнитульно не нужно
    };
    
    bool stop;

    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::queue<BarrierData> barriers;

    std::mutex mutex;
    std::condition_variable condition;
    std::condition_variable finish;
    size_t tasksCount;
    size_t busyWorkersCount;
  };
}

#endif
