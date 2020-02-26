#include "effects_component.h"

#include "EntityComponentSystem.h"
#include "attributes_component.h"
#include "global_components_indicies.h"

const size_t bit_mask = 0x7fffffff;

constexpr bool has_bit(const size_t &var) {
  return (var & (~bit_mask)) == (~bit_mask);
}

constexpr size_t set_bit(const size_t &var, const bool val) {
  return val ? var | (~bit_mask) : var & bit_mask;
}

namespace devils_engine {
  namespace components {
    effects::computed_data::computed_data() : container(0) {}
    effects::computed_data::computed_data(const uint8_t &stacks) : container(0) {
      container = container | size_t(stacks);
    }
    
    void effects::computed_data::set_stacks_count(const uint8_t &value) {
      container = container | (0xf & size_t(value));
    }
    
    uint8_t effects::computed_data::get_stacks_count() const {
      return uint8_t(container & 0xf);
    }
    
    void effects::computed_data::dec_stacks(const uint8_t &value) {
      const size_t a = get_stacks_count();
      set_stacks_count(uint8_t(std::min(a - value, size_t(max_stacks))));
    }
    
    void effects::computed_data::inc_stacks(const uint8_t &value) {
      const size_t a = get_stacks_count();
      set_stacks_count(uint8_t(std::min(a + value, size_t(max_stacks))));
    }
    
    void effects::computed_data::delete_next() {
      const size_t mask = 1 << 8;
      container = container | mask;
    }
    
    bool effects::computed_data::deletion() {
      const size_t mask = 1 << 8;
      return (container & mask) == mask;
    }
    
    void effects::computed_data::set_changed(const bool value) {
      const size_t mask = 1 << 9;
      container = value ? container | mask : container & ~mask;
    }
    
    bool effects::computed_data::changed() const {
      const size_t mask = 1 << 9;
      return (container & mask) == mask;
    }
    
    effects::effects(const create_info &info) : ent(info.ent) {}
    
    // может быть проблемой если time переменная величина
    void effects::update(const size_t &time) {
      auto attribs = ent->at<attributes>(game::monster::effects).get();
      if (attribs == nullptr) attribs = ent->at<attributes>(game::player::effects).get();
      
      for (size_t i = 0; i < datas.size(); ++i) {
        datas[i].stack_data.set_changed(false);
        if (datas[i].stack_data.deletion()) {
          if (i != datas.size()-1) std::swap(datas[i], datas.back());
          datas.pop_back();
          --i;
          continue;
        }
        
        datas[i].current_time += time;
        
        const bool period = datas[i].effect != nullptr && datas[i].data.period != SIZE_MAX && datas[i].current_time > time && (datas[i].current_time % datas[i].data.period <= time);
        // как пометить что стаки собирать нужно/не нужно, добавлять значения нужно/не нужно
        // нужно ли добавлять новый контейнер (думаю что это мало вероятно)
        // период только для effect != nullptr?
        
        // добавляем
        if (period) {
          datas[i].stack_data.inc_stacks(uint32_t(datas[i].effect->type.periodic_increase_stack())); // ???
          compute_add(attribs, datas[i], datas[i].effect->type.periodic_add());
          //add(datas[i].data, datas[i].effect, datas[i].source); ???
          datas[i].stack_data.set_changed(datas[i].effect->type.periodic_add());
        }
        
        // удаляем
        if (datas[i].current_time >= datas[i].data.time) {
          // нужен флаг тип ремувим сразу все или по стакам
          // что делать когда datas[i].effect == nullptr ?
          // я так понял нам скорее нужно не указатель на тип передавать
          // а собирать эффект с нуля почти, то есть копировать все данные
          // ну окей а как сейчас сделать?
          
          if ((datas[i].effect != nullptr && datas[i].effect->type.complete_remove()) || datas[i].effect == nullptr) {
            for (size_t i = 0; datas[i].effect->type.timed_remove() && i < datas[i].stack_data.get_stacks_count(); ++i) {
              compute_remove(attribs, datas[i], true); // это изменение то мне тоже нужно запомнить
            }
            datas[i].stack_data.set_changed(datas[i].effect->type.timed_remove());
            datas[i].stack_data.delete_next();
            continue;
          }
          
          //--datas[i].stacks;
          datas[i].stack_data.dec_stacks();
          compute_remove(attribs, datas[i], datas[i].effect->type.timed_remove());
          datas[i].stack_data.set_changed(datas[i].effect->type.timed_remove());
          if (datas[i].stack_data.get_stacks_count() == 0) {
            datas[i].stack_data.delete_next();
          }
        }
      }
      
      // перевычисляем аттрибуты тоже здесь? вряд ли
      // добавляются эффекты -> effects::update -> attributes::update
    }
    
