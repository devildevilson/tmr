#include "Optimizers.h"

// #include "VulkanRender2.h"
#include "Globals.h"
#include "Utility.h"

#ifdef _DEBUG
#include <cassert>
#define ASSERT(expr) assert(expr)
#else
#define ASSERT(expr)
#endif

#define MONSTER_PIPELINE_NAME "monster_pipeline"
#define MAP_PIPELINE_NAME "map_pipeline"
#define DESCRIPTOR_POOL_NAME "default_descriptor_pool"

const RenderType MonsterOptimizer::renderType = RENDER_TYPE_MONSTER;
  
MonsterOptimizer::MonsterOptimizer() : objCount(0) {
  // ?
}

MonsterOptimizer::~MonsterOptimizer() {
//   device->destroy(instanceBuffer);
//   device->destroy(pipe);
}

uint32_t MonsterOptimizer::add(const GraphicsIndices &idx) {
  uint32_t index;
  
  if (freeIndex == UINT32_MAX) {
    index = objs.size();
    objs.push_back(idx);
  } else {
    index = freeIndex;
    freeIndex = objs[freeIndex].transform;
    objs[index] = idx;
  }
  
  ++objCount;
  
//   std::cout << "objCount " << objCount << "\n";
  
  return index;
}

void MonsterOptimizer::remove(const uint32_t &index) {
//   std::cout << "Index " << index << "\n";
  
  ASSERT(objCount != 0);
  ASSERT(index != UINT32_MAX);
  
  objs[index].transform = freeIndex;
  objs[index].texture = UINT32_MAX;
  freeIndex = index;
  --objCount;
}

void MonsterOptimizer::markAsVisible(const uint32_t &index) {
  ASSERT(index < objs.size());
  ASSERT(objs[index].texture != UINT32_MAX);
  
  visible.push_back(index);
}

// void MonsterOptimizer::setup(const SetupInfo &info) {
//   this->render = info.render;
//   this->device = info.render->getDevice();
//   
//   uniformBuffer = render->getUniformBuffer();
//   images = render->getImageDescriptor();
//   samplers = render->getSamplerDescriptor();
//   pipe = render->getDeferredPipe();
//   
// //   occludee = render->occludeePipe();
// //   visibles = render->visiblesBuffer();
//   
//   {
//     const yavf::BufferCreateInfo info{
//       0,
//       100 * (sizeof(Texture) + sizeof(glm::mat4)),
//       VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
//       100,
//       VMA_MEMORY_USAGE_CPU_ONLY
//     };
// 
//     instanceBuffer = device->createBuffer(info);
//     
//     memset(instanceBuffer->ptr(), 0, instanceBuffer->param().size);
//   }
// }

void MonsterOptimizer::setInputBuffers(const InputBuffers &buffers) {
  transforms = buffers.transforms;
  matrices = buffers.matrices;
  rotationDatasCount = buffers.rotationDatasCount;
  rotationDatas = buffers.rotationDatas;
  textures = buffers.textures;
  
  frustumPairs = buffers.frustumPairs;
}

void MonsterOptimizer::setOutputBuffers(const OutputBuffers &buffers) {
  instDatas = buffers.instDatas;
}

uint32_t MonsterOptimizer::getInstanceCount() const {
  return visible.size();
}

// void MonsterOptimizer::setRenderTarget(yavf::RenderTarget* target) {
//   if (localTask != nullptr) {
//     localTask->setRenderTarget(target, false);
//   }
// }

void MonsterOptimizer::prepare() {
  throw std::runtime_error("Occlusion culling not implemented yet");
}

// void MonsterOptimizer::populateSecondaryOcclusionTask() {
//   throw std::runtime_error("Occlusion culling not implemented yet");
// }
// 
// void MonsterOptimizer::occlusionTest(yavf::GraphicTask* task) {
//   throw std::runtime_error("Occlusion culling not implemented yet");
// }
// 
// yavf::TaskInterface* MonsterOptimizer::secondaryOcclusionTask() {
//   throw std::runtime_error("Occlusion culling not implemented yet");
// }

