#include "Event.h"

EventP::EventP() {}

EventP::EventP(const EventP &other) {
  this->pos = other.pos;
  this->event = other.event;
}

EventP::EventP(const size_t &e, const size_t &p) {
  this->pos = p;
  this->event = e;
}

void EventP::operator=(const EventP &other) {
  this->pos = other.pos;
  this->event = other.event;
}

EventsContainer::FunctionId::FunctionId() {}

EventsContainer::FunctionId::FunctionId(const bool &deleteFunction, const uint64_t &id, const std::function<bool(const EventData1 &ev)> &f) {
  this->deleteFunction = deleteFunction;
  this->id = id;
  this->f = f;
}

EventsContainer::EventsContainer() {}
EventsContainer::~EventsContainer() {}

EventP EventsContainer::addListener(const size_t &eventType, const std::function<bool(const EventData1 &ev)> &f) {
  events[eventType].emplace_back(false, newId, f);
  size_t pos = newId;
  newId++;
  
  return {eventType, pos};
}

EventP EventsContainer::addListener(const Type &eventType, const std::function<bool(const EventData1 &ev)> &f) {
  events[eventType.getType()].emplace_back(false, newId, f);
  size_t pos = newId;
  newId++;
  
  return {eventType.getType(), pos};
}

void EventsContainer::removeListener(const EventP &pointer) {
  auto itr = events.find(pointer.event);
  if (itr == events.end()) return;
  
  auto &vec = itr->second;
  
  for (uint64_t i = 0; i < vec.size(); ++i) {
    if (vec[i].id == pointer.pos) {
      if (i != vec.size()-1) {
        std::swap(vec[i], vec[vec.size()-1]);
      }
      
      vec.pop_back();
      break;
    }
  }
}

void EventsContainer::removeEvent(const size_t &eventType) {
  events.erase(eventType);
}

void EventsContainer::removeEvent(const Type &eventType) {
  events.erase(eventType.getType());
}

// УДАЛЯТЬ ФУНКЦИИ ПО ВОЗВРАЩЕНИЮ true ЛУЧШАЯ ИДЕЯ

void EventsContainer::fireEvent(const size_t &eventType) {
  auto itr = events.find(eventType);
  if (itr == events.end()) return;
  
  auto &vec = itr->second;
  
  EventData1 dummyData;
  
  for (int64_t i = vec.size()-1; i >= 0; --i) {
    if (vec[i].f(dummyData)) {
      if (i != vec.size()-1) vec[i] = vec[vec.size()-1];
      
      vec.pop_back();
    }
  }
}

void EventsContainer::fireEvent(const size_t &eventType, const EventData1 &data) {
  auto itr = events.find(eventType);
  if (itr == events.end()) return;
  
  auto &vec = itr->second;
  
  for (int64_t i = vec.size()-1; i >= 0; --i) {
    if (vec[i].f(data)) {
      if (i != vec.size()-1) vec[i] = vec[vec.size()-1];
      
      vec.pop_back();
    }
  }
}

void EventsContainer::fireEvent(const Type &eventType) {
  auto itr = events.find(eventType.getType());
  if (itr == events.end()) return;
  
  auto &vec = itr->second;
  
  EventData1 dummyData;
  
  for (int64_t i = vec.size()-1; i >= 0; --i) {
    if (vec[i].f(dummyData)) {
      if (i != vec.size()-1) vec[i] = vec[vec.size()-1];
      
      vec.pop_back();
    }
  }
}

void EventsContainer::fireEvent(const Type &eventType, const EventData1 &data) {
  auto itr = events.find(eventType.getType());
  if (itr == events.end()) return;
  
  auto &vec = itr->second;
  
  for (int64_t i = vec.size()-1; i >= 0; --i) {
    if (vec[i].f(data)) {
      if (i != vec.size()-1) vec[i] = vec[vec.size()-1];
      
      vec.pop_back();
    }
  }
}

uint64_t EventsContainer::newId = 0;
