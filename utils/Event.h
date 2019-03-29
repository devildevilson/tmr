#ifndef EVENT_H
#define EVENT_H

#include <string>
#include <unordered_map>
#include <map>
#include <list>
#include <vector>
#include <functional>

#include "Type.h"

#define EVENT_UNDEFINED 0

// struct Event {
//   uint64_t type = 0;
//   std::string str;
//   //int buttonData = 0;
//   
//   void* userData = nullptr;
//   
//   bool operator==(const Event &other);
// };
// 
// namespace std {
//   template<>
//   struct hash<Event> {
//     size_t operator() (const Event &e) {
//       return hash<uint64_t>()(e.type);
//     }
//   };
// }
// 
// class EventSubscriber {
// public:
//   EventSubscriber();
//   EventSubscriber(const std::function<bool(void* relatedObject, const Event &event)> &f, void* relatedObject);
//   ~EventSubscriber();
//   
//   void call(const Event &event);
//   void mute();
//   void unMute();
//   bool isMuted();
//   bool isValid();
//   
//   bool operator==(const EventSubscriber &other);
// protected:
//   bool muted = false;
//   std::function<bool(void* relatedObject, const Event &event)> subscriberFunc;
//   void* relatedObject = nullptr;
// };
// 
// class EventsContainer {
// public:
//   uint64_t addEvent(const std::string &name);
//   Event getEvent(const uint64_t &eventType);
//   bool subscribeEvent(const uint64_t &eventType, const EventSubscriber &sub);
//   bool subscribeEvent(const Event &event, const EventSubscriber &sub);
//   bool fireEvent(const uint64_t &eventType, void* userData = nullptr);
//   bool fireEvent(const Event &event, void* userData = nullptr);
// private:
//   std::unordered_map<Event, std::list<EventSubscriber>> events;
// };

class EventsContainer;

struct EventData1 {
  int64_t int1 = 0;
  int64_t int2 = 0;
  int64_t int3 = 0;
  
  float float1 = 0.0f;
  float float2 = 0.0f;
  float float3 = 0.0f;
  
  void* data1 = nullptr;
  void* data2 = nullptr;
  void* data3 = nullptr;
};

class EventP {
  friend EventsContainer;
public:
  EventP();
  EventP(const EventP &other);
  EventP(const size_t &e, const size_t &p);
  void operator=(const EventP &other);
private:
  size_t pos = 0;
  size_t event;
};

class EventsContainer {
protected:
  struct FunctionId {
    FunctionId();
    FunctionId(const bool &deleteFunction, const uint64_t &id, const std::function<bool(const EventData1 &ev)> &f);
    
    bool deleteFunction = false;
    uint64_t id = 0;
    std::function<bool(const EventData1 &ev)> f;
  };
public:
  EventsContainer();
  virtual ~EventsContainer();
  
  // как удалять функцию? (можно к каждой функции добавлять уникальный айдишник, и по нему искать)
  EventP addListener(const size_t &eventType, const std::function<bool(const EventData1 &ev)> &f);
  EventP addListener(const Type &eventType, const std::function<bool(const EventData1 &ev)> &f);
  void removeListener(const EventP &pointer);
  void removeEvent(const size_t &eventType);
  void removeEvent(const Type &eventType);
  void fireEvent(const size_t &eventType);
  void fireEvent(const size_t &eventType, const EventData1 &data);
  void fireEvent(const Type &eventType);
  void fireEvent(const Type &eventType, const EventData1 &data);
protected:
  std::unordered_map<size_t, std::vector<FunctionId>> events;
  static uint64_t newId;
};

#endif
