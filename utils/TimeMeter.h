#ifndef TIMEMETER_H
#define TIMEMETER_H

#include <chrono>
#include "shared_time_constant.h"

struct frame_time {
  std::chrono::steady_clock::time_point point;
  size_t time;
  
  frame_time();
  void next_frame();
};

class TimeMeter {
public:
//   TimeMeter();
  TimeMeter(const size_t &reportInterval = 0);
  ~TimeMeter();
  
//   void init(size_t reportInterval = 0);
  
  void start();
  void stop();
  
  size_t time() const; // frameTime() + sleepTime()
  
  size_t frameTime() const;
  size_t sleepTime() const;
  
  size_t lastFrameTime() const;
  size_t lastSleepTime() const;
  
  // может быть понядобится третий кадр
  
  size_t accumulatedFrameTime() const;
  size_t accumulatedSleepTime() const;
  
  size_t lastIntervalFrameTime() const;
  size_t lastIntervalSleepTime() const;
  
  uint32_t framesCount() const;
  float fps() const;
  
  std::chrono::steady_clock::time_point getStart() const;
  std::chrono::steady_clock::time_point getStop() const;
private:
  std::chrono::steady_clock::time_point startingPoint;
  std::chrono::steady_clock::time_point endingPoint;
  
  float fpsVar;
  uint32_t frameCount;
  size_t frameDuration; // между start и end
  size_t sleepDuration; // между end и start
  
  size_t timeStorage;
  
  size_t frameTimeStorage;
  size_t sleepTimeStorage;
  
  size_t lastFrameTimeStorage;
  size_t lastSleepTimeStorage;
  
  size_t lastIntervalFrameTimeStorage;
  size_t lastIntervalSleepTimeStorage;
  
  size_t lastReportTime;
  size_t lastSleepTimeVar;
  uint32_t lastFrameCount;
  
  size_t reportInterval;
};

#endif
