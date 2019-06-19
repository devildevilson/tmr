#include "RenderStages.h"

#include "Globals.h"

#include "SceneData.h"

#include "Window.h"
// #include "imgui/imgui.h"
#include "nuklear_header.h"
#include "Variable.h"

#include "RAII.h"

#include "ParticleSystem.h"

#ifdef _DEBUG
  #include <cassert>
  #define ASSERT(expr) assert(expr)
#else
  #define ASSERT(expr)
#endif

BeginTaskStage::BeginTaskStage() {}
BeginTaskStage::~BeginTaskStage() {}

void BeginTaskStage::begin() {}

void BeginTaskStage::doWork(RenderContext* context) {
//   std::cout << "BeginTaskStage start" << "\n";
  
  context->interface()->begin();
//   task[index]->begin();
}

void BeginTaskStage::recreate(const uint32_t &width, const uint32_t &height) {
  (void)width;
  (void)height;
}

EndTaskStage::EndTaskStage() {}
EndTaskStage::~EndTaskStage() {}

void EndTaskStage::begin() {}

void EndTaskStage::doWork(RenderContext* context) {
//   std::cout << "EndTaskStage start" << "\n";
  
//   task[index]->end();
  context->interface()->end();
}

void EndTaskStage::recreate(const uint32_t &width, const uint32_t &height) {
  (void)width;
  (void)height;
}

GBufferStage::GBufferStage(const size_t &containerSize, const CreateInfo &info) : container(containerSize) {
  // тут нужно создать все
  // пайплайны, ресурсы и прочее
  
  this->device = info.device;
//   this->task = info.task;
//   this->images = info.images;
//   this->samplers = info.samplers;
  this->uniformBuffer = info.uniformBuffer;
  
  target.create(device, 1, info.width, info.height);
  //task->setRenderTarget(&target, false);
}

GBufferStage::~GBufferStage() {
  for (uint32_t i = 0; i < parallelParts.size(); ++i) {
    container.destroyStage(parallelParts[i]);
  }
}

void GBufferStage::recreatePipelines(ImageResourceContainer* data) {
  for (uint32_t i = 0; i < parallelParts.size(); ++i) {
    parallelParts[i]->recreatePipelines(data);
  }
}

void GBufferStage::begin() {
//   RegionLog rl("GBufferStage::begin()");
  
  for (uint32_t i = 0; i < parallelParts.size(); ++i) {
    parallelParts[i]->begin();
  }
  
  target.nextframe();
}

void GBufferStage::doWork(RenderContext* context) {
  yavf::GraphicTask* task = context->graphics();
  
  task->setRenderTarget(&target);
  task->beginRenderPass();
  for (uint32_t i = 0; i < parallelParts.size(); ++i) {
    parallelParts[i]->doWork(context);
  }

  task->endRenderPass();
}

void GBufferStage::recreate(const uint32_t &width, const uint32_t &height) {
  target.resize(width, height);
}

yavf::DescriptorSet* GBufferStage::getDeferredRenderTargetDescriptor() const {
  return target.get().desc;
}

yavf::DescriptorSetLayout GBufferStage::getDeferredRenderTargetLayoutDescriptor() const {
  return target.getSetLayout();
}

yavf::Image* GBufferStage::getDepthBuffer() const {
  return target.get().depth;
}

MonsterGBufferStage::MonsterGBufferStage(MonsterGPUOptimizer* monsterOptimiser) : pipe(VK_NULL_HANDLE, VK_NULL_HANDLE) {
  //this->monsterDefault = monsterDefault;
  this->monsterOptimiser = monsterOptimiser;
}

MonsterGBufferStage::~MonsterGBufferStage() {
//   device->deallocate(localTask);
  
  device->destroy(pipe.layout());
  device->destroy(pipe);
  
  device->destroy(monsterDefault);
}

void MonsterGBufferStage::create(const CreateInfo &info) {
  this->device = info.device;
  this->uniformBuffer = info.uniformBuffer;
//   this->images = info.images;
//   this->samplers = info.samplers;
  this->target = info.target;
  
  instanceData.construct(device, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 100);
  yavf::DescriptorPool pool = device->descriptorPool(DEFAULT_DESCRIPTOR_POOL_NAME);
  yavf::DescriptorSetLayout storage_layout = device->setLayout(STORAGE_BUFFER_LAYOUT_NAME);
  
  {
    yavf::DescriptorMaker dm(device);
    
    auto desc = dm.layout(storage_layout).create(pool)[0];
    const size_t i = desc->add({instanceData.vector().handle(), 0, instanceData.vector().buffer_size(), 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
    instanceData.vector().setDescriptor(desc, i);
  }
  
  // я забыл переделать буферы хранящие данные карты, поэтому вот эти буферы у меня просто перезаписывались
  {
    monsterDefault = device->create(yavf::BufferCreateInfo::buffer(monsterDefaultVerticesSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT), VMA_MEMORY_USAGE_GPU_ONLY);
    yavf::Buffer buffer(device, yavf::BufferCreateInfo::buffer(monsterDefaultVerticesSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT), VMA_MEMORY_USAGE_CPU_ONLY);
    
    memcpy(buffer.ptr(), monsterDefaultVertices, monsterDefaultVerticesSize);
    //memcpy(monsterDefaultStaging->ptr(), monsterDefaultVertices, 4*sizeof(Vertex));
    
    yavf::TransferTask* task = device->allocateTransferTask();
    
    task->begin();
    task->copy(&buffer, monsterDefault, 0, 0, monsterDefaultVerticesSize);
    task->end();
    
    task->start();
    task->wait();
    
    device->deallocate(task);
  }
  
  monsterOptimiser->setOutputBuffers({&instanceData});
}

void MonsterGBufferStage::recreatePipelines(ImageResourceContainer* data) {
  this->images = data->imageDescriptor();
  this->samplers = data->samplerDescriptor();
  
  yavf::DescriptorSetLayout uniform_layout = device->setLayout(UNIFORM_BUFFER_LAYOUT_NAME);
  //VkDescriptorSetLayout storage_layout = device->setLayout("storage_layout");
  
  yavf::PipelineLayout deferred_layout = device->layout(MONSTER_PIPELINE_LAYOUT_NAME);
  if (deferred_layout != VK_NULL_HANDLE) {
    device->destroyLayout(MONSTER_PIPELINE_LAYOUT_NAME);
    deferred_layout = VK_NULL_HANDLE;
  }
  
  {
    yavf::PipelineLayoutMaker plm(device);
    
    deferred_layout = plm.addDescriptorLayout(uniform_layout)
                         .addDescriptorLayout(data->samplerSetLayout())
                         .addDescriptorLayout(data->imageSetLayout())
//                          .addDescriptorLayout(minmax_layout)
                         .create(MONSTER_PIPELINE_LAYOUT_NAME);
  }
  
  uint32_t constants[2] = {data->samplerCount(), data->imageCount()};
  
  if (pipe.handle() != VK_NULL_HANDLE) {
    device->destroyPipeline(MONSTER_PIPELINE_NAME);
    pipe = yavf::Pipeline(VK_NULL_HANDLE, VK_NULL_HANDLE);
  }
  
  {
    yavf::raii::ShaderModule vertex (device, Global::getGameDir() + "shaders/deferredObj.vert.spv");
    yavf::raii::ShaderModule fagment(device, Global::getGameDir() + "shaders/deferredObj.frag.spv");
    
    yavf::PipelineMaker pm(device);
    pm.clearBlending();
    
    // почему то получается какая то параша если MonsterGPUOptimizer::InstanceData не выровнен по 16 байт
    // с этим кстати могут быть связаны и эти идиотские лаги при движение вперед всторону и при повороте камеры
    // если не учитывать того что этих лагов нет в вине и винде
    // возможно что нужно переустановить драйверы и vulkan, надо кстати на венде проверить выравнивание по 16 байт
    
    pipe = pm.addShader(VK_SHADER_STAGE_VERTEX_BIT, vertex)
             .addShader(VK_SHADER_STAGE_FRAGMENT_BIT, fagment)
               .addSpecializationEntry(0, 0, sizeof(uint32_t))
               .addSpecializationEntry(1, sizeof(uint32_t), sizeof(uint32_t))
               .addData(2*sizeof(uint32_t), constants)
             .vertexBinding(0, sizeof(MonsterGPUOptimizer::InstanceData), VK_VERTEX_INPUT_RATE_INSTANCE) // никогда не забывать заполнить эти поля ВЕРНО
               .vertexAttribute(0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(MonsterGPUOptimizer::InstanceData, mat) + sizeof(glm::vec4)*0)
               .vertexAttribute(1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(MonsterGPUOptimizer::InstanceData, mat) + sizeof(glm::vec4)*1)
               .vertexAttribute(2, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(MonsterGPUOptimizer::InstanceData, mat) + sizeof(glm::vec4)*2)
               .vertexAttribute(3, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(MonsterGPUOptimizer::InstanceData, mat) + sizeof(glm::vec4)*3)
               .vertexAttribute(4, 0, VK_FORMAT_R32_UINT,   offsetof(MonsterGPUOptimizer::InstanceData, textureData) + offsetof(TextureData, t) + offsetof(Texture, imageArrayIndex))
               .vertexAttribute(5, 0, VK_FORMAT_R32_UINT,   offsetof(MonsterGPUOptimizer::InstanceData, textureData) + offsetof(TextureData, t) + offsetof(Texture, imageArrayLayer))
               .vertexAttribute(6, 0, VK_FORMAT_R32_UINT,   offsetof(MonsterGPUOptimizer::InstanceData, textureData) + offsetof(TextureData, t) + offsetof(Texture, samplerIndex))
               //.vertexAttribute(7,  0, VK_FORMAT_R32_SFLOAT, offsetof(MonsterOptimizer::InstanceData, textureData) + offsetof(TextureData, mirroredU))
               //.vertexAttribute(8,  0, VK_FORMAT_R32_SFLOAT, offsetof(MonsterOptimizer::InstanceData, textureData) + offsetof(TextureData, mirroredV))
               .vertexAttribute(7, 0, VK_FORMAT_R32_SFLOAT, offsetof(MonsterGPUOptimizer::InstanceData, textureData) + offsetof(TextureData, movementU))
               .vertexAttribute(8, 0, VK_FORMAT_R32_SFLOAT, offsetof(MonsterGPUOptimizer::InstanceData, textureData) + offsetof(TextureData, movementV))
             .vertexBinding(1, sizeof(Vertex))
               .vertexAttribute(9,  1, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, pos))
               .vertexAttribute(10, 1, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, color))
               .vertexAttribute(11, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, texCoord))
             .depthTest(VK_TRUE)
             .depthWrite(VK_TRUE)
             .assembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN)
             .viewport()
             .scissor()
             .dynamicState(VK_DYNAMIC_STATE_VIEWPORT)
             .dynamicState(VK_DYNAMIC_STATE_SCISSOR)
             .colorBlendBegin(VK_FALSE)
               .colorWriteMask(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)
             .colorBlendBegin(VK_FALSE)
               .colorWriteMask(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)