    size_t effects::has(const utils::id &type) const {
      std::unique_lock<std::mutex> lock(mutex);
      
      size_t count = 0;
      size_t index = find(0, type);
      while (index != SIZE_MAX) {
        ++count;
        index = find(index+1, type);
      }
      
      return count;
    }
    
    size_t effects::has(const utils::id &type, const yacs::entity* source) const {
      std::unique_lock<std::mutex> lock(mutex);
      
      size_t count = 0;
      size_t index = find(0, type, source);
      while (index != SIZE_MAX) {
        ++count;
        index = find(index+1, type);
      }
      
      return count;
    }
    
    bool effects::reset_timer(const utils::id &id, size_t &mem) {
      std::unique_lock<std::mutex> lock(mutex);
      size_t index = find(mem, id);
      mem = index + size_t(index != SIZE_MAX);
      if (index != SIZE_MAX) {
        datas[index].current_time = 0;
        return true;
      }
      
      return false;
    }
    
    bool effects::reset_timer(const utils::id &id, const yacs::entity* source, size_t &mem) {
      std::unique_lock<std::mutex> lock(mutex);
      size_t index = find(mem, id, source);
      mem = index + size_t(index != SIZE_MAX);
      if (index != SIZE_MAX) {
        datas[index].current_time = 0;
        return true;
      }
      
      return false;
    }
    
    bool effects::increase_stack(const utils::id &id, const bool add, size_t &mem) {
      std::unique_lock<std::mutex> lock(mutex);
      auto attribs = ent->at<attributes>(game::monster::effects).get();
      if (attribs == nullptr) attribs = ent->at<attributes>(game::player::effects).get();
      
      size_t index = find(mem, id);
      mem = index + size_t(index != SIZE_MAX);
      if (index != SIZE_MAX) {
        datas[index].stack_data.inc_stacks();
        compute_add(attribs, datas[index], add);
        datas[index].stack_data.set_changed(add);
        return true;
      }
      
      return false;
    }
    
    bool effects::increase_stack(const utils::id &id, const yacs::entity* source, const bool add, size_t &mem) {
      if (mem == SIZE_MAX) return false;
      
      std::unique_lock<std::mutex> lock(mutex);
      auto attribs = ent->at<attributes>(game::monster::effects).get();
      if (attribs == nullptr) attribs = ent->at<attributes>(game::player::effects).get();
      
      const size_t index = find(mem, id, source);
      mem = index + size_t(index != SIZE_MAX);
      if (index != SIZE_MAX) {
        datas[index].stack_data.inc_stacks();
        compute_add(attribs, datas[index], add);
        datas[index].stack_data.set_changed(add);
        return true;
      }
      
      return false;
    }
    
