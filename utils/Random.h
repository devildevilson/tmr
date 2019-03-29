#ifndef RANDOM_H
#define RANDOM_H

#include <cstdint>

const uint32_t CMWC_CYCLE = 4096; // as Marsaglia recommends
const uint32_t CMWC_C_MAX = 809430660; // as Marsaglia recommends

class Random {
public:
  Random();
  Random(const uint64_t &seed);
  
  uint32_t get();
  float norm();
  
   int32_t range ( int32_t min,  int32_t max);  // [min, max)
  uint32_t rangeU(uint32_t min, uint32_t max);
  float    rangeF(   float min,    float max);
  
  float   gaussianF(   float mean,    float std_deviation);
  int32_t gaussian ( int32_t mean,  int32_t std_deviation);
  
   int32_t gaussianRange (  int32_t min,   int32_t max);
  uint32_t gaussianRangeU( uint32_t min,  uint32_t max);
  float    gaussianRangeF(    float min,     float max);
  
  uint32_t dice(const uint32_t &rolls, const uint32_t &faces, const float &add = 0.0f, const float &mul = 1.0f);
  
  bool chanse(const float &chansePersent);
private:
  uint32_t Q[CMWC_CYCLE];
  uint32_t c; // must be limited with CMWC_C_MAX
  uint32_t i;
};

#endif