//           .rasterizationSamples(VK_SAMPLE_COUNT_8_BIT)
             .create(MONSTER_PIPELINE_NAME, deferred_layout, target->renderPass());
  }
}

void MonsterGBufferStage::begin() {}

bool MonsterGBufferStage::doWork(RenderContext* context) {
  // в будущем нужно будет передавать верный сабпасс сюда
  const uint32_t instanceCount = monsterOptimiser->getInstanceCount();
  if (instanceCount == 0) return false;
  
  yavf::GraphicTask* task = context->graphics();
  
  task->setPipeline(pipe);
  task->setDescriptor({uniformBuffer->descriptorSet()->handle(), samplers->handle(), images->handle()}, 0);
  task->setVertexBuffer(instanceData.vector().handle(), 0);
  task->setVertexBuffer(monsterDefault, 1);
  task->draw(monsterDefaultVerticesCount, instanceCount, 0, 0);
  
  monsterOptimiser->clear();
  
  return true;
}

GPUArray<MonsterGPUOptimizer::InstanceData>* MonsterGBufferStage::getInstanceData() {
  return &instanceData;
}

GeometryGBufferStage::GeometryGBufferStage(yavf::Buffer* worldMapVertex, GeometryGPUOptimizer* opt) : pipe(VK_NULL_HANDLE, VK_NULL_HANDLE) {
  this->worldMapVertex = worldMapVertex;
  this->opt = opt;
}

GeometryGBufferStage::~GeometryGBufferStage() {
  device->destroy(pipe.layout());
  device->destroy(pipe);
}

void GeometryGBufferStage::create(const CreateInfo &info) {
  this->device = info.device;
  this->uniformBuffer = info.uniformBuffer;
//   this->images = info.images;
//   this->samplers = info.samplers;
  this->target = info.target;
  
//   localTask = device->allocateGraphicTask(1, false);
//   localTask->setRenderTarget(info.target, false);
//   this->localTask = info.task;
  
  indices.construct(device, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 100);
  instances.construct(device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 100);
  yavf::DescriptorPool pool = device->descriptorPool(DEFAULT_DESCRIPTOR_POOL_NAME);
  yavf::DescriptorSetLayout instances_layout = device->setLayout("geometry_rendering_data");
  
  {
    yavf::DescriptorLayoutMaker dlm(device);
    
    if (instances_layout == VK_NULL_HANDLE) {
      instances_layout = dlm.binding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT).
                             binding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT).
                             create("geometry_rendering_data");
    }
    
    yavf::DescriptorMaker dm(device);
    
    auto desc = dm.layout(instances_layout).create(pool)[0];
    size_t i = desc->add({instances.vector().handle(), 0, instances.vector().buffer_size(), 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
    instances.vector().setDescriptor(desc, i);
    
    i = desc->add({indices.vector().handle(), 0, indices.vector().buffer_size(), 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
    indices.vector().setDescriptor(desc, i);
  }
  
  yavf::DescriptorSetLayout layout = device->setLayout(STORAGE_BUFFER_LAYOUT_NAME);
  {
    yavf::DescriptorMaker dm(device);
    
    auto desc = dm.layout(layout).create(pool)[0];
    const size_t i = desc->add({instances.vector().handle(), 0, instances.vector().buffer_size(), 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
    instances.vector().setDescriptor(desc, i);
  }
  
  opt->setOutputBuffers({&indices, &instances});
}
  
void GeometryGBufferStage::recreatePipelines(ImageResourceContainer* data) {
  this->images = data->imageDescriptor();
  this->samplers = data->samplerDescriptor();
  
  yavf::DescriptorSetLayout uniform_layout = device->setLayout(UNIFORM_BUFFER_LAYOUT_NAME);
  yavf::DescriptorSetLayout storage_layout = device->setLayout(STORAGE_BUFFER_LAYOUT_NAME);
  
  yavf::PipelineLayout deferred_layout2 = device->layout(GEOMETRY_PIPELINE_LAYOUT_NAME);
  if (deferred_layout2 != VK_NULL_HANDLE) {
    device->destroyLayout(GEOMETRY_PIPELINE_LAYOUT_NAME);
    deferred_layout2 = VK_NULL_HANDLE;
  }
  
  {
    yavf::PipelineLayoutMaker plm(device);
    
    deferred_layout2 = plm.addDescriptorLayout(uniform_layout)
                          .addDescriptorLayout(data->samplerSetLayout())
                          .addDescriptorLayout(data->imageSetLayout())
                          .addDescriptorLayout(storage_layout)
                          .create(GEOMETRY_PIPELINE_LAYOUT_NAME);
  }
  
  uint32_t constants[2] = {data->samplerCount(), data->imageCount()};
  
  if (pipe.handle() != VK_NULL_HANDLE) {
    device->destroyPipeline(GEOMETRY_PIPELINE_NAME);
    pipe = yavf::Pipeline(VK_NULL_HANDLE, VK_NULL_HANDLE);
  }
  
  {
    yavf::raii::ShaderModule vertex (device, Global::getGameDir() + "shaders/deferred.vert.spv");
    yavf::raii::ShaderModule fagment(device, Global::getGameDir() + "shaders/deferred.frag.spv");
    
    yavf::PipelineMaker pm(device);
    pm.clearBlending();
    
    pipe = pm.addShader(VK_SHADER_STAGE_VERTEX_BIT, vertex)
             .addShader(VK_SHADER_STAGE_FRAGMENT_BIT, fagment)
               .addSpecializationEntry(0, 0, sizeof(uint32_t))
               .addSpecializationEntry(1, sizeof(uint32_t), sizeof(uint32_t))
               .addData(2*sizeof(uint32_t), constants)
             .vertexBinding(0, sizeof(Vertex))
               .vertexAttribute(0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, pos))
               .vertexAttribute(1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, color))
               .vertexAttribute(2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, texCoord))
             .depthTest(VK_TRUE)
             .depthWrite(VK_TRUE)
//              .assembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN)
             .assembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN, VK_TRUE)
             .viewport()
             .scissor()
             .dynamicState(VK_DYNAMIC_STATE_VIEWPORT)
             .dynamicState(VK_DYNAMIC_STATE_SCISSOR)
             .colorBlendBegin(VK_FALSE)
               .colorWriteMask(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)
             .colorBlendBegin(VK_FALSE)
               .colorWriteMask(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)
//                          .rasterizationSamples(VK_SAMPLE_COUNT_8_BIT)
             .create(GEOMETRY_PIPELINE_NAME, deferred_layout2, target->renderPass());
  }
}