void MonsterOptimizer::optimize() {
  if (visible.empty()) return;
  
//   std::cout << "Optimizing " << visible.size() << " objects" << "\n";

  if (visible.size() > instDatas->size()) instDatas->resize(visible.size());

  // тут еще нужно будет учесть матрицы
  const glm::vec3 &rot = Global::getPlayerRot();
  for (size_t i = 0; i < visible.size(); ++i) {
    const uint32_t objIndex = visible[i];
    const uint32_t textureIndex = objs[objIndex].texture;
    ASSERT(textures->size() > textureIndex);
    ASSERT(instDatas->size() > i);
    const TextureData &text = textures->at(textureIndex);
    instDatas->at(i).textureData = text;
    
//     instDatas->at(i).imageIndex = text.imageArrayIndex;
//     instDatas->at(i).imageLayer = text.imageArrayLayer;
//     instDatas->at(i).samplerIndex = text.samplerIndex;
    
    const glm::vec4 &pos = transforms->at(objs[objIndex].transform).pos;
    const glm::vec4 &scale = transforms->at(objs[objIndex].transform).scale;
    const glm::vec4 &vulkanScale = glm::vec4(scale.x, -scale.y, scale.z, scale.w);
    
    instDatas->at(i).mat = glm::translate(glm::mat4(1.0f), glm::vec3(pos));
    instDatas->at(i).mat = glm::rotate(instDatas->at(i).mat, glm::half_pi<float>() - rot.y, glm::vec3(0.0f, 1.0f, 0.0f));
    //instDatas->at(i).mat = glm::scale(instDatas->at(i).mat, glm::vec3(1.0f, -1.0f, 1.0f));
    instDatas->at(i).mat = glm::scale(instDatas->at(i).mat, glm::vec3(vulkanScale));
  }
}

// void MonsterOptimizer::populateSecondaryDrawTask() {
//   if (localTask == nullptr) throw std::runtime_error("Secondary draw task was not created");
// }
// 
// void MonsterOptimizer::draw(yavf::GraphicTask* task) {
//   if (visible.empty()) return;
//   //if (visibleCount == 0) return;
//   
//   //std::cout << "Drawing " << visibleCount << " monsters!" << "\n";
//   
//   task->setPipeline(pipe);
//   task->setDescriptor(uniformBuffer, 0);
//   task->setDescriptor(samplers, 1);
//   task->setDescriptor(images, 2);
//   task->setInstanceBuffer(instanceBuffer, 0);
//   task->setVertexBuffer(monsterDefault, 1);
//   task->draw();
// }
// 
// yavf::TaskInterface* MonsterOptimizer::secondaryDrawTask() {
//   return localTask;
// }

void MonsterOptimizer::clear() {
  visible.clear();
  
//   objCount = 0;
}

size_t MonsterOptimizer::size() const {
  return objCount;
  //return visible.size();
}

// void MonsterOptimizer::setMonsterDefault(yavf::Buffer* buffer) {
//   monsterDefault = buffer;
// }

const RenderType GeometryOptimizer::renderType = RENDER_TYPE_GEOMETRY;
  
GeometryOptimizer::GeometryOptimizer() : faceCount(0), indicesCount(0), objCount(0) {
  // ?
}

GeometryOptimizer::~GeometryOptimizer() {
//   device->destroy(worldMapImageIndices);
//   device->destroy(worldMapIndex);
}

uint32_t GeometryOptimizer::add(const GraphicsIndices &idx) {
  uint32_t index;
  
  if (freeIndex == UINT32_MAX) {
    index = objs.size();
    objs.push_back(idx);
  } else {
    index = freeIndex;
    freeIndex = objs[freeIndex].vertexOffset;
    objs[index] = idx;
  }
  
  ++objCount;
  
  return index;
}

void GeometryOptimizer::remove(const uint32_t &index) {
  ASSERT(objCount != 0);
  
  objs[index].vertexOffset = freeIndex;
  objs[index].texture = UINT32_MAX;
  freeIndex = index;
  --objCount;
}

void GeometryOptimizer::markAsVisible(const uint32_t &index) {
  ASSERT(index < objs.size());
  ASSERT(objs[index].texture != UINT32_MAX);
  
  visible.push_back(index);
  
  faceCount = glm::max(objs[index].faceIndex, faceCount);
  indicesCount += objs[index].vertexCount + 1;
}

