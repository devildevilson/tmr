#ifndef DELAYED_WORK_SYSTEM_H
#define DELAYED_WORK_SYSTEM_H

#include "ThreadPool.h"

class DelayedWorkSystem {
public:
  struct CreateInfo {
    dt::thread_pool* pool;
  };
  DelayedWorkSystem(const CreateInfo &info);
  ~DelayedWorkSystem();
  
  // возможно ли какие нибудь работы делать параллельно? первое что приходит в голову заделэить звук, загрузку с диска, взаимодействие объектов, удаление/создание энтити
  // явно придется расширять функционал в будущем, было бы неплохо чтобы так можно было бы загружать и выгружать локации в играх с открытым миром
  void add_work(const std::function<void()> &func);
  void do_work();
  void detach_work();
  void wait();
private:
  dt::thread_pool* pool;
  std::mutex mutex;
  std::vector<std::function<void()>> works;
//   std::vector<std::future<void>> futures; // скорее всего будут нужны, но потом
// наверное future будет только один
// скорее всего тут еще потребуется механизм сокращения количества выполняемых задач в одном кадре
  
// если мы собираемся делэить такие вещи как загрузку с диска и загрузку/выгрузку локации то 100% нам потребуется способ ждать только определенные работы
// сейчас пока мы сделаем отдельно только работы по созданию/удалению энтити
// если мы создадим несколько DelayedWorkSystem, то мы можем моделировать несколкьо параллельных независимых работ
// думаю что этого будет достаточно для большинства задач
  
// для того чтобы добиться здесь хорошей параллельности, нужно переделать тредпул
// должны быть условно отдельные WorkersPool и JobsPool, пул воркеров скорее всего один,
// а вот пулов задач может быть много, каждый пул задач может взять произвольное количество воркеров
// выполнение задач в пуле - синхронизация для всех воркеров взятых пулом несколько пулов могут выполняться параллельно и последовательно
// синхонизация в таком случае должна быть отдельно? см cpp файл
};

#endif