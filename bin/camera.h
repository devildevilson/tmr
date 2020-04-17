#ifndef CAMERA_H
#define CAMERA_H

namespace yacs {
  class entity;
}

namespace devils_engine {
  namespace camera {
    void first_person(const yacs::entity* ent, const bool menu_is_open);
  }
}

#endif