void GeometryGBufferStage::begin() {}

bool GeometryGBufferStage::doWork(RenderContext* context) {
  const uint32_t indexCount = opt->getIndicesCount();
  if (indexCount == 0) return false;
  
  yavf::GraphicTask* task = context->graphics();
  
  task->setPipeline(pipe);
  task->setDescriptor({uniformBuffer->descriptorSet()->handle(), samplers->handle(), images->handle(), instances.vector().descriptorSet()->handle()}, 0);
  task->setVertexBuffer(worldMapVertex, 0);
  task->setIndexBuffer(indices.vector().handle());
  task->drawIndexed(indexCount, 1, 0, 0, 0);
  
  opt->clear();
  
  ASSERT(task->getFamily() < 4);
  
  return true;
}

GPUArray<uint32_t>* GeometryGBufferStage::getIndicesArray() {
  return &indices;
}

GPUArray<GeometryGPUOptimizer::InstanceData>* GeometryGBufferStage::getInstanceData() {
  return &instances;
}

ComputeParticleGBufferStage::ComputeParticleGBufferStage(const StageCreateInfo &info) : 
  device(nullptr),
//   task(info.task), 
  uniformBuffer(nullptr),
  particlesUniformBuffer(info.particlesUniformBuffer), 
  particles(info.particles), 
  particlesCount(info.particlesCount), 
  matrixes(info.matrixes), 
  gbuffer(info.gbuffer), 
  gbufferLayout(info.gbufferLayout) {}

ComputeParticleGBufferStage::~ComputeParticleGBufferStage() {}

void ComputeParticleGBufferStage::create(const CreateInfo &info) {
  this->device = info.device;
  this->target = info.target;
  this->uniformBuffer = info.uniformBuffer;
  
//   yavf::DescriptorPool pool = device->descriptorPool(DEFAULT_DESCRIPTOR_POOL_NAME);
  yavf::DescriptorSetLayout layout = device->setLayout(STORAGE_BUFFER_LAYOUT_NAME);
  yavf::DescriptorSetLayout uniformLayout = device->setLayout(UNIFORM_BUFFER_LAYOUT_NAME);
  yavf::PipelineLayout pipeLayout = VK_NULL_HANDLE;
  yavf::PipelineLayout sortLayout = VK_NULL_HANDLE;
  {
    yavf::PipelineLayoutMaker plm(device);
    
    pipeLayout = plm.addDescriptorLayout(uniformLayout)
                    .addDescriptorLayout(uniformLayout)
                    .addDescriptorLayout(layout)
                    .addDescriptorLayout(layout)
                    .addDescriptorLayout(uniformLayout)
                    .addDescriptorLayout(gbufferLayout)
                    .create("compute_particle_pipeline_layout");
                    
    sortLayout = plm.addDescriptorLayout(layout).addDescriptorLayout(layout).create("sorting_particle_pipeline_layout");
  }
  
  {
    yavf::ComputePipelineMaker cpm(device);
    
    yavf::raii::ShaderModule particlesShader(device, Global::getGameDir() + "shaders/particles.comp.spv");
    
    particlesPipe = cpm.shader(particlesShader).create("compute_particle_pipeline", pipeLayout);
    
    yavf::raii::ShaderModule sortingShader(device, Global::getGameDir() + "shaders/particlesSorting.comp.spv");
    
    sortPipe = cpm.shader(sortingShader).create("sorting_particle_pipeline", sortLayout);
  }
}

void ComputeParticleGBufferStage::recreatePipelines(ImageResourceContainer* data) {}

void ComputeParticleGBufferStage::begin() {}

bool ComputeParticleGBufferStage::doWork(RenderContext* context) {
  const size_t particlesCountVariable = Global::particles()->count();
  if (particlesCountVariable == 0) return true;
  
  yavf::CombinedTask* task = context->combined();
  
  task->endRenderPass();
  
  // тут у нас две вещи:
  // компут шейдер с данными геометрии
  // сортировка всех частиц
  
  // естественно все на гпу
  
  //const size_t particlesCount = reinterpret_cast<Transform*>(particlesCount->ptr());
  #define PARTICLES_WORKGROUP_SIZE 256
  const uint32_t dispatchX = std::ceil(float(particlesCountVariable) / float(PARTICLES_WORKGROUP_SIZE));
  
  task->setPipeline(particlesPipe);
  task->setDescriptor({uniformBuffer->descriptorSet()->handle(), 
                              particlesUniformBuffer->descriptorSet()->handle(), 
                              particles->descriptorSet()->handle(), 
                              particlesCount->descriptorSet()->handle(), 
                              matrixes->descriptorSet()->handle(), 
                              gbuffer->handle()}, 0);
  task->dispatch(dispatchX, 1, 1); // тут мы разделим общее количество на блоки по 256 (512? 1024?) 
  
  task->setBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                          VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT);
  
  task->setPipeline(sortPipe);
  task->setDescriptor({particles->descriptorSet()->handle(), particlesCount->descriptorSet()->handle()}, 0);
  task->dispatch(1, 1, 1);
  
  task->beginRenderPass();
  
  return true;
}

ParticleGBufferStage::ParticleGBufferStage(const StageCreateInfo &info) : particlesUniformBuffer(info.particlesUniformBuffer), particles(info.particles), particlesCount(info.particlesCount) {}

ParticleGBufferStage::~ParticleGBufferStage() {}

void ParticleGBufferStage::create(const CreateInfo &info) {
  this->device = info.device;
  this->target = info.target;
  this->uniformBuffer = info.uniformBuffer;
//   this->localTask = info.task;
}

void ParticleGBufferStage::recreatePipelines(ImageResourceContainer* data) {
  images = data->imageDescriptor();
  samplers = data->samplerDescriptor();
  
  yavf::PipelineLayout layout = device->layout("particle_gbuffer_pipeline_layout");
  
  if (layout != VK_NULL_HANDLE) {
    device->destroyLayout("particle_gbuffer_pipeline_layout");
    layout = VK_NULL_HANDLE;
  }
  
  yavf::DescriptorSetLayout uniformLayout = device->setLayout(UNIFORM_BUFFER_LAYOUT_NAME);
  yavf::DescriptorSetLayout storageLayout = device->setLayout(STORAGE_BUFFER_LAYOUT_NAME);
  {
    yavf::PipelineLayoutMaker plm(device);
    
    layout = plm.addDescriptorLayout(uniformLayout)
                .addDescriptorLayout(uniformLayout)
                .addDescriptorLayout(data->imageSetLayout())
                .addDescriptorLayout(data->samplerSetLayout())
                .create("particle_gbuffer_pipeline_layout");
  }
  
  if (pipe.handle() != VK_NULL_HANDLE) {
    device->destroy(pipe);
    pipe = yavf::Pipeline(VK_NULL_HANDLE, VK_NULL_HANDLE);
  }
  
  {
    yavf::PipelineMaker pm(device);
    pm.clearBlending();
    
    yavf::raii::ShaderModule vert(device, Global::getGameDir() + "shaders/particles.vert.spv");
    yavf::raii::ShaderModule geom(device, Global::getGameDir() + "shaders/particles.geom.spv");
    yavf::raii::ShaderModule frag(device, Global::getGameDir() + "shaders/particles.frag.spv");
    
    uint32_t constants[2] = {data->samplerCount(), data->imageCount()};
    
    pipe = pm.addShader(VK_SHADER_STAGE_VERTEX_BIT, vert).
              addShader(VK_SHADER_STAGE_GEOMETRY_BIT, geom).
              addShader(VK_SHADER_STAGE_FRAGMENT_BIT, frag).
                addSpecializationEntry(0, 0, sizeof(uint32_t)).
                addSpecializationEntry(1, sizeof(uint32_t), sizeof(uint32_t)).
                addData(2*sizeof(uint32_t), constants).
              vertexBinding(0, sizeof(Particle)).
                vertexAttribute(0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Particle, cur)).
                vertexAttribute(1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Particle, vel)).
                vertexAttribute(2, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Particle, col)).
                vertexAttribute(3, 0, VK_FORMAT_R32G32B32A32_UINT,   offsetof(Particle, currentTime)).
                vertexAttribute(4, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Particle, maxScale)).
                vertexAttribute(5, 0, VK_FORMAT_R32G32B32A32_UINT,   offsetof(Particle, texture)).
              depthTest(VK_TRUE).
              depthWrite(VK_TRUE).
