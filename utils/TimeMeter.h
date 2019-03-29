#ifndef TIMEMETER_H
#define TIMEMETER_H

#include <chrono>

#define MICRO 1000000
#define MILLI 1000

#define ONE_FRAME_TIME_SEC 0.01666667f
#define ONE_FRAME_TIME_MILLISEC 16.66666667f
#define ONE_FRAME_TIME_MICROSEC 16666.66667f
#define FRAME_INT 16667

enum TimePrecise : uint8_t {
  MICROSECONDS = 0,
  MILLISECONDS = 1,
  SECONDS      = 2
};

class TimeMeter {
public:
  TimeMeter();
  TimeMeter(uint64_t reportInterval = 0); // обновлять вывод только после определенного промежутка времени
  ~TimeMeter();
  
  void init(uint64_t reportInterval = 0);
  
  void start();
  void stop();
  
  uint64_t getTime() const; 
  uint64_t getReportTime(TimePrecise tp = MICROSECONDS) const;
  uint64_t getSleepTime(TimePrecise tp = MICROSECONDS) const;
  uint64_t getStartStopTime() const;
  uint64_t getStopStartTime() const;
  // названия конечно полное дерьмище
  uint64_t getIntervalFramesTime(TimePrecise tp = MICROSECONDS) const; // возвращает время без учета времени между stop и start (без времени сна)
  
  uint32_t getFramesCount() const;
  float getFPS() const;
  
  std::chrono::steady_clock::time_point getStart() const;
  std::chrono::steady_clock::time_point getStop() const;
private:
  std::chrono::steady_clock::time_point startingPoint;
  std::chrono::steady_clock::time_point endingPoint;
  
  float fps = 0.0f;
  uint32_t frameCount = 0;
  uint64_t frameDuration = 0; // между start и end
  uint64_t sleepDuration = 0; // между end и start
  
  uint64_t timeStorage = 0;
  uint64_t frameTimeStorage = 0;
  
  uint64_t lastReportTime = 1;
  uint64_t lastSleepTime = 1;
  uint32_t lastFrameCount = 1;
  uint64_t lastFrameTimeStorage = 1;
  
  uint64_t reportInterval = 0;
};

#endif