// void GeometryOptimizer::setup(const SetupInfo &info) {
//   this->render = info.render;
//   this->device = info.render->getDevice();// device;
// 
//   //pipe = device->pipeline(MAP_PIPELINE_NAME);
//   pipe = render->getDeferredPipe();
//   images = render->getImageDescriptor();
//   samplers = render->getSamplerDescriptor();
//   
//   uniformBuffer = render->getUniformBuffer();
//   
// //   occluder = render->occluderPipe();
// //   visibles = render->visiblesBuffer();
// 
// //   {
// //     yavf::BufferCreateInfo info{
// //       0,
// //       100 * sizeof(VkDrawIndexedIndirectCommand),
// //       VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
// //       100,
// //       VMA_MEMORY_USAGE_CPU_ONLY
// //     };
// // 
// //     helperBuffer = device->createBuffer(info);
// //     
// //     memset(helperBuffer->ptr(), 0, helperBuffer->param().size);
// //   }
// 
//   {
//     yavf::BufferCreateInfo info{
//       0,
//       100 * sizeof(Texture),
//       VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, //VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
//       100,
//       VMA_MEMORY_USAGE_CPU_ONLY
//     };
// 
//     worldMapImageIndices = device->createBuffer(info);
//     
//     memset(worldMapImageIndices->ptr(), 0, worldMapImageIndices->param().size);
//     
//     yavf::DescriptorSetLayout layout = device->setLayout("storage_layout");
//     yavf::DescriptorPool pool = device->descriptorPool(DESCRIPTOR_POOL_NAME);
//     
//     yavf::DescriptorMaker dm(device);
//     //auto descs = dm.layout(layout).data(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER).create(pool);
//     auto descs = dm.layout(layout).create(pool);
//     
//     const yavf::DescriptorUpdate du{
//       VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
//       0,
//       0,
//       descs[0]
//     };
//     worldMapImageIndices->setDescriptorData(du);
//     
//     worldMapImageIndices->updateDescriptor();
//   }
//   
//   {
//     yavf::BufferCreateInfo info{
//       0,
//       100 * sizeof(uint32_t),
//       VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
//       100,
//       VMA_MEMORY_USAGE_CPU_ONLY
//     };
//     
//     worldMapIndex = device->createBuffer(info);
//     memset(worldMapIndex->ptr(), 0, worldMapIndex->param().size);
//   }
//   
//   // должно ли здесь быть создание пайплайна?
//   // прикол в том что у меня пайплайн нужно пересоздавать каждый раз как сменяется уровень
//   // вообще я еще не до конца все это продумал
//   // но если я хочу здесь создавать пайплайн, то по крайней мере мне нобходимо создать отдельную функцию
//   // где я буду принимать разные вещи для пайплайна
//   // пока наверное оставлю как было
// //   {
// //     yavf::PipelineMaker pm(device);
// //     
// //     
// //   }
// }

void GeometryOptimizer::setInputBuffers(const InputBuffers &buffers) {
  transforms = buffers.transforms;
  matrices = buffers.matrices;
  rotationDatasCount = buffers.rotationDatasCount;
  rotationDatas = buffers.rotationDatas;
  textures = buffers.textures;
  
  frustumPairs = buffers.frustumPairs;
}

void GeometryOptimizer::setOutputBuffers(const OutputBuffers &buffers) {
  indices = buffers.indices;
  instanceDatas = buffers.instanceDatas;
}

// void GeometryOptimizer::setRenderTarget(yavf::RenderTarget* target) {
//   if (localTask != nullptr) {
//     localTask->setRenderTarget(target, false);
//   }
// }

uint32_t GeometryOptimizer::getIndicesCount() const {
  return indicesCount;
}

void GeometryOptimizer::prepare() {
  throw std::runtime_error("Occlusion culling not implemented yet");
}

// void GeometryOptimizer::populateSecondaryOcclusionTask() {
//   throw std::runtime_error("Occlusion culling not implemented yet");
// }
// 
// void GeometryOptimizer::occlusionTest(yavf::GraphicTask* task) {
//   throw std::runtime_error("Occlusion culling not implemented yet");
// }
// 
// yavf::TaskInterface* GeometryOptimizer::secondaryOcclusionTask() {
//   throw std::runtime_error("Occlusion culling not implemented yet");
// }