    bool effects::add(const struct game::effect_t::container &data, const game::effect_t* effect, yacs::entity* source) {
      std::unique_lock<std::mutex> lock(mutex);
      auto attribs = ent->at<attributes>(game::monster::effects).get();
      if (attribs == nullptr) attribs = ent->at<attributes>(game::player::effects).get();
      
      // может быть ситуация когда нам нужно сделать уникальные эффекты у сорсов
      // лучше так не делать
      if (effect != nullptr) {
        size_t index = find(0, effect->id);
        if (index == SIZE_MAX) {
          const computed_effect eff{
            data,
            effect,
            source,
            0,
            computed_data(1)
          };
          
          // if (data.time != 0) // я все равно старательно допускаю ошибку по которой не смогу потом получить данные о источнике
          datas.push_back(eff);
          compute_add(attribs, eff, effect->type.add());
          datas.back().stack_data.set_changed(effect->type.add());
          return true;
        }
        
        while (index != SIZE_MAX) {
          if (effect->type.timer_reset()) datas[index].current_time = 0;
          if (effect->type.stackable()) {
            datas.back().stack_data.inc_stacks();
            compute_add(attribs, datas[index], effect->type.add());
            datas.back().stack_data.set_changed(effect->type.add());
          }
          
          if (effect->type.update()) {
            compute_remove(attribs, datas[index], true);
            datas[index].data = data;
            compute_add(attribs, datas[index], true);
            datas.back().stack_data.set_changed(true);
          }
          
          if (effect->type.unique()) return true;
          index = find(index+1, effect->id);
        }
      }

      const computed_effect eff{
        data,
        effect,
        source,
        0,
        computed_data(1)
      };
      
      // if (data.time != 0) 
      datas.push_back(eff);
      compute_add(attribs, eff, (effect != nullptr && effect->type.add()) || effect == nullptr);
      datas.back().stack_data.set_changed((effect != nullptr && effect->type.add()) || effect == nullptr);
      return true;
    }
    
    bool effects::remove(const utils::id &id, const bool complete_remove) {
      std::unique_lock<std::mutex> lock(mutex);
      auto attribs = ent->at<attributes>(game::monster::effects).get();
      if (attribs == nullptr) attribs = ent->at<attributes>(game::player::effects).get();
      
      const size_t index = find(0, id);
      if (index == SIZE_MAX) return false;
      
      if (complete_remove || datas.back().stack_data.get_stacks_count() == 1) {
        for (size_t i = 0; i < datas.back().stack_data.get_stacks_count(); ++i) {
          compute_remove(attribs, datas[index], datas[index].effect->type.remove());
        }
        datas.back().stack_data.set_changed(datas[index].effect->type.remove());
        datas.back().stack_data.delete_next();
        return true;
      }
      
      datas.back().stack_data.dec_stacks();
      compute_remove(attribs, datas[index], datas[index].effect->type.remove());
      datas.back().stack_data.set_changed(datas[index].effect->type.remove());
      return true;
    }
    
    bool effects::remove(const utils::id &id, const yacs::entity* source, const bool complete_remove) {
      std::unique_lock<std::mutex> lock(mutex);
      auto attribs = ent->at<attributes>(game::monster::effects).get();
      if (attribs == nullptr) attribs = ent->at<attributes>(game::player::effects).get();
      
      const size_t index = find(0, id, source);
      if (index == SIZE_MAX) return false;
      
      if (complete_remove || datas.back().stack_data.get_stacks_count() == 1) {
        for (size_t i = 0; i < datas.back().stack_data.get_stacks_count(); ++i) {
          compute_remove(attribs, datas[index], datas[index].effect->type.remove());
        }
        datas.back().stack_data.set_changed(datas[index].effect->type.remove());
        datas.back().stack_data.delete_next();
        return true;
      }
      
      datas.back().stack_data.dec_stacks();
      compute_remove(attribs, datas[index], datas[index].effect->type.remove());
      datas.back().stack_data.set_changed(datas[index].effect->type.remove());
      return true;
    }
    
    effects::effect_source effects::next(size_t &mem) const {
      const size_t index = mem++;
      
      std::unique_lock<std::mutex> lock(mutex);
      if (index >= datas.size()) return {nullptr, nullptr};
      return {datas[index].effect, datas[index].source};
    }
    
//     effects::change_effect effects::next(const utils::id &attrib_id, size_t &mem) const {
//       for (size_t i = mem; i < changes.size(); ++i) {
//         if (changes[i].bonus.attrib == attrib_id) {
//           mem = i+1;
//           return changes[i];
//         }
//       }
//       
//       return {nullptr, nullptr, {utils::id(), {0.0f, 0.0f}}};
//     }
    
