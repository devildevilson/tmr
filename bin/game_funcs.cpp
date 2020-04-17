#include "game_funcs.h"

#include "EntityComponentSystem.h"
#include "id.h"
#include "inventory_component.h"
#include "type_info_component.h"
#include "abilities_component.h"
#include "states_component.h"
#include "weapon.h"
#include "Globals.h"
#include "delayed_work_system.h"
#include "global_components_indicies.h"
#include "interaction.h"
#include "TransformComponent.h"
#include "PhysicsComponent.h"
#include "shared_collision_constants.h"
// #include "UserDataComponent.h"
#include "entity_loader.h"
#include "entity_creator_resources.h"
#include "ai_component.h"
#include "core_funcs.h"
#include "effects_component.h"
#include "graphics_component.h"

enum class action {
  attack,
  use,
  jump,
  croach,
  none
};

namespace devils_engine {
  namespace game {
    void weapon_ready(yacs::entity* ent, const size_t &looping_time) {
      // в этой функции мы должны получить команды от игрока
      // и на основе них поменять состояние
      // осталось только понять какие команды
      // в думе проверялись клавиши, я хочу скрыть реализацию клавиш
      // но при этом оставить гибкость, что я могу сделать?
      
      // две вещи: в думе есть отличие между оружиями по части атаки
      // бфг и ракеты не могут стрелять постоянно
      // и
      // ??? (забыл)
      
      // в моем случае мне нужно вернуть положение меча на правую сторону
      // через какое то время, но при этом мне нужно каждый кадр вызывать функцию weapon_ready
      // и проверять нажатие кнопок, это значит что у меня нет возможности отследить время
      // для конкретного состояния
      // задачу можно решить несколькими способами: ввести много состояний вызывающих друг друга (но нужно понятное дело миллион состояний)
      // использовать рефайр - то есть функция после основной атаки позволяющая запустить атаку снова быстрее
      // ввести еще одну переменную - время повтора, то есть время пока вызывается одно и тоже состояние 
      // последнее немного топорно получится + откуда взять следующее состояние
      // можно отправлять время сюда и уже здесь решать, большая часть эта переменная времени правда будет бесполезной
      // но выглядит более менее честно говоря
      
      auto weapons = ent->get<components::weapons>();
      auto states = ent->get<components::states>();
      
      if (looping_time >= ONE_SECOND) {
        states->set(weapons->current()->states[2]);
        return;
      }
      
      // нужно получать эвенты, я так думаю нужно регистрировать эвенты в игре
      // присваивать им кнопки, даже регистрировать эвенты для персонажей
      const action a = action::attack;
      switch (a) {
        case action::attack: {
          states->set(weapons->current()->states[1]);
          break;
        }
        
        default: {
          
          break;
        }
      }
      
      // покачивание оружия
    }
    
    void fireball_ability_cast(yacs::entity* ent) {
      // здесь мы должны создать энтити
      // теперь мы можем все данные расчитать тут
      // нужно определить функцию вызываемую при коллизии
      // это явно какая то глобальная функция
      // может указать функцию колизии при создании объекта
      // вообще история с проперти неплоха
      // properties collision... в нем можно указать функцию
      // и ограничения на тех кого мы в эту функцию засовываем
      // но закончится все может тем что мы запускаем крайне тяжелые функции
      // каждый кадр да еще и в одном потоке по всей видимости
      // вызов каждый кадр означает что у нас есть продолжительная по времени коллизия
      // какие это объекты? предмет/монстр + пол
      // проблема еще вот в чем, мы можем хотеть вызывать или не вызывать эти функции 
      // например огненный шар при первом столкновении переходит в состояние взрыва
      // и в этом состоянии уже не пересекается ни с чем
      // это вполне можно сделать флажком
      // нужно ли ограничить объекты к которым применяется триггер? было бы неплохо
      // как это сделать? по идее обычным коллижн фильтром, идея хорошая, 
      // но в абсолютном большинстве случаев достаточно фильтрации по интеракции
      // поэтому не надо
      // кстати можно закидывать функции прямо во время исполнения текущих
      // но работает это только после исполнения юзердефайнед логики
      // 
    }
    
//     const utils::id medkit = utils::id::get("medkit");
//     const utils::id bullshit = utils::id::get("bullshit");
//     
//     // по идее здесь всегда должен приходить игрок, а вторым объектом предмет с properties::pickup
//     void item_pickup(yacs::entity* ent, yacs::entity* item) {
//       auto pickup = item->get<properties::pickup>();
//       auto inv = ent->get<components::inventory>();
//       if (pickup->type == medkit) {
//         // должен быть глобальный доступ к эффектам? видимо
//       }
//       
//       if (pickup->type == bullshit) {
//         inv->add(pickup->type, pickup->count);
//       }
//       
//       core::remove(item);
//       // статистика
//       
//     }
//     
//     void item_pickup2(yacs::entity* ent, yacs::entity* item, const utils::id &prop, const size_t &quantity) {
//       // примерно тоже что и у предыдущего
//       // может все же спрятать prop и quantity?
//       // в принципе однозначно то какой из энтити является айтемом, можно у него брать информацию
//       // с другой стороны зачем?
//       
//       std::cout << "item pickup property " << prop.name() << "\n";
//     }
    
