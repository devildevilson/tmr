#include "DelayedWorkSystem.h"

DelayedWorkSystem::DelayedWorkSystem(const CreateInfo &info) : pool(info.pool) {}
DelayedWorkSystem::~DelayedWorkSystem() {}

// возможно ли какие нибудь работы делать параллельно? первое что приходит в голову заделэить звук, загрузку с диска, взаимодействие объектов, удаление/создание энтити
// явно придется расширять функционал в будущем, было бы неплохо чтобы так можно было бы загружать и выгружать локации в играх с открытым миром
void DelayedWorkSystem::add_work(const std::function<void()> &func) {
  std::unique_lock<std::mutex> lock(mutex);
  works.push_back(func);
}

void DelayedWorkSystem::do_work() {
  std::function<void()> func;
  while (true) {
    {
      std::unique_lock<std::mutex> lock(mutex);
      if (works.empty()) return;
      func = std::move(works.back());
      works.pop_back();
    }
    
    func();
  }
}

void DelayedWorkSystem::detach_work() {
  pool->submitnr([this] () {
    this->do_work();
  });
}

void DelayedWorkSystem::wait() {
  pool->wait();
}

// пул потоков будет опрашивать JobsPool на наличие работ, если работа есть, выполняем
// если работ нет, но при этом пул заполненный - это по идее синхронизация и нам нужно подождать пока работы появятся в пуле
// если работ нет, и при этом пул незаполнен, удалем пул из очереди
// синхронизация между пулами? по идее должно контроллироваться тем как мы добавляем пулы, то есть последовательно или параллельно
// у нас получится что то вроде бехавиор три только для воркеров + должна быть возможность из текущей функции в текущий пул 
// (или в текущий массив задач до сихронизации) призвольно добавлять задачи, наверное между пуллами и будет синхронизация
// значит пулов нужно создавать много и произвольно, еще синхронизацию можно сделать по количеству задач 
// (то есть выполняем все задачи пока их не станет 0, и ждем до этого момента, не получится правильно синхронизовать случай когда задачи добавляются в процессе выполнения собственно задачи)
// короч для всего этого нужен что то вроде thread job graph, граф выполнения задач
// или даже скорее всего таск три, тип есть дерево выполнения, листья в котором это пулы задач
// class WorkersPool {
// public:
//   
// };
// 
// class JobsPool {
// public:
//   struct CreateInfo {
//     // тут по идее никак не посчитать сколько воркеров выполняют задачи из конкретного пула
// //     size_t maxWorkersCount;
//   };
//   JobsPool();
//   ~JobsPool();
//   
//   // добавление/уданелие задач
//   
//   bool hasNext() const;
//   std::function<void()> getNext();
// //   size_t maxWorkers() const;
// //   size_t workersCount() const;
//   size_t taskCount() const;
// private:
//   bool manualyCreated;
// //   const size_t maxWorkersCount;
//   
// //   size_t currentWorkersCount;
//   std::queue<std::function<void()>> tasks;
//   
//   mutable std::mutex mutex;
// };

// если правильно тут все продумать, то мы в каждую систему добавим новый метод, принимающий часть таск три
// и заполняющий его нодами, это будет происходить каждый кадр, но при заполнении треды уже могут начать выполнение
// в принципе если понять че делать с независимостью от времени (delta time см физику)
// то можно составить таск три лишь раз, а потом просто заполнять необходимые JobsPool
// еще было бы неплохо сделать обзервер, который почекает когда именно таски начинают/заканчиват выполнять задачи

// у такого такс флоу минус в том что задачи ищутся какое то время, на мой взгляд достаточно долго, ко всему прочему мне нужно 
// определить выполнили ли мы задачи в определенном JobsPool или они еще не пришли, в этом случае нам нужен какой-то метод, end() у JobsPool
// чтобы определить закончили ли мы добавлять задачи в пул из вне, может ли быть ситуация когда мы не вызвали end()?
// может быть из-за невнимательности, а, когда мы добавляем не из вне задачи в пул в принципе нам ничего не стоит вызвать end() при добавлении
// но при этом может быть ситуация когда мы добавляем задачи медленее чем они успевают обработаться, тогда вполне можно пропустить несколько важных задач
// эту проблему можно решить дизайном самого дерева, ПОКА в пул не добавлены все задачи, то есть пока не выполнены все задачи из предыдущего пула
// тут у нас появляются зависимости между нодами, как с ними быть? легко если пулы стоят в последовательных нодах, в ином случае, нужно 
// хранить указатели на другие ноды, что может привести тогда к другой проблеме - кольцевые зависимости, что уже гораздо гораздо серьезнее.
// то есть не понятно все ли задачи мы уже добавили в пул или еще не все? если мы добавляем задачи в потоке, 
// как гарантировать что мы не перейдем дальше по дереву? ну вообще то мы должны закончить выполнение задачи для этого, 
// а значит если проверка условия "задач нет и ни один из потоков не выполняет этот нод" будет верной то и проблем быть не должно
// namespace task_flow {
//   namespace tree {
//     class Node {
//     public:
//       Node();
//       virtual ~Node() {}
//       
//       // ищем следующую задачу и выполняем ее, либо ждем если задачи нет (в случае если у параллельных нодов и у текущего последовательного закончились задачи)
//       virtual bool update() = 0;
//       
//       // по родительским нодам мы должны проверить выполняются ли задачи из текущего нода
//       virtual Node* parent() = 0;
//       virtual Node* next() = 0;
//       
//     private:
//       // ?
//     };
//     
//     class ParallelNode : public Node {
//     public:
//       // потоки последовательно обходят все ноды, чтобы получить задачу
//       bool update() override;
//     private:
//       std::vector<Node*> nodes;
//     };
//     
//     class SequentNode : public Node {
//     public:
//       // получаем задачу из текущего нода, как только все задачи в ноде кончились
//       // берем следующий, как определить момент когда надо брать следующий? задач нет и ни один из потоков не выполняет этот нод
//       // 
//       bool update() override;
//     private:
//       size_t currentIndex;
//       std::vector<Node*> nodes;
//     };
//     
//     class OneJob : public Node {
//     public:
//       bool update() override;
//     private:
//       std::function<void()> job;
//     };
//     
//     class JobsPool : public Node {
//     public:
//       // поток пришедший сюда, получает новую задачу и начинает с ней работать
//       bool update() override;
//     private:
//       std::queue<std::function<void()>> tasks;
//     };
//   }
// }