#ifndef CORE_FUNCS_H
#define CORE_FUNCS_H

#include "id.h"
#include "Utility.h"
#include <functional>
#include "sound_info.h"

// может быть проще ко всем функциям которые отвечают за взаимодействие
// передавать еще и каллбек функцию? так проще будет обозначить 
// какое именно взаимодействие мы хотим получить
// так даже можно эффект не передавать

// нужно еще глобальную систему частиц сделать (по большому счету требуется просто создать частицу)
// и систему декалей
// ну и я так полагаю это все

// я так полагаю частицы нужно не только просто создать, но и сделать какие то типы источников
// например сферический источник - частицы разлетаются в разные стороны и проч

namespace yacs {
  class entity;
}

namespace devils_engine {
  namespace game {
    struct effect_t;
    struct ability_t;
  }
  
  namespace core {
    // в любом случае нужна будет интеракция (нужно найти точку удара)
    using interaction_callback = std::function<void(yacs::entity*, yacs::entity*, const game::effect_t*)>;
    
    bool is_player(const yacs::entity* ent);
    bool is_dead(const yacs::entity* ent);
    bool set_dead(yacs::entity* ent);
    bool deleted_state(const yacs::entity* ent);
    bool has_collision_trigger(const yacs::entity* ent);
    bool set_collision_trigger(yacs::entity* ent, const bool value);
    void remove(yacs::entity* ent); // должна быть потоко независима
    bool set_bit(yacs::entity* ent, const uint32_t &index, const bool value);
    bool get_bit(const yacs::entity* ent, const uint32_t &index);
    int64_t get_int(const yacs::entity* ent, const uint32_t &index);
    int64_t set_int(yacs::entity* ent, const uint32_t &index, const int64_t &value); // может быть стоит сделать все эти операции и с const указателем
    int64_t inc_int(yacs::entity* ent, const uint32_t &index, const int64_t &value);
    int64_t dec_int(yacs::entity* ent, const uint32_t &index, const int64_t &value);
    int64_t and_int(yacs::entity* ent, const uint32_t &index, const int64_t &value);
    int64_t or_int (yacs::entity* ent, const uint32_t &index, const int64_t &value);
    int64_t xor_int(yacs::entity* ent, const uint32_t &index, const int64_t &value);
    
    simd::vec4 player_pos();
    
    // эта функция тоже очень "опасная" но она легко попадает в систему работ
    void teleport_entity(yacs::entity* ent, const simd::vec4 &new_pos);
    
    // в принципе любому создаваемому энтити полезно посчитать аттрибуты
    // неплохо было бы получить аттрибуты типа, 
    // сам энтити понятное дело лучше создать отдельно от этой функции
    // энтити считать атрибуты совсем не обязательно, единственный аттрибут который имеет смысл менять это максимальную скорость
    // да и то будет ли он существовать? так ли необходимо это делать?
    void create_entity(const utils::id &type, const yacs::entity* parent, const game::ability_t* ability, const simd::vec4 &pos, const simd::vec4 &rot, const simd::vec4 &vel);
    
    // такие атаки можно делать двумя способами
    // создавать на указаное время взаимодействие, либо создавать пока вызывается эта функция
    // лучше конечно первое, но для этого придется ввести еще одну функцию
    enum class complex_attack_state {
      creating,
      proccessing,
      done
    };
    complex_attack_state slashing_attack(
      yacs::entity* ent, 
      const game::effect_t* effect, 
      const float &distance, 
      const float &start_angle, 
      const float &end_angle, 
      const size_t &time, 
      const float &speed, 
      const float plane[4], 
      const uint32_t &tick_count, 
      const size_t &tick_time
    );
    bool remove_slashing_attack(yacs::entity* ent, const game::effect_t* effect);
    
    // то же самое для stabbing (нужно ли тут указывать угол? хотя я думаю что нет, должна быть константа какая то не очень большая)
    complex_attack_state stabbing_attack(
      yacs::entity* ent, 
      const game::effect_t* effect, 
      const float &min_dist, 
      const float &max_dist, 
      const size_t &time, 
      const float &speed, 
      const uint32_t &tick_count, 
      const size_t &tick_time
    );
    bool remove_stabbing_attack(yacs::entity* ent, const game::effect_t* effect);
    
    // так же есть необходимость кастить лучи
    enum class ray_state {
      creating,
      ray_path_clear,
      ray_blocked
    };
    ray_state check_visability(const yacs::entity* ent, const yacs::entity* target);
    bool ray_shot(yacs::entity* ent, const simd::vec4 &dir, const game::effect_t* effect);
    bool ray_shot(yacs::entity* ent, const simd::vec4 &dir, const game::effect_t* effect, const interaction_callback &callback);
    void hit_target(yacs::entity* ent, yacs::entity* target, const game::effect_t* effect);
    void hit_target(yacs::entity* ent, yacs::entity* target, const game::effect_t* effect, const interaction_callback &callback);
    
    // чем плохи коллбеки? я знаю когда они будут вызываться, я знаю что примерно там должно быть
    // я знаю что колбек будет так или иначе связан с изменением состояния двух энтити
    // но я не могу предугадать сложность функции сделанной игроком
    // в общем то я и в стандартном случае не мог этого сделать
    // колбек функции - нормальный способ оформить взаимодействие
    // но с другой стороны, делать по функции для каждого взаимодействия возможно каждого монстра
    // в любом случае скорее всего нужно передать еще какую-то дополнительную информацию
    // вот этот вопрос нужно решить? передавать только функцию, только некую инфу, а потом использовать в одной "глобальной" функции
    // или передавать и функцию и дополнительные данные? последнее выглядит самым идеальным вариантом
    // а второе выглядит идеальным вариантом с точки зрения наполнения, первое ни то ни се, 
    // но можно просто определить внешнюю функцию которая будет вызывать одну с определенными данными
    // либо сделать вообще два варианта и глобальную функцию и возможность вызова локальной
    
    bool start_sound(const sound::info &info);
    bool stop_sound(const yacs::entity* ent);
    
    // это не случайные функции, с другой стороны они врядли понадобятся без рандома
    int32_t range(const uint32_t &num, int32_t min, int32_t max);
    
    utils::id current_map();
    utils::id current_episod();
    
    void next_map();
    void next_map_secret();
    void change_map(const utils::id &episod, const utils::id &map);
  }
  
  // продублировать некоторые функции из random.h?
  namespace random {
    uint32_t get();
    float norm();
    bool chance(const float &percent);
    int32_t range(const int32_t &min, const int32_t &max);
    uint32_t dice(const uint32_t &rolls, const uint32_t &faces);
  }
}

#endif
