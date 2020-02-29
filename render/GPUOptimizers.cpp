#include "GPUOptimizers.h"

#include "Globals.h"

struct MonsterOptimizerCount {
  uint32_t count;
  uint32_t dummy1;
  uint32_t dummy2;
  uint32_t dummy3;
  glm::vec4 playerRotation;
};

//tasks(info.tasks), 
MonsterGPUOptimizer::MonsterGPUOptimizer(const CreateInfo &info) : transforms(nullptr), matrices(nullptr), textures(nullptr), device(info.device), uniform(info.uniform) {
  //indices.construct(device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 100);
  indices.construct(device);
  
  yavf::DescriptorPool pool = device->descriptorPool(DEFAULT_DESCRIPTOR_POOL_NAME);
  yavf::DescriptorSetLayout storage_layout = device->setLayout(STORAGE_BUFFER_LAYOUT_NAME);
  {
    yavf::DescriptorMaker dm(device);
    
    auto desc = dm.layout(storage_layout).create(pool)[0];
    const size_t i = desc->add({indices.vector().handle(), 0, indices.vector().buffer_size(), 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
    indices.vector().setDescriptor(desc, i);
  }
  
  // создать дескриптор
  yavf::DescriptorSetLayout uniform_layout = device->setLayout(UNIFORM_BUFFER_LAYOUT_NAME);
  
  yavf::PipelineLayout pipeLayout = VK_NULL_HANDLE;
  {
    yavf::PipelineLayoutMaker plm(device);
    
    pipeLayout = plm.addDescriptorLayout(uniform_layout)
                    .addDescriptorLayout(uniform_layout)
                    .addDescriptorLayout(storage_layout)
                    .addDescriptorLayout(storage_layout)
                    .addDescriptorLayout(storage_layout)
                    .addDescriptorLayout(storage_layout)
                    .addDescriptorLayout(storage_layout)
                    .create("monster_optimizer_pipeline_layout");
  }
  
  {
    yavf::ComputePipelineMaker cpm(device);
    
    yavf::raii::ShaderModule shader(device, Global::getGameDir() + "shaders/monsterOptimizer.comp.spv");
    
    pipe = cpm.shader(shader).create("monster_optimizer_pipeline", pipeLayout);
  }
  
  objCount = device->create(yavf::BufferCreateInfo::buffer(sizeof(MonsterOptimizerCount), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT), VMA_MEMORY_USAGE_CPU_ONLY);
  {
    yavf::DescriptorMaker dm(device);
    
    auto desc = dm.layout(uniform_layout).create(pool)[0];
    const size_t i = desc->add({objCount, 0, objCount->info().size, 0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER});
    objCount->setDescriptor(desc, i);
  }
}

MonsterGPUOptimizer::~MonsterGPUOptimizer() {
  device->destroyPipeline("monster_optimizer_pipeline");
  device->destroyLayout("monster_optimizer_pipeline_layout");
  device->destroy(objCount);
}

void MonsterGPUOptimizer::setInputBuffers(const InputBuffers &buffers) {
  transforms = buffers.transforms;
  matrices = buffers.matrices;
  textures = buffers.textures;
}

void MonsterGPUOptimizer::setOutputBuffers(const OutputBuffers &buffers) {
  instDatasArray = buffers.instDatas;
}

void MonsterGPUOptimizer::add(const GraphicsIndices &idx) {
  std::unique_lock<std::mutex> lock(mutex);
  
  indices.push_back(idx);
}

uint32_t MonsterGPUOptimizer::getInstanceCount() const {
  return indices.size();
}

void MonsterGPUOptimizer::begin() {
  // здесь наверное нужно юниформ буфер обновить
  
  auto* data = reinterpret_cast<MonsterOptimizerCount*>(objCount->ptr());
  data->count = indices.size();
  const glm::vec3 rot = Global::getPlayerRot();
  data->playerRotation = glm::vec4(rot.x, rot.y, rot.z, 0.0f);
  
  if (indices.size() > instDatasArray->size()) {
//    PRINT_VAR("instDatasArray->resize", indices.size())
    instDatasArray->resize(indices.size());
  }
}

#define WORKGROUP_SIZE 256

void MonsterGPUOptimizer::doWork(RenderContext* context) {
  if (indices.size() == 0) return;
  
  yavf::ComputeTask* task = context->compute();
  
  yavf::Buffer* transforms = reinterpret_cast<yavf::Buffer*>(this->transforms->gpu_buffer());
  yavf::Buffer* matrices = reinterpret_cast<yavf::Buffer*>(this->matrices->gpu_buffer());
  yavf::Buffer* textures = reinterpret_cast<yavf::Buffer*>(this->textures->gpu_buffer());
  yavf::Buffer* instDatas = reinterpret_cast<yavf::Buffer*>(this->instDatasArray->gpu_buffer());
  
  ASSERT(transforms != nullptr);
  ASSERT(matrices != nullptr);
  ASSERT(textures != nullptr);
  ASSERT(instDatas != nullptr);
  
  task->setPipeline(pipe);
  task->setDescriptor({uniform->descriptorSet()->handle(),
                      objCount->descriptorSet()->handle(),

                      transforms->descriptorSet()->handle(),
                      matrices->descriptorSet()->handle(),
                      textures->descriptorSet()->handle(),

                      instDatas->descriptorSet()->handle(),

                      indices.vector().descriptorSet()->handle()}, 0);

  const uint32_t count = std::ceil(float(indices.size()) / float(WORKGROUP_SIZE));
  ASSERT(count > 0);
  task->dispatch(count, 1, 1);

//  auto* data = reinterpret_cast<MonsterOptimizerCount*>(objCount->ptr());
//  const size_t count = data->count;
//  for (size_t i = 0; i < count; ++i) {
//    const uint transformIndex = indices[i].transform;
//    const uint matrixIndex = indices[i].matrix;
//    const uint textureIndex = indices[i].texture;
//
//    const simd::vec4 pos = this->transforms->at(transformIndex).pos;
//    const simd::vec4 vulkanScale = this->transforms->at(transformIndex).scale * simd::vec4(1.0f, -1.0f, 1.0f, 0.0f);
//
//    const simd::mat4 matOne = simd::mat4(
//            1.0f, 0.0f, 0.0f, 0.0f,
//            0.0f, 1.0f, 0.0f, 0.0f,
//            0.0f, 0.0f, 1.0f, 0.0f,
//            0.0f, 0.0f, 0.0f, 1.0f
//    );
//
//    const glm::vec3 rot = Global::getPlayerRot();
//
//    simd::mat4 mat = simd::translate(matOne, pos);
//    mat = simd::rotate(mat, PI_H - rot.y, simd::vec4(0.0f, 1.0f, 0.0f, 0.0f));
//    mat = simd::scale(mat, vulkanScale);
//
//    mat[0].storeu(&instDatasArray->at(i).mat[0].x);
//    mat[1].storeu(&instDatasArray->at(i).mat[1].x);
//    mat[2].storeu(&instDatasArray->at(i).mat[2].x);
//    mat[3].storeu(&instDatasArray->at(i).mat[3].x);
//
//    // тут же можно умножить эту матрицу на вид, чтобы не делать это в вершинном шейдере
//
//    instDatasArray->at(i).textureData = this->textures->at(textureIndex);
//  }
}

void MonsterGPUOptimizer::recreate(const uint32_t &width, const uint32_t &height) {
  (void)width;
  (void)height;
}

void MonsterGPUOptimizer::optimize() {}

void MonsterGPUOptimizer::clear() {
  indices.vector().clear();
  indices.update();
}

size_t MonsterGPUOptimizer::size() const {
  return indices.size();
}

struct GeometryOptimizerCount {
  uint32_t count;
  uint32_t internalVar;
  uint32_t dummy2;
  uint32_t dummy3;
};

GeometryGPUOptimizer::GeometryGPUOptimizer(const CreateInfo &info) : 
transforms(nullptr), 
matrices(nullptr), 
rotationDatas(nullptr), 
textures(nullptr), 
faceCount(0), 
indicesCount(0), 
device(info.device), 
// tasks(info.tasks), 
uniform(info.uniform) {
  //objs.construct(device,  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 100);
  objs.construct(device);
  objCount = device->create(yavf::BufferCreateInfo::buffer(sizeof(GeometryOptimizerCount), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT), VMA_MEMORY_USAGE_CPU_ONLY);
  
  yavf::DescriptorPool pool = device->descriptorPool(DEFAULT_DESCRIPTOR_POOL_NAME);
  yavf::DescriptorSetLayout storage_layout = device->setLayout(STORAGE_BUFFER_LAYOUT_NAME);
  yavf::DescriptorSetLayout layout = VK_NULL_HANDLE;
  {
    yavf::DescriptorLayoutMaker dlm(device);
    
    layout = dlm.binding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT).
                 binding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT).
                 create("geometry_optimizer_descriptor_layout");
  }
  
  {
    yavf::DescriptorMaker dm(device);
    
    auto desc = dm.layout(layout).create(pool)[0];
    size_t i = desc->add({objCount, 0, objCount->info().size, 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
    objCount->setDescriptor(desc, i);
    i = desc->add({objs.vector().handle(), 0, objs.vector().buffer_size(), 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
    objs.vector().setDescriptor(desc, i);
  }
  
  // создать дескриптор
  yavf::DescriptorSetLayout uniform_layout = device->setLayout(UNIFORM_BUFFER_LAYOUT_NAME);
  yavf::DescriptorSetLayout instances_layout = device->setLayout("geometry_rendering_data");
  
  {
    yavf::DescriptorLayoutMaker dlm(device);
    
    if (instances_layout == VK_NULL_HANDLE) {
      instances_layout = dlm.binding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT).
                             binding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT).
                             create("geometry_rendering_data");
    }
  }
  
  yavf::PipelineLayout pipeLayout = VK_NULL_HANDLE;
  {
    yavf::PipelineLayoutMaker plm(device);
    
    pipeLayout = plm.addDescriptorLayout(uniform_layout)
                    .addDescriptorLayout(layout)
                    .addDescriptorLayout(storage_layout)
                    .addDescriptorLayout(storage_layout)
                    .addDescriptorLayout(storage_layout)
                    .addDescriptorLayout(storage_layout)
                    .addDescriptorLayout(instances_layout)
                    .create("geometry_optimizer_pipeline_layout");
  }
  
  {
    yavf::ComputePipelineMaker cpm(device);
    
    yavf::raii::ShaderModule shader(device, Global::getGameDir() + "shaders/geometryOptimizer.comp.spv"); 
    
    pipe = cpm.shader(shader).create("geometry_optimizer_pipeline", pipeLayout);
  }
}

GeometryGPUOptimizer::~GeometryGPUOptimizer() {
  device->destroyPipeline("geometry_optimizer_pipeline");
  device->destroyLayout("geometry_optimizer_pipeline_layout");
  device->destroy(objCount);
}

void GeometryGPUOptimizer::setInputBuffers(const InputBuffers &buffers) {
  transforms = buffers.transforms;
  matrices = buffers.matrices;
  rotationDatas = buffers.rotationDatas;
  textures = buffers.textures;
}

void GeometryGPUOptimizer::setOutputBuffers(const OutputBuffers &buffers) {
  indicesArray = buffers.indices;
  instanceDatas = buffers.instanceDatas;
}

void GeometryGPUOptimizer::add(const GraphicsIndices &idx) {
  std::unique_lock<std::mutex> lock(mutex);
  
  objs.push_back(idx);
  faceCount = glm::max(idx.faceIndex+1, faceCount);
  indicesCount += idx.vertexCount + 1; 
  
  //const auto tex = textures->at(idx.texture);

  ASSERT(idx.vertexCount >= 3);

//  PRINT_VAR("indicesCount", indicesCount)
//  PRINT_VAR("objs.size()", objs.size())

//  PRINT_VAR("texture index", idx.texture)
//  ASSERT(idx.texture > 5003 && "wtf");

//  const TextureData &data = textures->at(idx.texture);
//  PRINT_VAR("texture index", idx.texture)
//  PRINT_VAR("image index  ", data.t.imageArrayIndex)
//  PRINT_VAR("image layer  ", data.t.imageArrayLayer)
}

uint32_t GeometryGPUOptimizer::getIndicesCount() const {
  return indicesCount;
}

void GeometryGPUOptimizer::begin() {
  GeometryOptimizerCount* data = reinterpret_cast<GeometryOptimizerCount*>(objCount->ptr());
  data->count = objs.size();
  data->internalVar = 0;
  
  if (indicesCount > indicesArray->size()) {
//    PRINT_VAR("indicesArray->resize", indicesCount)
    indicesArray->resize(indicesCount);
  }
//  else {
//    PRINT_VAR("indicesCount", indicesCount)
//    PRINT_VAR("indicesArray->size()", indicesArray->size())
//    PRINT_VAR("objs.size()", objs.size())
//    throw std::runtime_error("???");
//  }
  if (faceCount > instanceDatas->size()) {
//    PRINT_VAR("instanceDatas->resize", faceCount)
    instanceDatas->resize(faceCount);
  }
}

void GeometryGPUOptimizer::doWork(RenderContext* context) {
  if (objs.size() == 0) return;
  
  yavf::ComputeTask* task = context->compute();
  
  yavf::Buffer* transforms = reinterpret_cast<yavf::Buffer*>(this->transforms->gpu_buffer());
  yavf::Buffer* matrices = reinterpret_cast<yavf::Buffer*>(this->matrices->gpu_buffer());
  yavf::Buffer* rotationDatas = reinterpret_cast<yavf::Buffer*>(this->rotationDatas->gpu_buffer());
  yavf::Buffer* textures = reinterpret_cast<yavf::Buffer*>(this->textures->gpu_buffer());
  yavf::Buffer* indices = reinterpret_cast<yavf::Buffer*>(this->indicesArray->gpu_buffer());
//  auto* instances = reinterpret_cast<yavf::Buffer*>(this->instanceDatas->gpu_buffer());
  
  ASSERT(transforms != nullptr);
  ASSERT(matrices != nullptr);
  ASSERT(rotationDatas != nullptr);
  ASSERT(textures != nullptr);
  ASSERT(indices != nullptr);

  // чтобы удобнее и быстрее обновлять дескрипторы, можно сделать descriptor view
  // в нем будет содержаться VkDescriptorUpdateTemplate, чаще всего нужно обновить целый дескриптор
  // также хорошей идеей будет отказаться от обновления дескриптора внутри буфера, и обновлять дескриптор отдельно
  
  // больше 8 дескрипторов драйвер нвидии не поддерживает на линухе =(
  task->setPipeline(pipe);
  task->setDescriptor({uniform->descriptorSet()->handle(),
                      objCount->descriptorSet()->handle(),

                      transforms->descriptorSet()->handle(),
                      matrices->descriptorSet()->handle(),
                      rotationDatas->descriptorSet()->handle(),
                      textures->descriptorSet()->handle(),

                      indices->descriptorSet()->handle()}, 0); // instDatas->descriptorSet()->handle()

  const uint32_t count = std::ceil(float(objs.size()) / float(WORKGROUP_SIZE));
  ASSERT(count > 0);
  task->dispatch(count, 1, 1);

//  static bool first = false;
//
//  memset(indicesArray->data(), 0, indicesArray->size()*sizeof(indicesArray->at(0)));
//  memset(instanceDatas->data(), 0, instanceDatas->size()*sizeof(instanceDatas->at(0)));
//
//  auto* data = reinterpret_cast<GeometryOptimizerCount*>(objCount->ptr());
//  const size_t count = data->count;
//  for (size_t i = 0; i < count; ++i) {
//    const uint32_t vertexOffset = objs[i].vertexOffset;
//    const uint32_t vertexCount = objs[i].vertexCount;
//
//    const uint32_t start = data->internalVar;
//    data->internalVar += vertexCount+1;
//
//    for (uint32_t j = 0; j < vertexCount; ++j) {
//      indicesArray->at(start+j) = vertexOffset + j;
//    }
//
//    indicesArray->at(start+vertexCount) = UINT32_MAX;
//
//    const uint32_t textureIndex = objs[i].texture;
//    const uint32_t faceIndex = objs[i].faceIndex;
//
//    instanceDatas->at(faceIndex).textureData = this->textures->at(textureIndex);
//    //instanceDatas->at(faceIndex).textureData.t.imageArrayLayer = instanceDatas->at(faceIndex).textureData.t.imageArrayLayer != 1 ? 5 : instanceDatas->at(faceIndex).textureData.t.imageArrayLayer;
//
////    PRINT_VAR("faceIndex", faceIndex)
//
////    if (!first) {
////      if (instanceDatas->at(faceIndex).textureData.t.imageArrayLayer == 1) {
////        PRINT_VAR("textureIndex", textureIndex)
////        PRINT_VAR("faceIndex   ", faceIndex)
////        PRINT_VAR("data->count ", data->count)
////        PRINT_VAR("i           ", i)
////        first = true;
////
//////        throw std::runtime_error("no more");
////      }
////    }
//  }

//  PRINT_VAR("count", count)

//  throw std::runtime_error("no more");
}

void GeometryGPUOptimizer::recreate(const uint32_t &width, const uint32_t &height) {
  (void)width;
  (void)height;
}

void GeometryGPUOptimizer::optimize() {}

void GeometryGPUOptimizer::clear() {
  objs.vector().clear();
  objs.update();
  
  indicesCount = 0;
}

size_t GeometryGPUOptimizer::size() const {
  return objs.size();
}
