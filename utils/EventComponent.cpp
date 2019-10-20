#include "EventComponent.h"

BasicEventFunctor::BasicEventFunctor(const CreateInfo &info) : func(info.func) {}
event BasicEventFunctor::call(const Type &type, const EventData &data, yacs::entity* entity) {
  (void)entity;
  return func(type, data);
}

EventComponent::EventComponent(const CreateInfo &info) : entity(info.entity) {}
EventComponent::~EventComponent() {}

//void EventComponent::update(const uint64_t &time)  { (void)time; }
//void EventComponent::init(void* userData) { (void)userData; }

void EventComponent::registerEvent(const Type &type, EventFunctor* func) {
  //events[type].push_back(func);
  events.push_back(func);
}

bool EventComponent::hasEvent(const Type &type) const {
//  auto itr = events.find(type);
//  if (itr == events.end()) return false;
//
//  return !itr->second.empty();

//  for () {
//
//  }

  return false;
}

event EventComponent::fireEvent(const Type &type, const EventData &data) {
//  auto itr = events.find(type);
//  if (itr == events.end()) return failure;
//
//  event val = success;
//
//  for (size_t i = 0; i < itr->second.size(); ++i) {
//    const event ret = itr->second[i]->call(type, data, entity);
//
//    if ((ret & can_be_deleted) == can_be_deleted) {
//      std::swap(itr->second[i], itr->second.back());
//      itr->second.pop_back();
//      --i;
//    }
//
//    const uint32_t mask = ~static_cast<uint32_t>(can_be_deleted);
//    const event tmp = static_cast<event>(mask & ret);
//    val = std::max(val, tmp);
//  }
//
//  if (itr->second.empty()) events.erase(itr);
//
//  return val;

  event val = success;

  for (size_t i = 0; i < events.size(); ++i) {
    const event ret = events[i]->call(type, data, entity);

    if ((ret & can_be_deleted) == can_be_deleted) {
      std::swap(events[i], events.back());
      events.pop_back();
      --i;
    }

    const uint32_t mask = ~static_cast<uint32_t>(can_be_deleted);
    const event tmp = static_cast<event>(mask & ret);
    val = std::max(val, tmp);
  }

  return val;
}

void EventComponent::registerEvent_save(const Type &type, EventFunctor* func) {
  std::unique_lock<std::mutex> lock(local_mutex);
//  events[type].push_back(func);
  events.push_back(func);
}

bool EventComponent::hasEvent_save(const Type &type) const {
  std::unique_lock<std::mutex> lock(local_mutex);
//  auto itr = events.find(type);
//  if (itr == events.end()) return false;
//
//  return !itr->second.empty();
  return false;
}

event EventComponent::fireEvent_save(const Type &type, const EventData &data) {
  std::unique_lock<std::mutex> lock(local_mutex);
////   {
////     std::unique_lock<std::mutex> lock(local_mutex);
//    auto itr = events.find(type);
//    if (itr == events.end()) return failure;
////   }
//
//  event val = success;
//
//  static const uint32_t mask = ~static_cast<uint32_t>(can_be_deleted);
//
////   {
////     std::unique_lock<std::mutex> lock(local_mutex);
//
//    for (size_t i = 0; i < itr->second.size(); ++i) {
//      const event ret = itr->second[i]->call(type, data, entity);
//
//      if ((ret & can_be_deleted) == can_be_deleted) {
//        std::swap(itr->second[i], itr->second.back());
//        itr->second.pop_back();
//        --i;
//      }
//
//      const event tmp = static_cast<event>(mask & ret);
//      val = std::max(val, tmp);
//    }
//
//    if (itr->second.empty()) events.erase(itr);
////   }
//
//  return val;

  event val = success;

  for (size_t i = 0; i < events.size(); ++i) {
    const event ret = events[i]->call(type, data, entity);

    if ((ret & can_be_deleted) == can_be_deleted) {
      std::swap(events[i], events.back());
      events.pop_back();
      --i;
    }

    const uint32_t mask = ~static_cast<uint32_t>(can_be_deleted);
    const event tmp = static_cast<event>(mask & ret);
    val = std::max(val, tmp);
  }

  return val;
}
