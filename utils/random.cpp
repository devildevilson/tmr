#include "random.h"

#include <cstdlib>
#include <cmath>
#include <algorithm>

const float rand_divf = 1.0f / float(INT32_MAX);
const float urand_divf = 1.0f / float(UINT32_MAX); // проблема наверное здесь, float нехватает точности чтобы положить UINT32_MAX
const double rand_div = 1.0 / double(INT32_MAX);
const double urand_div = 1.0 / double(UINT32_MAX); // проблема наверное здесь, float нехватает точности чтобы положить UINT32_MAX

const uint64_t sconst[] = {
  0x8b5ad4ce914ecdf7, 0xdbc8915f4b1cd961, 0x3a16e0c51fa593d9, 0x1794da529ec6d70b,
  0x8fc49b2a752f643b, 0xde07a518fba03571, 0xb1d2e4762d58906b, 0x478f6219da719b05,
  0x41857dc34a2fdc05, 0xb9425ed8e351a06f, 0x9235eb64c35eab7d, 0x91f0e7b8e0536af7,
  0x4f0581abb194f75b, 0xdab4e53c95408d1f, 0xf23ba0c5410ceb3b, 0x912a0b4ce102a36d,
  0x92a73b40b46a2e71, 0x46ca273b5fde168d, 0xf9b8ad61743910b5, 0x490ceb3d865e4bc9,
  0xa12e0dcfbf6471cf, 0xa54c91db6dc0fe37, 0x08c3564a5c031727, 0xe3296d17c14795bd,
  0x5387014db793f24f, 0x6d47af052931fe47, 0xd138c9ef735c0e8f, 0xa790fbc8ebf02d3b,
  0x4a1b027867c953fb, 0x49a180de9567182d, 
};

template <typename T>
T clamp(const T& n, const T& lower, const T& upper) {
  return std::max(lower, std::min(n, upper));
}

uint32_t rand32() {
  uint32_t result = rand();
  return result << 16 | rand();
}

uint32_t urand() {
  uint32_t a = rand();
  uint16_t b = rand();
  return (a << 16) | uint32_t(b);
};

struct xorshift32_state {
  uint32_t a;
};

