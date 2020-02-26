#ifndef RANDOM_H
#define RANDOM_H

#include <cstdint>
#include <mutex>

namespace devils_engine {
  namespace utils {
    class random {
    public:
      static const uint32_t CMWC_CYCLE = 4096; // as Marsaglia recommends
      static const uint32_t CMWC_C_MAX = 809430660; // as Marsaglia recommends
      static const uint32_t default_seed = 67871251;
      
      struct xorwow_state {
        uint32_t a, b, c, d;
        uint32_t counter;
      };
      
      // примерно в 2 раза лучше xorwow_state (то есть булевы значения генерит с отклонением в 2 раза меньше)
      // но при этом гпл код, да и врядли сильно сыграет это отклонение
      struct msws_state {
        uint64_t x, w, s;
      };
      
      random();
      random(const uint32_t &seed);
      
      uint32_t get();
      float norm();
      
       int32_t rangei( int32_t min,  int32_t max);  // [min, max]
      uint32_t rangeu(uint32_t min, uint32_t max);
      float    rangef(   float min,    float max);
      
      float   gaussianf(   float mean,    float std_deviation);
      int32_t gaussiani( int32_t mean,  int32_t std_deviation);
      
       int32_t gaussian_rangei( int32_t min,  int32_t max);
      uint32_t gaussian_rangeu(uint32_t min, uint32_t max);
      float    gaussian_rangef(   float min,    float max);
      
      uint32_t dice(const uint32_t &rolls, const uint32_t &faces, const float &add = 0.0f, const float &mul = 1.0f);
      
      bool chanse(const float &chansePersent);
      uint32_t seed() const;
    private:
//       uint32_t Q[CMWC_CYCLE];
//       uint32_t c; // must be limited with CMWC_C_MAX
//       uint32_t i;
      uint32_t seed_m;
      xorwow_state state;
      std::mutex mutex;
//       msws_state state;
    };
  }
}

#endif
