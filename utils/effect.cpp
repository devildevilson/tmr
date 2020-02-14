#include "effect.h"

#include <cstring>

namespace devils_engine {
  namespace game {
//     effect_t::container::container(const size_t &time, const size_t &period, const size_t &size) : time(time), period(period), size(size), bonuses(new attrib_bonus[size]) {}
//     void effect_t::container::clear() { delete [] bonuses; }
    effect_t::container::container() : time(0), period(SIZE_MAX) {}
    effect_t::container::container(const container &c) : time(c.time), period(c.period) {
      memcpy(bonuses, c.bonuses, sizeof(attrib_bonus)*max_bonuses);
    }
    
    enum EffectTypeEnum : uint32_t {
      EFFECT_TYPE_RAW = (1 << 0),
      EFFECT_TYPE_ADD = (1 << 1),
      EFFECT_TYPE_REMOVE = (1 << 2),
//       EFFECT_TYPE_PERIODICALY_APPLY = (1 << 3),
//       EFFECT_TYPE_COMPUTE_EFFECT = (1 << 4),
//       EFFECT_TYPE_CAN_RESIST = (1 << 5),
//       EFFECT_TYPE_ONE_TIME_EFFECT = (1 << 6),
      EFFECT_TYPE_RESET_TIMER = (1 << 3),
//       EFFECT_TYPE_STACKABLE = (1 << 8),
      EFFECT_TYPE_EASY_STACK = (1 << 4),
      EFFECT_TYPE_UNIQUE = (1 << 5),
      EFFECT_TYPE_UPDATE = (1 << 6),
      EFFECT_TYPE_COMPLETE_REMOVE = (1 << 7),
      EFFECT_TYPE_PERIODIC_ADD = (1 << 8),
      EFFECT_TYPE_PERIODIC_INCREASE_STACK = (1 << 9),
      EFFECT_TYPE_TIMED_REMOVE = (1 << 10),
    };

    effect_t::type::type() : container(0) {}
    effect_t::type::type(const struct effect_t::type &t) : container(t.container) {}
    effect_t::type::type(const bool raw, const bool add, const bool remove, const bool timer_reset, const bool easy_stack, const bool unique, const bool update, const bool complete_remove, const bool periodic_add, const bool periodic_increase_stack, const bool timed_remove) : container(0) {
      make(raw, add, remove, timer_reset, easy_stack, unique, update, complete_remove, periodic_add, periodic_increase_stack, timed_remove);
    }

    void effect_t::type::make(const bool raw, const bool add, const bool remove, const bool timer_reset, const bool easy_stack, const bool unique, const bool update, const bool complete_remove, const bool periodic_add, const bool periodic_increase_stack, const bool timed_remove) {
      container = (raw * EFFECT_TYPE_RAW) |
                  (add * EFFECT_TYPE_ADD) |
                  (remove * EFFECT_TYPE_REMOVE) |
                  (timer_reset * EFFECT_TYPE_RESET_TIMER) |
                  (easy_stack * EFFECT_TYPE_EASY_STACK) | 
                  (unique * EFFECT_TYPE_UNIQUE) | 
                  (update * EFFECT_TYPE_UPDATE) |
                  (complete_remove * EFFECT_TYPE_COMPLETE_REMOVE) |
                  (periodic_add * EFFECT_TYPE_PERIODIC_ADD) |
                  (periodic_increase_stack * EFFECT_TYPE_PERIODIC_INCREASE_STACK) |
                  (timed_remove * EFFECT_TYPE_TIMED_REMOVE);
    }

    bool effect_t::type::raw() const {
      return (container & EFFECT_TYPE_RAW) == EFFECT_TYPE_RAW;
    }

    bool effect_t::type::add() const {
      return (container & EFFECT_TYPE_ADD) == EFFECT_TYPE_ADD;
    }

    bool effect_t::type::remove() const {
      return (container & EFFECT_TYPE_REMOVE) == EFFECT_TYPE_REMOVE;
    }

    bool effect_t::type::timer_reset() const {
      return (container & EFFECT_TYPE_RESET_TIMER) == EFFECT_TYPE_RESET_TIMER;
    }

    bool effect_t::type::stackable() const {
      return (container & EFFECT_TYPE_EASY_STACK) == EFFECT_TYPE_EASY_STACK;
    }
    
    bool effect_t::type::unique() const {
      return (container & EFFECT_TYPE_UNIQUE) == EFFECT_TYPE_UNIQUE;
    }
    
    bool effect_t::type::update() const {
      return (container & EFFECT_TYPE_UPDATE) == EFFECT_TYPE_UPDATE;
    }
    
    bool effect_t::type::complete_remove() const {
      return (container & EFFECT_TYPE_COMPLETE_REMOVE) == EFFECT_TYPE_COMPLETE_REMOVE;
    }
    
    bool effect_t::type::periodic_add() const {
      return (container & EFFECT_TYPE_PERIODIC_ADD) == EFFECT_TYPE_PERIODIC_ADD;
    }
    
    bool effect_t::type::periodic_increase_stack() const {
      return (container & EFFECT_TYPE_PERIODIC_INCREASE_STACK) == EFFECT_TYPE_PERIODIC_INCREASE_STACK;
    }
    
    bool effect_t::type::timed_remove() const {
      return (container & EFFECT_TYPE_TIMED_REMOVE) == EFFECT_TYPE_TIMED_REMOVE;
    }
  }
}
