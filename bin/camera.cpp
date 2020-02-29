#include "camera.h"

#include "Globals.h"
#include "EntityComponentSystem.h"
#include "TransformComponent.h"
#include "PhysicsComponent.h"
#include "InputComponent.h"
#include "global_components_indicies.h"
#include "VulkanRender.h"
// #include "sound_system.h"

namespace devils_engine {
  namespace camera {
    void first_person(const yacs::entity* ent) {
      auto trans = ent->at<TransformComponent>(game::entity::transform);
      auto phys = ent->at<PhysicsComponent>(game::entity::physics);
      auto input = ent->at<UserInputComponent>(game::entity::input);
      
      if (!trans.valid() || !phys.valid() || !input.valid()) throw std::runtime_error("bad camera entity");
      
      static const simd::mat4 toVulkanSpace = simd::mat4( 1.0f, 0.0f, 0.0f, 0.0f,
                                                          0.0f,-1.0f, 0.0f, 0.0f,
                                                          0.0f, 0.0f, 1.0f, 0.0f,
                                                          0.0f, 0.0f, 0.0f, 1.0f);

      const simd::vec4 pos = trans->pos();
      const simd::vec4 dir = trans->rot();

      glm::vec4 pos1;
      pos.storeu(&pos1.x);

      const simd::mat4 view = toVulkanSpace * simd::lookAt(pos, pos + dir, simd::vec4(0.0f, 1.0f, 0.0f, 0.0f));
      //const simd::mat4 view = toVulkanSpace * simd::lookAt(pos, pos + dir, input->up());

      Global::render()->setView(view);
      Global::render()->setCameraPos(pos);
      Global::render()->setCameraDir(dir);

      Global::render()->updateCamera();

      //Global::get<systems::sound>()->update_listener(ent);
      
      // лучик из камеры кидать нужно, но для того чтобы что-то использовать, такой луч будет кидаться в другом месте
      // этот луч нужен только в редакторе
      const RayData ray{
        pos,
        trans->rot()
      };
      Global::get<PhysicsEngine>()->add(ray);

      const simd::mat4 &frustum = Global::render()->getViewProj();
      Global::get<PhysicsEngine>()->add(frustum, pos);
    }
  }
}
