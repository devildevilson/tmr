#ifndef ENTITY_CREATOR_RESOURCES_H
#define ENTITY_CREATOR_RESOURCES_H

#include "id.h"
#include "resource_container.h"
#include "entity_creator.h"

namespace devils_engine {
  namespace game {
    using entity_creators_container = const utils::resource_container_array<utils::id, core::entity_creator, 30>;
    using entity_creators_container_load = utils::resource_container_array<utils::id, core::entity_creator, 30>;
  }
}

#endif
