#ifndef TRANSITION_H
#define TRANSITION_H

#include <cstddef>
#include <algorithm>

template <typename T>
class Transition {
public:
  struct CreateInfo {
    T startValue;
    T endValue;
    size_t transitionTime;
  };
  Transition(const CreateInfo &info) : startValue(info.startValue), endValue(info.endValue), transitionTime(info.transitionTime), currentTime(0) {}

  void update(const size_t &time) { currentTime += time; }
  void loop() { if (currentTime >= transitionTime) currentTime -= transitionTime; }
  bool finish() const { return currentTime >= transitionTime; }
  void swapValues() { std::swap(startValue, endValue); }

  T value() const {
    const float trans = std::min(float(currentTime) / float(transitionTime), 1.0f);
    return startValue + (endValue - startValue) * trans;
  }

  T start() const { return startValue; }
  T end() const { return endValue; }
  size_t transition() const { return transitionTime; }
  size_t current() const { return currentTime; }
private:
  T startValue;
  T endValue;
  size_t transitionTime;
  size_t currentTime;
};

#endif //TRANSITION_H