/* The state word must be initialized to non-zero */
uint32_t xorshift32(xorshift32_state &state) {
  /* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
  uint32_t x = state.a;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  return state.a = x;
}

namespace devils_engine {
  namespace utils {
    /* The state array must be initialized to not be all zero in the first four words */
    uint32_t xorwow(random::xorwow_state &state) {
      /* Algorithm "xorwow" from p. 5 of Marsaglia, "Xorshift RNGs" */
      uint32_t t = state.d;

      const uint32_t s = state.a;
      state.d = state.c;
      state.c = state.b;
      state.b = s;

      t ^= t >> 2;
      t ^= t << 1;
      t ^= s ^ (s << 4);
      state.a = t;

      state.counter += 362437;
      return t + state.counter;
    }
    
    uint32_t msws(random::msws_state &state) {
      state.x *= state.x; 
      state.x += (state.w += state.s); 
      return state.x = (state.x>>32) | (state.x<<32);
    }
    
    uint64_t init_msws(const uint64_t &n) {
      uint32_t a, c, i, j, k, m;
      uint64_t r, t, u;

      /* initialize state for local msws rng */
      random::msws_state local_state;

      r = n / 100000000;
      t = n % 100000000; 
      local_state.s = sconst[r%30];
      r /= 30;
      local_state.x = local_state.w = t*local_state.s + r*local_state.s*100000000;

      /* get 8 different random digits */

      for (m = 0, a = 0, c = 0; m < 32;) {
          j = msws(local_state);       /* get 32-bit random word */
          for (i = 0; i < 32; i+=4) {
            k = (j >> i) & 0xf;        /* get a nibble */
            if ((c & (1 << k)) == 0) { /* verify not used previously */
                c |= (1 << k);
                a |= (k << m);         /* add nibble to output */
                m += 4;
                if (m >= 32) break;
            }
          } 
      }

      u = a; u <<= 32;                 /* save in upper 32 bits */

      /* get 8 different random digits */

      for (m = 0, a = 0, c = 0; m < 32;) {
          j = msws(local_state);       /* get 32-bit random word */
          for (i = 0; i < 32; i += 4) {
            k = (j >> i) & 0xf;        /* get a nibble */
            if ((c & (1 << k)) == 0) { /* verify not used previously */
                c |= (1 << k);
                a |= (k << m);         /* add nibble to output */
                m += 4;
                if (m >= 32) break;
            }
          } 
      }

      return u | a | 1;              /* set least significant bit to 1 */
    }
    
    random::random() : seed_m(default_seed) {
      srand(seed_m);
      
      state.a = urand();
      state.b = urand();
      state.c = urand(); 
      state.d = urand();
      state.counter = 0;
      
//       state.x = state.w = state.s = init_msws(seed_m);
      
//       for (uint32_t i = 0; i < CMWC_CYCLE; ++i) {
//         Q[i] = rand32();
//       }
//       
//       do {
//         c = rand32();
//       } while (c >= CMWC_C_MAX);
//       
//       i = CMWC_CYCLE - 1;
    }
    
    random::random(const uint32_t &seed) : seed_m(seed) {
      srand(seed_m);
  
      state.a = urand();
      state.b = urand();
      state.c = urand(); 
      state.d = urand();
      state.counter = 0;
      
//       state.x = state.w = state.s = init_msws(seed_m);
      
//       for (uint32_t i = 0; i < CMWC_CYCLE; ++i) {
//         Q[i] = rand32();
//       }
//       
//       do {
//         c = rand32();
//       } while (c >= CMWC_C_MAX);
//       
//       i = CMWC_CYCLE - 1;
    }
    
    uint32_t random::get() {
      std::unique_lock<std::mutex> lock(mutex);
//       static const uint64_t a = 18782; // as Marsaglia recommends
//       static const uint32_t m = 0xfffffffe; // as Marsaglia recommends
//       uint64_t t;
//       uint32_t x;
// 
//       i = (i + 1) & (CMWC_CYCLE - 1);
//       t = a * Q[i] + c;
//       
//       /* Let c = t / 0xfffffff, x = t mod 0xffffffff */
//       
//       c = t >> 32;
//       x = t + c;
//       
//       if (x < c) {
//         x++;
//         c++;
//       }
//       
//       Q[i] = m - x;
//       
//       return Q[i];
      return xorwow(state);
//       return msws(state);
    }
    
    float random::norm() {
      return float(get()) * urand_div;
      //return float(get()) / float(UINT32_MAX);
      //return float(get()) / double(UINT32_MAX));
    }
    
    // для исключения max можно использовать float генерацию и округлять к нижнему значению
    int32_t random::rangei(int32_t min, int32_t max) {
      if (min == max) return min;
      else if (min > max) std::swap(min, max);
      
      const int32_t delta = max - min + 1;
      
      return get() % delta + min;
    }
      
    uint32_t random::rangeu(uint32_t min, uint32_t max) {
      if (min == max) return min;
      else if (min > max) std::swap(min, max);
      
      const uint32_t delta = max - min + 1;
      
      return get() % delta + min;
    }
    
    float random::rangef(float min, float max) {
      if (min == max) return min;
      else if (min > max) std::swap(min, max);
      
      const float delta = max - min + 1;
      
      return norm() * delta + min;
    }
    
    float random::gaussianf(float mean, float std_deviation) {
      float x1, x2, w, y1;
      static float y2;
      static bool again = false;
      float ret;
      
      if (again) ret = mean + y2 * std_deviation;
      else {
        do {
          x1 = norm() * 2.0f - 1.0f;
          x2 = norm() * 2.0f - 1.0f;
          w = x1 * x1 + x2 * x2;
        } while (w >= 1.0f);
        
        w = sqrt((-2.0f * log(w)) / w);
        y1 = x1 * w;
        y2 = x2 * w;
        ret = mean + y1 * std_deviation;
      }
      
      again = !again;
      return ret;
    }
    
    int32_t random::gaussiani(int32_t mean,  int32_t std_deviation) {
      float num = gaussianf((float)mean, (float)std_deviation);
      return (num >= 0.0f ? (int32_t)(num + 0.5f) : (int32_t)(num - 0.5f));
    }
    
    int32_t random::gaussian_rangei( int32_t min,  int32_t max)  {
      return (int32_t)gaussian_rangef((float)min, (float)max);
    }
    
    uint32_t random::gaussian_rangeu(uint32_t min, uint32_t max)  {
      return (int32_t)gaussian_rangef((float)min, (float)max);
    }
    
    float random::gaussian_rangef(float min, float max) {
      if (min > max) std::swap(min, max);
  
      float mean = (min + max) / 2.0f;
      float std_deviation = (max - min) / 6.0f;
      
      float ret = gaussianf(mean, std_deviation);
      
      return clamp(ret, min, max);
    }
    
    uint32_t random::dice(const uint32_t &rolls, const uint32_t &faces, const float &add, const float &mul) {
      uint32_t result = 0;
      for (uint32_t counter = 0; counter < rolls; ++counter) {
        result += rangeu(1, faces);
      }
      
      result = (result + add) * mul;
      
      return result;
    }
    
    bool random::chanse(const float &chansePersent) {
//       if (chansePersent >= 1.0f) return true;
//       float rnd = norm();
//       return rnd < chansePersent;
      const uint32_t num = get();
      return num < chansePersent * UINT32_MAX;
    }
    
    uint32_t random::seed() const {
      return seed_m;
    }
  }
}
