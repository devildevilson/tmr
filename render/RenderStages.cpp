#include "RenderStages.h"

#include "Globals.h"

#include "SceneData.h"

#include "Window.h"
#include "imgui/imgui.h"
#include "nuklear_header.h"
#include "Variable.h"

#include "RAII.h"

#ifdef _DEBUG
  #include <cassert>
  #define ASSERT(expr) assert(expr)
#else
  #define ASSERT(expr)
#endif

BeginTaskStage::BeginTaskStage(yavf::TaskInterface** task) : task(task) {}
BeginTaskStage::~BeginTaskStage() {}

void BeginTaskStage::begin() {}

void BeginTaskStage::doWork(const uint32_t &index) {
//   std::cout << "BeginTaskStage start" << "\n";
  
  task[index]->begin();
}

void BeginTaskStage::recreate(const uint32_t &width, const uint32_t &height) {
  (void)width;
  (void)height;
}

EndTaskStage::EndTaskStage(yavf::TaskInterface** task) : task(task) {}
EndTaskStage::~EndTaskStage() {}

void EndTaskStage::begin() {}

void EndTaskStage::doWork(const uint32_t &index) {
//   std::cout << "EndTaskStage start" << "\n";
  
  task[index]->end();
}

void EndTaskStage::recreate(const uint32_t &width, const uint32_t &height) {
  (void)width;
  (void)height;
}

