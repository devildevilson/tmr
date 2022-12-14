#ifndef PHYSICS_CORE_CONTEXT_H
#define PHYSICS_CORE_CONTEXT_H

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
  namespace physics {
    namespace core {
      struct context_interface {
        virtual ~context_interface() {}
      };
    }
  }
}

#endif