//                          .assembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN)
              assembly(VK_PRIMITIVE_TOPOLOGY_POINT_LIST).
              viewport().
              scissor().
              dynamicState(VK_DYNAMIC_STATE_VIEWPORT).
              dynamicState(VK_DYNAMIC_STATE_SCISSOR).
              colorBlendBegin(VK_FALSE).
                colorWriteMask(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT).
              colorBlendBegin(VK_FALSE).
                colorWriteMask(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT).
              create("particle_gbuffer_pipeline", layout, target->renderPass());
  }
}

void ParticleGBufferStage::begin() {}

bool ParticleGBufferStage::doWork(RenderContext* context) {
  const size_t particlesCountVariable = Global::particles()->count();
  if (particlesCountVariable == 0) return true;
  
  yavf::GraphicTask* task = context->graphics();
  
  task->setPipeline(pipe);
  task->setDescriptor({uniformBuffer->descriptorSet()->handle(), particlesUniformBuffer->descriptorSet()->handle(), images->handle(), samplers->handle()}, 0);
  task->setVertexBuffer(particles, 0);
  task->drawIndirect(particlesCount, 1);
  
  return true;
}

DecalsGBufferStage::DecalsGBufferStage(DecalOptimizer* optimizer) : device(nullptr), optimizer(optimizer), uniformBuffer(nullptr), target(nullptr), images(nullptr), samplers(nullptr) {}
DecalsGBufferStage::~DecalsGBufferStage() {}

void DecalsGBufferStage::create(const CreateInfo &info) {
  this->device = info.device;
  this->uniformBuffer = info.uniformBuffer;
//   this->images = info.images;
//   this->samplers = info.samplers;
  this->target = info.target;
  
//   localTask = device->allocateGraphicTask(1, false);
//   localTask->setRenderTarget(info.target, false);
//   this->localTask = info.task;
  
  vertices.construct(device, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 100);
  indices.construct(device, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 100);
  instances.construct(device, 100);
  
  yavf::DescriptorPool pool = device->descriptorPool(DEFAULT_DESCRIPTOR_POOL_NAME);
  yavf::DescriptorSetLayout layout = device->setLayout(STORAGE_BUFFER_LAYOUT_NAME);
  {
    yavf::DescriptorMaker dm(device);
    
    auto desc = dm.layout(layout).create(pool)[0];
    const size_t i = desc->add({instances.vector().handle(), 0, instances.vector().buffer_size(), 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
    instances.vector().setDescriptor(desc, i);
  }
  
  optimizer->setOutputBuffers({&vertices, &indices, &instances});
}

void DecalsGBufferStage::recreatePipelines(ImageResourceContainer* data) {
  this->images = data->imageDescriptor();
  this->samplers = data->samplerDescriptor();
  
  yavf::DescriptorSetLayout uniform_layout = device->setLayout(UNIFORM_BUFFER_LAYOUT_NAME);
  yavf::DescriptorSetLayout storage_layout = device->setLayout(STORAGE_BUFFER_LAYOUT_NAME);
  
  yavf::PipelineLayout deferred_layout2 = device->layout(DECALS_PIPELINE_LAYOUT_NAME);
  if (deferred_layout2 != VK_NULL_HANDLE) {
    device->destroyLayout(DECALS_PIPELINE_LAYOUT_NAME);
    deferred_layout2 = VK_NULL_HANDLE;
  }
  
  {
    yavf::PipelineLayoutMaker plm(device);
    
    deferred_layout2 = plm.addDescriptorLayout(uniform_layout)
                          .addDescriptorLayout(data->samplerSetLayout())
                          .addDescriptorLayout(data->imageSetLayout())
                          .addDescriptorLayout(storage_layout)
                          .create(DECALS_PIPELINE_LAYOUT_NAME);
  }
  
  uint32_t constants[2] = {data->samplerCount(), data->imageCount()};
  
  if (pipe.handle() != VK_NULL_HANDLE) {
    device->destroyPipeline(DECALS_PIPELINE_NAME);
    pipe = yavf::Pipeline(VK_NULL_HANDLE, VK_NULL_HANDLE);
  }
  
  {
    yavf::raii::ShaderModule vertex (device, Global::getGameDir() + "shaders/deferred.vert.spv");
    yavf::raii::ShaderModule fagment(device, Global::getGameDir() + "shaders/deferred.frag.spv");
    
    yavf::PipelineMaker pm(device);
    pm.clearBlending();
    
    // по идее ничего особо не поменяется здесь по сравнению с рендерингом геометрии
    pipe = pm.addShader(VK_SHADER_STAGE_VERTEX_BIT, vertex)
             .addShader(VK_SHADER_STAGE_FRAGMENT_BIT, fagment)
               .addSpecializationEntry(0, 0, sizeof(uint32_t))
               .addSpecializationEntry(1, sizeof(uint32_t), sizeof(uint32_t))
               .addData(2*sizeof(uint32_t), constants)
             .vertexBinding(0, sizeof(Vertex))
               .vertexAttribute(0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, pos))
               .vertexAttribute(1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, color))
               .vertexAttribute(2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, texCoord))
             .depthTest(VK_TRUE)
             .depthWrite(VK_TRUE)
//                          .assembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN)
             .assembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN, VK_TRUE)
             .viewport()
             .scissor()
             .dynamicState(VK_DYNAMIC_STATE_VIEWPORT)
             .dynamicState(VK_DYNAMIC_STATE_SCISSOR)
             .colorBlendBegin(VK_FALSE)
               .colorWriteMask(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)
             .colorBlendBegin(VK_FALSE)
               .colorWriteMask(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)
//                          .rasterizationSamples(VK_SAMPLE_COUNT_8_BIT)
             .create(GEOMETRY_PIPELINE_NAME, deferred_layout2, target->renderPass());
  }
}

void DecalsGBufferStage::begin() {
  optimizer->optimize();
}

bool DecalsGBufferStage::doWork(RenderContext* context) {
  const uint32_t indexCount = optimizer->getIndicesCount();
  if (indexCount == 0) return false;
  
  yavf::GraphicTask* task = context->graphics();
  
  task->setPipeline(pipe);
  task->setDescriptor({uniformBuffer->descriptorSet()->handle(), samplers->handle(), images->handle(), instances.vector().descriptorSet()->handle()}, 0);
  task->setVertexBuffer(vertices.vector().handle(), 0);
  task->setIndexBuffer(indices.vector().handle());
  
  // нужно будет это установить в пайплайне
  //localTask[index]->setDepthBias(EPSILON, 0.0f, 1.0f);
  
  task->drawIndexed(indexCount, 1, 0, 0, 0);
  
  optimizer->clear();
  
  ASSERT(task->getFamily() < 4);
  
  return true;
}

