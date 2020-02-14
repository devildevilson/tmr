#ifndef MOVEMENT_COMPONENT_H
#define MOVEMENT_COMPONENT_H

#include <cstddef>
#include <cstdint>

#include "id.h"
#include "Utility.h"
#include "pathfinder_system.h"

// должен обрабатывать любой способ перемещения
// должен обрабатывать поиск пути
// двери тоже должны пользоваться этим компонентом
// двери должны обладать несколькими состояниями
// старт, движение, конец, движение в обратную сторону
// нужно определить когда необходимо остановиться
// (можно по времени, можно по положению)
// (для того чтобы ориентироваться в пространтсве, нужно запоминать положение)
// как приводить объект в движение, в смысле как оповещать разные объекты
// в думе игрок "использует" линейный сектор, 
// в секторе можно указать тип и данные двери
// к сектору привязан тэг, когда игрок открывает дверь 
// он отправляет номер тега и скорость, все объекты с этим тегом
// начинают открываться и заканчивают движение в зависимости от окружающих
// объектов (6 едениц до нужного сектора)
// как можно сделать мне? у меня две проблемы: как передать объекту
// сообщение о том что мы хотим привести в движение произвольное количество объектов
// и как определить закрыта или открыта дверь
// и тут две проблемы: мне необходимо учитывать двери двух типов: 
// которые контролируются movement и которые контролируются гравитацией
// мне это нужно для того чтобы контролировать поиск пути 
// более менее понятно что делать с обычной дверью, нужно запомнить
// вершину на которую опускается дверь, и "отключить" ее
// вот что я думаю сделать
// нужно разделить несколько типов взаимодейтсвий 
// одно из которых будет взаимодействие со "связанным" объектом 
// components::link - должен хранить указатель (способ связи) на другой
// объект, получаю его (или отправляю команды всем по какому то правилу)
// и меняю ему состояние, лучше использовать тэг 
// (тэг - индекс массива связанных объектов)

// взаимодействие использование вызывает функцию от двух энтити
// мы должны как то запомнить тэг, в линке? 
// не помешало бы просто числа уметь запоминать (например вести статистику игрока)
// выделить 32 числа на энтити

namespace yacs {
  class entity;
}

yacs::entity* tag_entity(uint32_t tag);
yacs::entity* next(yacs::entity* ent);

// энтити можно расположить в глобальном массиве попарно с тэгами
// причем все энтити, тэг присваивается в файле карты
// 
yacs::entity* next(uint32_t tag, yacs::entity* ent);
yacs::entity* next(uint32_t tag, size_t &mem);

namespace devils_engine {
  namespace components {
    class movement {
    public:
      enum class path_state {
        found,
        finding,
        not_found
      };
      
      enum class state {
        path_not_exist,
        travel_old_path,
        travel_path,
        end_travel
      };
      
      struct create_info {
        yacs::entity* ent;
        utils::id path_find_type;
      };
      movement(const create_info &info);
      
      path_state find_path(const yacs::entity* target); // по идее мы можем указать любой объект на уровне
      state travel_path();
      bool path_exist() const;
      void move(const simd::vec4 &dir); // указываем направление
      
      // нужно ли делать версию с конкретной позицией? как сделать патруль? 
      // можно создать несколько псевдо объектов, и двигаться от одного к другому
      void pursue(const yacs::entity* target); // скорее всего просто двигаемся к цели
      void flee(const yacs::entity* target); // бежим от цели
    private:
      struct path_data {
        path::container* path;
        path::request req;
      };
      
      size_t pathfinding_time;
      size_t current_segment;
      
      yacs::entity* ent;
      utils::id path_find_type;
      path_data current_path;
      path_data old_path;
      
      // функции следования по пути мы поставим сюда
      simd::vec4 predict_pos(const size_t &predictionTime) const;
      simd::vec4 seek(const simd::vec4 &target);
      simd::vec4 flee(const simd::vec4 &target);
      simd::vec4 follow_path(const size_t &predictionTime, const path::container* path, const size_t &currentPathSegmentIndex);
      void stay_on_path(const size_t &predictionTime, const path::container* path);
      
      void mod_entity_rotation();
      
      void release_current_path();
      void release_old_path();
      void release_paths();
    };
    
    struct link {
      //uint32_t tag;
      yacs::entity* ent;
      link* next;
      link* tail;
    };
    
    struct memory {
      struct pair1 {
        size_t id;
        yacs::entity* ent;
      };
      
      struct pair2 {
        size_t id;
        int64_t data;
      };
      
      size_t size1;
      size_t size2;
      pair1 p1[32];
      pair2 p2[32];
    };
  }
}

#endif
