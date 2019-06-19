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
  
MonsterOptimizer::MonsterOptimizer() {}

MonsterOptimizer::~MonsterOptimizer() {}

// uint32_t MonsterOptimizer::add(const GraphicsIndices &idx) {
//   uint32_t index;
//   
//   if (freeIndex == UINT32_MAX) {
//     index = objs.size();
//     objs.push_back(idx);
//   } else {
//     index = freeIndex;
//     freeIndex = objs[freeIndex].transform;
//     objs[index] = idx;
//   }
//   
//   ++objCount;
//   
// //   std::cout << "objCount " << objCount << "\n";
//   
//   return index;
// }
// 
// void MonsterOptimizer::remove(const uint32_t &index) {
// //   std::cout << "Index " << index << "\n";
//   
//   ASSERT(objCount != 0);
//   ASSERT(index != UINT32_MAX);
//   
//   objs[index].transform = freeIndex;
//   objs[index].texture = UINT32_MAX;
//   freeIndex = index;
//   --objCount;
// }
// 
// void MonsterOptimizer::markAsVisible(const uint32_t &index) {
//   ASSERT(index < objs.size());
//   ASSERT(objs[index].texture != UINT32_MAX);
//   
//   visible.push_back(index);
// }

void MonsterOptimizer::add(const GraphicsIndices &idx) {
  objs.push_back(idx);
}

void MonsterOptimizer::setInputBuffers(const InputBuffers &buffers) {
  transforms = buffers.transforms;
  matrices = buffers.matrices;
  rotationDatasCount = buffers.rotationDatasCount;
  rotationDatas = buffers.rotationDatas;
  textures = buffers.textures;
  
//   frustumPairs = buffers.frustumPairs;
}

void MonsterOptimizer::setOutputBuffers(const OutputBuffers &buffers) {
  instDatas = buffers.instDatas;
}

uint32_t MonsterOptimizer::getInstanceCount() const {
//   return visible.size();
  return objs.size();
}

//#define PRINT_VEC(name, vec) std::cout << name << " (" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ")" << "\n";

void MonsterOptimizer::optimize() {
  if (objs.empty()) return;
  
//   std::cout << "Optimizing " << visible.size() << " objects" << "\n";

  if (objs.size() > instDatas->size()) instDatas->resize(objs.size());

  // тут еще нужно будет учесть матрицы
  const glm::vec3 &rot = Global::getPlayerRot();
  for (size_t i = 0; i < objs.size(); ++i) {
    const uint32_t objIndex = i;
    const uint32_t textureIndex = objs[objIndex].texture;
    ASSERT(textures->size() > textureIndex);
    ASSERT(instDatas->size() > i);
    const TextureData &text = textures->at(textureIndex);
    instDatas->at(i).textureData = text;
    
//     std::cout << "textureIndex " << textureIndex << "\n";
//     std::cout << "image index  " << text.t.imageArrayIndex << "\n";
//     std::cout << "layer index  " << text.t.imageArrayLayer << "\n";
    
//     instDatas->at(i).imageIndex = text.imageArrayIndex;
//     instDatas->at(i).imageLayer = text.imageArrayLayer;
//     instDatas->at(i).samplerIndex = text.samplerIndex;
    
    const simd::vec4 &pos = transforms->at(objs[objIndex].transform).pos;
    const simd::vec4 &scale = transforms->at(objs[objIndex].transform).scale;
    const simd::vec4 &vulkanScale = scale * simd::vec4(1.0f, -1.0f, 1.0f, 0.0f);
    
    instDatas->at(i).mat = simd::translate(simd::mat4(1.0f), pos);
    instDatas->at(i).mat = simd::rotate(instDatas->at(i).mat, glm::half_pi<float>() - rot.y, simd::vec4(0.0f, 1.0f, 0.0f, 0.0f));
    //instDatas->at(i).mat = glm::scale(instDatas->at(i).mat, glm::vec3(1.0f, -1.0f, 1.0f));
    instDatas->at(i).mat = simd::scale(instDatas->at(i).mat, vulkanScale);
    
    //PRINT_VEC("instDatas->at(i).mat ", instDatas->at(i).mat[0])
    //PRINT_VEC("                     ", instDatas->at(i).mat[1])
    //PRINT_VEC("                     ", instDatas->at(i).mat[2])
    //PRINT_VEC("                     ", instDatas->at(i).mat[3])
    
    //throw std::runtime_error("no more");
  }
}

void MonsterOptimizer::clear() {
  objs.clear();
  
//   objCount = 0;
}