void GeometryOptimizer::optimize() {
  if (visible.empty()) return;
  
  if (indicesCount > indices->size()) indices->resize(indicesCount);
  //std::cout << "faceCount+1 " << faceCount+1 << '\n';
  if (faceCount+1 > instanceDatas->size()) instanceDatas->resize(faceCount+1);
  
//   worldMapImageIndices->recreate((faceCount+1) * sizeof(glm::uvec4), faceCount+1);
//   worldMapIndex->recreate(indicesCount * sizeof(uint32_t), indicesCount);
  
  // тут нужно будет потом добавить ротацию
  size_t index = 0;
//   glm::uvec4* textureData = (glm::uvec4*)worldMapImageIndices->ptr();
//   uint32_t* indices = (uint32_t*)worldMapIndex->ptr();
  for (size_t i = 0; i < visible.size(); ++i) {
    const uint32_t visIndex = visible[i];
    
    for (size_t j = 0; j < objs[visIndex].vertexCount; ++j) {
      indices->at(index) = j + objs[visIndex].vertexOffset;
      ++index;
    }
    
    indices->at(index) = 0xFFFFFFFF;
    ++index;
    
//     const glm::uvec4 &text = textures->at(objs[visIndex].texture);
    const uint32_t textureIndex = objs[visIndex].texture;
    ASSERT(textures->size() > textureIndex);
    const TextureData &text = textures->at(textureIndex);
    instanceDatas->at(objs[visIndex].faceIndex).textureData = text;
    
//     instanceDatas->at(objs[visIndex].faceIndex).textureIndices.x = text.imageArrayIndex;
//     instanceDatas->at(objs[visIndex].faceIndex).textureIndices.y = text.imageArrayLayer;
//     instanceDatas->at(objs[visIndex].faceIndex).textureIndices.z = text.samplerIndex;
    
//     instanceDatas->at(objs[visIndex].faceIndex).textureIndices = text;
  }
}

// void GeometryOptimizer::populateSecondaryDrawTask() {
//   if (localTask == nullptr) throw std::runtime_error("Secondary draw task was not created");
// }
// 
// void GeometryOptimizer::draw(yavf::GraphicTask* task) {
//   if (visible.empty()) return;
//   
//   task->setPipeline(pipe);
//   task->setDescriptor({uniformBuffer->descriptor(), samplers, images, worldMapImageIndices->descriptor()}, 0);
// //   task->setDescriptor(uniformBuffer, 0);
// //   task->setDescriptor(samplers, 1);
// //   task->setDescriptor(images, 2);
// //   task->setDescriptor(worldMapImageIndices, 3);
//   task->setVertexBuffer(worldMapVertex, 0);
//   task->setIndexBuffer(worldMapIndex);
//   task->drawIndexed();
// }
// 
// yavf::TaskInterface* GeometryOptimizer::secondaryDrawTask() {
//   return localTask;
// }

void GeometryOptimizer::clear() {
  visible.clear();
  
  indicesCount = 0;
}

size_t GeometryOptimizer::size() const {
  return objCount;
}

// void GeometryOptimizer::setWorldMap(yavf::Buffer* vertex, yavf::Buffer* index) {
//   worldMapVertex = vertex;
//   //worldMapIndex = index;
// }

LightOptimizer::LightOptimizer() : objCount(0), freeIndex(UINT32_MAX) {}

LightOptimizer::~LightOptimizer() {}

uint32_t LightOptimizer::add(const LightRegisterInfo &info) {
  uint32_t index;
  
  if (freeIndex == UINT32_MAX) {
    index = lightData.size();
    lightData.push_back(info);
  } else {
    index = freeIndex;
    freeIndex = glm::floatBitsToUint(lightData[index].pos.x);
    lightData[index] = info;
  }
  
  return index;
}

void LightOptimizer::remove(const uint32_t &index) {
  lightData[index].pos.x = glm::uintBitsToFloat(freeIndex);
  freeIndex = index;
}

void LightOptimizer::markAsVisible(const uint32_t &index) {
  visible.push_back(index);
}

void LightOptimizer::setInputBuffers(const InputBuffers &buffers) {
  transforms = buffers.transforms;
}

void LightOptimizer::setOutputBuffers(const OutputBuffers &buffers) {
  lights = buffers.lights;
}

// тут тоже должен быть механизм похожий на markAsVisible
// после прохождения проверки на фрустум, мы должны собирать весь видимый свет