    void collision_func(yacs::entity* prop_ent, yacs::entity* ent, const utils::id &prop) {
      std::cout << "collision func property " << prop.name() << "\n";
    }
    
    void damage_ent(yacs::entity* source, yacs::entity* ent, const effect_t* effect) {
      auto info = source->at<components::type_info>(game::entity::type_info);
      if (info->created_ability == nullptr) {
        // это не проджектайл
      }
      
      // да кстати если мы не будем использовать константный указатель source 
      // то мы сможем сделать "неуспешную" атаку
      // 
    }
    
    // не забываем все yacs::entity в луа будут константными, здесь чтобы не писать лишнего буду так оставлять
    core::int_type health_func(yacs::entity* ent, const struct core::attribute_t<core::int_type>::type* attrib_type, const core::int_type &base, const core::int_type &raw_add, const float &raw_mul, const core::int_type &final_add, const float &final_mul) {
      core::int_type current = base;
      current += raw_add;
      current *= raw_mul;
      
      current = std::min(current, core::int_type(100));
      
      current += final_add;
      current *= final_mul;
      
      current = std::min(current, core::int_type(200));
      
      // сначало кажется что это выглядит убого, но если сделать адекватную функцию поиска
      // изменений, то сойдет для одной функции, как сделать изменения?
      // флажок? в принципе тоже выглядит плохо из-за метода next_change
      // по идее этот метод покрывает большую часть потребностей, 
      // у нас очень маловероятно может возникнуть ситуация когда два игрока одновременно бьют
      // может возникнуть ситуация когда игрок хилит и в тот же момент умирает челик 
      // но наверное если проверить тип эффекта то можно этот баг избежать
      // как то так
      if (current <= 0) {
        auto effects = ent->at<components::effects>(game::monster::effects);
        if (!effects.valid()) effects = ent->at<components::effects>(game::player::effects);
        
        size_t mem = 0;
        yacs::entity* player = nullptr;
        auto tmp = effects->next_change(attrib_type->id, mem);
        while (tmp.attrib_bonus.attrib.valid()) {
          if (core::is_player(tmp.source)) {
            player = tmp.source; // tmp.attrib_bonus.bonus.add < 0.0f && 
            break;
          }
          tmp = effects->next_change(attrib_type->id, mem);
        }
        if (player != nullptr) core::inc_int(player, 0, 1); // игрока мы кстати должны мочь найти и в глобале
        //core::remove(ent);
        // change_state
        core::set_dead(ent);
      }
      
      return current;
    }
    
    void pickup_test_item(yacs::entity* prop_ent, yacs::entity* ent, const utils::id &prop) {
      if (!core::is_player(ent)) return;
      static const utils::id tmr_item_pickup1 = utils::id::get("tmr_item_pickup1");
      
      auto type = prop_ent->at<components::type_info>(game::entity::type_info);
      auto interface = ent->at<components::player_interface>(game::player::interface);
//       const char u[] = {"\u25A0"};
      // █■∀ \u2588\u25A0\u2200 - эти символы присутствуют далеко не везде
      // я так понимаю мне нужно сделать отдельный шрифт для сообщений в чате (в консоле?)
      //const std::string str = "picked up item "+type->id.name()+" property "+prop.name()+" ";
      const std::string str = "A chainsaw! Find some meat";
      std::cout << str << "\n"; 
      interface->info_message(str);
//       interface->info_message(str+"2");
//       interface->info_message(str+"3");
//       interface->info_message(str+"4");
      // пишем сообщение о взятии предмета в интерфейс
      core::start_sound(sound::info(ent, tmr_item_pickup1, 1.0f)); // саунд инфо можно сократить?
      core::remove(prop_ent);
    }
    
    // уже гораздо лучше, с другой стороны такая фигня с inc_int требуется только в хп
//     core::int_type health_func2(yacs::entity* ent, const struct core::attribute_t<core::int_type>::type* attrib_type, const core::int_type &prev, const components::effects* effects) {
//       core::int_type current = prev;
//       core::int_type raw_add = 0, final_add = 0;
//       yacs::entity* player = nullptr;
//       
//       size_t mem = 0;
//       auto tmp = effects->next(attrib_type->id, mem);
//       while (tmp.bonus.attrib.valid()) {
//         if (tmp.effect->type.raw()) raw_add += tmp.bonus.bonus.add;
//         else final_add += tmp.bonus.bonus.add;
//         if (tmp.bonus.bonus.add < 0.0f) player = tmp.source;
//         auto tmp = effects->next(attrib_type->id, mem);
//       }
//       
//       current += raw_add;
//       current = std::min(current, core::int_type(100));
//       
//       current += final_add;
//       current = std::min(current, core::int_type(200));
//       
//       if (current <= 0) {
//         if (player != nullptr) core::inc_int(player, 0, 1);
//         core::remove(ent);
//       }
//       
//       return current;
//     }
  }
}
