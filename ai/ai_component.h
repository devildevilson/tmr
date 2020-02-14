#ifndef AI_COMPONENT_H
#define AI_COMPONENT_H

#include <functional>
#include <vector>
#include "id.h"

// теперь когда мы используем yacs::entity более менее полностью нет необходимости делать сложный ии класс
// по сути у нас три состояния: ии нет, ии - функция, ии - дерево поведения
// причем возможно даже для отсутствующего ии нужно добавить возможность запоминать какие то цели

// теперь совсем не обязательно создавать для каждого объекта basic_ai
// точнее динамические объекты просто будут требовать обновить положение 
// я тут подумал, корректировать положение в вершине достаточно быстро можно
// спроецировав положение объекта на плоскость (ну и точки плоскости тоже)
// эта несложная математика позволяет быстро и точно посчитать изменения в графе
// необходимо только бросить луч в первый раз и затем все последующие движения 
// никогда не будут слишком большими и можно будет проверять лишь проекцию

namespace yacs {
  class entity;
}

namespace tb {
  class BehaviorTree;
  class Node;
}

namespace devils_engine {
  namespace ai {
    enum class status {
      success,
      failure,
      running
    };
  }
  
  namespace components {
    struct basic_ai {
      static const size_t memory_max_size = 8;
      
      yacs::entity* ent;
      
      //size_t memory_size;
      std::pair<utils::id, const yacs::entity*> data[memory_max_size];
      
      uint32_t ray_index;
      yacs::entity* ground;
      yacs::entity* old_ground;
      
      struct create_info {
        yacs::entity* ent;
      };
      basic_ai(const create_info &info);
      // чекаем валидность entity, чекаем землю
      void new_frame();
      void teleported();
    };
    
    struct tree_ai : public basic_ai {
      // в дерево видимо теперь нужно отправлять yacs::entity
      tb::BehaviorTree* tree;
      tb::Node* running_node;
      
      size_t update_time;
      size_t current_time;
      
      struct create_info {
        yacs::entity* ent;
        tb::BehaviorTree* tree;
        size_t update_time;
      };
      tree_ai(const create_info &info);
      void update(const size_t &time);
    };
    
    struct func_ai : public basic_ai {
      // функция может работать с графом, но в графе я так понимаю данные должны храниться строго константными
      ai::status last_status;
      std::function<ai::status(yacs::entity*)> ai_func;
      
      size_t update_time;
      size_t current_time;
      
      struct create_info {
        yacs::entity* ent;
        std::function<ai::status(yacs::entity*)> func;
        size_t update_time;
      };
      func_ai(const create_info &info);
      void update(const size_t &time);
    };
  }
}

#endif