DefferedLightStage::DefferedLightStage(const CreateInfo &info) : lightArray(info.device) {
  this->device = info.device;
  this->uniformBuffer = info.uniformBuffer;
  this->matrixBuffer = info.matrixBuffer;
//   this->task = info.task;
  this->optimizer = info.optimizer;
  this->gbuffer = info.gbuffer;
  this->gbufferLayout = info.gbufferLayout;
  
  output = device->create(yavf::ImageCreateInfo::texture2D({info.width, info.height}, 
                                                           VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, 
                                                           VK_FORMAT_R32G32B32A32_SFLOAT), 
                          VMA_MEMORY_USAGE_GPU_ONLY);
  yavf::ImageView* view = output->createView(VK_IMAGE_VIEW_TYPE_2D, {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});
  
  auto task = device->allocateTransferTask();
  
  task->begin();
  task->setBarrier(output, VK_IMAGE_LAYOUT_GENERAL);
  task->end();
  
  task->start();
  task->wait();
  
  device->deallocate(task);
  
  // создать пайплайн
  yavf::DescriptorSetLayout uniform_layout = device->setLayout(UNIFORM_BUFFER_LAYOUT_NAME);
  yavf::DescriptorSetLayout layout = device->setLayout(STORAGE_BUFFER_LAYOUT_NAME);
  yavf::DescriptorSetLayout storage_image_layout = device->setLayout("storage_image_layout");
//   yavf::DescriptorSetLayout matrixes = device->setLayout(MATRICES_BUFFER_LAYOUT_NAME);
  {
    yavf::DescriptorLayoutMaker dlm(device);
    
    if (layout == VK_NULL_HANDLE) {
      layout = dlm.binding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT).create(STORAGE_BUFFER_LAYOUT_NAME);
    }
//     
//     if (uniform_layout == VK_NULL_HANDLE) {
//       uniform_layout = dlm.binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT).create(UNIFORM_BUFFER_LAYOUT_NAME);
//     }
    
    if (storage_image_layout == VK_NULL_HANDLE) {
      storage_image_layout = dlm.binding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT).create("storage_image_layout");
    }
  }
  
  yavf::PipelineLayout pipelineLayout = VK_NULL_HANDLE;
  {
    yavf::PipelineLayoutMaker plm(device);
    
    pipelineLayout = plm.addDescriptorLayout(uniform_layout)
                        .addDescriptorLayout(gbufferLayout)
//                                  .addDescriptorLayout(minmax_layout)
                        .addDescriptorLayout(layout)
//                         .addDescriptorLayout(matrixes)
                        .addDescriptorLayout(uniform_layout)
                        .addDescriptorLayout(storage_image_layout)
                        .create("image_processing_layout");
  }
  
  {
    yavf::ComputePipelineMaker cpm(device);
    yavf::raii::ShaderModule compute(device, Global::getGameDir() + "shaders/tiling.comp.spv");
    
    pipe = cpm.shader(compute).create("compute_pipeline", pipelineLayout);
  }
  
  yavf::DescriptorPool pool = device->descriptorPool(DEFAULT_DESCRIPTOR_POOL_NAME);
  {
    lightArray.construct(device);
    
    yavf::DescriptorMaker dm(device);
    
    yavf::DescriptorSet* d = dm.layout(layout).create(pool)[0];
    const size_t i = d->add({lightArray.vector().handle(), 0, lightArray.vector().buffer_size(), 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
    lightArray.vector().setDescriptor(d, i);
    
    yavf::DescriptorSet* d2 = dm.layout(storage_image_layout).create(pool)[0];
    const size_t i2 = d2->add({VK_NULL_HANDLE, view, VK_IMAGE_LAYOUT_GENERAL, 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE});
    view->setDescriptor(d2, i2);
  }
  
  optimizer->setOutputBuffers({&lightArray});
}

DefferedLightStage::~DefferedLightStage() {
  device->destroy(output);
  device->destroySetLayout("storage_image_layout");
  device->destroyLayout("image_processing_layout");
  device->destroy(pipe);
}

void DefferedLightStage::begin() {
  optimizer->optimize();
}

void DefferedLightStage::doWork(RenderContext* context) {
  yavf::ComputeTask* task = context->compute();
  
  task->setPipeline(pipe);
  task->setDescriptor({uniformBuffer->descriptorSet()->handle(), 
                      gbuffer->handle(), 
                      lightArray.vector().descriptorSet()->handle(), 
                      matrixBuffer->descriptorSet()->handle(), 
                      output->view()->descriptorSet()->handle()}, 0);
  const uint32_t xCount = glm::ceil(static_cast<float>(output->info().extent.width)  / static_cast<float>(WORKGROUP_SIZE));
  const uint32_t yCount = glm::ceil(static_cast<float>(output->info().extent.height) / static_cast<float>(WORKGROUP_SIZE));
  task->dispatch(xCount, yCount, 1);
  
  // баррьер?
  
  task->setBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                   VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT);
}

void DefferedLightStage::recreate(const uint32_t &width, const uint32_t &height) {
  output->recreate({width, height, 1});
}

LightOptimizer* DefferedLightStage::getOptimizer() {
  return optimizer;
}

yavf::DescriptorSet* DefferedLightStage::getOutputDescriptor() const {
  return output->view()->descriptorSet();
}

ToneMappingStage::ToneMappingStage(const CreateInfo &info) {
  this->device = info.device;
//   this->task = info.task;
  this->highResImage = info.highResImage;
  
  output = device->create(yavf::ImageCreateInfo::texture2D({info.width, info.height}, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT), VMA_MEMORY_USAGE_GPU_ONLY);
  yavf::ImageView* view = output->createView(VK_IMAGE_VIEW_TYPE_2D, {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});
  
  auto task = device->allocateTransferTask();
  
  task->begin();
  task->setBarrier(output, VK_IMAGE_LAYOUT_GENERAL);
  task->end();
  
  task->start();
  task->wait();
  
  device->deallocate(task);
  
  yavf::DescriptorSetLayout storage_image_layout = device->setLayout("storage_image_layout");
  
  yavf::PipelineLayout tone_mapping_layout = VK_NULL_HANDLE;
  {
    yavf::PipelineLayoutMaker plm(device);
    
    tone_mapping_layout = plm.addDescriptorLayout(storage_image_layout)
                             .addDescriptorLayout(storage_image_layout)
                             .create("tone_mapping_layout");
  }
  
  {
    yavf::ComputePipelineMaker cpm(device);
    yavf::raii::ShaderModule compute(device, (Global::getGameDir() + "shaders/toneMapping.comp.spv").c_str());
    
    pipe = cpm.shader(compute).create("tone_mapping_pipeline", tone_mapping_layout);
  }
  
  yavf::DescriptorPool pool = device->descriptorPool(DEFAULT_DESCRIPTOR_POOL_NAME);
  {
    yavf::DescriptorMaker dm(device);
    
    yavf::DescriptorSet* d2 = dm.layout(storage_image_layout).create(pool)[0];

    const size_t i = d2->add({VK_NULL_HANDLE, view, VK_IMAGE_LAYOUT_GENERAL, 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE});
    view->setDescriptor(d2, i);
  }
}

ToneMappingStage::~ToneMappingStage() {
  device->destroy(output);
  device->destroyLayout("tone_mapping_layout");
  device->destroy(pipe);
}

void ToneMappingStage::begin() {
  // ничего видимо
}

void ToneMappingStage::doWork(RenderContext* context) {
  yavf::ComputeTask* task = context->compute();
  
  task->setPipeline(pipe);
  task->setDescriptor({highResImage->handle(), output->view()->descriptorSet()->handle()}, 0);
  const uint32_t xCount = glm::ceil(static_cast<float>(output->info().extent.width)  / static_cast<float>(WORKGROUP_SIZE));
  const uint32_t yCount = glm::ceil(static_cast<float>(output->info().extent.height) / static_cast<float>(WORKGROUP_SIZE));
  task->dispatch(xCount, yCount, 1);
  
  // барьер?
  
  task->setBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                   VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT);
}

void ToneMappingStage::recreate(const uint32_t &width, const uint32_t &height) {
  output->recreate({width, height, 1});
}

yavf::Image* ToneMappingStage::getOutputImage() const {
  return output;
}

CopyStage::CopyStage(const CreateInfo &info) {
  this->device = info.device;
//   this->task = info.task;
  this->src = info.src;
  this->depthSrc = info.depthSrc;
  this->presentFamily = info.presentFamily;
  this->window = info.window;
}

CopyStage::~CopyStage() {}

void CopyStage::begin() {
  // ничего?
}

