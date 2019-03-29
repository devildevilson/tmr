#include "TimeMeter.h"

TimeMeter::TimeMeter() {
  endingPoint = std::chrono::steady_clock::now();
}

TimeMeter::TimeMeter(uint64_t reportInterval) {
  this->reportInterval = reportInterval;
  endingPoint = std::chrono::steady_clock::now();
}

TimeMeter::~TimeMeter() {}

void TimeMeter::init(uint64_t reportInterval) {
  this->reportInterval = reportInterval;
  endingPoint = std::chrono::steady_clock::now();
}

void TimeMeter::start() {
  startingPoint = std::chrono::steady_clock::now();
  
  auto elapsed = startingPoint - endingPoint;
  sleepDuration = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
  
  timeStorage += sleepDuration;
}

void TimeMeter::stop() {
  endingPoint = std::chrono::steady_clock::now();
  frameCount++;
  
  auto elapsed = endingPoint - startingPoint;
  frameDuration = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
  
  timeStorage += frameDuration;
  frameTimeStorage += frameDuration;
  
  if (timeStorage > reportInterval) {
    fps = (float)(frameCount * MICRO) / (float)timeStorage;
    lastFrameCount = frameCount;
    lastReportTime = frameDuration;
    lastFrameTimeStorage = frameTimeStorage;
    lastSleepTime = sleepDuration;
    
    frameCount = 0;
    timeStorage = 0;
    frameTimeStorage = 0;
  }
}

uint64_t TimeMeter::getTime() const {
  return frameDuration + sleepDuration;
}

uint64_t TimeMeter::getReportTime(TimePrecise tp) const {
  if (tp == MILLISECONDS) return lastReportTime/MILLI;
  if (tp == SECONDS) return lastReportTime/MICRO;
  
  return lastReportTime;
}

uint64_t TimeMeter::getSleepTime(TimePrecise tp) const {
  if (tp == MILLISECONDS) return lastSleepTime/MILLI;
  if (tp == SECONDS) return lastSleepTime/MICRO;
  
  return lastSleepTime;
}

uint64_t TimeMeter::getStartStopTime() const {
  return frameDuration;
}

uint64_t TimeMeter::getStopStartTime() const {
  return sleepDuration;
}

uint32_t TimeMeter::getFramesCount() const {
  return lastFrameCount;
}

uint64_t TimeMeter::getIntervalFramesTime(TimePrecise tp) const {
  if (tp == MILLISECONDS) return lastFrameTimeStorage/MILLI;
  if (tp == SECONDS) return lastFrameTimeStorage/MICRO;
  
  return lastFrameTimeStorage;
}

float TimeMeter::getFPS() const {
  return fps;
}

std::chrono::steady_clock::time_point TimeMeter::getStart() const {
  return startingPoint;
}

std::chrono::steady_clock::time_point TimeMeter::getStop() const {
  return endingPoint;
}
