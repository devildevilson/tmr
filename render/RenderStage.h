#ifndef RENDER_STAGE_H
#define RENDER_STAGE_H

#include <cstdint>

namespace yavf {
  class TransferTask;
  class CombinedTask;
  class ComputeTask;
  class GraphicTask;
  class TaskInterface;
}

// так же тут можно возвращать буферы камеры и матриц например
class RenderContext {
public:
  virtual ~RenderContext() {}
  
  virtual yavf::TaskInterface* interface() const = 0;
  virtual yavf::CombinedTask* combined() const = 0;
  virtual yavf::ComputeTask* compute() const = 0;
  virtual yavf::GraphicTask* graphics() const = 0;
  virtual yavf::TransferTask* transfer() const = 0;
};

// короч это отдельный класс для разного рода команд рендера
// тип с помощью него составлять рендер
// что то вроде монстерс->драв(); ... постэффект->драв(); ... и так далее
// неплохо было бы подумать о том сколько чего мне потребуется
// и о том как бы этот механизм сделать удобно собираемым
// то есть сначало должны быть драверы для объектов
// затем пост эффект дравер (свет ... может что то еще)
// после интерфейс дравер и так далее
// но на каждом шаге их может быть несколько

// имеет смысл подумать на счет чего то вроде компонентов в рендере
// что то вроде RenderStage, каждый такой стейдж будет последовательно запускаться в рендере
// в некоторых конкретных стейджах может быть несколько других стейджев и тд
// было бы неплохо хранить стейджы в рендере (механизм выделения памяти для этого хранить в самом классе)
// проблема в том что одинаковых стейджей будет мало (нисколько наверное), но самих стейджей будет достаточно
// то есть было бы неплохо засунуть их всех просто в один большой массив
// они должны быть простокопируемы (тривиально копируемы) тогда
// + мне в будущем потребуется механизм который будет рисовать картинки для разных устройств отдельно
// это скорее всего пригодится для разных экранов (ну может еще какая подоплека)

// в этих рендер стейджах и будет происходить заполнение командных буферов
// рендер вседа должен отрабатывать их последовательно, но в самих стейджах может быть все распараллелено
// + возможно неплохой идеей будет убрать отсюда getSecondaryTask, а в конкретном стейдже просто придумать что то другое
// так и сделаю наверное
class RenderStage {
public:
  virtual ~RenderStage() {}
  
  // возможно стоит еще добавить метод который будет запускаться сразу после вызова рендера
  // но еще до непосредственной отрисовки, хотя это спорный метод
  // тип сюда можно добавить например отработку оптимизеров (может быть даже параллельно), но нужно ли это вообще?
  virtual void begin() = 0;
  
  // тут по сути нужно только сделать работу
  virtual void doWork(RenderContext* context) = 0;

  // должен быть еще метод на пересоздание рендертаргетов
  // это означает что нужно только передать размеры? или что то еще?
  // может быть эти стейджи передать в окно, пусть там со всем разбирается?
  virtual void recreate(const uint32_t &width, const uint32_t &height) = 0;
  
  // но например в случае с непосредственным рендерингом мне еще потребуется что-то отсюда получить
  // например таск заполненный командами для отрисовки например монстров
  //virtual void* getSecondaryTask() = 0;
};

#endif
