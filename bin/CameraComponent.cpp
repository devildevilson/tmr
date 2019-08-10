#include "CameraComponent.h"

#include "Globals.h"
#include "TransformComponent.h"
#include "InputComponent.h"
#include "PhysicsComponent.h"
#include "VulkanRender.h"
#include "SoundSystem.h"

CameraComponent::CameraComponent(const CreateInfo &info) : trans(info.trans), input(info.input), phys(info.phys) {}
CameraComponent::~CameraComponent() {}

void CameraComponent::update() {
  static const simd::mat4 toVulkanSpace = simd::mat4( 1.0f, 0.0f, 0.0f, 0.0f,
                                                      0.0f,-1.0f, 0.0f, 0.0f,
                                                      0.0f, 0.0f, 1.0f, 0.0f,
                                                      0.0f, 0.0f, 0.0f, 1.0f);

  const simd::vec4 pos = trans->pos();
  const simd::vec4 dir = trans->rot();

  glm::vec4 pos1;
  pos.storeu(&pos1.x);

  //const simd::mat4 view = toVulkanSpace * simd::lookAt(pos, pos + dir, simd::vec4(0.0f, 1.0f, 0.0f, 0.0f));
  const simd::mat4 view = toVulkanSpace * simd::lookAt(pos, pos + dir, input->up());

  Global::render()->setView(view);
  Global::render()->setCameraPos(pos);
  Global::render()->setCameraDir(dir);

  Global::render()->updateCamera();

  const ListenerData listenerData{
    pos,
    phys->getVelocity(),
    trans->rot(),
    //-Global::physics()->getGravityNorm()
    -PhysicsEngine::getGravityNorm()
  };
  Global::sound()->updateListener(listenerData);

  const RayData ray{
    pos,
    trans->rot()
  };

  Global::physics()->add(ray);

  const simd::mat4 &frustum = Global::render()->getViewProj();
  Global::physics()->add(frustum, pos);
}

