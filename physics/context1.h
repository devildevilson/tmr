#ifndef PHYSICS_CONTEXT_H
#define PHYSICS_CONTEXT_H

#include <cstddef>
#include <cstdint>

// у меня движение определялось раньше инпутом
// но в нем не особо много смысла
// в булете для объектов управляемых чемто
// используется btCharacterControllerInterface
// в котором можно выставить направление передвижения,
// прыжок, модификатор скорости на какое-то время
// проверить находится ли объект на земле
// видимо нужно вещи связанные с управлением 
// вытащить в отдельный степ

// еще было бы неплохо сделать составные объекты 
// персонаж сотоит например из неспольких боксов
// которые в свою очередь как то связаны констраинтами
// тогда разъединение физического тела от положения не имеет смысла
// и еще не понятно как рисовать это дело
// это скорее всего более высокий уровень абстракции 
// а здесь в физике мы должен просто вычислить все по правилам
// не особенно вдаваясь в подробности какой объект это персонаж а какой нет

namespace devils_engine {
  namespace utils {
    template <typename T>
    class array_interface;
  }
  
  namespace physics {
    struct transform;
    struct rigid_body;
    struct collision_shape;
    struct object;
    
    class core_context {
    public:
      virtual ~core_context() {}
      virtual utils::array_interface<transform>* transforms() = 0;
      virtual utils::array_interface<rigid_body>* bodies() = 0;
      virtual utils::array_interface<collision_shape>* shapes() = 0;
      virtual utils::array_interface<object>* objects() = 0;
    };
  }
}

#endif