void CopyStage::doWork(RenderContext* context) {  
  static const VkImageSubresourceRange range{
    VK_IMAGE_ASPECT_COLOR_BIT,
    0, 1, 0, 1
  };
  
  const VkRect2D &rect = window->size();
  
  const VkImageBlit blit{
    {
      VK_IMAGE_ASPECT_COLOR_BIT,
      0, 0, 1
    },
    {
      {0, 0, 0}, 
      {static_cast<int32_t>(rect.extent.width), static_cast<int32_t>(rect.extent.height), 1}
    },
    {
      VK_IMAGE_ASPECT_COLOR_BIT,
      0, 0, 1
    },
    {
      {0, 0, 0}, 
      {static_cast<int32_t>(rect.extent.width), static_cast<int32_t>(rect.extent.height), 1}
    }
  };
  
  yavf::GraphicTask* task = context->graphics();
  
  task->setBarrier(window->getImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, range, presentFamily, VK_QUEUE_FAMILY_IGNORED);
  task->setBarrier(src, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
  task->copyBlit(src, window->getImage(), blit);
  task->setBarrier(src, VK_IMAGE_LAYOUT_GENERAL);
  task->setBarrier(window->getImage(), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, range, VK_QUEUE_FAMILY_IGNORED, presentFamily);
  
  task->setBarrier(depthSrc, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);
  task->setBarrier(window->getDepth(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);
  task->copy(depthSrc, window->getDepth(), VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);
  task->setBarrier(depthSrc, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);
  task->setBarrier(window->getDepth(), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void CopyStage::recreate(const uint32_t &width, const uint32_t &height) {
  (void)width;
  (void)height;
  
  // ничего?
}

PostRenderStage::PostRenderStage(const size_t &containerSize, const CreateInfo &info) : container(containerSize) {
  this->device = info.device;
//   this->task = info.task;
//   this->target = info.target;
  this->window = info.window;
}

PostRenderStage::~PostRenderStage() {
  for (size_t i = 0; i < parts.size(); ++i) {
    container.destroyStage(parts[i]);
  }
}

void PostRenderStage::recreatePipelines(ImageResourceContainer* data) {
  for (size_t i = 0; i < parts.size(); ++i) {
    parts[i]->recreatePipelines(data);
  }
}

void PostRenderStage::begin() {
  for (size_t i = 0; i < parts.size(); ++i) {
    parts[i]->begin();
  }
}

void PostRenderStage::doWork(RenderContext* context) {
  yavf::GraphicTask* task = context->graphics();  
  
  task->setRenderTarget(window->currentRenderTarget());
  
  //VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS
  task->beginRenderPass();
  
  for (size_t i = 0; i < parts.size(); ++i) {
    parts[i]->doWork(context);
  }
  
//   for (uint32_t i = 0; i < parts.size(); ++i) {
//     task->execute(parts[i]->getSecondaryTask());
//   }
  
  task->endRenderPass();
}

void PostRenderStage::recreate(const uint32_t &width, const uint32_t &height) {
  (void)width;
  (void)height;
  
  // ничего?
}

// тут нужно бы еще передать дескриптор
void drawGUI(nuklear_data* data, const yavf::Pipeline &pipe, yavf::Buffer* vertexGui, yavf::Buffer* indexGui, yavf::Buffer* matrix, yavf::GraphicTask* task) {
  // прибиндим пайплайн и текстурку
  task->setPipeline(pipe);

  // прибиндим вершинный и буфер индексов
  task->setVertexBuffer(vertexGui, 0);
  task->setIndexBuffer(indexGui, VK_INDEX_TYPE_UINT16);
  
  task->setDescriptor(matrix->descriptorSet(), 0);
  
  uint32_t index_offset = 0;
  const nk_draw_command *cmd = nullptr;
  nk_draw_foreach(cmd, &data->ctx, &data->cmds) {
    if (cmd->elem_count == 0) continue;
    
    // так мы получаем какой то распозванательный знак тектуры
    //const auto &tex = reinterpret_cast<yavf::Image*>(cmd->texture.ptr);
    
    // капец, я вынужден констатировать что с указателями было удобнее, пролятье =(
    //yavf::Image* tex = reinterpret_cast<yavf::Image*>(cmd->texture.ptr);
    yavf::ImageView* tex = reinterpret_cast<yavf::ImageView*>(cmd->texture.ptr);
    
    // как его можно использовать? неплохо было бы добавить все эти данные в буфер
    // как у меня это было раньше, как это сделать? мне нужно всего лишь запомнить некий индекс
    // скорее всего придется еще раз пройтись по командам нк, либо добавлять текстурку как пуш константу
    // мне бы этого не хотелось, хотя почему бы и нет?
    
    // то есть мне нужен какой то массив из которого я смогу брать свою текстурку?
    // это с обной стороны, с другой если сэмлер у меня будет один...
    // будет ли он один? (может ли мне пригодиться линейный и ниарест сэмплер?)
    
    // текстурки скорее всего будут четко известны, точнее указатели на них
    // они будут находиться в какой нибудь стурктуре (media), откуда я буду их брать для того чтобы использовать в игре
    // то есть особо париться на счет указателей не нужно
    
    // насколько пуш константы быстрые/медленные?
    
    // как то так выглядит это дело
//     task->setConsts(0, sizeof(Texture), tex.ptr);
    
    // пока что это будет выглядеть вот так
    task->setDescriptor(tex->descriptorSet(), 1);

    const VkRect2D scissor{
      {
        static_cast<int32_t>(std::max(cmd->clip_rect.x * Global::data()->fbScaleX, 0.0f)),
        static_cast<int32_t>(std::max(cmd->clip_rect.y * Global::data()->fbScaleY, 0.0f)),
      },
      {
        static_cast<uint32_t>(cmd->clip_rect.w * Global::data()->fbScaleX),
        static_cast<uint32_t>(cmd->clip_rect.h * Global::data()->fbScaleY),
      }
    };
    
    task->setScissor(scissor);
    task->drawIndexed(cmd->elem_count, 1, index_offset, 0, 0);
    index_offset += cmd->elem_count;
  }
  
  nk_clear(&data->ctx);
}

struct gui_vertex {
  glm::vec2 pos;
  glm::vec2 uv;
  uint32_t color;
};

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_INDEX_BUFFER 128 * 1024

GuiStage::GuiStage(nuklear_data* data) : vertexGui(nullptr), indexGui(nullptr), matrix(nullptr), data(data) {}

GuiStage::~GuiStage() {
  device->destroy(vertexGui);
  device->destroy(indexGui);
  device->destroyLayout("gui_layout");
  device->destroy(pipe);
}

void GuiStage::create(const CreateInfo &info) {
  this->device = info.device;
//   this->target = info.target;
  this->window = info.window;
  
//   localTask = device->allocateGraphicTask(1, false);
//   localTask->setRenderTarget(info.target, false);
//   this->localTask = info.task;
  
  {
    vertexGui = device->create(yavf::BufferCreateInfo::buffer(MAX_VERTEX_BUFFER, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT), VMA_MEMORY_USAGE_CPU_ONLY);
  }
  
  {
    indexGui = device->create(yavf::BufferCreateInfo::buffer(MAX_INDEX_BUFFER, VK_BUFFER_USAGE_INDEX_BUFFER_BIT), VMA_MEMORY_USAGE_CPU_ONLY);
  }
  
  yavf::DescriptorSetLayout sampled_image_layout = device->setLayout(SAMPLED_IMAGE_LAYOUT_NAME);
  yavf::DescriptorSetLayout uniform_layout       = device->setLayout(UNIFORM_BUFFER_LAYOUT_NAME);
  {
    yavf::DescriptorLayoutMaker dlm(device);
    
    if (sampled_image_layout == VK_NULL_HANDLE) {
      sampled_image_layout = dlm.binding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT).create(SAMPLED_IMAGE_LAYOUT_NAME);
    }
  }
  
  {
    matrix = device->create(yavf::BufferCreateInfo::buffer(sizeof(glm::mat4), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT), VMA_MEMORY_USAGE_CPU_ONLY);
    
    yavf::DescriptorPool pool = device->descriptorPool(DEFAULT_DESCRIPTOR_POOL_NAME);
    
    yavf::DescriptorMaker dm(device);
    auto d = dm.layout(uniform_layout).create(pool)[0];
    const size_t index = d->add({matrix, 0, matrix->info().size, 0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER});
    matrix->setDescriptor(d, index);
  }
  
  yavf::PipelineLayout gui_layout = VK_NULL_HANDLE;
  {
    yavf::PipelineLayoutMaker plm(device);
    
    gui_layout = plm.addDescriptorLayout(uniform_layout).addDescriptorLayout(sampled_image_layout).addPushConstRange(0, sizeof(glm::vec2) + sizeof(glm::vec2)).create("gui_layout");
  }
  
  {
    yavf::PipelineMaker pm(device);
    pm.clearBlending();
    
//     pipe = pm.addShader(VK_SHADER_STAGE_VERTEX_BIT, Global::getGameDir() + "shaders/gui.vert.spv")
//              .addShader(VK_SHADER_STAGE_FRAGMENT_BIT, Global::getGameDir() + "shaders/gui.frag.spv")
//              .vertexBinding(0, sizeof(ImDrawVert))
//                .vertexAttribute(0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, pos))
//                .vertexAttribute(1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, uv))
//                .vertexAttribute(2, 0, VK_FORMAT_R8G8B8A8_UNORM, offsetof(ImDrawVert, col))
//              .assembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
//              .depthTest(VK_FALSE)
//              .depthWrite(VK_FALSE)
//              .clearBlending()
//              .colorBlendBegin()
//                .srcColor(VK_BLEND_FACTOR_SRC_ALPHA)
//                .dstColor(VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA)
//                .srcAlpha(VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA)
//                .dstAlpha(VK_BLEND_FACTOR_SRC_ALPHA)
//              .viewport()
//              .scissor()
//              .dynamicState(VK_DYNAMIC_STATE_VIEWPORT)
//              .dynamicState(VK_DYNAMIC_STATE_SCISSOR)
//              .create("gui_pipeline", gui_layout, target->renderPass());
    
    yavf::raii::ShaderModule vertex(device, (Global::getGameDir() + "shaders/gui.vert.spv").c_str());
    yavf::raii::ShaderModule fragment(device, (Global::getGameDir() + "shaders/gui.frag.spv").c_str());
    
    pipe = pm.addShader(VK_SHADER_STAGE_VERTEX_BIT, vertex)
             .addShader(VK_SHADER_STAGE_FRAGMENT_BIT, fragment)
             .vertexBinding(0, sizeof(gui_vertex))
               .vertexAttribute(0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(gui_vertex, pos))
               .vertexAttribute(1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(gui_vertex, uv))
               .vertexAttribute(2, 0, VK_FORMAT_R8G8B8A8_UINT, offsetof(gui_vertex, color))
             .assembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
             .depthTest(VK_FALSE)
             .depthWrite(VK_FALSE)
             .clearBlending()
             .colorBlendBegin()
               .srcColor(VK_BLEND_FACTOR_SRC_ALPHA)
               .dstColor(VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA)
               .srcAlpha(VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA)
               .dstAlpha(VK_BLEND_FACTOR_SRC_ALPHA)
             .viewport()
             .scissor()
             .dynamicState(VK_DYNAMIC_STATE_VIEWPORT)
             .dynamicState(VK_DYNAMIC_STATE_SCISSOR)
             .create("gui_pipeline", gui_layout, window->pass());
  }
}

void GuiStage::recreatePipelines(ImageResourceContainer* data) {
  (void)data;
}

void GuiStage::begin() {
//   RegionLog rl("GuiStage::begin()");
  
//   ImDrawData* draw_data = ImGui::GetDrawData();
//   
// //   if (draw_data == nullptr) throw std::runtime_error("????????");
// 
//   const size_t vertex_size = draw_data->TotalVtxCount * sizeof(ImDrawVert);
//   const size_t index_size = draw_data->TotalIdxCount * sizeof(ImDrawIdx);
// 
//   // переделаем вершинный буфер
//   vertexGui->recreate(vertex_size, draw_data->TotalVtxCount);
// 
//   // переделаем индексный буфер
//   indexGui->recreate(index_size, draw_data->TotalIdxCount);
// 
//   // скопируем вершины и индексы
//   ImDrawVert* vtx_dst = reinterpret_cast<ImDrawVert*>(vertexGui->ptr());
//   ImDrawIdx* idx_dst = reinterpret_cast<ImDrawIdx*>(indexGui->ptr());
//   for (int n = 0; n < draw_data->CmdListsCount; ++n) {
//     const ImDrawList* cmd_list = draw_data->CmdLists[n];
//     memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
//     memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
//     vtx_dst += cmd_list->VtxBuffer.Size;
//     idx_dst += cmd_list->IdxBuffer.Size;
//   }
  
  // тут мы должны получить ctx
  // у нас так же должна быть null тектура
  // я на самом деле так и не понял зачем она нужна
  // да и вообще наверное весь нуклир дата
  
  {
    void* vertices = vertexGui->ptr();
    void* elements = indexGui->ptr();
    
    /* fill convert configuration */
    static const struct nk_draw_vertex_layout_element vertex_layout[] = {
      {NK_VERTEX_POSITION, NK_FORMAT_FLOAT, offsetof(gui_vertex, pos)},
      {NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, offsetof(gui_vertex, uv)},
      {NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, offsetof(gui_vertex, color)},
      {NK_VERTEX_LAYOUT_END}
    };
    
    // нужен ли мне изменяющийся антиаляисинг?
    const nk_convert_config config{
      1.0f,
      NK_ANTI_ALIASING_ON,
      NK_ANTI_ALIASING_ON,
      22,
      22,
      22,
      data->null, // нулл текстура
      vertex_layout,
      sizeof(gui_vertex), // размер вершины
      alignof(gui_vertex) // алигн вершины
    };
    
    nk_buffer vbuf, ebuf;
    // мы создаем местные аналоги vkBuffer, то есть мета объект для памяти
    // а затем конвертим вершины в удобный нам формат
    nk_buffer_init_fixed(&vbuf, vertices, size_t(MAX_VERTEX_BUFFER));
    nk_buffer_init_fixed(&ebuf, elements, size_t(MAX_INDEX_BUFFER));
    nk_convert(&data->ctx, &data->cmds, &vbuf, &ebuf, &config);
  }
  
  glm::mat4* mat = reinterpret_cast<glm::mat4*>(matrix->ptr());
  *mat = glm::mat4(
     2.0f / window->currentRenderTarget()->viewport().width,  0.0f,  0.0f,  0.0f,
     0.0f,  2.0f / window->currentRenderTarget()->viewport().height,  0.0f,  0.0f,
     0.0f,  0.0f, -1.0f,  0.0f,
    -1.0f, -1.0f,  0.0f,  1.0f);
}

void GuiStage::doWork(RenderContext* context) {
  yavf::GraphicTask* task = context->graphics();
  
  drawGUI(data, pipe, vertexGui, indexGui, matrix, task);
}

MonsterDebugStage::MonsterDebugStage(const CreateInfo &info) : monsterDebug(nullptr), instanceCount(0) {
  this->uniformBuffer = info.uniformBuffer;
  this->optimizer = info.optimizer;
  this->monsterOptimiser = info.monsterOptimiser;
  this->monData = info.monData;
}

MonsterDebugStage::~MonsterDebugStage() {
  device->destroy(pipe);
  device->destroyLayout("monster_debug_layout");
  device->destroy(monsterDebug);
}

void MonsterDebugStage::create(const PostRenderPart::CreateInfo &info) {
  this->device = info.device;
//   this->target = info.target;
  this->window = info.window;
//   this->localTask = info.task;
  
  yavf::DescriptorSetLayout uniform_layout = device->setLayout(UNIFORM_BUFFER_LAYOUT_NAME);
  
  yavf::PipelineLayout monsterDebugLayout = VK_NULL_HANDLE;
  {
    yavf::PipelineLayoutMaker plm(device);
    
    monsterDebugLayout = plm.addDescriptorLayout(uniform_layout).create("monster_debug_layout");
  }
  
  {
    yavf::PipelineMaker pm(device);
    yavf::raii::ShaderModule vertex  (device, (Global::getGameDir() + "shaders/simpleAABB.vert.spv").c_str());
    yavf::raii::ShaderModule fragment(device, (Global::getGameDir() + "shaders/simpleAABB.frag.spv").c_str());
    
    pipe = pm.addShader(VK_SHADER_STAGE_VERTEX_BIT, vertex)
             .addShader(VK_SHADER_STAGE_FRAGMENT_BIT, fragment)
             //.vertexBinding(0, sizeof(glm::mat4), VK_VERTEX_INPUT_RATE_INSTANCE)
             .vertexBinding(0, sizeof(MonsterDebugOptimizer::InstanceData), VK_VERTEX_INPUT_RATE_INSTANCE) // MonsterOptimizer::InstanceData
               .vertexAttribute(0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(MonsterDebugOptimizer::InstanceData, mat) + sizeof(glm::vec4)*0)
               .vertexAttribute(1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(MonsterDebugOptimizer::InstanceData, mat) + sizeof(glm::vec4)*1)
               .vertexAttribute(2, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(MonsterDebugOptimizer::InstanceData, mat) + sizeof(glm::vec4)*2)
               .vertexAttribute(3, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(MonsterDebugOptimizer::InstanceData, mat) + sizeof(glm::vec4)*3)
             //.vertexBinding(1, sizeof(MonsterDebugOptimizer::InstanceData), VK_VERTEX_INPUT_RATE_INSTANCE)
               .vertexAttribute(4, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(MonsterDebugOptimizer::InstanceData, color))
             .vertexBinding(1, sizeof(Vertex))
               .vertexAttribute(5, 1, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, pos))
               .vertexAttribute(6, 1, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, color))
               .vertexAttribute(7, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, texCoord))
             .assembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP)
             .depthTest(VK_TRUE)
             .depthWrite(VK_FALSE)
             .clearBlending()
             .colorBlendBegin()
               .srcColor(VK_BLEND_FACTOR_SRC_ALPHA)
               .dstColor(VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA)
               .srcAlpha(VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA)
               .dstAlpha(VK_BLEND_FACTOR_SRC_ALPHA)
             .viewport()
             .scissor()
             .dynamicState(VK_DYNAMIC_STATE_VIEWPORT)
             .dynamicState(VK_DYNAMIC_STATE_SCISSOR)
             .create("monster_debug_pipeline", monsterDebugLayout, window->pass());
  }
  
  // мне еще нужно создать буфер с кубом
  yavf::Buffer* staging;
  {
    staging = device->create(yavf::BufferCreateInfo::buffer(cubeStripVerticesSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT), VMA_MEMORY_USAGE_CPU_ONLY);
    
    memcpy(staging->ptr(), cubeStripVertices, cubeStripVerticesSize);
  }
  
  monsterDebug = device->create(yavf::BufferCreateInfo::buffer(cubeStripVerticesSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT), VMA_MEMORY_USAGE_GPU_ONLY);
  
  {
    yavf::TransferTask* task = device->allocateTransferTask();
    
    task->begin();
    task->copy(staging, monsterDebug);
    task->end();
    
    task->start();
    task->wait();
    
    device->deallocate(task);
    device->destroy(staging);
  }
  
  {
    instData.vector().construct(device, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    instData.update();
    optimizer->setOutputBuffers({&instData});
  }
}

