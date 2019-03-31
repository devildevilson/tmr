#include "ThreadPool.h"

#include "Logging.h"

namespace dt {
  thread_pool::thread_pool(const size_t &size) {
    busyWorkersCount = size;
    tasksCount = 0;
    stop = false;

    for (size_t i = 0; i < size; ++i) {
      workers.emplace_back([this] () {
        // тут предлагается использовать бесконечный цикл
        // с другой стороны как возвращаться на исходную позицию?
        while (true) {
          std::function<void()> task;

          {
            std::unique_lock<std::mutex> lock(this->mutex);
            // могу ли я прочитать эту переменную
            --this->busyWorkersCount;
            finish.notify_one();

            // condition_variable освобождает мьютекс пока ждет
            this->condition.wait(lock, [this] () {
              return this->stop || !this->tasks.empty();
            });

            if (this->stop && this->tasks.empty()) return; // это гарантирует что мы выйдем когда закончим выполнение всех задач

            task = std::move(this->tasks.front());
            this->tasks.pop();

            ++this->busyWorkersCount;
            --this->tasksCount;
          }

          task();
        }
      });
    }
  }

  thread_pool::~thread_pool() {
    {
      std::unique_lock<std::mutex> lock(this->mutex);
      stop = true;
    }

    condition.notify_all();

    for(std::thread &worker : workers) {
      if (worker.joinable()) worker.join();
    }
  }

  void thread_pool::compute() {
    // RegionLog rl("thread_pool::compute()", true);

    while (true) {
      std::function<void()> task;

      {
        std::unique_lock<std::mutex> lock(this->mutex);
        if (this->tasks.empty()) return; // это гарантирует что мы выйдем когда закончим выполнение всех задач

        task = std::move(this->tasks.front());
        this->tasks.pop();
      }

      task();
    }
  }

  void thread_pool::compute(const size_t &count) {
    for (size_t i = 0; i < count; ++i) {
      std::function<void()> task;

      {
        std::unique_lock<std::mutex> lock(this->mutex);
        if (this->tasks.empty()) return; // это гарантирует что мы выйдем когда закончим выполнение всех задач

        task = std::move(this->tasks.front());
        this->tasks.pop();
      }

      task();
    }
  }
  void thread_pool::wait() {
    // RegionLog rl("thread_pool::wait()", true);
    // не уверен на сколько быстро это все работает
    // но по идее оверхед от атомарных переменных был бы больше
    if (is_dependent(std::this_thread::get_id())) return;

    std::unique_lock<std::mutex> lock(this->mutex);
    //std::cout << "wait " << this->tasks.empty() << " " << busyWorkersCount << '\n';
    //if (this->tasks.empty() && busyWorkersCount == 0) return;

    this->finish.wait(lock, [this] () {
      return this->tasks.empty() && (busyWorkersCount == 0);
    });
  }

  bool thread_pool::is_dependent(const std::thread::id &id) const {
    for (size_t i = 0; i < workers.size(); ++i) {
      if (workers[i].get_id() == id) return true;
    }

    return false;
  }

  size_t thread_pool::size() const {
    return workers.size();
  }

  size_t thread_pool::tasks_count() const {
    // по идее тут должен быть мьютекс, но я не могу его сюда поставить потому что конст
    // будет ли переменная работать правильно? по стандарту (если я правильно понял) не должна
    //std::unique_lock<std::mutex> lock(mutex);
    return tasksCount;
  }

  size_t thread_pool::working_count() const {
    //std::unique_lock<std::mutex> lock(mutex);
    return busyWorkersCount;
  }
}
