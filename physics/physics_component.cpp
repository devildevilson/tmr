#include "physics_component.h"
#include "physics_context.h"
#include "Globals.h"

namespace devils_engine {
  namespace components {
    physical_body::physical_body(const physics::core::rigid_body::create_info &info, void* user_data) : index(Global::get<physics::core::context>()->add_body(info, user_data)) {}
    physical_body::~physical_body() {
      Global::get<physics::core::context>()->remove_body(index);
    }
    
    physics::core::rigid_body & physical_body::get() {
      return Global::get<physics::core::context>()->bodies[index];
    }
    
    const physics::core::rigid_body & physical_body::get() const {
      return Global::get<physics::core::context>()->bodies[index];
    }
    
    void* physical_body::user_data() const {
      return Global::get<physics::core::context>()->get_user_data(index);
    }
  }
}