void MonsterDebugStage::recreatePipelines(ImageResourceContainer* data) {
  (void)data;
  // ничего
}

void MonsterDebugStage::begin() {
  instanceCount = monsterOptimiser->getInstanceCount();
}

void MonsterDebugStage::doWork(RenderContext* context) {
  static cvar debugDraw("debugDraw");
  if (!bool(debugDraw.getFloat())) return;
  if (instanceCount == 0) return;
  
  yavf::GraphicTask* task = context->graphics();
  
  task->setView(window->currentRenderTarget()->viewport());
  task->setScissor(window->currentRenderTarget()->scissor());
  
  task->setPipeline(pipe);
  task->setDescriptor(uniformBuffer->descriptorSet(), 0);
  task->setVertexBuffer(instData.vector().handle(), 0);
  task->setVertexBuffer(monsterDebug, 1);
  task->draw(cubeStripVerticesCount, instanceCount, 0, 0);
  
  optimizer->clear();
}

GeometryDebugStage::GeometryDebugStage(const CreateInfo &info) : indexCount(0) {
  this->uniformBuffer = info.uniformBuffer;
  this->optimizer = info.optimizer;
  this->geometryOptimiser = info.geometryOptimiser;
  this->indices = info.indices;
  this->worldMapVertex = info.worldMapVertex;
}

