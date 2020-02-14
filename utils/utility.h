#ifndef UTILITY_H_NEW
#define UTILITY_H_NEW

namespace devils_engine {
  namespace utils {
    template <typename T, typename K>
    constexpr T min(const T &first, const K &second) {
      return first < second ? first : T(second);
    }
    
    template <typename T, typename K>
    constexpr T max(const T &first, const K &second) {
      return first > second ? first : T(second);
    }
  }
}

#endif
