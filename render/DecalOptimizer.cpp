#include "DecalOptimizer.h"

#include <cstring>

DecalOptimizer::DecalOptimizer() 
  : textures(nullptr), 
    transforms(nullptr), 
    matrices(nullptr), 
    rotations(nullptr), 
    vertices(nullptr), 
    indices(nullptr), 
    instances(nullptr), 
    vertSize(0), 
    idxSize(0) {}
    
DecalOptimizer::~DecalOptimizer() {}

void DecalOptimizer:: setInputBuffers(const InputBuffers &buffers) {
  textures = buffers.textures;
  transforms = buffers.transforms;
  matrices = buffers.matrices;
  rotations = buffers.rotations;
}

void DecalOptimizer::setOutputBuffers(const OutputBuffers &buffers) {
  vertices = buffers.vertices;
  indices = buffers.indices;
  instances = buffers.instances;
}

void DecalOptimizer::add(const Data &data) {
  const size_t oldVertSize = vertSize;
  const size_t oldIdxSize = idxSize;
  
  vertSize += data.vertices.size();
  idxSize += data.vertices.size()+1;
  
  // atomic max не существует в стандартной библиотеке
  // мьютекс? либо записать индекс в vertex
  //maxFaces = std::max(maxFaces, data.faceIndex+1);
  
  const uint32_t currentFace = maxFaces;
  maxFaces += 1;
  
  if (vertices->size() < vertSize) vertices->resize(vertSize);
  if (indices->size() < idxSize) indices->resize(idxSize);
  
  memcpy(&vertices->at(oldVertSize), data.vertices.data(), data.vertices.size()*sizeof(Vertex));
  float arr[4];
  for (size_t i = 0; i < data.vertices.size(); ++i) {
    vertices->at(oldVertSize+i).color.storeu(arr);
    arr[3] = glm::uintBitsToFloat(currentFace);
    vertices->at(oldVertSize+i).color.loadu(arr);
  }
  
  for (size_t i = 0; i < data.vertices.size(); ++i) {
    indices->at(oldIdxSize+i) = oldVertSize+i;
  }
  indices->at(idxSize-1) = UINT32_MAX;
  
  const Indices id{
    data.transformIndex,
    data.textureIndex,
    data.matrixIndex,
    data.rotationIndex,
    //data.faceIndex,
    currentFace,
    oldVertSize,
    data.vertices.size()
  };
  datas.push_back(id);
}

uint32_t DecalOptimizer::getIndicesCount() const {
  return idxSize;
}

void DecalOptimizer::optimize() {
  if (datas.size() > instances->size()) instances->resize(datas.size());
  
  for (size_t i = 0; i < datas.size(); ++i) {
    const TextureData &texture = textures->at(datas[i].textureIndex);
    instances->at(datas[i].faceIndex).textureData = texture;
    
    simd::mat4 matrix = datas[i].transformIndex == UINT32_MAX ? simd::mat4(1.0f) : simd::translate(simd::mat4(1.0f), transforms->at(datas[i].transformIndex).pos);
    matrix = datas[i].matrixIndex == UINT32_MAX ? matrix : matrices->at(datas[i].matrixIndex) * matrix;
    matrix = datas[i].rotationIndex == UINT32_MAX ? matrix : rotations->at(datas[i].rotationIndex).matrix * matrix;
    
    instances->at(datas[i].faceIndex).matrix = matrix;
  }
}

void DecalOptimizer::clear() {
  datas.clear();
  vertSize = 0;
  idxSize = 0;
  maxFaces = 0;
}

size_t DecalOptimizer::size() const {
  return datas.size();
}