GeometryDebugStage::~GeometryDebugStage() {
  device->destroyLayout("geometry_debug_layout");
  device->destroy(pipe);
}

void GeometryDebugStage::create(const PostRenderPart::CreateInfo &info) {
  this->device = info.device;
//   this->target = info.target;
  this->window = info.window;
//   this->localTask = info.task;
  
  yavf::DescriptorSetLayout storage_layout = device->setLayout(STORAGE_BUFFER_LAYOUT_NAME);
  yavf::DescriptorSetLayout uniform_layout = device->setLayout(UNIFORM_BUFFER_LAYOUT_NAME);
  
  yavf::PipelineLayout geometryDebugLayout = VK_NULL_HANDLE;
  {
    yavf::PipelineLayoutMaker plm(device);
    
    geometryDebugLayout = plm.addDescriptorLayout(uniform_layout).addDescriptorLayout(storage_layout).create("geometry_debug_layout");
  }
  
  {
    yavf::PipelineMaker pm(device);
    yavf::raii::ShaderModule vertex(device, (Global::getGameDir() + "shaders/simplePoly.vert.spv").c_str());
    yavf::raii::ShaderModule fragment(device, (Global::getGameDir() + "shaders/simplePoly.frag.spv").c_str());
    
//     pipe = pm.addShader(VK_SHADER_STAGE_VERTEX_BIT, Global::getGameDir() + "shaders/simplePoly.vert.spv")
//              .addShader(VK_SHADER_STAGE_FRAGMENT_BIT, Global::getGameDir() + "shaders/simplePoly.frag.spv")
    pipe = pm.addShader(VK_SHADER_STAGE_VERTEX_BIT, vertex)
             .addShader(VK_SHADER_STAGE_FRAGMENT_BIT, fragment)
             .vertexBinding(0, sizeof(Vertex))
               .vertexAttribute(0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, pos))
               .vertexAttribute(1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, color))
               .vertexAttribute(2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, texCoord))
             .assembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN, VK_TRUE)
             .depthTest(VK_TRUE)
             .depthWrite(VK_FALSE)
             .clearBlending()
             .colorBlendBegin()
               .srcColor(VK_BLEND_FACTOR_SRC_ALPHA)
               .dstColor(VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA)
               .srcAlpha(VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA)
               .dstAlpha(VK_BLEND_FACTOR_SRC_ALPHA)
             .viewport()
             .scissor()
             .dynamicState(VK_DYNAMIC_STATE_VIEWPORT)
             .dynamicState(VK_DYNAMIC_STATE_SCISSOR)
             .create("geometry_debug_pipeline", geometryDebugLayout, window->pass());
  }
    
  {
    instData.vector().construct(device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    instData.update();
    optimizer->setOutputBuffers({&instData});
    
    yavf::DescriptorPool pool = device->descriptorPool(DEFAULT_DESCRIPTOR_POOL_NAME);
    
    yavf::DescriptorMaker dm(device);
    auto d = dm.layout(storage_layout).create(pool)[0];
    
//     yavf::DescriptorUpdate du{
//       VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
//       0,
//       0,
//       d
//     };
    
    const auto i = d->add({instData.vector().handle(), 0, instData.vector().buffer_size(), 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
    instData.vector().setDescriptor(d, i);
    //instData.vector().setDescriptorData(du);
  }
}

void GeometryDebugStage::recreatePipelines(ImageResourceContainer* data) {
  (void)data;
  // ничего
}

void GeometryDebugStage::begin() {
  indexCount = geometryOptimiser->getIndicesCount();
}

void GeometryDebugStage::doWork(RenderContext* context) {
  static cvar debugDraw("debugDraw");
  if (!bool(debugDraw.getFloat())) return;
  if (indexCount == 0) return;
  
  yavf::GraphicTask* task = context->graphics();
  
  task->setView(window->currentRenderTarget()->viewport());
  task->setScissor(window->currentRenderTarget()->scissor());
  
  task->setPipeline(pipe);
  task->setDescriptor({uniformBuffer->descriptorSet()->handle(), instData.vector().descriptorSet()->handle()}, 0);
  task->setVertexBuffer(worldMapVertex, 0);
  task->setIndexBuffer(indices->vector().handle());
  
  task->drawIndexed(indexCount, 1, 0, 0, 0);
  
  optimizer->clear();
}
