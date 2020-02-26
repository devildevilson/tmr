#ifndef GAME_FUNCS_H
#define GAME_FUNCS_H

#include "id.h"
#include "Utility.h"
#include "attribute.h"
#include <vector>

// у нас есть несколько типов функций, которые будут вызываться в разных частях игры
// во первых функции для состояний: эти функции будут вызываться когда у нас переключается состояние
// во вторых функции взаимодействия: функции, которые констролируют обработку атаки, коллизию и проч
// в третьих?

// короче опять беда с аттрибутами вообще не получается никак
// функции аттрибутов не учитывают убийство цели игроком
// а прямой доступ к аттрибуту теряют изменение аттрибута
// и после я не очень понимаю как отменить бафф например
// есть еще вариант: хранить модификаторы для каждого аттрибута
// нет, выдать аттрибуту все текущие модификаторы
// то есть массив вида: {эффект, сорс, бонус}
// next(attr_type) : {effect, source, bonus}
// нам нужно помечать какие эффекты послали изменения в этом кадре
// выдвать их ввиде как показано выше и по этим данным пытаться определить кто нас убил
// похоже на костыль
// добавлять именно изменения в специальный массив, а потом его обходить и применять изменения
// выглядит неплохо, не тоже самое, в этом случае, я применяю изменения сразу окончательно
// почему окончательно? потому что иначе не понятно как отделить разные эффекты друг от друга
// поэтому самый адекватный способ в этом случае хранить изменения

namespace yacs {
  class entity;
}

namespace devils_engine {
  namespace components {
    class effects;
  }
  
  namespace game {
    struct effect_t;
    struct ability_t;
    
    // первые функции вызываются от состояния и по хорошему не должны использовать синхронизацию
    // здесь потребуется строго определить как будут взаимодействовать объекты, через это как 
    // сделать синхронизацию
    void weapon_ready(yacs::entity* ent, const size_t &looping_time);
    void fireball_ability_cast(yacs::entity* ent);

    // вторые функции имеют по крайней мере два участника, как это дело синхронизовать?
    // что item_pickup, что damage_ent вряд ли будут вызываться часто в одном кадре 
    // (сомневаюсь что будет хоть один кадр в котором эти функции будут вызваны 2 раза одновременно)
    // поэтому их можно вызвать последовательно вместе с какой нибудь другой работой (например звуки и графика)
    void item_pickup(yacs::entity* ent, yacs::entity* item);
    void item_pickup2(yacs::entity* ent, yacs::entity* item, const utils::id &prop, const size_t &quantity);
    void collision_func(yacs::entity* prop_ent, yacs::entity* ent, const utils::id &prop);
    void damage_ent(yacs::entity* source, yacs::entity* ent, const effect_t* effect); // тут поди нужно добавить интеракцию
    core::int_type health_func(yacs::entity* ent, const struct core::attribute_t<core::int_type>::type* attrib_type, const core::int_type &base, const core::int_type &raw_add, const float &raw_mul, const core::int_type &final_add, const float &final_mul);
//     core::int_type health_func2(yacs::entity* ent, const struct core::attribute_t<core::int_type>::type* attrib_type, const core::int_type &prev, const components::effects* effects);
  }
}

// в ии компоненте в луа необходимо держать константные указатели на все данные
// изменять все нужно только через специальные интерфейсы

#endif
