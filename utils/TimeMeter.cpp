#include "TimeMeter.h"

frame_time::frame_time() : point(std::chrono::steady_clock::now()), time(0) {}

void frame_time::next_frame() {
  const auto old_point = point;
  point = std::chrono::steady_clock::now();
  
  const auto elapsed = point - old_point;
  time = std::chrono::duration_cast<CHRONO_TIME_TYPE>(elapsed).count();
}

TimeMeter::TimeMeter(const size_t &reportInterval)
  : startingPoint(std::chrono::steady_clock::now()),
    endingPoint(std::chrono::steady_clock::now()),
    fpsVar(0.0f), 
    frameCount(0), 
    frameDuration(0), 
    sleepDuration(0), 
    timeStorage(0), 
    frameTimeStorage(0), 
    sleepTimeStorage(0), 
    lastFrameTimeStorage(0), 
    lastSleepTimeStorage(0),
    lastIntervalFrameTimeStorage(0),
    lastIntervalSleepTimeStorage(0),
    lastReportTime(1), 
    lastSleepTimeVar(1), 
    lastFrameCount(1), 
    reportInterval(reportInterval) {}

TimeMeter::~TimeMeter() {}

void TimeMeter::start() {
  startingPoint = std::chrono::steady_clock::now();
  
  lastSleepTimeVar = sleepDuration;
  const auto elapsed = startingPoint - endingPoint;
  sleepDuration = std::chrono::duration_cast<CHRONO_TIME_TYPE>(elapsed).count();
  
  timeStorage += sleepDuration;
  sleepTimeStorage += sleepDuration;
}

void TimeMeter::stop() {
  endingPoint = std::chrono::steady_clock::now();
  ++frameCount;
  
  lastReportTime = frameDuration;
  const auto elapsed = endingPoint - startingPoint;
  frameDuration = std::chrono::duration_cast<CHRONO_TIME_TYPE>(elapsed).count();
  
  timeStorage += frameDuration;
  frameTimeStorage += frameDuration;
  
  if (timeStorage > reportInterval) {
    fpsVar = float(frameCount * TIME_PRECISION) / float(timeStorage);
    lastFrameCount = frameCount;
    lastFrameTimeStorage = frameTimeStorage;
    lastSleepTimeStorage = sleepTimeStorage;
    lastIntervalFrameTimeStorage = frameDuration;
    lastIntervalSleepTimeStorage = sleepDuration;
    
    frameCount = 0;
    timeStorage = 0;
    frameTimeStorage = 0;
    sleepTimeStorage = 0;
  }
}

size_t TimeMeter::time() const {
  return frameTime() + sleepTime();
}

size_t TimeMeter::frameTime() const {
  return frameDuration;
}

size_t TimeMeter::sleepTime() const {
  return sleepDuration;
}

size_t TimeMeter::lastFrameTime() const {
  return lastReportTime;
}

size_t TimeMeter::lastSleepTime() const {
  return lastSleepTimeVar;
}

uint32_t TimeMeter::framesCount() const {
  return lastFrameCount;
}

size_t TimeMeter::accumulatedFrameTime() const {
  return lastFrameTimeStorage;
}

size_t TimeMeter::accumulatedSleepTime() const {
  return lastSleepTimeStorage;
}

size_t TimeMeter::lastIntervalFrameTime() const {
  return lastIntervalFrameTimeStorage;
}

size_t TimeMeter::lastIntervalSleepTime() const {
  return lastIntervalSleepTimeStorage;
}

float TimeMeter::fps() const {
  return fpsVar;
}

std::chrono::steady_clock::time_point TimeMeter::getStart() const {
  return startingPoint;
}

std::chrono::steady_clock::time_point TimeMeter::getStop() const {
  return endingPoint;
}