size_t MonsterOptimizer::size() const {
  return objs.size();
}
  
GeometryOptimizer::GeometryOptimizer() : faceCount(0), indicesCount(0) {}
GeometryOptimizer::~GeometryOptimizer() {}

// uint32_t GeometryOptimizer::add(const GraphicsIndices &idx) {
//   uint32_t index;
//   
//   if (freeIndex == UINT32_MAX) {
//     index = objs.size();
//     objs.push_back(idx);
//   } else {
//     index = freeIndex;
//     freeIndex = objs[freeIndex].vertexOffset;
//     objs[index] = idx;
//   }
//   
//   ++objCount;
//   
//   return index;
// }
// 
// void GeometryOptimizer::remove(const uint32_t &index) {
//   ASSERT(objCount != 0);
//   
//   objs[index].vertexOffset = freeIndex;
//   objs[index].texture = UINT32_MAX;
//   freeIndex = index;
//   --objCount;
// }
// 
// void GeometryOptimizer::markAsVisible(const uint32_t &index) {
//   ASSERT(index < objs.size());
//   ASSERT(objs[index].texture != UINT32_MAX);
//   
//   visible.push_back(index);
//   
//   faceCount = glm::max(objs[index].faceIndex, faceCount);
//   indicesCount += objs[index].vertexCount + 1;
// }

void GeometryOptimizer::add(const GraphicsIndices &idx) {
  objs.push_back(idx);
  
  faceCount = glm::max(idx.faceIndex, faceCount);
  indicesCount += idx.vertexCount + 1;
}

void GeometryOptimizer::setInputBuffers(const InputBuffers &buffers) {
  transforms = buffers.transforms;
  matrices = buffers.matrices;
  rotationDatasCount = buffers.rotationDatasCount;
  rotationDatas = buffers.rotationDatas;
  textures = buffers.textures;
  
//   frustumPairs = buffers.frustumPairs;
}

void GeometryOptimizer::setOutputBuffers(const OutputBuffers &buffers) {
  indices = buffers.indices;
  instanceDatas = buffers.instanceDatas;
}

uint32_t GeometryOptimizer::getIndicesCount() const {
  return indicesCount;
}

void GeometryOptimizer::optimize() {
  if (objs.empty()) return;
  
  if (indicesCount > indices->size()) indices->resize(indicesCount);
  //std::cout << "faceCount+1 " << faceCount+1 << '\n';
  if (faceCount+1 > instanceDatas->size()) instanceDatas->resize(faceCount+1);
  
//   worldMapImageIndices->recreate((faceCount+1) * sizeof(glm::uvec4), faceCount+1);
//   worldMapIndex->recreate(indicesCount * sizeof(uint32_t), indicesCount);
  
  // тут нужно будет потом добавить ротацию
  size_t index = 0;
//   glm::uvec4* textureData = (glm::uvec4*)worldMapImageIndices->ptr();
//   uint32_t* indices = (uint32_t*)worldMapIndex->ptr();
  for (size_t i = 0; i < objs.size(); ++i) {
    const uint32_t visIndex = i;
    
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

void GeometryOptimizer::clear() {
  objs.clear();
  
  indicesCount = 0;
}

size_t GeometryOptimizer::size() const {
  return objs.size();
}

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
  lightData.clear();
}

size_t LightOptimizer::size() const {
  return lightData.size();
}

MonsterDebugOptimizer::MonsterDebugOptimizer() : count(0) {}
MonsterDebugOptimizer::~MonsterDebugOptimizer() {}

void MonsterDebugOptimizer::setDebugColor(const uint32_t &transformIndex, const simd::vec4 &color){
//   ASSERT(count > optimizerInstDatas->size());
  
  if (instDatas->size() < count+1) {
//     std::cout << "Monster" << "\n";
    instDatas->resize(count+1);
//     std::cout << "instDatas size " << instDatas->size() << "\n";
//     std::cout << "count+1   size " << (count+1) << "\n";
  }
  
  instDatas->at(count).mat = simd::translate(simd::mat4(), transforms->at(transformIndex).pos);
  instDatas->at(count).mat = simd::scale(instDatas->at(count).mat, transforms->at(transformIndex).scale);
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

void GeometryDebugOptimizer::setDebugColor(const uint32_t &index, const simd::vec4 &color) {
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

void GeometryDebugOptimizer::optimize() {
  // тут наверное ничего (все происходит в setDebugColor)
}

void GeometryDebugOptimizer::clear() {
  count = 0;
}

size_t GeometryDebugOptimizer::size() const {
  return count;
}