    effects::change_effect effects::next_change(const utils::id &attrib_id, size_t &mem) const {
      for (size_t i = mem; i < datas.size(); ++i) {
        if (!datas[i].stack_data.changed()) continue;
        
        for (size_t j = 0; j < game::effect_t::max_bonuses; ++j) {
          if (datas[i].data.bonuses[j].attrib == attrib_id) {
            mem = i+1;
            return {datas[i].effect, datas[i].source, datas[i].data.bonuses[j]};
          }
        }
      }
      
      mem = SIZE_MAX;
      return {nullptr, nullptr, {utils::id(), {0.0f, 0.0f}}};
    }
    
    size_t effects::find(const size_t &start_index, const utils::id &type) const {
      for (size_t i = start_index; i < datas.size(); ++i) {
        if (datas[i].effect->id == type) return i;
      }
      
      return SIZE_MAX;
    }
    
    size_t effects::find(const size_t &start_index, const utils::id &type, const yacs::entity* source) const {
      for (size_t i = start_index; i < datas.size(); ++i) {
        if (datas[i].effect->id == type && datas[i].source == source) return i;
      }
      
      return SIZE_MAX;
    }
    
    void effects::compute_add(attributes* attribs, const computed_effect &effect, const bool is_needed) const {
      if (!is_needed) return;
      
      for (size_t i = 0; i < game::effect_t::max_bonuses; ++i) { // effect.data.size
        const auto &b = effect.data.bonuses[i];
        if (!b.attrib.valid()) continue;
        
        {
          auto attrib = attribs->find<core::float_type>(b.attrib);
          if (attrib != nullptr) {
            if (effect.effect->type.raw()) attrib->raw_add(b.bonus);
            else attrib->final_add(b.bonus);
          }
        }
        
        {
          auto attrib = attribs->find<core::int_type>(b.attrib);
          if (attrib != nullptr) {
            if (effect.effect->type.raw()) attrib->raw_add(b.bonus);
            else attrib->final_add(b.bonus);
          }
        }
      }
    }
    
    void effects::compute_remove(attributes* attribs, const computed_effect &effect, const bool is_needed) const {
      if (!is_needed) return;
      
      for (size_t i = 0; i < game::effect_t::max_bonuses; ++i) { // effect.data.size
        const auto &b = effect.data.bonuses[i];
        if (!b.attrib.valid()) continue;
        
        {
          auto attrib = attribs->find<core::float_type>(b.attrib);
          if (attrib != nullptr) {
//             for (size_t i = 0; i < effect.stacks; ++i) {
              if (effect.effect->type.raw()) attrib->raw_remove(b.bonus);
              else attrib->final_remove(b.bonus);
//             }
          }
        }
        
        {
          auto attrib = attribs->find<core::int_type>(b.attrib);
          if (attrib != nullptr) {
//             for (size_t i = 0; i < effect.stacks; ++i) {
              if (effect.effect->type.raw()) attrib->raw_remove(b.bonus);
              else attrib->final_remove(b.bonus);
//             }
          }
        }
      }
    }
    
//     void effects::compute_add(const computed_effect &effect, const bool is_needed) {
//       if (!is_needed) return;
//       
//       for (size_t i = 0; i < game::effect_t::max_bonuses; ++i) {
//         const auto &b = effect.data.bonuses[i];
//         if (!b.attrib.valid()) continue;
//         changes.push_back({
//           effect.effect,
//           effect.source,
//           {
//             b.attrib,
//             {b.bonus.add, b.bonus.mul}
//           }
//         });
//       }
//     }
//     
//     void effects::compute_remove(const computed_effect &effect, const bool is_needed) {
//       if (!is_needed) return;
//       
//       for (size_t i = 0; i < game::effect_t::max_bonuses; ++i) {
//         const auto &b = effect.data.bonuses[i];
//         if (!b.attrib.valid()) continue;
//         changes.push_back({
//           effect.effect,
//           effect.source,
//           {
//             b.attrib,
//             {-b.bonus.add, -b.bonus.mul}
//           }
//         });
//       }
//     }
    
  }
}
