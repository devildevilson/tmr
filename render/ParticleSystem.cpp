#include "ParticleSystem.h"

ParticleSystem::ParticleSystem(yavf::Device* device) : device(device), countVar(0) {
  particles.construct(device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 100);
  gpuCount.construct(device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);
  uniform.construct(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
}

ParticleSystem::~ParticleSystem() {
  
}

void ParticleSystem::update(const uint64_t &time) {
  gpuCount.data()->count = countVar;
  gpuCount.data()->dummy1 = 1;
  gpuCount.data()->dummy2 = 0;
  gpuCount.data()->dummy3 = 0;
  gpuCount.data()->powerOfTwo = nextPowerOfTwo(countVar);
  
  //if (gpuCount.data()->powerOfTwo >= particles.size()) particles.resize(gpuCount.data()->powerOfTwo*2);
  
  uniform.data()->time = time;
}

void ParticleSystem::updateFrustrum(const Frustum &frustum) {
  for (uint32_t i = 0; i < 6; ++i) {
    frustum.planes[i].storeu(&uniform.data()->planes[i].x);
  }
}

void ParticleSystem::add(SmallEmitter* emitter) {}

void ParticleSystem::addParticle(const Particle &particle) {
  // проверка на выход за пределы массива
  // тут еще можно добавить свет
  
//   const size_t id = count.fetch_add(1);
  size_t id;
  {
    std::unique_lock<std::mutex> lock(mutex);
    id = countVar;
    ++countVar;
    
    if (countVar >= particles.size()) {
      const uint32_t newCount = nextPowerOfTwo(countVar+1);
      particles.resize(newCount);
      
      for (uint32_t i = countVar; i < particles.size(); ++i) {
        particles[i] = {
          glm::vec4(0.0f),
          glm::vec4(0.0f),
          glm::vec4(0.0f),
          UINT32_MAX,
          1,
          0,
          0,
          0.0f, 
          0.0f, 
          0.0f,
          0.0f,
          glm::uvec4(0)
        };
      }
    }
  }
  
  particles[id] = particle;
//   ++count;
}

void ParticleSystem::postRenderUpdate() {
  countVar = gpuCount.data()->count;
}

size_t ParticleSystem::count() const {
  return countVar;
}

yavf::Buffer* ParticleSystem::particlesBuffer() const {
  return particles.vector().handle();
}

yavf::Buffer* ParticleSystem::countBuffer() {
  return gpuCount.buffer();
}

yavf::Buffer* ParticleSystem::uniformBuffer() {
  return uniform.buffer();
}
