#ifndef UNIQUE_ID_H
#define UNIQUE_ID_H

#include <functional>
#include <atomic>

class UniqueID {
public:
  UniqueID();
  UniqueID(const UniqueID &another);
  
  size_t id() const;
  
  UniqueID & operator=(const UniqueID &another);
  bool operator==(const UniqueID &another) const;
  bool operator!=(const UniqueID &another) const;
private:
  size_t data;
  static size_t newId; // атомарная операция?
};

namespace std {
  template<>
  struct hash<UniqueID> {
    size_t operator()(const UniqueID &obj) const {
      return hash<size_t>()(obj.id());
    }
  };
}

#endif
