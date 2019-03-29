#include "Random.h"

#include <cstdlib>
#include <cmath>
#include <algorithm>

static const float rand_div = 1.0f / (float)(0xffffffff);

template <typename T>
T clamp(const T& n, const T& lower, const T& upper) {
  return std::max(lower, std::min(n, upper));
}

uint32_t rand32() {
  uint32_t result = rand();
  return result << 16 | rand();
}

Random::Random() {
  srand(1);
  for (uint32_t i = 0; i < CMWC_CYCLE; ++i) {
    this->Q[i] = rand32();
  }
  
  do {
    this->c = rand32();
  } while (this->c >= CMWC_C_MAX);
  
  this->i = CMWC_CYCLE - 1;
}

Random::Random(const uint64_t &seed) {
  srand(seed);
  
  for (uint32_t i = 0; i < CMWC_CYCLE; ++i) {
    this->Q[i] = rand32();
  }
  
  do {
    this->c = rand32();
  } while (this->c >= CMWC_C_MAX);
  
  this->i = CMWC_CYCLE - 1;
}

uint32_t Random::get() {
  static const uint64_t a = 18782; // as Marsaglia recommends
  static const uint32_t m = 0xfffffffe; // as Marsaglia recommends
  uint64_t t;
  uint32_t x;

  this->i = (this->i + 1) & (CMWC_CYCLE - 1);
  t = a * this->Q[this->i] + this->c;
  
  /* Let c = t / 0xfffffff, x = t mod 0xffffffff */
  
  this->c = t >> 32;
  x = t + this->c;
  
  if (x < this->c) {
    x++;
    this->c++;
  }
  
  this->Q[this->i] = m - x;
  
  return this->Q[this->i];
}

float Random::norm() {
  return float(get()) * rand_div;
}

int32_t Random::range (int32_t min, int32_t max) {
  if (min == max) return min;
  else if (min > max) std::swap(min, max);
  
  int32_t delta = max - min + 1;
  
  return get() % delta + min;
}

uint32_t Random::rangeU(uint32_t min, uint32_t max) {
  if (min == max) return min;
  else if (min > max) std::swap(min, max);
  
  uint32_t delta = max - min + 1;
  
  return get() % delta + min;
}

float Random::rangeF(float min, float max) {
  if (min == max) return min;
  else if (min > max) std::swap(min, max);
  
  float delta = max - min + 1;
  
  return float(get()) * rand_div * delta + min;
}

float Random::gaussianF(float mean, float std_deviation) {
  float x1, x2, w, y1;
  static float y2;
  static bool again = false;
  float ret;
  
  if (again) ret = mean + y2 * std_deviation;
  else {
    do {
      x1 = float(get()) * rand_div * 2.0f - 1.0f;
      x2 = float(get()) * rand_div * 2.0f - 1.0f;
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

int32_t Random::gaussian (int32_t mean, int32_t std_deviation) {
  float num = gaussianF((float)mean, (float)std_deviation);
  return (num >= 0.0f ? (int32_t)(num + 0.5f) : (int32_t)(num - 0.5f));
}

int32_t Random::gaussianRange (int32_t min, int32_t max) {
  return (int32_t)gaussianRangeF((float)min, (float)max);
}

uint32_t Random::gaussianRangeU(uint32_t min, uint32_t max) {
  return (uint32_t)gaussianRangeF((float)min, (float)max);
}

float Random::gaussianRangeF(float min, float max) {
  if (min > max) std::swap(min, max);
  
  float mean = (min + max) / 2.0f;
  float std_deviation = (max - min) / 6.0f;
  
  float ret = gaussianF(mean, std_deviation);
  
  return clamp(ret, min, max);
}

uint32_t Random::dice(const uint32_t &rolls, const uint32_t &faces, const float &add, const float &mul) {
  uint32_t result = 0;
  
  for (uint32_t counter = 0; counter < rolls; ++counter) {
    result += rangeU(1, faces);
  }
  
  result = (result + add) * mul;
  
  return result;
}

bool Random::chanse(const float &chansePersent) {
  float rnd = float(get()) * rand_div;
  if (rnd < chansePersent) return true;
  
  return false;
}
