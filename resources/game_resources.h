#ifdef ABILITIES_CONTAINER
#ifndef ABILITIES_RESOURCES
#define ABILITIES_RESOURCES

#include "ability.h"
#include "id.h"
#include "resource_container.h"

namespace devils_engine {
  namespace game {
    const size_t abilities_container_size = 30;
    using abilities_container = const utils::resource_container_array<utils::id, game::ability_t, abilities_container_size>;
    using abilities_container_load = utils::resource_container_array<utils::id, game::ability_t, abilities_container_size>;
  }
}

#endif
#endif

#ifdef EFFECTS_CONTAINER
#ifndef EFFECTS_RESOURCES
#define EFFECTS_RESOURCES

#include "effect.h"
#include "id.h"
#include "resource_container.h"

namespace devils_engine {
  namespace game {
    const size_t effects_container_size = 30;
    using effects_container = const utils::resource_container_array<utils::id, game::effect_t, effects_container_size>;
    using effects_container_load = utils::resource_container_array<utils::id, game::effect_t, effects_container_size>;
  }
}

#endif
#endif

#ifdef STATES_CONTAINER
#ifndef STATES_RESOURCES
#define STATES_RESOURCES

#include "state.h"
#include "id.h"
#include "resource_container.h"

namespace devils_engine {
  namespace game {
    const size_t states_container_size = 30;
    using states_container = const utils::resource_container_array<utils::id, core::state_t, states_container_size>;
    using states_container_load = utils::resource_container_array<utils::id, core::state_t, states_container_size>;
  }
}

#endif
#endif

#ifdef WEAPONS_CONTAINER
#ifndef WEAPONS_RESOURCES
#define WEAPONS_RESOURCES

#include "weapon.h"
#include "id.h"
#include "resource_container.h"

namespace devils_engine {
  namespace game {
    const size_t weapons_container_size = 30;
    using weapons_container = const utils::resource_container_array<utils::id, game::weapon_t, weapons_container_size>;
    using weapons_container_load = utils::resource_container_array<utils::id, game::weapon_t, weapons_container_size>;
  }
}

#endif
#endif

#ifdef ATTRIBUTE_TYPES_CONTAINER
#ifndef ATTRIBUTE_TYPES
#define ATTRIBUTE_TYPES

#include "attribute.h"
#include "id.h"
#include "resource_container.h"

namespace devils_engine {
  namespace game {
    const size_t float_attribute_types_container_size = 16;
    const size_t int_attribute_types_container_size = 16;
    using float_attribute_types_container = const utils::resource_container_array<utils::id, struct core::attribute_t<core::float_type>::type, float_attribute_types_container_size>;
    using int_attribute_types_container = const utils::resource_container_array<utils::id, struct core::attribute_t<core::int_type>::type, int_attribute_types_container_size>;
    using float_attribute_types_container_load = utils::resource_container_array<utils::id, struct core::attribute_t<core::float_type>::type, float_attribute_types_container_size>;
    using int_attribute_types_container_load = utils::resource_container_array<utils::id, struct core::attribute_t<core::int_type>::type, int_attribute_types_container_size>;
  }
}

#endif
#endif

// #include "ability.h"
// #include "effect.h"
// #include "state.h"
// #include "entity_creator.h"
// #include "weapon.h"
// 
// namespace devils_engine {
//   namespace game {
//     const size_t abilities_container_size = 30;
//     const size_t effects_container_size = 30;
//     const size_t states_container_size = 30;
//     const size_t weapons_container_size = 30;
//     
//     using abilities_container = const utils::resource_container_array<utils::id, game::ability_t, ability_container_size>;
//     using effects_container = const utils::resource_container_array<utils::id, game::effect_t, effects_container_size>;
//     using states_container = const utils::resource_container_array<utils::id, core::state_t, states_container_size>;
//     using weapons_container = const utils::resource_container_array<utils::id, game::weapon_t, weapons_container_size>;
//   }
// }