void LightOptimizer::prepare() {
  throw std::runtime_error("Not implemented yet");
}

void LightOptimizer::optimize() {
//   if (visible.empty()) return;
//   
//   if (lights->size() < visible.size()) lights->resize(visible.size());
//   
//   for (size_t i = 0; i < visible.size(); ++i) {
//     const uint32_t index = visible[i];
//     lights->at(i) = {
//       lightData[index].pos,
//       lightData[index].radius,
//       lightData[index].color,
//       lightData[index].cutoff
//     };
//   }
  
  const size_t size = lightData.size() + 1;
  if (size > lights->size()) lights->resize(size);
  
  glm::uvec4* count = lights->structure_from_begin<glm::uvec4>();
  LightData* datas = lights->data_from<glm::uvec4>();
  
  count->x = lightData.size();
  for (size_t i = 0; i < lightData.size(); ++i) {
    datas[i] = {
      lightData[i].pos,
      lightData[i].radius,
      lightData[i].color,
      lightData[i].cutoff
    };
  }
}

void LightOptimizer::clear() {
  
}

size_t LightOptimizer::size() const {
  
}

MonsterDebugOptimizer::MonsterDebugOptimizer() : count(0) {}
MonsterDebugOptimizer::~MonsterDebugOptimizer() {}

void MonsterDebugOptimizer::setDebugColor(const uint32_t &transformIndex, const glm::vec4 &color){
//   ASSERT(count > optimizerInstDatas->size());
  
  if (instDatas->size() < count+1) {
//     std::cout << "Monster" << "\n";
    instDatas->resize(count+1);
//     std::cout << "instDatas size " << instDatas->size() << "\n";
//     std::cout << "count+1   size " << (count+1) << "\n";
  }
  
  instDatas->at(count).mat = glm::translate(glm::mat4(), glm::vec3(transforms->at(transformIndex).pos));
  instDatas->at(count).mat = glm::scale(instDatas->at(count).mat, glm::vec3(transforms->at(transformIndex).scale));
  instDatas->at(count).color = color;
  //instDatas->at(count).mat = optimizerInstDatas->at(count).mat;
  
  ++count;
}

void MonsterDebugOptimizer::setInputBuffers(const InputBuffers &buffers) {
//   optimizerInstDatas = buffers.instDatas;
  transforms = buffers.transforms;
}

void MonsterDebugOptimizer::setOutputBuffers(const OutputBuffers &buffers) {
  instDatas = buffers.instDatas;
  
  instDatas->resize(1000);
}

void MonsterDebugOptimizer::prepare() {
  throw std::runtime_error("remove prepare");
}

void MonsterDebugOptimizer::optimize() {
  // тут наверное ничего (все происходит в setDebugColor)
}

void MonsterDebugOptimizer::clear() {
  count = 0;
}

size_t MonsterDebugOptimizer::size() const {
  return count;
}

GeometryDebugOptimizer::GeometryDebugOptimizer() : count(0) {}
GeometryDebugOptimizer::~GeometryDebugOptimizer() {}

void GeometryDebugOptimizer::setDebugColor(const uint32_t &index, const glm::vec4 &color) {
  count = std::max(count, size_t(index));
  
  // не очень правильно пересоздаются массивы
  // сюда приходит раньше чем заполняется массив с GeometryOptimizer::InstanceData
  // короч, нужн наверное у монстров разделить цвет и матрицы
  
  if (instDatas->size() < count) {
//     std::cout << "Geometry" << "\n";
    instDatas->resize(count);
//     std::cout << "instDatas size " << instDatas->size() << "\n";
//     std::cout << "count     size " << count << "\n";
  }
  
  instDatas->at(index).color = color;
}

void GeometryDebugOptimizer::setInputBuffers(const InputBuffers &buffers) {
  optimizerInstDatas = buffers.instDatas;
}

void GeometryDebugOptimizer::setOutputBuffers(const OutputBuffers &buffers) {
  instDatas = buffers.instDatas;
}

void GeometryDebugOptimizer::prepare() {
  throw std::runtime_error("remove prepare");
}

void GeometryDebugOptimizer::optimize() {
  // тут наверное ничего (все происходит в setDebugColor)
}

void GeometryDebugOptimizer::clear() {
  count = 0;
}

size_t GeometryDebugOptimizer::size() const {
  return count;
}
