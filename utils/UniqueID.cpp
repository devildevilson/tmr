#include "UniqueID.h"

UniqueID::UniqueID() {
  if (newId == SIZE_MAX) throw std::runtime_error("last unique value");
  data = newId;
  ++newId;
}

UniqueID::UniqueID(const UniqueID &another) : data(another.data) {}

size_t UniqueID::id() const {
  return data;
}

UniqueID & UniqueID::operator=(const UniqueID &another) {
  data = another.data;
  return *this;
}

bool UniqueID::operator==(const UniqueID &another) const {
  return data == another.data;
}

bool UniqueID::operator!=(const UniqueID &another) const {
  return data != another.data;
}

size_t UniqueID::newId = 0;
