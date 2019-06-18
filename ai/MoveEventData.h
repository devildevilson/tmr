#ifndef MOVE_EVENT_DATA_H
#define MOVE_EVENT_DATA_H

enum class MoveEvent {
  point,
  direction
};

struct MoveEventData {
  MoveEvent type;
  float arr[4];
};

#endif