GBufferStage::GBufferStage(const size_t &containerSize, const CreateInfo &info) : container(containerSize) {
  // тут нужно создать все
  // пайплайны, ресурсы и прочее
  
  this->device = info.device;
  this->task = info.task;
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

void GBufferStage::doWork(const uint32_t &index) {
//   std::cout << "GBufferStage start" << "\n";
  
  //task->begin();
//   yavf::CombinedTask* tasks[parallelParts.size()];
  
//   ASSERT(task[index]->getFamily() < 4);
  
  task[index]->setRenderTarget(&target);
  
//   ASSERT(task[index]->getFamily() < 4);
  
  task[index]->beginRenderPass();
  
//   ASSERT(task[index]->getFamily() < 4);
  
//   throw std::runtime_error("GBufferStage::doWork");
  
//   size_t count = 0;
//   for (uint32_t i = 0; i < parallelParts.size(); ++i) {
//     if (parallelParts[i]->doWork(const uint32_t &index)) {
//       tasks[count] = parallelParts[i]->getSecondaryTask();
//       ++count;
//     }
//   }
  
  for (uint32_t i = 0; i < parallelParts.size(); ++i) {
    parallelParts[i]->doWork(index);
  }
  
//   ASSERT(task[index]->getFamily() < 4);
  
  //std::vector<yavf::CombinedTask*> array(parallelParts.size());
//   std::cout << parallelParts.size() << "\n";
//   for (uint32_t i = 0; i < parallelParts.size(); ++i) {
//     std::cout << parallelParts[i] << "\n";
//     task->execute(parallelParts[i]->getSecondaryTask());
//     //array[i] = parallelParts[i]->getSecondaryTask();
//   }

//   if (count != 0) task->execute(count, tasks);
  
  //task->execute(array);
  
  task[index]->endRenderPass();
  
//   ASSERT(task[index]->getFamily() < 4);
  
//   for (uint32_t i = 0; i < 3; ++i) {
//     std::cout << "task pointer " << i << " " << task[i] << " family " << task[index]->getFamily() << "\n";
//   }
  
  // как то так это выглядит
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

MonsterGBufferStage::MonsterGBufferStage(MonsterOptimizer* monsterOptimiser) : pipe(VK_NULL_HANDLE, VK_NULL_HANDLE) {
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
  
//   localTask = device->allocateGraphicTask(1, false);
//   localTask->setRenderTarget(info.target, false);
  this->localTask = info.task;
  
  instanceData.construct(device, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 100);
  
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
  
//   struct InstanceData {
//     glm::mat4 mat;
//     uint32_t imageIndex;
//     uint32_t imageLayer;
//     uint32_t samplerIndex;
//   };
  
  uint32_t constants[2] = {data->samplerCount(), data->imageCount()};
  
  if (pipe.handle() != VK_NULL_HANDLE) {
    device->destroyPipeline(MONSTER_PIPELINE_NAME);
    pipe = yavf::Pipeline(VK_NULL_HANDLE, VK_NULL_HANDLE);
  }
  
  {
    yavf::raii::ShaderModule vertex (device, (Global::getGameDir() + "shaders/deferredObj.vert.spv").c_str());
    yavf::raii::ShaderModule fagment(device, (Global::getGameDir() + "shaders/deferredObj.frag.spv").c_str());
    
    yavf::PipelineMaker pm(device);
    pm.clearBlending();
    
    pipe = pm.addShader(VK_SHADER_STAGE_VERTEX_BIT, vertex)
             .addShader(VK_SHADER_STAGE_FRAGMENT_BIT, fagment)
               .addSpecializationEntry(0, 0, sizeof(uint32_t))
               .addSpecializationEntry(1, sizeof(uint32_t), sizeof(uint32_t))
               .addData(2*sizeof(uint32_t), constants)
             .vertexBinding(0, sizeof(MonsterOptimizer::InstanceData), VK_VERTEX_INPUT_RATE_INSTANCE) // никогда не забывать заполнить эти поля ВЕРНО
               .vertexAttribute(0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(MonsterOptimizer::InstanceData, mat) + sizeof(glm::vec4)*0)
               .vertexAttribute(1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(MonsterOptimizer::InstanceData, mat) + sizeof(glm::vec4)*1)
               .vertexAttribute(2, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(MonsterOptimizer::InstanceData, mat) + sizeof(glm::vec4)*2)
               .vertexAttribute(3, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(MonsterOptimizer::InstanceData, mat) + sizeof(glm::vec4)*3)
               .vertexAttribute(4, 0, VK_FORMAT_R32_UINT,   offsetof(MonsterOptimizer::InstanceData, textureData) + offsetof(TextureData, t) + offsetof(Texture, imageArrayIndex))
               .vertexAttribute(5, 0, VK_FORMAT_R32_UINT,   offsetof(MonsterOptimizer::InstanceData, textureData) + offsetof(TextureData, t) + offsetof(Texture, imageArrayLayer))
               .vertexAttribute(6, 0, VK_FORMAT_R32_UINT,   offsetof(MonsterOptimizer::InstanceData, textureData) + offsetof(TextureData, t) + offsetof(Texture, samplerIndex))
               //.vertexAttribute(7,  0, VK_FORMAT_R32_SFLOAT, offsetof(MonsterOptimizer::InstanceData, textureData) + offsetof(TextureData, mirroredU))
               //.vertexAttribute(8,  0, VK_FORMAT_R32_SFLOAT, offsetof(MonsterOptimizer::InstanceData, textureData) + offsetof(TextureData, mirroredV))
               .vertexAttribute(7, 0, VK_FORMAT_R32_SFLOAT, offsetof(MonsterOptimizer::InstanceData, textureData) + offsetof(TextureData, movementU))
               .vertexAttribute(8, 0, VK_FORMAT_R32_SFLOAT, offsetof(MonsterOptimizer::InstanceData, textureData) + offsetof(TextureData, movementV))
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

void MonsterGBufferStage::begin() {
//   RegionLog rl("MonsterGBufferStage::begin()");
  
  monsterOptimiser->optimize();
}

bool MonsterGBufferStage::doWork(const uint32_t &index) {
//   std::cout << "MonsterGBufferStage start" << "\n";
  
//   RegionLog rl("MonsterGBufferStage::doWork()");
  
  // в будущем нужно будет передавать верный сабпасс сюда
  const uint32_t instanceCount = monsterOptimiser->getInstanceCount();
  if (instanceCount == 0) return false;
  
//   localTask->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT);
  
//   localTask->setView(target->viewport());
//   localTask->setScissor(target->scissor());
  
//   std::cout << "instanceCount " << instanceCount << "\n";
  
  localTask[index]->setPipeline(pipe);
  localTask[index]->setDescriptor({uniformBuffer->descriptorSet()->handle(), samplers->handle(), images->handle()}, 0);
  localTask[index]->setVertexBuffer(instanceData.vector().handle(), 0);
  localTask[index]->setVertexBuffer(monsterDefault, 1);
  
  localTask[index]->draw(monsterDefaultVerticesCount, instanceCount, 0, 0);
  
//   throw std::runtime_error("fwewsad " + std::to_string(((Vertex*)monsterDefault->ptr())->pos.x));
  
  monsterOptimiser->clear();
  
  //ASSERT(localTask[index]->getFamily() < 4);
  
//   localTask->end();
  
  return true;
}

GPUArray<MonsterOptimizer::InstanceData>* MonsterGBufferStage::getInstanceData() {
  return &instanceData;
}

GeometryGBufferStage::GeometryGBufferStage(yavf::Buffer* worldMapVertex, GeometryOptimizer* opt) : pipe(VK_NULL_HANDLE, VK_NULL_HANDLE) {
  this->worldMapVertex = worldMapVertex;
  this->opt = opt;
}

GeometryGBufferStage::~GeometryGBufferStage() {
//   device->deallocate(localTask);
  
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
  this->localTask = info.task;
  
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
                          .create("deferred_layout2");
  }
  
  uint32_t constants[2] = {data->samplerCount(), data->imageCount()};
  
  if (pipe.handle() != VK_NULL_HANDLE) {
    device->destroyPipeline(GEOMETRY_PIPELINE_NAME);
    pipe = yavf::Pipeline(VK_NULL_HANDLE, VK_NULL_HANDLE);
  }
  
  {
    yavf::raii::ShaderModule vertex (device, (Global::getGameDir() + "shaders/deferred.vert.spv").c_str());
    yavf::raii::ShaderModule fagment(device, (Global::getGameDir() + "shaders/deferred.frag.spv").c_str());
    
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

void GeometryGBufferStage::begin() {
//   RegionLog rl("GeometryGBufferStage::begin()");
  opt->optimize();
}

bool GeometryGBufferStage::doWork(const uint32_t &index) {
//   std::cout << "GeometryGBufferStage start" << "\n";
  
//   RegionLog rl("GeometryGBufferStage::doWork()");
  
  // как передать данные для секондари таска?
  // вообще у меня какое то решение было
  
  // мне нужно не делать ничего если нечего рисовать
  
  // в будущем нужно будет передавать верный сабпасс сюда
  const uint32_t indexCount = opt->getIndicesCount();
  if (indexCount == 0) return false;
  
//   localTask->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT);
  
//   localTask->setView(target->viewport());
//   localTask->setScissor(target->scissor());
  
//   std::cout << "indexCount " << indexCount << "\n";
  
  localTask[index]->setPipeline(pipe);
  localTask[index]->setDescriptor({uniformBuffer->descriptorSet()->handle(), samplers->handle(), images->handle(), instances.vector().descriptorSet()->handle()}, 0);
  localTask[index]->setVertexBuffer(worldMapVertex, 0);
  //localTask->setInstanceBuffer(instances.vector().handle(), 0);
  localTask[index]->setIndexBuffer(indices.vector().handle());
  
  localTask[index]->drawIndexed(indexCount, 1, 0, 0, 0);
  
  opt->clear();
  
  ASSERT(localTask[index]->getFamily() < 4);
  
//   localTask->end();
  
  return true;
}

GPUArray<uint32_t>* GeometryGBufferStage::getIndicesArray() {
  return &indices;
}

GPUArray<GeometryOptimizer::InstanceData>* GeometryGBufferStage::getInstanceData() {
  return &instances;
}

DefferedLightStage::DefferedLightStage(const CreateInfo &info) : lightArray(info.device) {
  this->device = info.device;
  this->uniformBuffer = info.uniformBuffer;
  this->matrixBuffer = info.matrixBuffer;
  this->task = info.task;
  this->optimizer = info.optimizer;
  this->gbuffer = info.gbuffer;
  this->gbufferLayout = info.gbufferLayout;
  
  output = device->create(yavf::ImageCreateInfo::texture2D({info.width, info.height}, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT), VMA_MEMORY_USAGE_GPU_ONLY);
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
    
//     std::cout << "uniform_layout " << uniform_layout << "\n";
//     std::cout << "gbufferLayout " << gbufferLayout << "\n";
//     std::cout << "layout " << layout << "\n";
//     std::cout << "matrixes " << matrixes << "\n";
//     std::cout << "storage_image_layout " << storage_image_layout << "\n";
    
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
    yavf::raii::ShaderModule compute(device, (Global::getGameDir() + "shaders/tiling.comp.spv").c_str());
    
    pipe = cpm.shader(compute).create("compute_pipeline", pipelineLayout);
  }
  
  yavf::DescriptorPool pool = device->descriptorPool(DEFAULT_DESCRIPTOR_POOL_NAME);
//   {
//     yavf::DescriptorPoolMaker dpm(device);
//     
//     if (pool == VK_NULL_HANDLE) {
//       pool = dpm.poolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 40).poolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 5).create("graphics_descriptor_pool");
//     }
//   }
  
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
//   RegionLog rl("DefferedLightStage::begin()");
  optimizer->optimize();
}

void DefferedLightStage::doWork(const uint32_t &index) {
//   std::cout << "DefferedLightStage start" << "\n";
  
//   RegionLog rl("DefferedLightStage::doWork()", true);
  
  task[index]->setPipeline(pipe);
  task[index]->setDescriptor({uniformBuffer->descriptorSet()->handle(), 
                              gbuffer->handle(), 
                              lightArray.vector().descriptorSet()->handle(), 
                              matrixBuffer->descriptorSet()->handle(), 
                              output->view()->descriptorSet()->handle()}, 0);
  const uint32_t xCount = glm::ceil(static_cast<float>(output->info().extent.width)  / static_cast<float>(WORKGROUP_SIZE));
  const uint32_t yCount = glm::ceil(static_cast<float>(output->info().extent.height) / static_cast<float>(WORKGROUP_SIZE));
  task[index]->dispatch(xCount, yCount, 1);
  
  // баррьер?
  
  task[index]->setBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
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
  this->task = info.task;
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

void ToneMappingStage::doWork(const uint32_t &index) {
//   std::cout << "ToneMappingStage start" << "\n";
  
//   RegionLog rl("ToneMappingStage::doWork()");
  
  task[index]->setPipeline(pipe);
  task[index]->setDescriptor({highResImage->handle(), output->view()->descriptorSet()->handle()}, 0);
  const uint32_t xCount = glm::ceil(static_cast<float>(output->info().extent.width)  / static_cast<float>(WORKGROUP_SIZE));
  const uint32_t yCount = glm::ceil(static_cast<float>(output->info().extent.height) / static_cast<float>(WORKGROUP_SIZE));
  task[index]->dispatch(xCount, yCount, 1);
  
  // барьер?
  
  task[index]->setBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
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
  this->task = info.task;
  this->src = info.src;
  this->depthSrc = info.depthSrc;
  this->presentFamily = info.presentFamily;
  this->window = info.window;
}

CopyStage::~CopyStage() {}

void CopyStage::begin() {
  // ничего?
}

void CopyStage::doWork(const uint32_t &index) {
//   std::cout << "CopyStage start" << "\n";
  
//   RegionLog rl("CopyStage::doWork()");
  
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
  
  task[index]->setBarrier(window->getImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, range, presentFamily, VK_QUEUE_FAMILY_IGNORED);
  task[index]->setBarrier(src, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
  task[index]->copyBlit(src, window->getImage(), blit);
  task[index]->setBarrier(src, VK_IMAGE_LAYOUT_GENERAL);
  task[index]->setBarrier(window->getImage(), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, range, VK_QUEUE_FAMILY_IGNORED, presentFamily);
  
  task[index]->setBarrier(depthSrc, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);
  task[index]->setBarrier(window->getDepth(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);
  task[index]->copy(depthSrc, window->getDepth(), VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);
  task[index]->setBarrier(depthSrc, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);
  task[index]->setBarrier(window->getDepth(), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void CopyStage::recreate(const uint32_t &width, const uint32_t &height) {
  (void)width;
  (void)height;
  
  // ничего?
}

PostRenderStage::PostRenderStage(const size_t &containerSize, const CreateInfo &info) : container(containerSize) {
  this->device = info.device;
  this->task = info.task;
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

void PostRenderStage::doWork(const uint32_t &index) {
//   RegionLog rl("PostRenderStage::doWork()");
  
//   std::cout << "PostRenderStage start" << "\n";
  
  task[index]->setRenderTarget(window->currentRenderTarget());
  
  //VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS
  task[index]->beginRenderPass();
  
  for (size_t i = 0; i < parts.size(); ++i) {
    parts[i]->doWork(index);
  }
  
//   for (uint32_t i = 0; i < parts.size(); ++i) {
//     task->execute(parts[i]->getSecondaryTask());
//   }
  
  task[index]->endRenderPass();
}

void PostRenderStage::recreate(const uint32_t &width, const uint32_t &height) {
  (void)width;
  (void)height;
  
  // ничего?
}

// void drawGUI(const yavf::Pipeline &pipe, yavf::Buffer* vertexGui, yavf::Buffer* indexGui, yavf::GraphicTask* task) {
//   ImGuiIO& io = ImGui::GetIO();
//   ImDrawData* draw_data = ImGui::GetDrawData();
// 
//   // посмотрим как это все выглядит
//   yavf::ImageView* t = reinterpret_cast<yavf::ImageView*>(io.Fonts->TexID);
// 
//   // прибиндим пайплайн и текстурку
//   task->setPipeline(pipe);
//   task->setDescriptor(t->descriptorSet(), 0);
// 
//   // прибиндим вершинный и буфер индексов
//   task->setVertexBuffer(vertexGui, 0);
//   task->setIndexBuffer(indexGui, VK_INDEX_TYPE_UINT16);
// 
//   // ЧЕКНУТЬ!!!
//   // вроде что с вьюпортом, что без
//   // вьпорт уже прибинден ранее
// 
//   // размер и положение (?)
//   // это дело можно в юниформ засунуть
//   glm::vec4 vec(2.0f/io.DisplaySize.x, 2.0f/io.DisplaySize.y, -1.0f, -1.0f);
//   task->setConsts(0, sizeof(glm::vec4), &vec);
// 
//   // Setup scale and translation:
//   // {
//   //   float scale[2];
//   //   scale[0] = 2.0f/io.DisplaySize.x;
//   //   scale[1] = 2.0f/io.DisplaySize.y;
//   //   float translate[2];
//   //   translate[0] = -1.0f;
//   //   translate[1] = -1.0f;
//   //   graphicDevice->pushConstants(graphicDevice->getBasicGuiPIndex(), VK_SHADER_STAGE_VERTEX_BIT, sizeof(float) * 0, sizeof(float) * 2, scale);
//   //   graphicDevice->pushConstants(graphicDevice->getBasicGuiPIndex(), VK_SHADER_STAGE_VERTEX_BIT, sizeof(float) * 2, sizeof(float) * 2, translate);
//   // }
// 
//   // отрисовываем
//   int vtx_offset = 0;
//   int idx_offset = 0;
//   for (int n = 0; n < draw_data->CmdListsCount; ++n) {
//     const ImDrawList* cmd_list = draw_data->CmdLists[n];
//     for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; ++cmd_i) {
//       const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
// 
//       if (pcmd->UserCallback) pcmd->UserCallback(cmd_list, pcmd);
//       else {
//         VkRect2D rect{
//           {
//             pcmd->ClipRect.x > 0.0f ? (int32_t)(pcmd->ClipRect.x) : 0,
//             pcmd->ClipRect.y > 0.0f ? (int32_t)(pcmd->ClipRect.y) : 0
//           },
//           {
//             uint32_t(pcmd->ClipRect.z - pcmd->ClipRect.x),
//             uint32_t(pcmd->ClipRect.w - pcmd->ClipRect.y + 1) // FIXME: Why +1 here?
//           }
//         };
// 
//         task->setScissor(rect);
//         task->drawIndexed(pcmd->ElemCount, 1, idx_offset, vtx_offset, 0);
//       }
// 
//       idx_offset += pcmd->ElemCount;
//     }
//     vtx_offset += cmd_list->VtxBuffer.Size;
//   }
//   
//   // мне нужно понять, во первых, как рисовать кастомные картинки?
//   // во вторых, че по стилям? 
// }

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

// GuiStage::GuiStage(const CreateInfo &info) {
//   this->device = info.device;
//   this->task = info.task;
//   this->target = info.target;
//   
//   {
//     const yavf::BufferCreateInfo info{
//       0,
//       100*sizeof(ImDrawVert),
//       VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
//       1,
//       VMA_MEMORY_USAGE_CPU_ONLY
//     };
//     
//     vertexGui = device->createBuffer(info);
//   }
//   
//   {
//     const yavf::BufferCreateInfo info{
//       0,
//       100*sizeof(ImDrawIdx),
//       VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
//       1,
//       VMA_MEMORY_USAGE_CPU_ONLY
//     };
//     
//     indexGui = device->createBuffer(info);
//   }
//   
//   yavf::DescriptorSetLayout sampled_image_layout = device->setLayout(SAMPLED_IMAGE_LAYOUT_NAME);
//   
//   yavf::PipelineLayout gui_layout = VK_NULL_HANDLE;
//   {
//     yavf::PipelineLayoutMaker plm(device);
//     
//     gui_layout = plm.addDescriptorLayout(sampled_image_layout).addPushConstRange(0, sizeof(glm::vec2) + sizeof(glm::vec2)).create("gui_layout");
//   }
//   
//   {
//     yavf::PipelineMaker pm(device);
//     pm.clearBlending();
//     
//     pipe = pm.addShader(VK_SHADER_STAGE_VERTEX_BIT, Global::getGameDir() + "shaders/vertGui.spv")
//              .addShader(VK_SHADER_STAGE_FRAGMENT_BIT, Global::getGameDir() + "shaders/fragGui.spv")
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
//   }
// }

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
  this->localTask = info.task;
  
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

void GuiStage::doWork(const uint32_t &index) {
//   RegionLog rl("GuiStage::doWork()");
  
//   std::cout << "GuiStage start" << "\n";
  
//   task->setRenderTarget(target, false);
  
//   task->beginRenderPass();
  
//   for (size_t i = 0; i < debug.size(); ++i) {
//     debug[i](deferredData.combTask);
//   }
  
//   localTask->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT);
  
//   localTask->setView(target->viewport());
//   localTask->setScissor(target->scissor());
  
  // сюда нужно будет передать data
  //drawGUI(pipe, vertexGui, indexGui, localTask);
  drawGUI(data, pipe, vertexGui, indexGui, matrix, localTask[index]);
//   localTask->end();
  
//   task->endRenderPass();
}

// void GuiStage::recreate(const uint32_t &width, const uint32_t &height) {
//   (void)width;
//   (void)height;
//   
//   // ничего?
// }

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
  this->localTask = info.task;
  
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

void MonsterDebugStage::doWork(const uint32_t &index) {
//   std::cout << "MonsterDebugStage start" << "\n";
  
  // почему то жутко лагает отрисовка
  // связано это 200% с дебагом
  // причем с дебагом геометрии
  
  //const uint32_t instanceCount = monsterOptimiser->getInstanceCount();
  
  static cvar debugDraw("debugDraw");
  if (!bool(debugDraw.getFloat())) return;
  if (instanceCount == 0) return;
  
  localTask[index]->setView(window->currentRenderTarget()->viewport());
  localTask[index]->setScissor(window->currentRenderTarget()->scissor());
  
  localTask[index]->setPipeline(pipe);
  localTask[index]->setDescriptor(uniformBuffer->descriptorSet(), 0);
//   localTask->setInstanceBuffer(monData->vector().handle(), 0);
  localTask[index]->setVertexBuffer(instData.vector().handle(), 0);
  localTask[index]->setVertexBuffer(monsterDebug, 1);
  
//   std::cout << "instanceCount " << instanceCount << '\n';
  localTask[index]->draw(cubeStripVerticesCount, instanceCount, 0, 0);
  
  optimizer->clear();
  
//   throw std::runtime_error("qoewkfkwjefijwqfji");
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
  this->localTask = info.task;
  
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

void GeometryDebugStage::doWork(const uint32_t &index) {
//   std::cout << "GeometryDebugStage start" << "\n";
  
  //const uint32_t indexCount = geometryOptimiser->getIndicesCount();
  
  static cvar debugDraw("debugDraw");
  if (!bool(debugDraw.getFloat())) return;
  if (indexCount == 0) return;
  
  localTask[index]->setView(window->currentRenderTarget()->viewport());
  localTask[index]->setScissor(window->currentRenderTarget()->scissor());
  
  localTask[index]->setPipeline(pipe);
  localTask[index]->setDescriptor({uniformBuffer->descriptorSet()->handle(), instData.vector().descriptorSet()->handle()}, 0);
  localTask[index]->setVertexBuffer(worldMapVertex, 0);
  //localTask->setInstanceBuffer(instances.vector().handle(), 0);
  localTask[index]->setIndexBuffer(indices->vector().handle());
  
//   std::cout << "indexCount " << indexCount << '\n';
  localTask[index]->drawIndexed(indexCount, 1, 0, 0, 0);
  
  optimizer->clear();
}
